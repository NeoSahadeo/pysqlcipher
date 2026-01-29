# # LICENSE MIT Neo Sahadeo 2026
#
# import sys
# from setuptools import setup, Extension
#
# sqlcipher_module = Extension('sqlcipher',
#                              sources=['sqlcipher_pyext.c'],
#                              extra_objects=['sqlcipher/sqlite3.o'],
#                              include_dirs=['.', './sqlcipher'],
#                              extra_link_args=['-lcrypto'],
#                              define_macros=[
#                                  ('SQLITE_HAS_CODEC', None),
#                                  ('SQLITE_TEMP_STORE', '2'),
#                                  ('SQLITE_EXTRA_INIT', 'sqlcipher_extra_init'),
#                                  ('SQLITE_EXTRA_SHUTDOWN', 'sqlcipher_extra_shutdown')
#                              ])
#
# setup(name='sqlcipher',
#       version='0.1',
#       author="Neo Sahadeo",
#       description='Python binding for SQLCipher',
#       ext_modules=[sqlcipher_module])

# LICENSE MIT Neo Sahadeo 2026
import os
import subprocess
import sys
from pathlib import Path
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class CustomBuildExt(build_ext):
    def build_before(self):
        sqlcipher_dir = Path("sqlcipher")
        if not sqlcipher_dir.exists():
            subprocess.check_call([sys.executable, "-m", "pip", "install", "git+https://github.com/sqlcipher/sqlcipher"])
            subprocess.check_call(["git", "clone", "https://github.com/sqlcipher/sqlcipher.git", str(sqlcipher_dir)])
        else:
            subprocess.check_call(["git", "-C", str(sqlcipher_dir), "pull"])

        os.chdir(sqlcipher_dir)
        subprocess.check_call(["./configure", "--with-tempstore=yes",
                               "CFLAGS=-DSQLITE_HAS_CODEC -DSQLITE_EXTRA_INIT=sqlcipher_extra_init -DSQLITE_EXTRA_SHUTDOWN=sqlcipher_extra_shutdown",
                               "LDFLAGS=-lcrypto"])
        subprocess.check_call(["make", "sqlite3.o"])
        os.chdir("..")

    def run(self):
        self.build_before()
        super().run()


sqlcipher_module = Extension('sqlcipher',
                             sources=['sqlcipher_pyext.c'],
                             extra_objects=['sqlcipher/sqlite3.o'],
                             include_dirs=['.', './sqlcipher'],
                             extra_link_args=['-lcrypto'],
                             define_macros=[
                                 ('SQLITE_HAS_CODEC', None),
                                 ('SQLITE_TEMP_STORE', '2'),
                                 ('SQLITE_EXTRA_INIT', 'sqlcipher_extra_init'),
                                 ('SQLITE_EXTRA_SHUTDOWN', 'sqlcipher_extra_shutdown')
                             ])

setup(
    name='sqlcipher',
    version='0.1',
    author="Neo Sahadeo",
    description='Python binding for SQLCipher',
    ext_modules=[sqlcipher_module],
    cmdclass={'build_ext': CustomBuildExt},
    zip_safe=False,
)
