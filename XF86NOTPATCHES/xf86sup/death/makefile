all:
	@echo Use one of the following targets:
	@echo make gcc
	@echo make watcom
	@echo make vacpp

gcc:
	gcc -O -c deathapi.c -o deathgcc.o
	ar cr deathgcc.a deathgcc.o
	gcc -O -o death.exe death.c -L. -ldeathgcc

watcom:
	wcc386 deathapi.c -w4 -zq -otexan -of+ -4s -bt=os2 -mf
	wlib -b -c -n -q -p=512 deathwcc.lib +deathapi.obj

vacpp:
	icc /C+ /G4 /Ms /O+ /Os- /Sa /Sp4 /W3 /Rn /I. deathapi.c
	-@del deathvac.lib
	lib @deathvac.rsp

clean:
	-@del *.obj
	-@del *.lst
	-@del *.map
	-@del *.lib
	-@del *.o
	-@del *.a
	-@del *.bak
