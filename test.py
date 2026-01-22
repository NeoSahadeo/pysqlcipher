# LICENSE MIT Neo Sahadeo 2026

import sqlcipher

db = sqlcipher.open("test.db", "test1234")
# print(db.execute("SELECT * FROM movies WHERE id=?", ('1',)))
print(db.execute("SELECT * FROM movies WHERE id=?", ("' OR '1'='1",)))
db.close()
