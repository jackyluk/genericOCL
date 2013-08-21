ALL = helloworld addition matrix

all: $(ALL)


helloworld: FORCE
	$(MAKE) -C helloworld

addition: FORCE
	$(MAKE) -C addition

matrix: FORCE
	$(MAKE) -C matrix

clean: FORCE
	$(MAKE) -C helloworld/ clean
	$(MAKE) -C addition/ clean
	$(MAKE) -C matrix/ clean

FORCE:
