VERSION = 1.0.2
VERSION_DEFINE = '-DPPTP_LINUX_VERSION="${VERSION}"'

CC	= gcc -Wall
DEBUG	= -g
INCLUDE =
CFLAGS  = -O9 $(VERSION_DEFINE) $(DEBUG) $(INCLUDE)
LIBS	= -lsocket
LDFLAGS	=

#################################################################
# CHANGE THIS LINE to point to the location of your pppd binary.

CFLAGS += '-DPPPD_BINARY="pppd.exe"'
#################################################################

PPTP_BIN = pptp
PPTP_OBJS = pptp.o pptp_gre.o ppp_fcs.o pty.o \
            pptp_ctrl.o dirutil.o vector.o \
            inststr.o util.o version.o
PPTP_DEPS = pptp_callmgr.h pptp_gre.h ppp_fcs.h pty.h util.h

CALLMGR_BIN = pptp_callmgr
CALLMGR_OBJS = pptp_callmgr.o pptp_ctrl.o dirutil.o util.o vector.o version.o
CALLMGR_DEPS = pptp_callmgr.h pptp_ctrl.h dirutil.h pptp_msg.h vector.h

all: $(PPTP_BIN) $(CALLMGR_BIN)

$(PPTP_BIN): $(PPTP_OBJS) $(PPTP_DEPS)
	$(CC) -o $(PPTP_BIN) $(PPTP_OBJS) $(LDFLAGS) $(LIBS)

$(CALLMGR_BIN): $(CALLMGR_OBJS) $(CALLMGR_DEPS)
	$(CC) -o $(CALLMGR_BIN) $(CALLMGR_OBJS) $(LDFLAGS) $(LIBS)

vector_test: vector_test.o vector.o
	$(CC) -o vector_test vector_test.o vector.o

clean:
	$(RM) *.o *~

clobber: clean
	$(RM) $(PPTP_BIN) $(CALLMGR_BIN) vector_test

distclean: clobber
	$(RM) pptp-linux-*.tar.gz
	$(RM) -r pptp-linux-*

test: vector_test

dist: clobber
	$(RM) pptp-linux-$(VERSION).tar.gz
	$(RM) -r pptp-linux-$(VERSION)
	mkdir pptp-linux-$(VERSION)
	mkdir pptp-linux-$(VERSION)/Documentation
	mkdir pptp-linux-$(VERSION)/Reference
	cp \
AUTHORS COPYING INSTALL Makefile NEWS README TODO USING		\
dirutil.c dirutil.h inststr.c inststr.h ppp_fcs.c ppp_fcs.h	\
pptp.c pptp_callmgr.c pptp_callmgr.h pptp_ctrl.c pptp_ctrl.h	\
pptp_gre.c pptp_gre.h pptp_msg.h pptp_options.h			\
pty.c pty.h util.c util.h vector.c vector.h vector_test.c	\
version.c version.h \
  pptp-linux-$(VERSION)
	cp \
Documentation/DESIGN.CALLMGR	\
Documentation/DESIGN.PPTP	\
Documentation/PORTING	\
  pptp-linux-$(VERSION)/Documentation
	cp \
Reference/README	\
Reference/ms-chap.txt	\
Reference/pptp-draft.txt	\
Reference/rfc1661.txt	\
Reference/rfc1662.txt	\
Reference/rfc1701.txt	\
Reference/rfc1702.txt	\
Reference/rfc1990.txt	\
Reference/rfc791.txt	\
Reference/rfc793.txt	\
  pptp-linux-$(VERSION)/Reference
	tar czf pptp-linux-$(VERSION).tar.gz pptp-linux-$(VERSION)
	$(RM) -r pptp-linux-$(VERSION)
