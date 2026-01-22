# LICENSE MIT Neo Sahadeo 2026

import sqlcipher

db = sqlcipher.SQLiteDB("test.db", "test1234")
print(db.execute("SELECT * FROM movies;"))
db.close()
