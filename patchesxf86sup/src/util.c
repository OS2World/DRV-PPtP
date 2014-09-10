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
#include "version.h"

extern ULONG qmutex[];

/* Request a mutex semaphore */
int qrequest(int n)
{
	ULONG far* mutex = &qmutex[n];

	switch (SemRequest(mutex,5000l))
	{
  	case ERROR_SEM_TIMEOUT:
  		return RP_EGEN;
  	case ERROR_INTERRUPT:
  		return RP_EINTR;
  	default:
  		return RPDONE;
	}
}

void qrelease(int n)
{
	SemClear(&qmutex[n]);
}

extern char hexcvt[];
extern int p10[];

char i2hex(int n)
{
	return hexcvt[n&15];
}

void i2ascii(char far *p, int n)
{
	int i;
	char far *c = p;

	for (i=0; i<5; i++) {
		*c = '0';
		while (n >= p10[i]) {
			n -= p10[i];
			(*c)++;
		}
		c++;
	}
	*c = 0;
}

/* manage the *_DRVID ioctl */
int drvid(void far* dp, int dlen, int id)
{
	ULONG far *lp = (ULONG far *)dp;

	/* if called from DosDevIOCtl, dlen is 0 */
	if (dlen==0) dlen = 3*sizeof(ULONG);

	if (Verify(dp,dlen,1)) return RP_EINVAL;

	lp[0] = XTYDRV_MAGIC;
	lp[1] = (LONG)id;
	if (dlen==3*sizeof(ULONG))
		lp[2] = VERSIONID;
	return RPDONE;	
}

int drvname(char far* d, int dlen, char far* str)
{
	if ((dlen && dlen != 14) || Verify(d, 14, 1))
		return RP_EINVAL;
	bmove(d, str, 14);
	return RPDONE;
}

/* to avoid memcpy from the MSC library */
void bmove(char far* to, char far* from, USHORT nmove)
{
	while (nmove-- > 0)
		*to++ = *from++;
}

/* to avoid memset from the MSC library */
void bset(char far* to, char val, int nmove)
{
	while (nmove-- > 0)
		*to++ = val;
}

/* selopen: for selreg */
int selopen(ULONG far* tgt, ULONG far* src)
{
	/* pointer valid? */
	if (!tgt) {

#ifdef PRE1_354
		if (*src) {
			*src = 0l;
			return 1;	/* rejected */
		} else return 0;	/* nothing to do */
#else
		return 0;
#endif
	}

	if (*tgt) {	/* we already have one */
		/* close old one if different */
		if (*tgt == *src)
			return 0;
		CloseSEV(*tgt);
	}

	/* and open new one */
	*tgt = *src;
	return (*tgt && OpenSEV(*tgt) < 0) ? -1 : 0;
}

/* set select semaphores according to parameter packet and report status
 * A process may submit a new semaphore (non-null value in a field) or
 * ask for the current available ones (null fields).
 */
int selreg(void far* pp, void far* dp, ULONG far* rsel, ULONG far* xsel)
{
	struct pt_sel far* p = (struct pt_sel far*)pp;
	struct pt_sel far* d = (struct pt_sel far*)dp;
	int ret;

	if (Verify(pp,sizeof(struct pt_sel),0)) return RP_EINVAL;
	if (Verify(dp,sizeof(struct pt_sel),1)) return RP_EINVAL;

	/* copy param pkt to data pkt */
	bmove((char far*)d,(char far*)p,sizeof(struct pt_sel));
	d->code = 0;

	if ((ret = selopen(rsel,&d->rsel)))
		BSET(d->code, XTYSEL_RSEL);
	if (ret<0) return RP_EINVAL;
	if ((ret = selopen(xsel,&d->xsel)))
		BSET(d->code, XTYSEL_XSEL);
	return (ret<0) ? RP_EINVAL : RPDONE;
}

/* selarm: prepare the driver for posting a semaphore according to mask */
int selarm(ULONG mask, ULONG far* state, ULONG rsel)
{
	/* force semaphores to reset, will be posted separately afterwards */
	if (rsel && ResetSEV(rsel) < 0)
		return RP_EGEN;

	/* set the bits in the state to mark semas as armed */
	BSET(*state, mask);
	return RPDONE;
}

int selraise(ULONG sel, ULONG mask, ULONG far* state)
{
	/* post semaphore */
	if (sel && ISSET(*state, mask) && PostSEV(sel) < 0)
		return RP_EINVAL;

	/* and clear the corresponding bit */
	BCLR(*state, mask);
	return RPDONE;		
}

/* Request packet queue management */

/* initialize a request packet queue */
void rpqinit(RPQ rpq)
{
	rpq->head = rpq->tail = 0;
}

/* add an entry to the end of the queue */
void rpqenque(RPQ rpq, RP rp)
{
	RP tail = rpq->tail;
	RPLINK(rp) = 0;

	if (tail)
	{
		RPLINK(tail) = rp;
		rpq->tail = rp;
	}
	else
		rpq->head = rpq->tail = rp;
}

/* remove the first entry of the queue */
RP rpqdeque(RPQ rpq)
{
	RP head = rpq->head;
	if ((rpq->head = RPLINK(head)) == 0)
		rpq->tail = 0;
	RPLINK(head) = 0;
	return head;
}

/* discard a specified entry from the queue */
void rpqdiscard(RPQ rpq, RP rp)
{
	RP p = rpq->head;
	if (p == rp)
	{
		rpqdeque(rpq);
		return;
	}
	while (p && RPLINK(p) != rp)
		p = RPLINK(p);
	if (p)
	{
		if (RPLINK(p) == rpq->tail)
		{
			rpq->tail = p;
			RPLINK(p) = 0;
		}
		else
			RPLINK(p) = RPLINK(RPLINK(p));
	} /* else attn: the system is unstable! */
}

/* character queue management */

UCHAR qmask[] = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80 };

/* initialize a queue */
void qinit(CQ q)
{
	if (q) qflush(q);
}

/* get a char from queue, if empty return -1 */
#define PEEK 1
#define NOPEEK 0
int getc(CQ q, int peek)
{
	register int qp = q->qpos;
	int c;

	/* if empty, leave */
	if (qempty(q))
	  return -1;

	/* get the char at qpos */
	c = q->q[qp];

	if (peek==NOPEEK)
	{
		/* point to next char */
		qp++; q->qlen--;
		if (qp == CQSIZE || q->qlen == 0)
			qp = 0;
		q->qpos = qp;
	}

	return c & 0xff;
}

#ifdef NOTUSED
int peekc(CQ q,int far *pos)
{
	register int c;
	register int nx;

	/* no chars */
	if (qempty(q)) return -1;

	/* initialize if pointer negative */
	nx = (*pos < 0) ? q->qpos : *pos;

	/* reached the end of buffer? */
	if (nx == ((q->qpos+q->qlen) % CQSIZE))
		return -1;

	/* get the char at selected position */
	c = q->q[nx];

	/* point to next char */
	nx++;
	if (nx == CQSIZE) nx = 0;

	/* set the new pointer */
	*pos = nx;
	return c;
}
#endif

/* put char into queue, if full, return -1 */
int putc(int c,CQ q)
{
	/* calculate write position */
	register int wpos = (q->qpos + q->qlen) % CQSIZE;

	/* full? return */
# if 1
  /* This will fill only up to CQ_HIWWATER, but data is written up to CQSIZE */
	if (qfull(q)) return -1;
# else
  if ( q->qlen == CQSIZE )
    return -1;
# endif

	/* store the character */
	q->q[wpos] = (UCHAR)(c & CHARMASK);

	/* one more char */
	q->qlen++;

	return c;
}

#ifdef NOTUSED
/* unput last pushed char from queue, if empty, return -1 */
int unputc(CQ q)
{
	/* calculate position of last write */
	register int lastw = (q->qpos + q->qlen - 1) % CQSIZE;
	int c;
	
	/* no chars in queue? */
	if (qempty(q)) return -1;

	/* get char and quote if necessary */
	c = q->q[lastw];
	
	/* one char less */
	q->qlen--;
	return c;
}
#endif

int sendsig(int sig)
{
	switch (sig) {
	case SIGINT:
		SendEvent(2,0);
		return 0;
	case SIGBREAK:
		SendEvent(1,0);
		return 0;
	default:
		return RP_EINVAL;
	}
}

int pgsignal(int pgrp, int sig)
{
	pgrp=pgrp;

	/* ignore pgrp for now */
	return sendsig(sig);
}
