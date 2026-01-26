_:

gen_so:
	python setup.py build_ext --inplace


test: gen_so
	python test.py
