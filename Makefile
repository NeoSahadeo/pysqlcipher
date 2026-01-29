_:


gen_so:
	python setup.py build_ext --inplace


test: gen_so
	python test.py


linuxbuild:
	git clone --depth=1 https://github.com/sqlcipher/sqlcipher
	cd sqlcipher && ./configure --with-tempstore=yes CFLAGS="-DSQLITE_HAS_CODEC -DSQLITE_EXTRA_INIT=sqlcipher_extra_init -DSQLITE_EXTRA_SHUTDOWN=sqlcipher_extra_shutdown" LDFLAGS="-lcrypto" && make sqlite3.o
	python -m venv .venv && source .venv/bin/activate && pip install wheel setuptools && pip wheel . -w dist/ --no-build-isolation
