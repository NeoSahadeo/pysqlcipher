# LICENSE MIT Neo Sahadeo 2026

import sys
from setuptools import setup, Extension

if 'build_ext' in sys.argv:
    print("Using pre-built .so - skipping compilation")
    sys.exit(0)

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

setup(name='sqlcipher',
      version='0.1',
      author="Neo Sahadeo",
      description='Python binding for SQLCipher',
      package_data={'': ['sqlcipher.cpython-314-x86_64-linux-gnu.so']},
      include_package_data=True,
      ext_modules=[sqlcipher_module])
