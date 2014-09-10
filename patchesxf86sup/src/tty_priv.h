/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#include "../api/ptyos2.h"
#include "util.h"

#define _POSIX_VDISABLE	((UCHAR)'\377')
#ifndef _POSIX_SOURCE
#define CCEQ(val,c)	(c==val ? val != _POSIX_VDISABLE : 0)
#endif
#define QUOTE	0x100
#define CHARMASK 0xff

struct tty
{
	short		  scnt,ccnt;	/* open counts */
	USHORT		pgrp;		    /* os/2 session */
	USHORT		pid;	  	  /* process that is session leader of this tty */
	ULONG		  state;	  	/* device state */
	SHORT		  dev;    		/* device # for backannotation */
	UCHAR		  send,ucntl;	/* pkt/user mode flags */
	RPQUEUE		crq;		    /* ctrlr read request packet Q on outq */
	RPQUEUE		cwq;		    /* ctrlr write request packet Q on inq */
	RPQUEUE		srq;		    /* slave read request packet Q on inq */
	RPQUEUE		swq;		    /* ctrlr write request packet Q on outq */
	CQUEUE		inq;		    /* ctrlr write to slave's inq */
	CQUEUE		outq;		    /* ctrlr reads from slave's outq */
	int		    ocol;   		/* output column */
	int		  nbsave1,nbsave2;    /* storage for last ptr to transf. data */
	ULONG		crsel,srsel,cxsel;	/* select semaphores */
					                    /* no write semas yet! */
	struct  pt_termios	termios;
	struct  pt_winsize	winsize;
};

typedef struct tty far* TTY;

#define	NPTY		32
extern TTY		ttys[NPTY];
extern TTY		constty;

#define BSET(f,b)	f |= (b)
#define BCLR(f,b)	f &= ~(b)
#define ISSET(f,b)	(((f) & (b))==(b))
#define ISCLR(f,b)	(((f) & (b))==0)

/* bits in state */
#define	ST_COPEN	0x0001	/* controller is open */
#define ST_SOPEN	0x0002	/* slave is open */
#define ST_XCLUDE	0x0004	/* exclusive open */
#define ST_STOPPED	0x0008	/* is stopped */
#define	PF_PKT		0x0010	/* packet mode */
#define	PF_STOPPED	0x0020	/* user told stopped */
#define	PF_REMOTE	0x0040	/* remote and flow controlled input */
#define	PF_NOSTOP	0x0080
#define PF_UCNTL	0x0100	/* user control mode */
#define ST_CRSEEN	0x0200	/* last char was CR */
#define ST_ENADUP	0x0400	/* signal a coming dup() */
#define ST_DRAIN	0x0800	/* block on drain outq */
#define ST_CTTY		0x1000	/* is ctty of pgrp */
#define ST_PTMSLOCK	0x2000	/* locked for ptms access */
#define ST_NDELAY	0x4000	/* non-blocking (ctrlr) */
#define ST_NDELAYS	0x8000	/* non-blocking (slave) */
#define ST_ARMRSEL	0x10000 /* enable RSEL posting (ctrlr) */
#define ST_ARMXSEL	0x20000 /* enable XSEL posting (ctrlr) */
#define ST_ARMRSELS	0x40000 /* enable RSEL posting (slave) */

#define ST_EOFSEEN	0x80000  /* have seen an EOF */
#define ST_EOFSIGB	0x100000 /* send SIGBREAK on EOF */
#define ST_EOFSIGC	0x200000 /* send sigint on EOF */
#define ST_EOFCTLZ	0x400000 /* return ^Z on EOF */
#define ST_EOFCTLC	0x800000 /* return ^C on EOF */
#define ST_EOFCTLD	0xc00000 /* return ^D on EOF */
#define ST_EOF (ST_EOFSIGB|ST_EOFSIGC|ST_EOFCTLD)

#define	t_cc		  termios.c_cc
#define	t_cflag		termios.c_cflag
#define	t_iflag		termios.c_iflag
#define	t_ispeed	termios.c_ispeed
#define	t_lflag		termios.c_lflag
#define	t_min	  	termios.c_min
#define	t_oflag		termios.c_oflag
#define	t_ospeed	termios.c_ospeed
#define	t_time		termios.c_time

