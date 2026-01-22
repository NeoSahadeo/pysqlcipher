SqlCipherMethods# Python Bindings for SQL Cipher

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

## Compiling



## LICENSE(S)

For all binaries and sources that belong to ZETETIC LLC and that are used in
bindings belong to and have the respected copyright of BSD 3-Clause License.

My source files are licensed under MIT
