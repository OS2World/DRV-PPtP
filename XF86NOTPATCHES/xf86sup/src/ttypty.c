/* Copyright (C) 1995, 1996 by Holger Veit (Holger.Veit@gmd.de) */
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
#include "..\api\conidc.h"
#include "tty_priv.h"

/* CONTROLLER, MASTER PART */
extern int ptc_init(RP);
extern int ptc_read(RP,int);
extern int ptc_write(RP,int);
extern int ptc_ndr(RP,int);
extern int ptc_open(RP,int);
extern int ptc_close(RP,int);
extern int ptc_shutdown(int);
extern int ptc_sinput(int);
extern int ptc_soutput(int);
extern int ptc_select(TTY);

/* SLAVE PART */
extern int pts_init(void);
extern int pts_read(RP,int);
extern int pts_write(RP,int);
extern int pts_ndr(RP,int);
extern int ptx_iflush(int);
extern int ptx_oflush(int);
extern int pts_open(RP,int);
extern int pts_close(RP,int);
extern int pts_sinput(int);
extern int pts_soutput(int);
extern int pts_shutdown(void);
extern int ptx_ioctl(RP,int,int);
extern int pts_select(TTY);

extern int ptmsstrategy(RP rp);
extern int ptms_open(void);
extern int ptms_ioctl(RP rp);

extern void ttinit(TTY tp);
extern void ttstart(TTY tp);
extern void ttread(RP rp, TTY tp, char far* bp, int nb);
extern int ttoutput(TTY tp, int c);
extern int ttecho(TTY tp,int c);
extern int ttflush(TTY tp, int mode);
extern int ttinput(TTY tp,int c);
extern int ttwait(TTY tp);

/* abbreviations, because noone knows what cl makes from '\n' and '\r' */ 
#define BELL	0x07
#define BS	0x08
#define TAB	0x09
#define LF	0x0a
#define CR	0x0d
#define ESC	0x1b
#define DEL	0x7f

#define ISCTRL(c) (c<0x20 || c==0x7f)

/*
 * Control Character Defaults
 */
#define CTRL(x)	(x&037)
#define	CEOF	CTRL('d')
#define	CEOL	((UCHAR)0)
#define	CERASE	0177
#define	CKILL	CTRL('u')
#define	CINTR	CTRL('c')
#define	CQUIT	034
#define CMIN	1
#define	CTIME	0
#define	CSUSP	CTRL('z')
#define	CSTOP	CTRL('s')
#define	CSTART	CTRL('q')

/*
 * #define TTYDEFCHARS to include an array of default control characters.
 * This is compatible to EMX.
 */
UCHAR	ttydefchars[NCCS] = {
	CINTR, CQUIT, CERASE, CKILL, CEOF, 
	CEOL, CMIN, CTIME, CSUSP, CSTOP,
	CSTART
};

#define	TTYDEF_IFLAG	(0)
#define TTYDEF_OFLAG	(OPOST | ONOCR | OXTABS)
#define TTYDEF_LFLAG	(ECHO | ICANON | ISIG )
#define TTYDEF_CFLAG	(CREAD | CS8)
#define TTYDEF_SPEED	(B9600)

#define HISTMUTEX	33

char ttybufinit = 0;
extern char far* buf1;
extern char far* buf2;

void ttybuf_initr0(void)
{
	int i;
	TTY tp;
	char far *ptr;

	/* allocate memory */
	buf1 = AllocBuf(32l*sizeof(struct tty));
	buf2 = AllocBuf(32l*sizeof(struct tty));

	/* fill the data structures */
	constty = 0;
	ptr = buf1;
	for (i=0; i<NPTY; i++) {
		ptr = i<16 ? buf1 : buf2;
		tp = (TTY)(ptr+((i%16)*sizeof(struct tty)));
		ttys[i] = tp;
		tp->state = 0;
		tp->send = tp->ucntl = 0;
		tp->lptr = 0;
		qinit(&tp->inq);
		qinit(&tp->outq);
		tp->dev = i;
		tp->pgrp = tp->pid = 0;
		tp->ccnt = tp->scnt = 0;
		tp->ocol = 0;
		rpqinit(&tp->crq);
		rpqinit(&tp->cwq);
		rpqinit(&tp->srq);
		rpqinit(&tp->swq);
		tp->crsel = tp->srsel = tp->cxsel = 0;
		tp->tracelvl = 0;
	}

	ttybufinit = 1;
}

void ttinit(TTY tp)
{
	bmove(tp->t_cc, ttydefchars, sizeof(ttydefchars));
	tp->t_iflag = TTYDEF_IFLAG;
	tp->t_oflag = TTYDEF_OFLAG;
	tp->t_lflag = TTYDEF_LFLAG;
	tp->t_cflag = TTYDEF_CFLAG;
	bset((char far*)&tp->winsize, 0, sizeof(struct pt_winsize));
	qinit(&tp->inq);
	qinit(&tp->outq);
}

void ttstart(TTY tp)
{
	ULONG state = tp->state;

	if (ISSET(state, ST_STOPPED))
		return;
	if (state & PF_STOPPED) {
		BCLR(tp->state, PF_STOPPED);
		tp->send = TIOCPKT_START;
		ptc_select(tp);
	}
}

void handle_eof(RP rp, TTY tp, char far* bp)
{
	ULONG state = tp->state;

	/* seen EOF */
	if (state & ST_EOF) {
		traceeof(tp,TR_SLAVE|TR_READ);

		/* send a signal? */
		if (ISSET(state,ST_EOFSIGB))
			sendsig(SIGBREAK);
		else if(ISSET(state,ST_EOFSIGC))
			sendsig(SIGINT);

		/* what to return? */
		if (ISSET(state,ST_EOFCTLD)) {
			bp[0] = CTRL('d');
			traceput(tp,CTRL('d'),TR_SLAVE|TR_READ);
			RWCNT(rp) = 1;
		} else if (ISSET(state,ST_EOFCTLC)) {
			bp[0] = CTRL('c');
			traceput(tp,CTRL('c'),TR_SLAVE|TR_READ);
			RWCNT(rp) = 1;
		} else if (ISSET(state,ST_EOFCTLZ)) {
			bp[0] = CTRL('z');
			traceput(tp,CTRL('c'),TR_SLAVE|TR_READ);
			RWCNT(rp) = 1;
		} else {
			RWCNT(rp) = 0;
		}
	}
}

/*
 * Process read
 */
void ttread(RP rp, TTY tp, char far* bp, int nb)
{
	USHORT lflag = tp->t_lflag;
	CQ qp = &tp->inq;
	int cnt = 0;
	int eof = 1;	
	int c;

	/* read chars from queue */
	while (nb != 0) {
		/* get a char */
		c = getc(qp,NOPEEK);
		
		/* if there is none, exit */
		if (c < 0) break;

		/* if ISIG and char is the CINTR char, produce a signal
		 * We must do this here synchronously, in order to
		 * send the signal to the process which is currently reading
		 * If we would do SendEvent already in ttinput, it would shoot
		 * the sender process
		 */
		if (ISSET(lflag,ISIG) && CCEQ(tp->t_cc[VINTR],c)) {
			SendEvent(2,0); /* XXX should be sendsig() */
		}

		/* simplified ICANON processing */
		if (ISSET(lflag,ICANON)) {
			if (CCEQ(tp->t_cc[VEOF],c)) {
				eof = 1;
				break;
			}
		}

		bp[cnt++] = (char)c;
		traceput(tp,c,TR_SLAVE|TR_READ);
		nb--;
		eof = 0;
		if (ISSET(lflag,ICANON) && (CCEQ(tp->t_cc[VEOL],c) || c==LF)) 
			break;
	}
	RWCNT(rp) = cnt;
	if (ISSET(lflag,ICANON) && eof)
		handle_eof(rp, tp, bp);

	return;
}

/*
 * Perform OPOST processing, queue character into outq,
 * return -1 if outq full
 */ 
int ttoutput(TTY tp, int c)
{
	CQ qp = &tp->outq;
	USHORT oflag = tp->t_oflag;
	USHORT lflag = tp->t_lflag;

	if (ISSET(oflag, OPOST)) {
		/* OPOST: do tab expansion */
		if (c==TAB && ISSET(oflag,OXTABS) && ISCLR(lflag,EXTPROC)) {
			for (c = 8-(tp->ocol & 7); c>0; c--) {
				if (putc(' ',qp) < 0)
					break;
			}
			tp->ocol += 8-(tp->ocol & 7)-c;
			return c ? -1 : TAB;
		} else if (CCEQ(tp->t_cc[VEOF],c) && ISSET(oflag,ONOEOT))
			return c;
		else if (c == LF) {
			tp->ocol = 0;
			/* translate into \r\n (BSD, tty.c) */
			if (ISSET(oflag,ONLCR))
				if (putc(CR,qp) < 0) return -1;
			return (putc(c,qp) < 0) ? -1 : c;
		} else if (c == CR) {
			tp->ocol = 0;
			if (ISSET(oflag,OCRNL))
				c = LF;
			return (putc(c,qp) < 0) ? -1 : c;
		} else if (c >= 'a' && c <= 'z' && ISSET(oflag,OLCUC))
			c -= ('a'-'A');
		else if ((c&0x7f) < 0x20 || c==0x7f) {
			if (ISSET(lflag,ECHOCTL)) {
				ttoutput(tp,'^'); /* recurse! */
				c = c==0x7f ? '?' : (c+'A'-1);
			} else if (c==CTRL('c') || c==CTRL('d'))
				return c; /* don't echo that stuff */
		}
	}

	if (putc(c,qp) < 0)
		return -1;
	if (c==TAB)
		tp->ocol = (tp->ocol + 8) & ~7;
	else if (!ISCTRL(c))
		tp->ocol++;
	/* else CTRL: don't change ocol */
	return c;
}

int ttecho(TTY tp,int c)
{
	USHORT lflag = tp->t_lflag;
	CQ qp = &tp->outq;

	if ((ISSET(lflag, ECHO) || (ISSET(lflag, ECHONL) && c==LF)) &&
	    ISCLR(lflag, EXTPROC)) {
		return ttoutput(tp, c);
	}
	return 0;
}

void ptsstop(TTY tp, int fl)
{
	if (!fl) {
		fl = TIOCPKT_STOP;
		BSET(tp->state, PF_STOPPED);
	} else	
		BCLR(tp->state, PF_STOPPED);
	BSET(tp->send, fl);
	ptc_select(tp);
}

int ttflush(TTY tp, int mode)
{
	if (mode & 1) {
		qflush(&tp->inq);
	}
	if (mode & 2) {
		BCLR(tp->state, ST_STOPPED);
		ptsstop(tp,mode);
		qflush(&tp->outq);
		if (ISSET(tp->state,ST_DRAIN)) {
			BCLR(tp->state, ST_DRAIN);
			Run(&tp->outq);
		}
	}

	return RPDONE;
}

void ttemit(TTY tp)
{
	CQ rq = &tp->inq;
	int i;

	for (i=0; i<tp->lsz; i++) {
		putc(tp->line[i],rq);
		tp->line[i] = ' ';
	}
	tp->lsz = tp->lptr = 0;
}

void ttechostr(TTY tp,char *str,int n)
{
	int i;
	for (i=0; i<n; i++)
		ttecho(tp,(int)str[i]);
}

void emit_if_overflow(TTY tp)
{
	if (tp->lsz==(LBUFSZ-2)) {
		/* overflow in buffer, emit it to queue */
		ttecho(tp,BELL);
		ttemit(tp);
	}
}

void linegoto(TTY tp,int n) /* <0 or >0 */
{
	if (n==0) return;
	if (n < 0) { /* backward */
		while (n++ < 0) {
			tp->lptr--;
			ttecho(tp,BS);
		}
	} else { /* forward */
		if (tp->lptr == tp->lsz) {
			tp->line[tp->lptr++] = ' ';
			ttecho(tp,' ');
		} else {
			while (n-- > 0)
				ttecho(tp,tp->line[tp->lptr++]);
		}
		if (tp->lptr >= tp->lsz)
			tp->lsz = tp->lptr;
	}
}

void lineecho(TTY tp,int n)
{
	int i;
	for (i=n; i<tp->lsz; i++)
		ttecho(tp,tp->line[i]);
	i = tp->lsz - tp->lptr;
	while (i-- > 0) 
		ttecho(tp,BS);
}

void lineput(TTY tp, int c)
{
	char *line = tp->line;

	emit_if_overflow(tp);
	if (ISCLR(tp->state,ST_OVWRITE)) { /* insert mode */
		int i;
		for (i=tp->lsz; i>=tp->lptr; i--) { /* make space */
			line[i+1] = line[i];
		}
		tp->lsz++;
		ttecho(tp,c);
		line[tp->lptr++] = (char)c;
		lineecho(tp,tp->lptr);
	} else { /* overwrite mode */
		line[tp->lptr++] = (char)c;
		ttecho(tp,c);
	}
	if (tp->lptr >= tp->lsz)
		tp->lsz = tp->lptr;
}

void ttdump(TTY tp)
{
	int i;
	char buf[10];

	ttechostr(tp,"\r\nLinebuf=>",11);
	for (i=0; i<tp->lptr; i++)
		ttecho(tp,(int)tp->line[i]);
	ttecho(tp,'|');
	ttecho(tp,(int)tp->line[i]);
	ttecho(tp,'|');
	for (i=tp->lptr+1; i<tp->lsz; i++)
		ttecho(tp,(int)tp->line[i]);
	ttechostr(tp,"< ins=",6);
	ttecho(tp,ISSET(tp->state,ST_OVWRITE) ? '0' : '1');
	ttechostr(tp," pos=",5);
	i2ascii(buf,tp->lptr); ttechostr(tp,buf+2,3);
	ttechostr(tp," sz=",4);
	i2ascii(buf,tp->lsz); ttechostr(tp,buf+2,3);
	ttechostr(tp,"\r\n",2);
	lineecho(tp,0);
}

void linedelchar(TTY tp)
{
	char *line = tp->line;
	int i;

	/* remove the char at the cursor position */
	line[tp->lsz]=' ';
	line[tp->lsz+1]=' ';
	for (i=tp->lptr; i<tp->lsz; i++)
		line[i]=line[i+1];
	lineecho(tp,tp->lptr);
	tp->lsz--;
}

void linedeleoline(TTY tp)
{
	char *line = tp->line;
	while (tp->lptr < tp->lsz)
		linedelchar(tp);
}

/* the historybuffer is a scrolling fifo buffer which always appends the
 * last line entered. If the buffer is full when a new one is about to be
 * entered, the oldest messages are discarded.
 * A line longer than the buffer size is never stored.
 * The lines in the buffer are 0-terminated, without a LF char.
 * histptr points to the next available write position
 * curhistptr points to the beginning of the line last retrieved, or
 * histptr. 
 */

void historyset(TTY tp, char far* hbuf)
{
	int i;

	/* this does not request the sema HISTMUTEX, 
	 * because it is already owned 
	 */
	linegoto(tp,-tp->lptr); /* goto first char */
	linedeleoline(tp);
	for (i=tp->curhistptr; hbuf[i]; i++)
		lineput(tp,hbuf[i]);
}

void historyput(TTY tp)
{
	char far *hbuf;
	char c;
	short lsz;
	short hp;

	qrequest(HISTMUTEX);
	hbuf = MapHist(tp->histbuf);
	if (!hbuf) goto ex;

	/* calculate necessary number of chars */
	lsz = tp->lsz;
	while ((c=tp->line[lsz])==' ' || c==TAB || c==LF || c==CR) lsz--;
	lsz++;

	/* make space for LSZ+1 chars */
	hp = tp->histptr;

	/* lsz==1: empty line */
	if (lsz <= 1 || (lsz+1) >= histbufsz) goto ex;
	while ((lsz+1) > (histbufsz-hp)) {
		/* discard oldest entry */
		int i;
		for (i=0; hbuf[i] && i<hp; i++);
		bmove(hbuf,&hbuf[i+1],hp-i-1);
		tp->histptr -= (i+1);
		hp = tp->histptr;
	}
	/*copy line */
	bmove(&hbuf[hp],tp->line,lsz);
	hbuf[hp+lsz] = 0;
	tp->histptr += (lsz+1);
	tp->curhistptr = tp->histptr;
ex:
	qrelease(HISTMUTEX);

}

void historyup(TTY tp)
{
	char far *hbuf;
	short chp;

	qrequest(HISTMUTEX);

	hbuf = MapHist(tp->histbuf);
	if (!hbuf) goto ex;

	chp = tp->curhistptr;

	chp--; /* should be a 0 byte */
	if (chp < 0) {
		if (tp->histptr==0) goto ex; /* history empty */
		chp = tp->histptr-1; /* wrap around */
	}
	for (;;) { /* go backward until pos 0 or Null byte. */
		chp--;
		if (chp < 0 || hbuf[chp]==0) break;	
	}
	/* now points to a beginning of line, or is -1 */
	chp++;
	tp->curhistptr = chp;

	/* set the value of curhistptr into line buffer */
	historyset(tp,hbuf);	
ex:
	qrelease(HISTMUTEX);
}

void historydown(TTY tp)
{
	char far *hbuf;
	short chp,hp;

	qrequest(HISTMUTEX);
	hbuf = MapHist(tp->histbuf);
	if (!hbuf) return;

	hp = tp->histptr;
	if (hp==0) goto ex;	/* empty history buffer */
	chp = tp->curhistptr;

	if (chp == hp) 
		chp = 0;
	else {
		/* search next null byte */
		while (chp < hp && hbuf[chp]) chp++;
		chp++;
	}
	tp->curhistptr = chp;

	/* set the value of curhistptr into line buffer */
	historyset(tp,hbuf);
ex:
	qrelease(HISTMUTEX);
}

void historydump(TTY tp)
{
	int i,k,n;
	char far *hbuf;
	char far *num;

	qrequest(HISTMUTEX);
	hbuf = MapHist(tp->histbuf);
	if (!hbuf) goto ex;

	ttechostr(tp,"\r\nhistory buffer\r\n",18);
	for (i=n=0; i < tp->histptr; i = (k+1),n++) {
		for (k=i; hbuf[k]; k++);
		num = itoa(n);
		ttechostr(tp, &num[1], 4);
		ttecho(tp, i==tp->curhistptr ? ':' : ' ');	
		ttecho(tp,'>');
		ttechostr(tp, &hbuf[i], k-i);
		ttecho(tp,'<');
		ttechostr(tp, "\r\n", 2);
	}
	ttechostr(tp, "\r\n", 2);
	lineecho(tp, 0);
ex:
	qrelease(HISTMUTEX);
}

void ttynamedump(TTY tp)
{
	char dpbuf[12];

	ttechostr(tp,"\r\nTTY used =",12);
	makename((char far*)dpbuf,0,tp->dev);
	ttechostr(tp,dpbuf,10);
	ttechostr(tp,"\r\n",2);
	lineecho(tp,0);
}

/* 
 * Put a character into linebuf/inq and do ICANON processing
 */
int putc2(int c, TTY tp)
{
	CQ rq = &tp->inq;
	USHORT lflag = tp->t_lflag;
	char esc;

	if (ISSET(lflag,ICANON)) {
		ULONG state = tp->state;

		/* canonical processing */
		/* functions:
		 *	LF, VEOL: flush complete line to inq, echo
		 *	^H: remove char to the left, and echo BS
		 *		if not yet at start of line
		 *	INSERT: Toggle insert/overwrite mode
		 *	DELETE: delete char under cursor
		 *NOTYET	TAB: store as TAB and echo as spaces
		 *	^U: delete complete linebuf
		 *	^V: Quote next char
		 *	^A: goto 1st char
		 *	^E: goto last char
		 *	^F: forward char
		 *	^B: backward char
		 *	ESC [ C: forward one char
		 *	ESC [ D: backward one char
		 *	ESC [ F: goto last char
		 *	ESC [ H: goto 1st char
		 *	ESC [ A: scroll up history
		 *	ESC [ B: scroll down history
		 *	^P: scroll up history
		 *	^N: scroll down history
		 *	^K: delete to end of line
		 *	ESC?H: debug history
		 *	ESC?T: debug ttyname
		 *	ESC?L: debug lineedit
		 *	ESC?C: clear history buffer
		 *
		 * escstate = 0: no ESC in progress
		 *          = 1: ESC seen
		 *	    = 2: ESC[ seen
		 *	    = 3: ESC[2 seen
		 *          = 4: ESC? seen
		 */
		if (ISSET(state,ST_QUOTE))
			goto norm;
		if (CCEQ(tp->t_cc[VEOL],c) || c==LF) {
			/* flush line */
			historyput(tp);
			ttemit(tp);
			goto raw;
		} else switch(c) {
		case ESC:
			if (tp->escstate > 0)
				goto norm;
			tp->escstate = 1;		/* ESC prefix */
			return c;
		case '[':
			if (tp->escstate==1) {
				tp->escstate = 2;	/* ESC[ prefix */
				return c;
			}
			goto norm;			
		case '2':
			if (tp->escstate==2) {
				tp->escstate = 3;	/* ESC[2 prefix */
				return c;
			}
			goto norm;
		case '~':
			if (tp->escstate==3) {
				tp->escstate = 0;	/* ESC[2~ */
				if (ISSET(tp->state,ST_OVWRITE))
					BCLR(tp->state,ST_OVWRITE);
				else
					BSET(tp->state,ST_OVWRITE);
				return c;
			}
			goto norm;
		case 'A':
			if (tp->escstate != 2)
				goto norm;
			tp->escstate = 0;	/* ESC[A */
			c = CTRL('P');
			/*fallthru*/
		case CTRL('P'):
			historyup(tp);
			return c;
		case 'B':
			if (tp->escstate != 2)
				goto norm;
			tp->escstate = 0;	/* ESC[B */
			c = CTRL('N');
			/*fallthru*/
		case CTRL('N'):
			historydown(tp);
			return c;
		case '?':
			if (tp->escstate == 1) {
				tp->escstate=4;
				return c;		/* ESC? */
			}
			goto norm;
		case 'L':
			if (tp->escstate == 4) {
				tp->escstate = 0;
				ttdump(tp);		/* ESC?L */
				return c;
			}
			goto norm;
		case DEL: /* the DEL key on the keypad */
			linedelchar(tp);
			return c;
		case BS: /* the key with the <---- */
			if (tp->lptr > 0) {
				tp->lptr--;
				ttechostr(tp,"\b \b",3);
				linedelchar(tp);
			} else
				ttecho(tp,BELL);
			return c;
		case CTRL('U'):
			ttechostr(tp,"^U\r\n",4);
			tp->lptr = tp->lsz = 0;
			return c;
		case CTRL('V'):
			BSET(tp->state,ST_QUOTE);
			return c;
		case 'C':
			if (tp->escstate==4) {	/* ESC?C */
				tp->escstate = 0;
				tp->histptr = tp->curhistptr = 0;
				return c;
			} else if (tp->escstate!=2)
				goto norm;
			tp->escstate = 0;	/* ESC[C */
			c = CTRL('F');
			/*fallthru*/
		case CTRL('F'):
			emit_if_overflow(tp);
			linegoto(tp,1);
			return c;
		case 'D':
			if (tp->escstate != 2)
				goto norm;
			tp->escstate = 0;	/* ESC[D */
			c = CTRL('B');
			/*fallthru*/
		case CTRL('B'):
			if (tp->lptr > 0)
				linegoto(tp,-1);
			else
				ttecho(tp,BELL);
			return c;
		case CTRL('K'):
			linedeleoline(tp);
			return c;
		case 'T':
			if (tp->escstate==4) {
				tp->escstate = 0;
				ttynamedump(tp);
				return c;
			}
			goto norm;
		case 'H':
			if (tp->escstate==2) {
				tp->escstate = 0;	/* ESC[H */
				c = CTRL('A');
				/*fallthru*/
			} else if (tp->escstate==4) { /* ESC?H */
				tp->escstate =0;
				historydump(tp);
				return c;
			} else goto norm;
			/*fallthru*/
		case CTRL('A'):
			linegoto(tp,-tp->lptr);
			return c;
		case CTRL('E'):
			linegoto(tp,tp->lsz - tp->lptr);
			return c;
		case TAB: /* not yet processed correctly on delete */
		default:
			break;
		}
		/* normal character */
norm:		/* pending things? */
		esc = tp->escstate; tp->escstate = 0;
		if (esc>=1) lineput(tp,ESC);
		if (esc==2||esc==3) lineput(tp,'[');
		if (esc==3) lineput(tp,'2');
		if (esc==4) lineput(tp,'?');
		BCLR(tp->state,ST_QUOTE);
		lineput(tp,c);
		tp->curhistptr = tp->histptr;
		return c;
	}
raw:

	/* raw, no canon processing */
	if (putc(c,rq) < 0) return -1;
	return ttecho(tp, c);
}

/*
 * Process an input character, with input processing
 * return -1, if queue full.
 */
int ttinput(TTY tp, int c)
{
	USHORT lflag = tp->t_lflag;
	USHORT iflag = tp->t_iflag;
	ULONG state = tp->state;
	char *cc = tp->t_cc;
	CQ rq = &tp->inq;
	int ret = 0;

	/* only 8 bits */
	c &= 0xff;
	if (ISSET(iflag,ISTRIP))
		BCLR(c,0x80);

	if (c==CR) {
		if (ISSET(iflag,IGNCR)) {
			goto endproc;
		} else if (ISSET(iflag,ICRNL)) {
			c = LF;
		} else {
			traceput(tp, CR, TR_MASTER|TR_WRITE);
			if (putc2(CR,tp) < 0) return -1;
			return 0;
		}
	} else if (ISCLR(lflag,EXTPROC)) {

		/* internal processing */
		if (ISSET(lflag,ISIG)) {
			if (CCEQ(cc[VINTR],c) || CCEQ(cc[VQUIT],c)) {
				if (ISCLR(lflag,NOFLSH))
					ttflush(tp,3);
				ttecho(tp,c);
				BSET(tp->state,ST_SIGTTY);
				/*pgsignal(tp->pgrp, SIGINT);*/
				goto endproc;
			}
		}
	} else {
		/* else was an LF */
		if (c==LF && ISSET(iflag,INLCR))
			c = CR;
	}

	if (ret >= 0) {
		/* regular char or CR/LF: put in queue */
		traceput(tp, c, TR_MASTER|TR_WRITE);
		ret = putc2(c,tp);
	}
endproc:
	/* reaches here if something has been processed */
	ttstart(tp);
	return ret;
}

int ttwait(TTY tp)
{
	while (qsize(&tp->outq) != 0 && ISSET(tp->state,ST_COPEN) && 
	       ISSET(tp->state,ST_SOPEN)) {
		BSET(tp->state,ST_DRAIN);
		/* Wait until output is empty */
		if (Block(&tp->outq,-1l,0)==2)
			return RP_EINTR;
	}
	return RPDONE;
}

/**********************************************************************
 * Thread management routines
 *********************************************************************/

/* this routine blocks a reader until there is data available to read */
int block_read(RPQ rpq, RP rp, CQ qp)
{
	/* data available? */
	if (qempty(qp)) {
		/* enqueue packet */
		rpqenque(rpq,rp);

		/* no, block on this request */	
		for(;;) {
			RPSTAT(rp) = RPDONE;
			/**** returns here if this packet is run */
			if (Block(rp,-1l,0)) {
				/* spurious, block again */
				if (RPSTAT(rp) != RPDONE) continue;
				/* interrupted */
				if (rp != rpqtop(rpq)) {
					/* this is not the current packet,
					 * unlink it, and return it as 0 bytes 
					 */
					rpqdiscard(rpq,rp);
					RWCNT(rp) = 0;
					return RPDONE;
				}
			}
			break; /* leave loop */
		}
		/* we get here after block ended, get packet */
		rp = rpqdeque(rpq);
	}
	return 0;
}

void start_reader(RPQ rside)
{
	if (!rpqempty(rside))
		Run(rpqtop(rside));
}

void run_thread(CQ qp, RPQ rside, RPQ wside)
{
	/* more data in queue? run more reader threads */
	if (qsize(qp) > 0) {
		/* run more readers */
		start_reader(rside);
	}

	/* some space freed? run some writer threads */
	if (!rpqempty(wside) && !qfull(qp))
		Run(rpqtop(wside));
}

int block_write(RP rp, RPQ rpq)
{
	/* enqueue packet */
	rpqenque(rpq,rp);
	for (;;) {
		RPSTAT(rp) = RPDONE;
		if (Block(rp,-1l,0)) {
			/* spurious */
			if (RPSTAT(rp) != RPDONE) continue;
			/* interrupted */
			if (rp != rpqtop(rpq)) {
				/* this is not the current packet,
				 * unlink it, and return it 
				 * as 0 bytes 
				 */
				rpqdiscard(rpq,rp);
				RWCNT(rp) = 0;
				return RPDONE;
			}
		}
		break; /* leave block */
	}
	rpqdeque(rpq);
	return 0;
}

/**********************************************************************
 * Slave routines
 *********************************************************************/

#define assert_nomon(rp) if ((rp)->rp_stat & 0x08) return RP_EBAD;

/*
 * Slave open. Will only succeed if also controller is open on this slot.
 * Set second flag (client also open)
 */
int pts_open(RP rp, int dev)
{
	TTY tp;
	ULONG state;

	if (!ttybufinit)
		ttybuf_initr0();

	tp = ttys[dev];
	state = tp->state;

	assert_nomon(rp);

	if (ISSET(state,ST_COPEN)) {
		if (ISSET(state,ST_XCLUDE))
			return RP_EUSE;
		if (!tp->scnt)
			ttinit(tp);

		BSET(tp->state,ST_SOPEN);
		BCLR(tp->state,ST_PTMSLOCK);
		tp->scnt++;
		return RPDONE;
	}
	return RP_ERDY;
}

/* 
 * Slave close.
 */
int pts_close(RP rp, int dev)
{
	TTY tp = ttys[dev];

	assert_nomon(rp);

	tp->scnt--;
	if (tp->scnt < 0) tp->scnt = 0;
	if (!tp->scnt) {
		BCLR(tp->state, ST_SOPEN);

		/* on last close, post semaphores */
	}
	return RPDONE;
}

/*
 * Read bytes from slave side (slave's inq)
 */
int pts_read(RP rp, int dev)
{
	TTY tp = ttys[dev];
	CQ qp = &tp->inq;
	RPQ rpq = &tp->srq;
	RPQ orpq = &tp->cwq;
	char far *bp, c;
	int nb, cnt;

	/* is a CTRL-C pending? */
#ifdef NOTYET
	if (ISSET(tp->state,ST_SIGTTY)) {
		BCLR(tp->state,ST_SIGTTY);
		SendEvent(2,0);
	}
#endif

	/* 0 bytes requested */
	if (RWCNT(rp)==0) {
		traceeof(tp,TR_SLAVE|TR_READ);
		RWCNT(rp) = 0;
		return RPDONE;
	}

	/* block until some data is available. If NDELAY is set,
	 * go directly to reader, may return 0 bytes
	 * if controller is away, deliver what is still there
	 */
	if (ISSET(tp->state,ST_COPEN) &&
	    ISCLR(tp->state,ST_NDELAYS) && 
	    block_read(rpq, rp, qp)) {
		traceeof(tp,TR_SLAVE|TR_READ);
		RWCNT(rp) = 0;
		return RPDONE;
	}

	/* get buffer address */
	if (P2V(RWBUF(rp),RWCNT(rp),&bp) != 0)
		return RP_EGEN;

	nb = RWCNT(rp);
	RWCNT(rp) = 0;
	cnt = 0;

	qrequest(tp->dev);

	/* blocked thread continues here: maybe there is data or not */
	if (ISSET(tp->state,PF_REMOTE)) {
		if (!qempty(qp)) {
			while (qsize(qp) > 1 && nb != 0) {
				c = (char)getc(qp,NOPEEK);
				bp[cnt++] = c;
				traceput(tp, c, TR_SLAVE|TR_READ);
			}
			if (qsize(qp)==1) (void)getc(qp,NOPEEK);
			RWCNT(rp) = cnt;
		}
	} else
		ttread(rp,tp,bp,nb);

	traceflush(tp);
	qrelease(tp->dev);

	/* run more threads */
	run_thread(qp, rpq, orpq);

	return RPDONE;
}

/*
 * Nondestructive read from slave side (slave's inq)
 */
int pts_ndr(RP rp, int dev)
{
	TTY tp = ttys[dev];
	CQ qp = &tp->inq;
	int ret = RPDONE;

	/* controller away? */
	if (ISCLR(tp->state,ST_COPEN))
		return RPDONE;
	
	qrequest(tp->dev);
	if (qempty(qp))
		ret = RPDONE|RPBUSY;
	else
		RWNBUF(rp) = (UCHAR)getc(qp,PEEK);
	qrelease(tp->dev);
	return ret;
}

/*
 * Slave Write
 */
int pts_write(RP rp,int dev)
{
	TTY tp = ttys[dev];
	CQ qp = &tp->outq;
	RPQ rpq = &tp->swq;
	RPQ orpq = &tp->crq;
	UCHAR far *bp;
	int nb, c, cnt, ret;

	/* is a CTRL-C pending? */
#ifdef NOTYET
	if (ISSET(tp->state,ST_SIGTTY)) {
		BCLR(tp->state,ST_SIGTTY);
		SendEvent(2,0);
	}
#endif

	/* no controller or attempt to write 0 bytes */
	if (ISCLR(tp->state,ST_COPEN) || RWCNT(rp)==0) {
		RWCNT(rp) = 0;
		traceeof(tp,TR_SLAVE|TR_WRITE);
		return RPDONE;
	}

	nb = RWCNT(rp);
	RWCNT(rp) = 0;
	cnt = 0;

	if (!rpqempty(rpq)) goto block_on_write;
resume:
	/* rp points to current request */
	qrequest(tp->dev);

	/* get buffer address */
	if (P2V(RWBUF(rp),cnt+nb,&bp) != 0)
		return RP_EGEN;

	/* write as much as possible */
	while (nb != 0) {
		/* controller away? */
		if (ISCLR(tp->state, ST_COPEN)) {
			RWCNT(rp) = 0;
			traceeof(tp,TR_SLAVE|TR_WRITE);
			qrelease(tp->dev);
			return RPDONE;
		}

		c = bp[cnt];
		traceput(tp, c, TR_SLAVE|TR_WRITE);
		if (ISSET(tp->t_oflag, OPOST))
			/* do output processing */
			ret = ttoutput(tp,c);
		else {
			/* simply queue stuff */
			ret = putc(c,qp);
		}
		if (ret < 0) /* outq full */
			break;
		cnt++;
		nb--;		
	}
	RWCNT(rp) = cnt;
	traceflush(tp);
	qrelease(tp->dev);

	/* we arrive here on matter whether we sent anything or not */
block_on_write:
	/* listener still there and not everything transferred? */
	if (ISSET(tp->state, ST_COPEN) && nb != 0) {
		if (cnt) {
			tp->nbsave1 = nb;

			/* handle select */
			ptc_select(tp);

			/* if written anything, start reader */
			run_thread(qp,orpq,rpq);
		}

		/* we block here, unless ST_NDELAY is set:
		 * prepare to get "0 bytes sent" then, this is not EOF
		 */
		if (ISCLR(tp->state,ST_NDELAYS) && !block_write(rp,rpq)) {
			cnt = RWCNT(rp);
			nb = cnt ? tp->nbsave1 : 0;
			goto resume;
		}
	}

	/* handle select */
	ptc_select(tp);

	/* run more threads */
	run_thread(qp,orpq,rpq);

	return RPDONE;
}

int ptx_iflush(int dev)
{
	TTY tp = ttys[dev];

	qrequest(tp->dev);
	ttflush(tp,1);
	qrelease(tp->dev);
	return RPDONE;
}

int ptx_oflush(int dev)
{
	TTY tp = ttys[dev];

	qrequest(tp->dev);
	ttflush(tp,2);
	qrelease(tp->dev);
	return RPDONE;
}

int pts_sinput(int dev)
{
	TTY tp = ttys[dev];
	CQ qp = &tp->inq;
	return (qempty(qp)) ? RPDONE|RPBUSY : RPDONE;
}

int pts_soutput(int dev)
{
	TTY tp = ttys[dev];
	return (qsize(&tp->outq) > CQ_HIWATER) ? RPDONE|RPBUSY : RPDONE;
}

int pts_shutdown()
{
	return RPDONE;
}

int pts_select(TTY tp)
{
	/* something to read? */
	if (qsize(&tp->inq) > 0) {
		return selraise(tp->srsel,ST_ARMRSELS,&tp->state);
	}

	return RPDONE;
}

int ptsstrategy(RP rp,int dev)
{

	if (rp->rp_cmd != CMDInit && rp->rp_cmd != CMDShutdown) int3();

	switch (rp->rp_cmd) {
	case CMDInit:
		return RPDONE;
	case CMDNDR:
		return pts_ndr(rp,dev);
	case CMDINPUT:
		return pts_read(rp,dev);
	case CMDOUTPUT:
	case CMDOUTPUTV:
		return pts_write(rp,dev);
	case CMDInputF:
		return ptx_iflush(dev);
	case CMDOutputF:
		return ptx_oflush(dev);
	case CMDOpen:
		return pts_open(rp,dev);
	case CMDClose:
		return pts_close(rp,dev);
	case CMDGenIOCTL:
		return ptx_ioctl(rp,dev,0);
	case CMDInputS:
		return pts_sinput(dev);
	case CMDOutputS:
		return pts_soutput(dev);
	case CMDShutdown:
		return pts_shutdown();
	default:
		return RP_EBAD;
	}	
}

/******************************************************************
 * Controller part
 *****************************************************************/

/*
 * Controller open, will fail if already open.
 */
int ptc_open(RP rp, int dev)
{
	TTY tp;
	ULONG state;

	if (!ttybufinit)
		ttybuf_initr0();

	tp = ttys[dev];
	state = tp->state;

	assert_nomon(rp);

	/* someone already opened this pty? */
	if (ISSET(state,ST_COPEN) || ISSET(state,ST_SOPEN)) {
		/* XXX OS/2 seems to open an fd again on dup(). Accept this
		 * if the owner and the opener are the same process or a direct child.
		 * AND if a process which has this open has issued XTY_ENADUP.
		 * The game will stop if the handle is inherited further
		 */
		if (tp->pgrp != GetPGRP() || ISCLR(state,ST_ENADUP))
			return RP_ERDY;

		/* close this channel again */
		BCLR(tp->state,ST_ENADUP);
	}

	tp->send = tp->ucntl = 0;
	tp->lptr = tp->lsz = tp->ocol = 0;
	tp->escstate = 0;

	if (!tp->ccnt) {
		if (!tp->pgrp) {
			tp->state = 0;
			tp->tracelvl = 0;
			tp->pgrp = GetPGRP();
			tp->pid = GetPID();
			tp->histbuf = AllocHist();
			tp->histptr = tp->curhistptr = 0;
		}
		/* on first open, reset semaphores */
		if (tp->crsel) ResetSEV(tp->crsel);
		if (tp->srsel) ResetSEV(tp->srsel);
		if (tp->cxsel) ResetSEV(tp->cxsel);
	}

	BSET(tp->state,ST_COPEN);
	BCLR(tp->state,ST_PTMSLOCK);
	tp->ccnt++;
	return RPDONE;
}

/*
 * Close controller. Invalidates all R/W of slave part.
 */
int ptc_close(RP rp,int dev)
{
	TTY tp = ttys[dev];

	assert_nomon(rp);

	tp->ccnt--;
	if (tp->ccnt < 0) tp->ccnt = 0;
	if (!tp->ccnt) {
		tp->state &= ST_SOPEN;	/* only keep ST_SOPEN */
		tp->pgrp = tp->pid = 0;
		FreeHist(tp->histbuf);
		tp->histbuf = 0;

		/* on last close, post semaphores */
		selraise(tp->crsel,ST_ARMRSEL,&tp->state);
		selraise(tp->cxsel,ST_ARMXSEL,&tp->state);
		selraise(tp->srsel,ST_ARMRSELS,&tp->state);
	}
	return RPDONE;
}

int ptc_ndr(RP rp,int dev)
{
	TTY tp = ttys[dev];
	CQ qp = &tp->outq;
	int ret = RPDONE;
	ULONG state = tp->state;

	qrequest(tp->dev);

	/* don't read anything unless slave is open */
	if (ISCLR(state,ST_SOPEN))
		ret = RPBUSY|RPDONE;

	/* packet mode */
	else if (ISSET(state, PF_PKT) && tp->send)
		RWNBUF(rp) = tp->send; 

	/* user control mode */
	else if (ISSET(state, PF_UCNTL) && tp->ucntl)
		RWNBUF(rp) = tp->ucntl;

	else if (qempty(qp))
		ret = RPBUSY|RPDONE;
	else
		RWNBUF(rp) = (UCHAR)getc(qp,PEEK);

	qrelease(tp->dev);
	return ret;
}

int ptc_read(RP rp,int dev)
{
	TTY tp = ttys[dev];
	RPQ rpq = &tp->crq;
	RPQ orpq = &tp->swq;
	CQ qp = &tp->outq;
	UCHAR far *bp;
	int nb;
	int cnt, c;
	ULONG state = tp->state;

	/* return 0 if someone wanted to read 0 bytes */
	if (RWCNT(rp)==0) {
		RWCNT(rp) = 0;
		traceeof(tp,TR_MASTER|TR_READ);
		return RPDONE;
	}

	/* block until data becomes available. If NDELAY is set,
	 * go directly to reader, may return 0 bytes
	 * if no writer, return what is there */
	if (ISSET(state,ST_SOPEN) &&
	    ISCLR(state,ST_NDELAY) && block_read(rpq, rp, qp)) {
		traceflush(tp);
		return RPDONE;
	}

	/* when we arrive here, it is either direct or dequeued entry in rp
	 * and there is some data (or not)
	 */

	/* get buffer address */
	if (P2V(RWBUF(rp),RWCNT(rp),&bp) != 0)
		return RP_EGEN;

	nb = RWCNT(rp);
	RWCNT(rp) = 0;
	cnt = 0;

	qrequest(tp->dev);

	/* packet mode */
	if (ISSET(state, PF_PKT) && tp->send) {
		traceput(tp, tp->send, TR_MASTER|TR_READ);
		bp[cnt++] = tp->send;
		nb--;
		if (tp->send & TIOCPKT_IOCTL) {
			int i,cc;
			cc = nb < sizeof(tp->termios) ? nb : sizeof(tp->termios);
			for (i=0; i<cc; i++,cnt++) {
				c = ((UCHAR far*)&tp->termios)[i];
				traceput(tp, c, TR_MASTER|TR_READ);
				bp[i+1] = (char)c;
			}
		}
		RWCNT(rp) = cnt;
		tp->send = 0;
		qrelease(tp->dev);
		traceflush(tp);
		return RPDONE;
	}
	/* user control mode */
	if (ISSET(state, PF_UCNTL) && tp->ucntl) {
		bp[0] = tp->ucntl;
		traceput(tp, tp->ucntl, TR_MASTER|TR_READ);
		RWCNT(rp) = 1;
		tp->ucntl = 0;
		qrelease(tp->dev);
		traceflush(tp);
		return RPDONE;
	}

	/* not stopped and data? */
	if (qsize(qp) == 0 || ISSET(state,ST_STOPPED)) {
		traceflush(tp);
		qrelease(tp->dev);
		return RPDONE;
	}

	/* have data */
	if (ISSET(state,PF_PKT) || ISSET(state,PF_UCNTL)) {
		bp[cnt++] = 0;
		traceput(tp, 0, TR_MASTER|TR_READ);
		nb--;
	}

	while (nb != 0) {
		c =getc(qp,NOPEEK);
		if (c < 0) break;
		bp[cnt++] = (char)c;
		traceput(tp, c, TR_MASTER|TR_READ);
		nb--;
	}
	traceflush(tp);
	qrelease(tp->dev);

	/* resume a thread that is waiting for drain output */
	if (ISSET(state,ST_DRAIN) && qsize(qp)==0) {
		BCLR(tp->state,ST_DRAIN);
		Run(&tp->outq);
	}

	/* run other related threads */
	run_thread(qp, rpq, orpq);

	RWCNT(rp) = cnt;
	return RPDONE;
}

int ptc_write(RP rp,int dev)
{
	TTY tp = ttys[dev];
	CQ qp = &tp->inq;
	RPQ rpq = &tp->cwq;
	RPQ orpq = &tp->srq;
	UCHAR far *bp;
	int nb, cnt, c;

	/* don't write anything unless slave is open
	 * also if someone wants to write 0 bytes, then write 0 bytes
	 */
	if (ISCLR(tp->state,ST_SOPEN) || RWCNT(rp)==0) {
		traceeof(tp,TR_MASTER|TR_WRITE);
		RWCNT(rp) = 0;
		return RPDONE;
	}

	nb = RWCNT(rp);
	RWCNT(rp) = 0;
	cnt = 0;

	if (!rpqempty(rpq)) goto block_on_write;
resume:
	/* rp points to the current request */

	/* get buffer address */
	if (P2V(RWBUF(rp),nb+cnt,&bp) != 0)
		return RP_EGEN;

	/* is there space in queue? */
	qrequest(tp->dev);
	if (!qfull(qp)) {
		/* yes, put as much as possible */
		
		/* remote mode */
		if (ISSET(tp->state, PF_REMOTE)) {

			/* no new data until the partner has consumed
			 * everything 
			 */
			if (!qempty(qp)) {
				qrelease(tp->dev);
				return RPDONE;
			}

			while (nb != 0 && qsize(qp) < CQ_HALF) {
				c = bp[cnt];
				traceput(tp,c,TR_MASTER|TR_WRITE);
				putc(c,qp);	/* cannot fail */
				cnt++; nb--;
			}
			putc(0,qp); cnt++;

			/* if the slave is away, return 0 bytes */
			if (ISCLR(tp->state,ST_SOPEN)) {
				RWCNT(rp) = 0;
				traceeof(tp,TR_MASTER|TR_WRITE);
			} else {
				RWCNT(rp) = cnt;
				traceflush(tp);
			}
			qrelease(tp->dev);
			return RPDONE;
		}

		/* not remote mode, normal processing */
		while (nb != 0) {
			c = bp[cnt];
			if (ttinput(tp,c) < 0)
				break;
			cnt++; nb--;
		}
		RWCNT(rp) = cnt;
	}
	traceflush(tp);
	qrelease(tp->dev);

	/* we arrive here no matter whether we sent anything or not */

block_on_write:
	/* is the listener still there and not everything transferred? */
	if (ISSET(tp->state, ST_SOPEN) && nb != 0) {
		if (cnt) {
			tp->nbsave2 = nb;

			/* handle select */
			pts_select(tp);

			/* if written anything, start reader */
			run_thread(qp,orpq,rpq);
		}

		/* we block here, unless ST_NDELAY is set:
		 * prepare to get "0 bytes sent" then, this is not EOF
		 */
		if (ISCLR(tp->state,ST_NDELAY) && !block_write(rp,rpq)) {
			cnt = RWCNT(rp);
			nb = cnt ? tp->nbsave2 : 0;
			goto resume;
		}
	}

	/* arrive here if we have finished this packet
	 * or if the slave is away, or non-blocking
	 */
	if (ISCLR(tp->state,ST_SOPEN)) {
		RWCNT(rp) = 0;
		traceeof(tp,TR_MASTER|TR_WRITE);
	}

	/* handle select */
	pts_select(tp);

	/* run more related threads */
	run_thread(qp, orpq, rpq);

	return RPDONE;
}

int ptc_shutdown(int dev)
{
#if 0
/* not aurora*/
	TTY tp = ttys[dev];

	/* if semaphores allocated, close them */
	if (tp->crsel) CloseSEV(tp->crsel);
	if (tp->srsel) CloseSEV(tp->srsel);
	if (tp->cxsel) CloseSEV(tp->cxsel);
#endif
	return RPDONE;
}

int ptc_sinput(int dev)
{
	TTY tp = ttys[dev];
	CQ qp = &tp->outq;
	if (qempty(qp))
		return RPDONE|RPBUSY;
	else
		return RPDONE;
}

int ptc_soutput(int dev)
{
	TTY tp = ttys[dev];

	if (qsize(&tp->inq) > CQ_HIWATER)
		return RPDONE|RPBUSY;
	else
		return RPDONE;
}

int ptc_select(TTY tp)
{
	int ret = RPDONE;

	/* something to read? */
	if (qsize(&tp->outq) > 0) {
		ret = selraise(tp->crsel,ST_ARMRSEL,&tp->state);
		if (ret != RPDONE) return ret;
	}
		
	if ((ISSET(tp->state,PF_PKT && tp->send) ||
	    (ISSET(tp->state,PF_UCNTL) && tp->ucntl)))
		ret = selraise(tp->cxsel,ST_ARMXSEL,&tp->state);
	 
	return ret;
}

int ptcstrategy(RP rp,short dev)
{
	if (dev==32) return ptmsstrategy(rp);

	if (rp->rp_cmd != CMDInit && rp->rp_cmd != CMDShutdown) int3();

	switch (rp->rp_cmd) {
	case CMDInit:
		return dev==0 ? ptc_init(rp) : RPDONE;
	case CMDNDR:
		return ptc_ndr(rp,dev);
	case CMDINPUT:
		return ptc_read(rp,dev);
	case CMDOUTPUT:
	case CMDOUTPUTV:
		return ptc_write(rp,dev);
	case CMDInputF:
		return ptx_oflush(dev);
	case CMDOutputF:
		return ptx_iflush(dev);
	case CMDOpen:
		return ptc_open(rp,dev);
	case CMDClose:
		return ptc_close(rp,dev);
	case CMDGenIOCTL:
		return ptx_ioctl(rp,dev,1);
	case CMDInputS:
		return ptc_sinput(dev);
	case CMDOutputS:
		return ptc_soutput(dev);
	case CMDShutdown:
		return ptc_shutdown(dev);
	default:
		return RP_EBAD;
	}	
}

#define assert_ibuf(nb) if ((plen && plen<nb) || Verify(pp,nb,0)) return RP_EINVAL;
#define assert_obuf(nb) if ((dlen && dlen<nb) || Verify(dp,nb,1)) return RP_EINVAL;
#define assert_ctrl() if (!sc) return RP_EBAD;
#define assert_slave() if (sc) return RP_EBAD;
#define FPI long far
#define FPU ULONG far

int setxty(char far* dp, int sc, int dev, int dlen)
{
	assert_obuf(14);

	makename(dp,sc,dev);
	return RPDONE;
}

static long bdtbl[] = {
	0,50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400
};

/* This odd code tries to fake a COM device to CMD.EXE, to make Dos*
 * functions think they are talking to a device rather than a file
 */
int com_ioctl_fake(RP rp, void far* pp, void far* dp,int cmd, TTY tp)
{
	char far *cdp = (char*)dp;
	short cflag = tp->t_cflag;
	int dlen = IODLEN(rp);

	rp = rp; pp = pp; dp = dp;

	/* this is rather pragmatic: I found during debugging, that
	 * these ioctls are called, so I implemented them
	 */
	switch(cmd) {
	case 62: /* return device characteristics */
		/* data packet bytes:
		 *	0=databits
		 *	1=parity
		 *	2=stopbits
		 *	3=break flag
		 */
		assert_obuf(4);
		cdp = (char*)dp;
		cdp[0] = (char)(((cflag & CSIZE) >> 2) + 4);
		cdp[1] = (char)((cflag & PARENB) ? ((cflag & PARODD) ? 1 : 2) : 0);
		cdp[2] = (char)((cflag & CSTOPB) ? 2 : 1);
		cdp[3] = (char)0;
		return RPDONE;
	case 63: /* return extended bit rate */
		/* data packet bytes:
		 *	0-4: current
		 *	5-9: min
		 *	10-14: max
		 */
		assert_obuf(15);
		*((long far*)cdp) =
		*((long far*)(cdp+5)) =
		*((long far*)(cdp+10)) = bdtbl[cflag & 0xf];
		cdp[4] = cdp[9] = cdp[14] = 0;
		return RPDONE;
	case 72: /* get comm event word */
		assert_obuf(2);
		/* only bit 0 (received char) and bit 2 (xmit queue empty)
		 * are meaningful
		 */
		*((short*)dp) = (qempty(&tp->outq) ? 2 : 0) |
				(qsize(&tp->inq) ? 1 : 0);
		return RPDONE;
	default:
		return RPDONE;
	}
}

int ptx_ioctl(RP rp, int dev, int sc) /* sc=1: controller */
{
	TTY tp = ttys[dev];
	void far* pp = (void far*)IOPARAM(rp);	
	void far* dp = (void far*)IODATA(rp);
	int plen = IOPLEN(rp);
	int dlen = IODLEN(rp);
	int cat = IOCAT(rp);
	int cmd = IOFUNC(rp);
	ULONG state = tp->state;
	USHORT lflag = tp->t_lflag;
	CQ qp = sc ? &tp->outq : &tp->inq;
	int nread,flag,pid;
	int err = RPDONE;
	int stop;
	UCHAR far *cc = tp->t_cc;

	if (cat == 0x01)
		return com_ioctl_fake(rp,pp,dp,cmd,tp);

	if (cat == XFREE86_USER) {
		if (ISSET(state, PF_UCNTL)) {
			tp->ucntl = (char)cmd;
			return ptc_select(tp);
		} else
			return RP_EBAD;
	}

	if (cat != XFREE86_PTY)
		return RP_EINVAL;

	if (sc && (cmd==XTY_TIOCSETA || cmd==XTY_TIOCSETAW ||
	    cmd==XTY_TIOCSETAF))
		ttflush(tp,2);

	switch (cmd) {
	case XTY_RESERVED0:
	case XTY_RESERVED1:
	case XTY_RESERVED2:
	case XTY_RESERVED3:
	case XTY_RESERVED4:
	case XTY_RESERVED5:
		err = RP_EINVAL;
		break;

	case XTY_TIOCEXT:	/* (ULONG) external processing */
		assert_ibuf(sizeof(ULONG));
		if (*(FPI*)pp) {
			if (ISSET(state,PF_PKT))
				tp->send |= TIOCPKT_IOCTL;
			BSET(tp->t_lflag,EXTPROC);
		} else {
			if (ISSET(lflag, EXTPROC) &&
			    ISSET(state,PF_PKT)) {
				tp->send |= TIOCPKT_IOCTL;
				ptc_select(tp);
			}
			BCLR(tp->t_lflag,EXTPROC);
		}
		break;

	case PTY_TIOCPKT:	/* (ULONG) packet mode */
		assert_ctrl();
		assert_ibuf(sizeof(ULONG));
		if (*(FPI*)pp) {
			if (ISSET(state,PF_UCNTL)) {
				err = RP_EINVAL;
				break;
			}
			BSET(tp->state,PF_PKT);
		} else
			BCLR(tp->state,PF_PKT);
		break;

	case PTY_TIOCUCNTL:	/* (ULONG) user control mode */
		assert_ctrl();
		assert_ibuf(sizeof(ULONG));
		if (*(FPI*)pp) {
			if (ISSET(state,PF_PKT)) {
				err = RP_EINVAL;
				break;
			}
			BSET(tp->state,PF_UCNTL);
		} else
			BCLR(tp->state,PF_UCNTL);
		break;

	case PTY_TIOCREMOTE:	/* (ULONG) remote mode */
		assert_ctrl();
		assert_ibuf(sizeof(ULONG));
		if (*(FPI*)pp)
			BSET(tp->state, PF_REMOTE);
		else
			BCLR(tp->state, PF_REMOTE);
		ttflush(tp,3);
		break;

	case XTY_TIOCSETA:	/* (struct pt_termios) set termios */
	case XTY_TIOCSETAW:	/* (struct pt_termios) drain and set termios */
	case XTY_TIOCSETAF:	/* (struct pt_termios) drain out, flush in, set */
		{
			struct pt_termios far *t = (struct pt_termios far*)pp;
			assert_ibuf(sizeof(struct pt_termios));
			
			if (cmd==XTY_TIOCSETAW || cmd==XTY_TIOCSETAF) {
				ttwait(tp);
				if (cmd==XTY_TIOCSETAF)
					ttflush(tp,2);
			}

			tp->t_iflag = t->c_iflag;
			tp->t_oflag = t->c_oflag;
			tp->t_cflag = t->c_cflag;

			/* make EXTPROC readonly */
			if (ISSET(tp->t_lflag, EXTPROC))
				BSET(t->c_lflag, EXTPROC);
			else
				BCLR(t->c_lflag, EXTPROC);
			tp->t_lflag = t->c_lflag;
			bmove((char far*)t->c_cc,(char far*)tp->t_cc, sizeof(t->c_cc));
			break;
		}
	case PTY_TIOCSIG:	/* (ULONG) generate sig */
		assert_ibuf(sizeof(ULONG));
		err = sendsig((int)*(FPI*)pp);
		if (*(FPI*)pp == SIGINT)
			BSET(tp->state,ST_SIGTTY);
		break;
	case XTY_TCFLUSH:	/* (ULONG) flush buffers */
		assert_ibuf(sizeof(ULONG));
		switch ((int)*(FPI*)pp) {
		case 0: flag = 1; break;
		case 1: flag = 2; break;
		default: flag = 3;
		}
		ttflush(tp,flag);
		break;

	case XTY_TIOCFLUSH:	/* (ULONG) flush buffers */
		assert_ibuf(sizeof(ULONG));
		flag = (int)*(FPI*)pp;
		if (!flag)
			flag = 3;
		else 
			flag &= 3;
		ttflush(tp,flag);
		break;

	case XTY_TIOCCONS:	/* (ULONG) become console */
		assert_ibuf(sizeof(ULONG));
		if (*(FPI*)pp) {
			if (constty && constty != tp &&
			    ISSET(state,ST_COPEN)) {
				err = RP_EUSE;
				break;
			}
/* XXX check rights */	
			constty = tp;
		} else if (constty == tp)
			constty = 0;
		break;

	case XTY_TIOCDRAIN:	/* (-) wait until output drained */
		ttwait(tp);
		break;

	case XTY_TIOCSTART:	/* (-) start output */
		if (ISSET(state, ST_STOPPED) || ISSET(lflag,FLUSHO)) {
			BCLR(tp->t_lflag, FLUSHO);
			BCLR(tp->state, ST_STOPPED);
			ttstart(tp);
		}
		break;

	case XTY_TIOCSTI:	/* (ULONG) simulate input */
		{
			/* XXX check */
			assert_ibuf(sizeof(ULONG));
			ttinput(tp,(int)*(FPI*)pp);
			break;
		}

	case XTY_TIOCSTOP:	/* (-) stop output */
		if (ISCLR(state,ST_STOPPED)) {
			BSET(tp->state, ST_STOPPED);
			ptsstop(tp,0);
		}
		break;

	case XTY_TIOCSPGRP:	/* (ULONG) set process group */
		assert_obuf(sizeof(ULONG));
		if (sc)
			*(FPI*)dp = tp->pgrp;
		else
			err = RP_EINVAL;
		break;

	case XTY_TIOCSWINSZ:	/* (struct winsize) set winsize */
		assert_ibuf(sizeof(struct pt_winsize));
		bmove((char far*)&tp->winsize,(char far*)pp,sizeof(struct pt_winsize));
		break;

	case XTY_TIOCSETC:	/* (struct tchars) set control chars */
		{
			struct pt_tchars far *tc = (struct pt_tchars far *)pp;
			UCHAR far* cc = tp->t_cc;
			assert_ibuf(sizeof(struct pt_tchars));
			cc[VINTR] = (UCHAR)tc->t_intrc;
			cc[VQUIT] = (UCHAR)tc->t_quitc;
			cc[VSTART] = (UCHAR)tc->t_startc;
			cc[VSTOP] = (UCHAR)tc->t_stopc;
			cc[VEOF] = (UCHAR)tc->t_eofc;
			cc[VEOL] = (UCHAR)tc->t_brkc;
		}
		break;

	case TTY_TIOCNOTTY:	/* (-) disassociate from session */
		/* if the process is not session leader, allow giving up
		 * the pty. Here: session leader==owner of this pty (!=Unix)
		 * 
		 */
		pid = GetPID();
		if (pid != tp->pid)
			tp->pgrp = GetPID();
		else
			err = RP_EINVAL;
		break;

	case TTY_TIOCSCTTY:	/* (ULONG) become controlling terminal */
		/* not yet */
		err = RP_EINVAL;
		break;

	case XTY_NAME:		/* return my name (pass 14 byte buffer) */
		err = setxty((char far*)dp,sc,dev,dlen);
		break;

	case XTY_DRVID:
		return drvid(dp,IODLEN(rp),sc ? PTYDRV_ID : TTYDRV_ID);

	case XTY_FIONREAD:	/* (ULONG) get # bytes to read */
		assert_obuf(sizeof(ULONG));
		nread = qsize(qp);
		if (ISCLR(lflag, ICANON)) {
			if (nread < tp->t_cc[VMIN]) nread = 0;
		}
		*(FPI*)dp = nread;
		break;

	case XTY_FIONBIO:	/* set/reset non blocking writes */
		assert_ibuf(sizeof(ULONG));
		if (*(FPI*)pp)
			BSET(tp->state,sc ? ST_NDELAY : ST_NDELAYS);
		else
			BCLR(tp->state,sc ? ST_NDELAY : ST_NDELAYS);
		break;

	case XTY_SELREG:	/* struct pt_sel */
		return sc ? selreg(pp, dp, &tp->crsel, &tp->cxsel) :
			    selreg(pp, dp, &tp->srsel, 0);
	case XTY_SELARM:	/* ulong */
		assert_ibuf(sizeof(ULONG));
		if (sc) { /* ctrlr */
			if (*(FPI*)pp & XTYSEL_ARMRSEL) {
				err = selarm(ST_ARMRSEL,&tp->state,tp->crsel);
				if (err != RPDONE) break;
			} else if (*(FPI*)pp & XTYSEL_ARMXSEL) {
				err = selarm(ST_ARMXSEL,&tp->state,tp->cxsel);
			}
			if (err==RPDONE)
				err = ptc_select(tp);
		} else { /* slave */
			if (*(FPI*)pp & XTYSEL_ARMRSEL) {
				err = selarm(ST_ARMRSELS,&tp->state,tp->srsel);
				if (err==RPDONE)
					err = pts_select(tp);
			}
		}
		break;
	case XTY_TIOCGETA:	/* (struct pt_termios) get termios */
		assert_obuf(sizeof(struct pt_termios));
		bmove((char far*)dp,(char far*)&tp->termios,
		      sizeof(struct pt_termios));
		break;

	case XTY_TIOCGWINSZ:	/* (struct winsize) get winsize */
		assert_obuf(sizeof(struct pt_winsize));
		bmove((char far*)dp,(char far*)&tp->winsize,
		      sizeof(struct pt_winsize));
		break;

	case XTY_TIOCOUTQ:	/* (ULONG) get out queue size */
		assert_obuf(sizeof(ULONG));
		*(FPI*)dp = qsize(&tp->outq);
		break;

	case XTY_TIOCGPGRP:	/* (ULONG) get process group */
		assert_obuf(sizeof(ULONG));
		if (sc)
			*(FPI*)dp = tp->pgrp;
		break;

	case XTY_TIOCGETC:	/* (struct tchars) get control chars */
		{
			struct pt_tchars far *tc = (struct pt_tchars far*)dp;
			char far *cc = (char far*)tp->t_cc;
			assert_obuf(sizeof(struct pt_tchars));
			tc->t_intrc = cc[VINTR];
			tc->t_quitc = cc[VQUIT];
			tc->t_startc= cc[VSTART];
			tc->t_stopc = cc[VSTOP];
			tc->t_eofc = cc[VEOF];
			tc->t_brkc = cc[VEOL];
		}
		break;

	case XTY_SEOFMODE: {
			int p;
			assert_ibuf(sizeof(ULONG));
			p = (int)*(FPI*)pp;
			if (ISSET(p,(ST_EOFSIGB|ST_EOFSIGC)))
				err = RP_EINVAL;
			else {
				p &= 15; p <<= 20;
				BCLR(tp->state,ST_EOF);
				BSET(tp->state,p);
			}
		}
		break;

	case XTY_GEOFMODE:
		assert_obuf(sizeof(ULONG));
		*(FPU*)dp = (tp->state >> 20) & 15;
		break;

	case XTY_ENADUP:
		assert_ibuf(sizeof(ULONG));
		if (*(FPU*)pp)
			BSET(tp->state,ST_ENADUP);
		else
			BCLR(tp->state,ST_ENADUP);
		break;

	case XTY_TRACE:
		assert_ibuf(sizeof(ULONG));
		tp->tracelvl = (char)*(FPU*)pp;
		cons_idc_init(); /* protects itself against multiple calls */
	}

	if (ISSET(tp->t_lflag,EXTPROC) && ISSET(tp->state,PF_PKT)) {
		switch (cmd) {
		case XTY_TIOCSETA:
		case XTY_TIOCSETAW:
		case XTY_TIOCSETAF:
		case XTY_TIOCSETC:
			tp->send |= TIOCPKT_IOCTL;
		}
	}
	stop = (ISSET(tp->t_iflag,IXON) && CCEQ(cc[VSTOP], CTRL('s')) &&
		CCEQ(cc[VSTART], CTRL('q')));
	if (ISSET(state, PF_NOSTOP)) {
		if (stop) {
			BCLR(tp->send,TIOCPKT_NOSTOP);
			BSET(tp->send,TIOCPKT_DOSTOP);
			BCLR(tp->state,PF_NOSTOP);
		}
	} else {
		if (!stop) {
			BCLR(tp->send,TIOCPKT_DOSTOP);
			BSET(tp->send,TIOCPKT_NOSTOP);
			BSET(tp->state,PF_NOSTOP);
		}
	}

	/* did the above cause anything to be put in the buffer? */
	if (err==RPDONE) err = ptc_select(tp);
	return err;
}


int ptms_open(void)
{
	if (!ttybufinit)
		ttybuf_initr0();
	return RPDONE;
}

int ptms_ioctl(RP rp)
{
	void far* pp = (void far*)IOPARAM(rp);
	void far* dp = (void far*)IODATA(rp);
	int plen = IOPLEN(rp);
	int dlen = IODLEN(rp);
	int cat = IOCAT(rp);
	int cmd = IOFUNC(rp);
	int err = RPDONE;
	int i,sc;
	TTY tp;

	if (cat != XFREE86_PTY)
		return RP_EINVAL;

	switch (cmd) {
	case PTMS_GETPTY:
		/* find a usable pty */
		for (i=0; i<32; i++) {
			tp = ttys[i];
			if (!tp->state) break;
		}
		if (i==32) return RP_EBAD;

		assert_ibuf(sizeof(long));
		sc = (int)(*((ULONG far*)pp));

		err = setxty((char far*)dp,sc,i,dlen);

		/* avoid reuse of this pty until opened */
		tp->state |= ST_PTMSLOCK;

		return err;
	case PTMS_NAME:
		return drvname((char far*)dp,dlen,"\\dev\\ptms$\0");
	case PTMS_DRVID:
		return drvid(dp,IODLEN(rp),PTMSDRV_ID);
	default:
		return RP_EBAD;
	}
	/*NOTREACHED*/
}

int ptmsstrategy(RP rp)
{
	if (rp->rp_cmd != CMDInit && rp->rp_cmd != CMDShutdown) int3();

	switch (rp->rp_cmd) {
	case CMDInit:
	case CMDShutdown:
		return RPDONE;
	case CMDINPUT:
	case CMDOUTPUT:
		RWCNT(rp) = 0;
		return RPDONE;
	case CMDNDR:
	case CMDOUTPUTV:
	case CMDInputF:
	case CMDOutputF:
	case CMDInputS:
	case CMDOutputS:
	default:
		return RP_EBAD;
	case CMDOpen:
		return ptms_open();
	case CMDClose:
		return RPDONE;
	case CMDGenIOCTL:
		return ptms_ioctl(rp);
	}	
}
