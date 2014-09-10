.SUFFIXES: .obj .c .asm .lst

TOOLS = f:\ddktools\toolkits
DDK = f:\ddkx86
ASM = $(TOOLS)\masm60\binb\masm
PATH=%PATH;$(TOOLS)\masm60\binb;$(TOOLS)\masm60\binp;$(TOOLS)\msc60\binp
CL = $(TOOLS)\msc60\binp\cl
CFLAGS = -W3 -c -Alfu /NT_TEXT -G2s -Zl -Zp -O -nologo
INC = $(DDK)\inc
CINC = $(TOOLS)\msc60\include
H = $(DDK)\h
LINK = $(DDK)\tools\link
LFLAGS = /MAP /NOI /NOE /nod /exepack /packd /a:16 /far
AFLAGS =  -MX -T -N
AFLAGS2 = ;
#ASM = alp
#AFLAGS = +Sv:MASM60 +Scs -M:ALL- -Ls
#AFLAGS2=
MAPSYM = mapsym
CLIB=$(TOOLS)\msc60\lib
DDKLIB=$(DDK)\lib
LIBS=$(DDKLIB)\os2286.lib $(DDKLIB)\os2286p.lib $(CLIB)\LLIBCE.LIB
INCLUDE=.;$(INC);$(H);$(CINC)
LIB=$(CLIB);$(DDKLIB)

TARGET=xf86sup.sys
OBJS=dvasm.obj conidc.obj dvdata.obj io.obj map.obj ttypty.obj \
     cons.obj util.obj dvinit.obj
DEF=xf86sup.def

all:	version $(TARGET)

dvinit.obj: version.h dvinit.c

conidc.asm: ..\api\conidc.asm
	@copy ..\api\conidc.asm .

conidc.obj: conidc.asm
	$(ASM) $(AFLAGS) conidc.asm $(AFLAGS2)

version:
	@newvers

clean:
	if exist *.obj del *.obj
	if exist *.sys del *.sys
	if exist *.sym del *.sym
	if exist *.lst del *.lst
	if exist *.map del *.map
	-del conidc.asm

.asm.obj:
	$(ASM) $(AFLAGS) $*.asm $(AFLAGS2)

.c.obj:
	$(CL) $(CFLAGS) $(CINC) $*.c

.c.lst:
	$(CL) $(CFLAGS) /Fc $(CINC) $*.c
	copy $*.cod $*.lst
	del $*.cod

$(TARGET): $(OBJS) $(LIBS) $(DEF) makefile
	$(LINK) $(LFLAGS) $(OBJS),$(TARGET),,$(LIBS),xf86sup.def
	$(MAPSYM) xf86sup.map
	copy $(TARGET) ..
	copy xf86sup.sym ..
	copy $(TARGET) e:\XFree86\lib
	copy xf86sup.sym e:\XFree86\lib
