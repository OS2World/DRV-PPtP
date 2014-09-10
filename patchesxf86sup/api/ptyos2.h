/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#ifndef _PTYOS2_H_
#define _PTYOS2_H_

/* This driver provides an emulation for the tty/pty mechanism
 * which is available in Unix systems.
 * The emulation is neither complete nor 1-to-1 (although it
 * shares certain code with the BSD tty code); providing a full
 * emulation is left to application software.
 *
 * THE DRIVER WAS SPECIFICALLY CREATED FOR SUPPORT OF XFREE86/OS2.
 *
 * What is a pty?
 * ==============
 * A pty or "pseudo-tty" is actually a bidirectional pipe (but there are
 * some properties that cannot be done by a UNIX or OS/2 pipe, not even
 * with the extended functionality of OS/2 pipes).
 *
 * A pty consists of two devices, the master device PTYXX and the slave
 * device TTYXX, where XX is p[0-f] or q[0-f], e.g. ptyp6 and ttyp6
 * belong together.
 *
 * To make a pty connection, you have to open a master PTY first.
 * Since the UNIX PTY mechanism does not have a standard method of
 * providing the next free pty, you are forced to try all of them
 * in a loop.
 * After you found and opened a free pty master, the corresponding TTY 
 * (slave) can be opened. Its name is simply the first letter of device
 * name changed from 'p' to 't'.
 *
 * From this time on, master and slave can exchange (both directions)
 * data through this channel.
 *
 * If the slave is closed, all data sent from the master will sent to nirvana,
 * and a read will return READ_FAULT. Some other slave, however, can connect 
 * to the master, as long as it is open.
 *
 * If the master is closed, the slave will receive READ_FAULT on an attempt
 * to read and WRITE_FAULT on any attempt to write.
 *
 * What does this pty do, what a pipe can't do?
 * ============================================
 *
 * 1. The pty/tty is unnamed, and you can only connect to it if you know
 *    the PTY-token - and even then, only one process can connect.
 *    In contrast to a named pipe there is no worrying about creation,
 *    server connecting/disconnecting and instances. The client can
 *    decide which pty to connect to, provided it knows its token.
 * 2. The pty/tty is a tty, and thus does processing of several bits in
 *    struct termios which can be submitted or read by an ioctl.
 * 3. The tty/pty knows the CONSOLE device and can grab console output by
 *    the equivalent of the UNIX TIOCCONS ioctl.
 */

/* All functions with PTY_ only apply to the master part of the tty/pty,
 * all functions with TTY_ only to the slave part, functions for both
 * are labeled XTY_.
 */

/* The device driver category */
#define XFREE86_PTY	0x76	/* 'v' */
#define XFREE86_USER	0x75	/* 'u' */
#define XFREE86_TTY	0x76	/* 'v' */

/* The device driver functions */
#define XTY_RESERVED0	0x40
#define XTY_RESERVED1	0x41
#define XTY_RESERVED2	0x42
#define XTY_RESERVED3	0x43

#define XTY_TIOCEXT	0x44	/* (ULONG) external processing */
#define PTY_TIOCPKT	0x45	/* (ULONG) packet mode */
#define		TIOCPKT_DATA		0x00	/* data packet */
#define		TIOCPKT_FLUSHREAD	0x01	/* flush packet */
#define		TIOCPKT_FLUSHWRITE	0x02	/* flush packet */
#define		TIOCPKT_STOP		0x04	/* stop output */
#define		TIOCPKT_START		0x08	/* start output */
#define		TIOCPKT_NOSTOP		0x10	/* no more ^S, ^Q */
#define		TIOCPKT_DOSTOP		0x20	/* now do ^S ^Q */
#define		TIOCPKT_IOCTL		0x40	/* state change of pty driver */
#define PTY_TIOCUCNTL	0x46	/* (ULONG) user control mode */
#define PTY_TIOCREMOTE	0x47	/* (ULONG) remote mode */

#define XTY_TIOCSETA	0x48	/* (struct pt_termios) set termios */
#define XTY_TIOCSETAW	0x49	/* (struct pt_termios) drain and set termios */
#define XTY_TIOCSETAF	0x4a	/* (struct pt_termios) drain out, flush in, set */
#define PTY_TIOCSIG	0x4b	/* (ULONG) generate sig */
#define XTY_TIOCFLUSH	0x4c	/* (ULONG) flush buffers */
#define XTY_TIOCCONS	0x4d	/* (ULONG) become console */
#define XTY_TIOCDRAIN	0x4e	/* (-) wait until output drained */
#define XTY_TIOCSTART	0x4f	/* (-) start output */
#define XTY_TIOCSTI	0x50	/* (ULONG) simulate input */
#define XTY_TIOCSTOP	0x51	/* (-) stop output */
#define XTY_TIOCSPGRP	0x52	/* (ULONG) set process group */
#define XTY_TIOCSWINSZ	0x53	/* (struct pt_winsize) set winsize */
#define XTY_TIOCSETC	0x54	/* (struct pt_tchars) set control chars */

#define TTY_TIOCNOTTY	0x56	/* (-) disassociate from session */
#define TTY_TIOCSCTTY	0x57	/* (ULONG) become controlling terminal, NOOP */
#define XTY_TCFLUSH	0x58	/* SYSV (ULONG) flush buffers */
#define XTY_SEOFMODE	0x59	/* (ULONG) specify behaviour on EOF */
#define		EOFMODE_NSIG	0x00	/* don't generate a signal on EOF */
#define		EOFMODE_SIGB	0x01	/* send SIGBREAK on EOF */
#define		EOFMODE_SIGC	0x02	/* send SIGINT on EOF */
#define		EOFMODE_NULL	0x00	/* return 0 bytes */
#define		EOFMODE_CTLZ	0x04	/* return ^Z on EOF */
#define		EOFMODE_CTLC	0x08	/* return ^C on EOF */
#define		EOFMODE_CTLD	0x0c	/* return ^D on EOF */
#define XTY_ENADUP	0x5a	/* (ULONG) prepare a dup operation */

#define XTY_NAME	0x60	/* return my name (pass 14 byte buffer) */
				/* will return "\\dev\\XtyXX\0" */
#define XTY_DRVID	0x61	/* (struct pt_drvid) returns a driver magic */
				/* common to all XF86SUP devices */

#define XTY_RESERVED4	0x62
#define XTY_RESERVED5	0x63

#define XTY_FIONREAD	0x64	/* (ULONG) get # bytes to read */
#define XTY_TIOCGETA	0x65	/* (struct pt_termios) get termios */
#define XTY_TIOCGWINSZ	0x66	/* (struct pt_winsize) get winsize */
#define XTY_TIOCOUTQ	0x67	/* (ULONG) get out queue size */
#define XTY_TIOCGPGRP	0x68	/* (ULONG) get process group */
#define XTY_TIOCGETC	0x69	/* (struct pt_tchars) get control chars */
#define XTY_FIONBIO	0x6a	/* (ULONG) set writes non blocking */
#define XTY_SELREG	0x6b	/* (struct pt_sel) register select semaphores */
#define XTY_SELARM	0x6c	/* (ULONG) switch on select single shot */
#define XTY_RESERVED6	0x6d	/* --- reserved for console --- */
#define XTY_GEOFMODE	0x6e	/* (ULONG) get EOF mode */

/* Special functions only for /dev/ptms driver */
#define PTMS_NAME	XTY_NAME	/* see XTY_NAME */
#define PTMS_DRVID	XTY_DRVID	/* see XTY_DRVID */
#define PTMS_GETPTY	0x64	/* return a usable pty device */
				/* pass 14 byte buffer */


#ifndef _TERMIOS_H

#ifndef __EMX__
#define VINTR		0	/* ISIG, EMX */
#define VQUIT		1	/* ISIG, EMX */
#define	VERASE		2	/* ICANON, EMX */
#define VKILL		3	/* ICANON, EMX */
#define	VEOF		4	/* ICANON, EMX */
#define	VEOL		5	/* ICANON, EMX */
#define VMIN		6	/* !ICANON, EMX */
#define VTIME		7	/* !ICANON, EMX */
#define VSUSP		8	/* ISIG, EMX */
#define VSTOP		9	/* IXON, IXOFF, EMX */
#define VSTART		10	/* IXON, IXOFF, EMX */


#endif /* !__EMX__ */

#ifndef NCCS
#define NCCS	11
#endif

#ifndef __EMX__
/* supported things in c_iflag */
#if !defined (IGNBRK)
#define IGNBRK		0x0001
#define BRKINT		0x0002
#define IGNPAR		0x0004
#define PARMRK		0x0008
#define INPCK		0x0010
#define ISTRIP		0x0020
#define INLCR		0x0040
#define IGNCR		0x0080
#define ICRNL		0x0100
#define IXON		0x0400
#define IXOFF		0x1000
#if !defined (_POSIX_SOURCE)
#define IUCLC		0x0200
#define IXANY		0x0800
#endif /* !_POSIX_SOURCE_ */
#endif /* !IGNBRK */
#endif /* !__EMX__ */

/* supported things in c_oflag */
#if !defined (OPOST)
#define OPOST		0x0001
#ifndef _POSIX_SOURCE
#define ONLCR		0x0004
#define OXTABS		0x0040
#define ONOEOT		0x0080
#define OLCUC		0x0002
#define OCRNL		0x0008
#define ONLRET		0x0020	/* not yet */
#define ONOCR		0x0010	/* not yet */
#endif /* !_POSIX_SOURCE */
#endif /* !OPOST */

/* supported things in c_lflag */
#ifndef ISIG
#define	ISIG		0x0001	/* enable signals INTR, QUIT, [D]SUSP */
#define	ICANON		0x0002	/* canonicalize input lines */
#define XCASE		0x0004
#define ECHO		0x0008	/* enable echoing */
#define	ECHOE		0x0010	/* visually erase chars */
#define	ECHOK		0x0020	/* echo NL after line kill */
#define	ECHONL		0x0040	/* echo NL even if ECHO is off */
#ifndef _POSIX_SOURCE
#define ECHOCTL  	0x0100	/* echo control chars as ^(Char) */
#endif  /* !_POSIX_SOURCE */
#define TOSTOP		0x0200	/* not yet */
#define EXTPROC		0x0400
#define NOFLSH		0x0080
#ifndef _POSIX_SOURCE
#define FLUSHO		0x1000
#endif /* !_POSIX_SOURCE */
#endif /* ISIG */

/* Default flags in c_cflag, all ignored */
#ifndef __EMX__
#if !defined (B0)               /* Symbols common to termio.h and termios.h */
#define B0		0x0000
#define B50		0x0001
#define B75		0x0002
#define B110		0x0003
#define B134		0x0004
#define B150		0x0005
#define B200		0x0006
#define B300		0x0007
#define B600		0x0008
#define B1200		0x0009
#define B1800		0x000a
#define B2400		0x000b
#define B4800		0x000c
#define B9600		0x000d
#define B19200		0x000e
#define B38400		0x000f
#define CSIZE		0x0030
#define CS5		0x0000
#define CS6		0x0010
#define CS7		0x0020
#define CS8		0x0030
#define CSTOPB		0x0040
#define CREAD		0x0080
#define PARENB		0x0100
#define PARODD		0x0200
#define HUPCL		0x0400
#define CLOCAL		0x0800
#define LOBLK		0x1000
#endif /* !B0 */

#ifndef SIGINT
#define SIGINT 2	/*EMX*/
#endif
#ifndef SIGBREAK
#define SIGBREAK 21	/*EMX*/
#endif

#endif /* !__EMX__ */

#endif /* TERMIOS_H */

struct pt_termios
{
	unsigned short	c_iflag;
	unsigned short	c_oflag;
	unsigned short	c_cflag;
	unsigned short	c_lflag;
	unsigned char	c_cc[NCCS];
	long		_reserved_[4];
};

struct pt_winsize {
	unsigned short	ws_row;		/* rows, in characters */
	unsigned short	ws_col;		/* columns, in characters */
	unsigned short	ws_xpixel;	/* horizontal size, pixels */
	unsigned short	ws_ypixel;	/* vertical size, pixels */
};

struct pt_tchars {
	char	t_intrc;
	char	t_quitc;
	char	t_startc;
	char	t_stopc;
	char	t_eofc;
	char	t_brkc;
};

/* common to all XF86SUP devices */
#define XTYDRV_MAGIC	0x36384f58	/* 'XF86' */
#define PTYDRV_ID	1
#define TTYDRV_ID	2
#define PTMSDRV_ID	6

struct pt_drvid {
	ULONG	magic;
	ULONG	drvtype;
	ULONG	version;
};

/* for select() syscall emulation */

/* for XTY_SELREG */
/* set by driver if there was already a semaphore set (collision),
 * means: use the semaphore you get in this field
 * Note only /dev/pty* (controller) accepts xsel, others ignore it
 */
#define XTYSEL_RSEL	1	/* bit 0: rsel field is modified */
#define XTYSEL_XSEL	2	/* bit 1: xsel field is modified */

struct pt_sel {
	/* set all fields to 0 to read the current setting */
	/* semaphores may be same for different selectors */
	ULONG	rsel;	/* 32 bit event semaphore for select on read */
	ULONG	xsel;	/* 32 bit event semaphore for select on exception */ 
	ULONG	code;	/* bit field, see above */
};

/* for XTY_SELARM */
#define XTYSEL_ARMRSEL	1	/* bit 0: set to single shot post rsel */
#define XTYSEL_ARMXSEL	2	/* bit 1: set to single shot post xsel (ctrlr only) */

#endif /* _PTYOS2_H_ */
