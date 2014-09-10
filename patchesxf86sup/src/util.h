/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#ifndef UTIL_H
#define UTIL_H

extern int qrequest(int n);
extern void qrelease(int n);

extern char i2hex(int n);
extern void i2ascii(char far *p, int n);
extern int drvname(char far* d, int dlen, char far *str);
extern int drvid(void far* dp, int dlen, int id);
extern void bmove(char far* to, char far* from, USHORT nmove);
extern void bset(char far* to, char val, int nmove);

/* selection services */

extern int selreg(void far*,void far*,ULONG far*, ULONG far*);
extern int selraise(ULONG sel,ULONG mask, ULONG far* state);
extern int selarm(ULONG mask, ULONG far* state, ULONG rsel);

/* request queue management */

struct rpqueue {
	void far* head;
	void far* tail;
};
typedef struct rpqueue RPQUEUE;
typedef struct rpqueue far* RPQ;

extern void rpqinit(RPQ rpq);
extern void rpqenque(RPQ rpq, RP rp);
extern RP rpqdeque(RPQ rpq);
extern void rpqdiscard(RPQ rpq, RP rp);
#define rpqempty(rpq) (rpq->head==0)
#define rpqtop(rpq) (rpq->tail)

/* character queue management */

#define CQSIZE	1600
/* these parameters differ from original driver */
#define CQ_HIWATER 1584	/* 99/100 * CQSIZE */
#define CQ_LOWATER 16	  /* 1/100 * CQSIZE  */
#define CQ_HALF 800	    /* 1/2 * CQSIZE    */

struct cqueue {
	short qpos;		/* position of next read */
	short qlen;		/* qpos+qlen = pos of next write */
	UCHAR q[CQSIZE];	/* data bits */
};
typedef struct cqueue CQUEUE;
typedef struct cqueue far* CQ;

extern void qinit(CQ q);
#define qsize(q) ((q)->qlen)
#define qflush(q) (q)->qlen = (q)->qpos = 0
#define qfull(q) ((q)->qlen>=CQ_HIWATER)
#define qempty(q) ((q)->qlen==0)
#define PEEK 1
#define NOPEEK 0
extern int getc(CQ q, int peek);
extern int putc(int c,CQ q);

/* interrupt services */
extern int sendsig(int);
extern int pgsignal(int,int);
#endif
