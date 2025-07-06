all: olm-python3

OLM_HEADERS = ../include/olm/olm.h ../include/olm/inbound_group_session.h \
	      ../include/olm/outbound_group_session.h

include/olm/olm.h: $(OLM_HEADERS)
	mkdir -p include/olm
	$(CPP) -DOLM_STATIC_DEFINE -I dummy -I ../include -o include/olm/olm.h ../include/olm/olm.h
# add memset to the header so that we can use it to clear buffers
	echo 'void *memset(void *s, int c, size_t n);' >> include/olm/olm.h

include/olm/pk.h: include/olm/olm.h ../include/olm/pk.h
	$(CPP) -DOLM_STATIC_DEFINE -I dummy  -I ../include -o include/olm/pk.h ../include/olm/pk.h

include/olm/sas.h: include/olm/olm.h ../include/olm/sas.h
	$(CPP) -DOLM_STATIC_DEFINE -I dummy -I ../include -o include/olm/sas.h ../include/olm/sas.h

include/olm/error.h: include/olm/olm.h ../include/olm/error.h
	$(CPP) -DOLM_STATIC_DEFINE -I dummy -I ../include -o include/olm/error.h ../include/olm/error.h

headers: include/olm/olm.h include/olm/pk.h include/olm/sas.h include/olm/error.h

olm-python3: headers
	DEVELOP=$(DEVELOP) python3 setup.py build

install: install-python3

install-python3: olm-python3
	python3 setup.py install --skip-build -O1 --root=$(DESTDIR)

test: olm-python3
	rm -rf install-temp
	mkdir -p install-temp/3
	PYTHONPATH=install-temp/3 python3 setup.py install --skip-build --install-lib install-temp/3 --install-script install-temp/bin
	PYTHONPATH=install-temp/3 python3 -m pytest
	PYTHONPATH=install-temp/3 python3 -m pytest --flake8 --benchmark-disable
	PYTHONPATH=install-temp/3 python3 -m pytest --isort --benchmark-disable
	PYTHONPATH=install-temp/3 python3 -m pytest --cov --cov-branch --benchmark-disable
	rm -rf install-temp

isort:
	isort -y -p olm

clean:
	rm -rf python_olm.egg-info/ dist/ __pycache__/
	rm -rf *.so _libolm.o
	rm -rf packages/
	rm -rf build/
	rm -rf install-temp/
	rm -rf include/

.PHONY: all olm-python3 install install-python3 clean test
