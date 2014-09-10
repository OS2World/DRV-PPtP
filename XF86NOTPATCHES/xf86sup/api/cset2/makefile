# Requires GNU MAKE
# requires ICC and ALP (Devcon CD)

.SUFFIXES: .asm .obj .c

CC = icc
AS = alp
CFLAGS = /C+ /G4 /Ms /O+ /Os- /Sa /Sp4 /W3 /Rn /I. /I..
LIB = lib

SRCS = map.c pty.c ptysv32.c
OBJS = $(SRCS:.c=.obj) iccio.obj iccio1.obj
OBJS1 = +map +pty +ptysv32 +iccio +iccio1

map.c:
	@copy ..\\map.c .

pty.c:
	@copy ..\\pty.c .

ptysv32.c:
	@copy ..\\ptysv32.c .

.asm.obj:
	$(AS) $<

.c.obj:
	$(CC) $(CFLAGS) $< 

all:: xf86_$(CC).lib

xf86_$(CC).lib:	$(OBJS)
	$(LIB) xf86_$(CC) $(OBJS1);
	-copy xf86_$(CC).lib ..

clean:
	-del ..\\*.lib
	-del *.lib
	-del *.c
	-del *.obj
