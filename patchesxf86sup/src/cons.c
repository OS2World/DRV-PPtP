/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#define INCL_ERRORS
#define INCL_DOS
#include <os2.h>
#include <devcmd.h>
#include "rp_priv.h"
#include "dh_priv.h"
#include "tty_priv.h"
#include "con_priv.h"

extern int con_init(RP rp);
extern int ptx_ioctl(RP,int,int);
extern int pts_read(RP,int);
extern int pts_write(RP,int);
extern int pts_ndr(RP,int);
extern int con_write(RP rp);

/* /dev/console implements a large 16K ring buffer to store messages.
 * Anyone opening /dev/console can append a message to it.
 * A process issuing the TIOCCONS ioctl will grab the console for reading
 * and processing messages.
 * Also a pty can grab the console, in case of which only the buffer
 * space of a pty is available.
 *
 * /dev/console exports its write function to IDC.
 */

#define CONSMUTEX	32

void makespace(int nb)
{
	int fence = CON_SZ - nb;
	char far *cp = conbuf + conpos;
	char far *cpend = conbuf + CON_SZ;

	if (conlen==0) return;
	while (conlen >= fence) {
		while (conlen-- > 0 && *cp++ != '\n') {
			if (cp==cpend) cp = conbuf; 
		}
		/* seen a '\n', cp points to char after that */
		if (cp==cpend) cp = conbuf;
		conpos = cp - conbuf;
	}
}

void putb(int c)
{
	int wpos = (conpos + conlen) % CON_SZ;
	conbuf[wpos] = (char)c;
	conlen++;
}

#define PEEK 1
#define NOPEEK 0
int getb(int peek)
{
	int c;
	if (conlen == 0)
		return -1;
	c = conbuf[conpos];
	if (peek==NOPEEK) {
		conpos++; conlen--;
		if (conpos == CON_SZ || conlen == 0)
			conpos = 0;
	}
	return c & 0xff;
}

int conidc(CONIDC pkt)
{
	/* don't check, whether buffer is okay. This is called from a
	 * driver, so crash if caller submits junk!
	 */
	int nb = pkt->buflen;
	int cnt = 0;
	char far* bp = pkt->buf;

	/* cannot write more than bufsiz at a time */
	if (nb >= CON_SZ)
		return RP_EINVAL;

	/* needs space in buffer for nb bytes */
	makespace(nb);

	/* write bytes */
	while (nb > 0) {
		putb(bp[cnt++]);
		nb--;
	}
	return RPDONE;
}

/* nondestructive read */
int con_ndr(RP rp)
{
	/* a pty that is console will be read instead of /dev/console */
	if (constty)
		return pts_ndr(rp,constty->dev);

	/* only a process having control may read */
	if (conpid==0 || GetPID() != conpid)
		return RP_EBAD;

	qrequest(CONSMUTEX);

	if (conlen == 0)
		return RPBUSY|RPDONE;
	RWNBUF(rp) = (UCHAR)getb(PEEK);

	qrelease(CONSMUTEX);
	return RPDONE;
}

int con_read(RP rp)
{
	char far * bp;
	int nb,cnt,c;

	/* a pty that is console will be read instead of /dev/console */
	if (constty)
		return pts_read(rp,constty->dev);

	/* only a process having control may read */
	if (conpid==0 || GetPID() != conpid)
		return RP_EBAD;

	if (P2V(RWBUF(rp),RWCNT(rp),&bp) != 0)
		return RP_EGEN;

	qrequest(CONSMUTEX);
	nb = RWCNT(rp);
	cnt = 0;

	makespace(nb);
	while (nb > 0) {
		c = getb(NOPEEK);
		if (c < 0) break;
		bp[cnt++] = (char)c;
		nb--;	
	}
	RWCNT(rp) = cnt;
	qrelease(CONSMUTEX);
	return RPDONE;
}

int con_write(RP rp)
{
	char far * bp;
	int nb,cnt;
	int ret = RPDONE;

	/* a pty that is console will receive everything written to
	 * /dev/console instead 
	 */
	if (constty)
		return pts_write(rp,constty->dev);

	if (P2V(RWBUF(rp),RWCNT(rp),&bp) != 0)
		return RP_EGEN;

	qrequest(CONSMUTEX);
	nb = RWCNT(rp);
	cnt = 0;

	/* cannot write more than bufsiz at a time */
	if (nb >= CON_SZ)
		return RP_EINVAL;

	/* needs space in buffer for nb bytes */
	makespace(nb);

	/* write bytes */
	while (nb > 0) {
		putb(bp[cnt++]);
		nb--;
	}

	RWCNT(rp) = cnt;

	/* satisfy a select */
	if (conlen > 0)
		ret = selraise(conrsel,ST_ARMRSEL,&constate);

	qrelease(CONSMUTEX);
	return ret;
}

extern char far* console;	/* ptr to pkt header */

int con_open(RP rp)
{
	/* no monitor support */
	if ((rp)->rp_stat & 0x08)
		return RP_EBAD;

	if (concnt==0) {
		constate = 0;
		conpid = 0;
		if (conrsel) ResetSEV(conrsel);
	}
	concnt++;
	
	/* anyone can open */
	return RPDONE;
}

int con_close(RP rp)
{
	/* no monitor support */
	if ((rp)->rp_stat & 0x08)
		return RP_EBAD;

	concnt--;

	/* close console */
	if (conpid && GetPID() == conpid)
		conpid = 0;

	if (concnt == 0) {
		selraise(conrsel,ST_ARMRSEL,&constate);
	}

	return RPDONE;
}

int con_flush(void)
{
	qrequest(CONSMUTEX);
	conlen = conpos = 0;
	qrelease(CONSMUTEX);	
	return RPDONE;
}

#define assert_rbuf(nb) if ((plen && plen<nb) || Verify(pp,nb,0)) return RP_EINVAL;
#define assert_wbuf(nb) if ((dlen && dlen<nb) || Verify(dp,nb,1)) return RP_EINVAL;
#define FPI long far

int con_ioctl(RP rp)
{
	void far* pp = (void far*)IOPARAM(rp);	
	void far* dp = (void far*)IODATA(rp);
	int plen = IOPLEN(rp);
	int dlen = IODLEN(rp);
	int pid, ret;

	/* if a pty is console, we want to talk to it */
	if (constty)
		return ptx_ioctl(rp,constty->dev,0);

	if (IOCAT(rp) != XFREE86_PTY)
		return RP_EINVAL;

	switch (IOFUNC(rp)) {
	case XTY_TIOCSETA:
	case XTY_TIOCSETAW:
	case XTY_TIOCSETAF:
	case XTY_TIOCDRAIN:
	case XTY_TIOCSTART:
	case XTY_TIOCSTOP:
	case XTY_TIOCSPGRP:
	case XTY_TIOCSWINSZ:
	case XTY_TIOCSETC:
	case TTY_TIOCNOTTY:
	case TTY_TIOCSCTTY:
	case XTY_TIOCGETC:
	case XTY_TIOCGETA:
	case XTY_TIOCGWINSZ:
	case XTY_TIOCGPGRP:
		/* accept, but ignore them: results are invalid! */
		return RPDONE;
	case XTY_TIOCFLUSH:
	case XTY_TCFLUSH:
		return con_flush();
	case XTY_TIOCSTI:
		assert_rbuf(sizeof(ULONG));
		qrequest(CONSMUTEX);
		putb((int)*(FPI*)pp);
		qrelease(CONSMUTEX);
		return RPDONE;
	case CONS_TIOCCONS:	/* (ULONG) become console */
		assert_rbuf(sizeof(ULONG));
		pid = GetPID();
		if (*(FPI*)pp) {
			if (conpid && conpid != pid)
				return RP_EUSE;
			conpid = pid;
			return RPDONE;
		} else if (conpid==pid)
			conpid = 0;
		return RPDONE;
	case CONS_OWNER:	/* return owner */
		/* XXX should ask device that owns the console, if owned */
		{
			char far *d = (char far*)dp;
			assert_wbuf(14);
			if (constty) {
				int dev = constty->dev;
				bmove(d,"\\dev\\ptyXX\0",11);
				d[8] = (char)((dev > 15) ? 'p' : 'q');
				d[9] = i2hex(dev);
			} else if (conpid) {
				bmove(d,"PID00000\0",9);
				i2ascii(d+3,conpid);
			} else
				bmove(d,"NONE\0",5);
		}
		return RPDONE;
	case CONS_NAME:		/* return my name (pass 14 byte buffer) */
		return drvname((char far*)dp, dlen, "\\dev\\console$\0");
	case CONS_DRVID:
		return drvid(dp,dlen,CONSDRV_ID);
	case XTY_TIOCOUTQ:	/* (ULONG) get out queue size */
	case XTY_FIONREAD:	/* (ULONG) get # bytes to read */
		assert_wbuf(sizeof(ULONG));
		*(FPI*)dp = conlen;
		return RPDONE;
	case XTY_FIONBIO:	/* console cannot block on write */
		return RPDONE;
	case XTY_SELREG:	/* struct pt_sel, only uses a rsel sema */
		return selreg(pp, dp, &conrsel, 0);
	case XTY_SELARM:	/* ULONG, arm event */
		assert_rbuf(sizeof(ULONG));
		if (*(FPI*)pp & XTYSEL_ARMRSEL) {
			ret = selarm(ST_ARMRSEL, &constate, conrsel);
			if (ret == RPDONE) {
				/* check if condition is already met */
				if (conlen > 0) 
					return selraise(conrsel,ST_ARMRSEL,&constate);
			}
		}
		return RPDONE;
	}
	return RP_EINVAL;
}

int constrategy(RP rp)
{
	switch (rp->rp_cmd) {
	case CMDInit:
		return con_init(rp);
	case CMDNDR:
		return con_ndr(rp);
	case CMDINPUT:
		return con_read(rp);
	case CMDOUTPUT:
	case CMDOUTPUTV:
		return con_write(rp);
	case CMDInputF:
	case CMDOutputF:
		return con_flush();
	case CMDOpen:
		return con_open(rp);
	case CMDClose:
		return con_close(rp);
	case CMDGenIOCTL:
		return con_ioctl(rp);
	case CMDInputS:
		return conlen ? RPDONE : RPDONE|RPBUSY;
	case CMDOutputS:
		/* can never become full */
		return RPDONE;
	case CMDShutdown:
		return RPDONE;
	default:
		return RP_EBAD;
	}	
}
