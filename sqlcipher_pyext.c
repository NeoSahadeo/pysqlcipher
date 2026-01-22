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
  Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* SQLiteDB_new(PyTypeObject* type,
                              PyObject* args,
                              PyObject* kwds) {
  static char* kwlist[] = {"path", "key", NULL};
  const char *path, *key;

  SQLiteDBObject* self;

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "ss", kwlist, &path, &key))
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
    int rc = sqlite3_open(self->db_path, &self->db);

    if (rc != SQLITE_OK) {
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

  PyList_Append(list, row);  // row refcount stolen
  Py_DECREF(row);
  return 0;
}

static PyObject* py_execute(SQLiteDBObject* self, PyObject* args) {
  const char* sql;
  char* errMsg;
  PyObject* result_list;

  if (!PyArg_ParseTuple(args, "s", &sql)) {
    return NULL;
  }

  result_list = PyList_New(0);
  if (sqlite3_exec(self->db, sql, py_callback, result_list, &errMsg) !=
      SQLITE_OK) {
    PyErr_SetString(PyExc_RuntimeError, errMsg);
    sqlite3_free(errMsg);
    return NULL;
  }

  return result_list;
}

static PyObject* py_close(SQLiteDBObject* self, PyObject* args) {
  sqlite3_close(self->db);
  Py_RETURN_NONE;
}

static PyMethodDef SqlCipherMethods[] = {
    {"execute", (PyCFunction)py_execute, METH_VARARGS, "Execute a SQL command"},
    {"close", (PyCFunction)py_close, METH_NOARGS,
     "Close the database connection"},
    {NULL, NULL, 0, NULL}};

static PyTypeObject SQLiteDBType = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_name = "sqlite_wrapper.SQLiteDB",
    .tp_doc = "SQLite database wrapper",
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
  if (PyModule_AddObject(m, "SQLiteDB", (PyObject*)&SQLiteDBType) < 0) {
    Py_DECREF(&SQLiteDBType);
    Py_DECREF(m);
    return NULL;
  }
  return m;
}
