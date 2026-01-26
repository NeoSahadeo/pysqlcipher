# LICENSE MIT Neo Sahadeo 2026

import sqlcipher

db = sqlcipher.open("test.db", "test1234")
db.execute("CREATE TABLE IF NOT EXISTS users ( salt BLOB NOT NULL )")
# db.execute("INSERT INTO users (salt) VALUES (?)", (b"Test1234\0 1234",))
print(db.execute("SELECT * FROM users"))
# print(db.execute("SELECT * FROM movies WHERE id=?", ('1',)))
# print(db.execute("SELECT * FROM movies WHERE id=?", ("' OR '1'='1",)))
db.close()
