/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de)
 * Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */
#ifndef _PTY_H_
#define _PTY_H_

/* must include <os2.h> and <termio.h> first */

#ifdef __cplusplus
extern "C" {
#endif

/* Warning: this header defines a EMX compatibility interface, by using
 * certain declarations from EMX header files. Some functions of the
 * driver are superior to functions offered by EMX; they are mapped down
 * to EMX data structures (struct termios) and thus might lose special
 * functionality.
 */

#if !defined (TCGETA)
#define TCGETA      1
#define TCSETA      2
#define TCSETAW     3
#define TCSETAF     4
#define TCFLSH      5
#define TCSBRK      6
#define TCXONC      7
#endif

/* some flags not in EMX */
#ifndef OXTABS
#define OXTABS	0x40
#define ONOEOT	0x80
#define ECHOCTL	0x100
#define EXTPROC	0x400
#define FLUSHO	0x1000
#endif

#define TIOCEXT		100
#define TIOCPKT		101
#ifndef TIOCPKT_DATA
#define		TIOCPKT_DATA		0x00	/* data packet */
#define		TIOCPKT_FLUSHREAD	0x01	/* flush packet */
#define		TIOCPKT_FLUSHWRITE	0x02	/* flush packet */
#define		TIOCPKT_STOP		0x04	/* stop output */
#define		TIOCPKT_START		0x08	/* start output */
#define		TIOCPKT_NOSTOP		0x10	/* no more ^S, ^Q */
#define		TIOCPKT_DOSTOP		0x20	/* now do ^S ^Q */
#define		TIOCPKT_IOCTL		0x40	/* state change of pty driver */
#endif
#define TIOCUCNTL	102
#define 	UIOCCMD(n)	(0x100+n)
#define TIOCREMOTE	103
#define TIOCSETA	104
#define TIOCSETAW	105
#define TIOCSETAF	106
#define TIOCFLUSH	107
#define TIOCCONS	108
#define TIOCDRAIN	109
#define TIOCSTART	110
#define TIOCSTI		111
#define TIOCSTOP	112
#define TIOCSWINSZ	113
#define TIOCSETC	114
#define TIOCNAME	115	/* extension */
#define TIOCGETA	116
#define TIOCGWINSZ	117
#define TIOCOUTQ	118
#define TIOCGETC	120
#define I_PUSH		121	/* SYSV 3.2 compat, no operation */

struct winsize {
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel;
	unsigned short ws_ypixel;
};

struct tchars {
	char t_intrc;
	char t_quitc;
	char t_startc;
	char t_stopc;
	char t_eofc;
	char t_brkc;
};

int ptioctl(int fd, int func, void* data);

#ifdef __cplusplus
}
#endif
#endif /* _PTY_H_ */
