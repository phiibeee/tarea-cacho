all:
	$(MAKE) -C src
	$(MAKE) -C api

clean:
	$(MAKE) -C src clean
	$(MAKE) -C api clean
