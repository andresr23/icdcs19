.PHONY: clean driver

default:
	$(MAKE) -C ./spectre
	$(MAKE) -C ./zenaccess
	$(MAKE) -C ./llc

driver:
	cd ./drivers/amd && make clean && make

clean:
	$(MAKE) -C ./spectre clean
	$(MAKE) -C ./zenaccess clean
	$(MAKE) -C ./llc clean
	cd ./drivers/amd && make clean
