# Python Bindings for SQL Cipher

__Provided methods__

- `open` - connect to a db
- `execute` - execute a sql commmand
- `close` - close a connection to a db


## Usage

```python
# Import using `sqlcipher`
import sqlcipher

# SQLiteDB takes in the name and key of the database
db = sqlcipher.open("test.db", "test1234")

# Execute will run an SQL command
print(db.execute("SELECT * FROM movies;"))

# Close the database connection
db.close()
```

## Installing

Compiled only for Linux x86-64.

Download the `sqlcipher.cpython-314-x86_64-linux-gnu.so` file.

## Compiling

__Linux Bash__

```bash
git clone --depth=1 https://github.com/NeoSahadeo/pysqlcipher
cd pysqlcipher
git clone --depth=1 https://github.com/sqlcipher/sqlcipher
cd sqlcipher
./configure --with-tempstore=yes CFLAGS="-DSQLITE_HAS_CODEC -DSQLITE_EXTRA_INIT=sqlcipher_extra_init -DSQLITE_EXTRA_SHUTDOWN=sqlcipher_extra_shutdown" \
	LDFLAGS="-lcrypto"
make sqlite3.o
cd ..
make gen_so
```

## Contributing

Contributions welcome. Currently in need of a python package for pip.

## LICENSE(S)

For all binaries and sources that belong to ZETETIC LLC and that are used in
bindings belong to and have the respected copyright of BSD 3-Clause License.

My source files are licensed under MIT
