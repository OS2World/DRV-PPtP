
CC = gcc
AS = as
CFLAGS = -Wall -I. -O
LIB = ar
LIBFLAGS = cr

.s.o:
	$(AS) -o $*.o $<

.c.o:
	$(CC) $(CFLAGS) -c $<

OBJS = map.o pty.o ptysv32.o gccio.o gccio1.o

all: xf86_api.a

xf86_api.a:	$(OBJS)
	$(LIB) $(LIBFLAGS) xf86_api.a $(OBJS)

clean:
	-del *.a
	-del *.o
	-del core
