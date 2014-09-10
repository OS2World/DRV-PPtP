.SUFFIXES: .obj .c .asm .lst


DDK   = E:\tools\ddk
BASE  = $(DDK)\base
TOOLS = $(BASE)\tools

CL     = $(TOOLS)\cl.exe
INC    = $(BASE)\inc
H      = $(BASE)\h
H386  = $(BASE)\H386
CFLAGS = -W3 -c -Anlw /NT_TEXT -G2s -Zl -Zp -O -nologo

MASM   = $(TOOLS)\masm.exe
MAPSYM = $(TOOLS)\mapsym.exe
MFLAGS =  -MX -T -N

LINK   = $(TOOLS)\link.exe
LFLAGS = /MAP /NOI /NOE /nod /exepack /packd /a:16 /far
CLIB   = $(MSCTOOLS)\msc60\lib
DDKLIB = $(BASE)\lib

LIBS    = $(DDKLIB)\os2286.lib $(DDKLIB)\os2286p.lib
INCLUDE = .;$(INC);$(H)
LIB     = $(CLIB);$(DDKLIB)

TARGET  = xf86sup.sys
OBJS    = dvasm.obj conidc.obj dvdata.obj io.obj map.obj ttypty.obj \
          cons.obj util.obj dvinit.obj
DEF     = xf86sup.def

all:	version $(TARGET)

dvinit.obj: version.h dvinit.c

conidc.obj: ..\api\conidc.asm
	$(MASM) $(MFLAGS) ..\api\conidc.asm,conidc.obj;

version:
	@newvers

clean:
	if exist *.obj del *.obj
	if exist *.sys del *.sys
	if exist *.sym del *.sym
	if exist *.lst del *.lst
	if exist *.map del *.map

.asm.obj:
	$(MASM) $(MFLAGS) $*.asm;

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
