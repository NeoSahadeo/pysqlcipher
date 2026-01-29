# LICENSE MIT Neo Sahadeo 2026

import os
import unittest
import sqlcipher

CIPHER = "test1234"
DB_NAME = "test.db"


class TestSQLExecute(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if os.path.exists(DB_NAME):
            os.remove(DB_NAME)
        cls._conn = sqlcipher.open(DB_NAME, CIPHER)

    @classmethod
    def tearDownClass(cls):
        cls._conn.close()

    def test_table_creation(self):
        self._conn.execute("CREATE TABLE IF NOT EXISTS users ( username STR, salt BLOB NOT NULL )")
        self._conn.execute("SELECT * from users")

    def test_insert(self):
        self.test_table_creation()
        self._conn.execute("INSERT INTO users (username, salt) VALUES (?, ?)", ("neo", b"12345"))
        data = self._conn.execute("SELECT * from users WHERE username = ?", ("neo",))
        assert (len(data) > 0)

    def test_key_change(self):
        self._conn.execute("PRAGMA rekey = '4321test'")
        self._conn.close()
        self._conn = sqlcipher.open(DB_NAME, CIPHER)
        with self.assertRaises(RuntimeError) as cm:
            print(self._conn.execute("SELECT * from users"))
        self.assertEqual(str(cm.exception), 'file is not a database')


if __name__ == "__main__":
    unittest.main()
