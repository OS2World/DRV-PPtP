# Attention! You need GNU MAKE for this file and levels below.
# Note: I will NOT change this to the obscure and bogus monsters that are
# usually coming with commercial tools.

all:
	@echo "Possible Targets are:"
	@echo "    make clean"
	@echo "    make gcc                  for EMX0.9B"
	@echo "    make cset2                for CSET++ 2.X"
	@echo "    make bcos2                for Borland C++ OS/2 >=1.0"
	@echo "    make watcom               for Watcom C++ OS/2 >=10.0"

clean:
	@cd gcc; $(MAKE) clean
	@cd cset2; $(MAKE) clean
	@cd bcos2; $(MAKE) clean
	@cd watcom; $(MAKE) clean

gcc:
	@cd gcc; $(MAKE) all

cset2:
	@cd cset2; $(MAKE) all

bcos2:
	@cd bcos2; $(MAKE) all

watcom:
	@cd watcom; $(MAKE) all

