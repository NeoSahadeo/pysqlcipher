// LICENSE MIT Neo Sahadeo 2026

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "sqlite3.h"

typedef struct {
  PyObject_HEAD sqlite3* db;
  char* db_path;
  char* cipher_key;
} SQLiteDBObject;

static void SQLiteDB_dealloc(SQLiteDBObject* self) {
  if (self->db) {
    sqlite3_close(self->db);
    self->db = NULL;
  }
  PyMem_Free(self->db_path);
  PyMem_Free(self->cipher_key);
  Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* SQLiteDB_new(PyTypeObject* type,
                              PyObject* args,
                              PyObject* kwds) {
  const char *path, *key;

  SQLiteDBObject* self;

  if (!PyArg_ParseTuple(args, "ss", &path, &key))
    return NULL;

  self = (SQLiteDBObject*)type->tp_alloc(type, 0);
  if (self != NULL) {
    self->db = NULL;
    self->db_path = PyMem_Malloc(strlen(path) + 1);
    self->cipher_key = PyMem_Malloc(strlen(key) + 1);

    if (!self->db_path) {
      Py_DECREF(self);
      return PyErr_NoMemory();
    }
    if (!self->cipher_key) {
      Py_DECREF(self);
      return PyErr_NoMemory();
    }

    strcpy(self->db_path, path);
    strcpy(self->cipher_key, key);
  }

  return (PyObject*)self;
}

static int SQLiteDB_init(SQLiteDBObject* self, PyObject* args, PyObject* kwds) {
  if (self->db_path) {
    if (sqlite3_open(self->db_path, &self->db) != SQLITE_OK) {
      PyErr_Format(PyExc_RuntimeError, "Failed to open database: %s",
                   sqlite3_errmsg(self->db));
      sqlite3_close(self->db);
      self->db = NULL;
      return -1;
    }

    sqlite3_key(self->db, self->cipher_key, strlen(self->cipher_key));
  }
  return 0;
}

static int py_callback(void* data, int columns, char** values, char** names) {
  PyObject* list = (PyObject*)data;
  PyObject* row = PyTuple_New(columns);

  for (int i = 0; i < columns; i++) {
    PyTuple_SetItem(row, i, PyUnicode_FromString(values[i] ? values[i] : ""));
  }

  if (PyList_Append(list, row) < 0) {
    PyErr_SetString(PyExc_RuntimeError, "Append to output list failed.");
    return 1;
  };
  // row refcount stolen by append
  Py_DECREF(row);
  return 0;
}

static PyObject* execute_sql(SQLiteDBObject* self, PyObject* args) {
  const char* sql;
  char* err_msg;
  PyObject* result_list;
  PyObject* params = NULL;
  sqlite3_stmt* stmt;

  if (!PyArg_ParseTuple(args, "s|O", &sql, &params)) {
    return NULL;
  }

  if (sqlite3_prepare_v2(self->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
    PyErr_SetString(PyExc_RuntimeError, sqlite3_errmsg(self->db));
    return NULL;
  }

  result_list = PyList_New(0);

  if (params && PyTuple_Check(params)) {
    Py_ssize_t param_count = PyTuple_Size(params);

    for (Py_ssize_t i = 0; i < param_count; i++) {
      PyObject* param = PyTuple_GetItem(params, i);

      if (PyUnicode_Check(param)) {
        const char* text = PyUnicode_AsUTF8(param);
        sqlite3_bind_text(stmt, i + 1, text, -1, SQLITE_TRANSIENT);

      } else if (PyLong_Check(param)) {
        sqlite3_bind_int64(stmt, i + 1, PyLong_AsLongLong(param));

      } else if (PyFloat_Check(param)) {
        sqlite3_bind_double(stmt, i + 1, PyFloat_AsDouble(param));

      } else if (PyBytes_Check(param)) {
        sqlite3_bind_blob(stmt, i + 1, PyBytes_AsString(param),
                          PyBytes_Size(param), SQLITE_TRANSIENT);

      } else if (param == Py_None) {
        sqlite3_bind_null(stmt, i + 1);
      }
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
      int columns = sqlite3_column_count(stmt);
      PyObject* row = PyList_New(columns);

      for (int i = 0; i < columns; i++) {
        switch (sqlite3_column_type(stmt, i)) {
          case SQLITE_TEXT:
            PyList_SetItem(row, i,
                           PyUnicode_FromStringAndSize(
                               (const char*)sqlite3_column_text(stmt, i),
                               sqlite3_column_bytes(stmt, i)));
            break;
          case SQLITE_BLOB:
            PyList_SetItem(
                row, i,
                PyBytes_FromStringAndSize(sqlite3_column_blob(stmt, i),
                                          sqlite3_column_bytes(stmt, i)));
            break;
          case SQLITE_INTEGER:
            PyList_SetItem(row, i,
                           PyLong_FromLongLong(sqlite3_column_int64(stmt, i)));
            break;
          case SQLITE_FLOAT:
            PyList_SetItem(row, i,
                           PyFloat_FromDouble(sqlite3_column_double(stmt, i)));
            break;
          case SQLITE_NULL:
            PyList_SetItem(row, i, Py_None);
            Py_INCREF(Py_None);
            break;
        }
      }

      PyList_Append(result_list, row);
      Py_DECREF(row);
    }
    sqlite3_finalize(stmt);
  } else {
    if (sqlite3_exec(self->db, sql, py_callback, result_list, &err_msg) !=
        SQLITE_OK) {
      PyErr_SetString(PyExc_RuntimeError, err_msg);
      sqlite3_free(err_msg);
      return NULL;
    }
  }

  return result_list;
}

static PyObject* close_db(SQLiteDBObject* self, PyObject* args) {
  sqlite3_close(self->db);
  Py_RETURN_NONE;
}

static PyMethodDef SqlCipherMethods[] = {
    {"execute", (PyCFunction)execute_sql, METH_VARARGS,
     "Execute a SQL command"},
    {"close", (PyCFunction)close_db, METH_NOARGS,
     "Close the database connection"},
    {NULL, NULL, 0, NULL}};

static PyTypeObject SQLiteDBType = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_name = "sqlscipher",
    .tp_doc = "Python bindings for SQL Cipher",
    .tp_basicsize = sizeof(SQLiteDBObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = SQLiteDB_new,
    .tp_init = (initproc)SQLiteDB_init,
    .tp_dealloc = (destructor)SQLiteDB_dealloc,
    .tp_methods = SqlCipherMethods,
};

static struct PyModuleDef sqlciphermodule = {
    PyModuleDef_HEAD_INIT, .m_name = "sqlcipher", .m_size = -1};

PyMODINIT_FUNC PyInit_sqlcipher(void) {
  PyObject* m;
  if (PyType_Ready(&SQLiteDBType) < 0)
    return NULL;

  m = PyModule_Create(&sqlciphermodule);
  if (m == NULL)
    return NULL;

  Py_INCREF(&SQLiteDBType);
  if (PyModule_AddObject(m, "open", (PyObject*)&SQLiteDBType) < 0) {
    Py_DECREF(&SQLiteDBType);
    Py_DECREF(m);
    return NULL;
  }

  return m;
}
