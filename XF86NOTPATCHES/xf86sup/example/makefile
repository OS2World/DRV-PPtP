.SUFFIXES: .c .obj .exe .o

all: mapexam.exe consvr.exe conexam.exe ptyexam.exe ptyexam1.exe ioexam.exe\
	ioexam1.exe ioexam2.exe openptys.exe consel.exe

CC = gcc
CFLAGS_OMF = -O -I. -I../api -Zomf
CFLAGS_NOOMF = -O -I. -I../api -Zsysv-signals
RM = del
LIBS = -L../api -lxf86_api

.c.obj:
	$(CC) $(CFLAGS_OMF) -c $<

.obj.exe:
	$(CC) $(CFLAGS_OMF) -o $*.exe $< $*.def

.c.o:
	$(CC) $(CFLAGS_NOOMF) -c $<

ptyexam.exe: ptyexam.o pty_util.o
	$(CC) -Zsysv-signals -o ptyexam.exe ptyexam.o pty_util.o $(LIBS)

ptyexam1.exe: ptyexam1.o pty_util.o
	$(CC) -Zsysv-signals -o ptyexam1.exe ptyexam1.o pty_util.o $(LIBS)

ioexam.exe: ioexam.o 
	$(CC) -o ioexam.exe ioexam.o $(LIBS)

ioexam1.exe: ioexam1.o 
	$(CC) -o ioexam1.exe ioexam1.o $(LIBS)

ioexam2.o: ioexam2.c
	$(CC) $(CFLAGS_NOOMF) -Zmtd -c ioexam2.c

ioexam2.exe: ioexam2.o 
	$(CC) -Zmtd -o ioexam2.exe ioexam2.o $(LIBS)

clean:
	-$(RM) *.exe
	-$(RM) *.obj
	-$(RM) *.o
	-$(RM) *.log
	-$(RM) core
