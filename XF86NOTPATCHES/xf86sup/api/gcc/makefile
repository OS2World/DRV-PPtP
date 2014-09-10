# Requires GNU MAKE

CC = gcc
AS = as
CFLAGS = -Wall -I. -I.. -O
LIB = ar
LIBFLAGS = cr

SRCS = map.o pty.o ptysv32.o
OBJS = $(SRCS:.c=.o) gccio.o gccio1.o

all::
	@copy ..\\map.c .
	@copy ..\\pty.c .
	@copy ..\\ptysv32.c .

.s.o:
	$(AS) -o $*.o $<

.c.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

all:: xf86_$(CC).a

xf86_$(CC).a:	$(OBJS)
	$(LIB) $(LIBFLAGS) $*.a $(OBJS)
	-copy $*.a ..

clean:
	-del ..\\*.a
	-del *.a
	-del *.c
	-del *.o
	-del core
