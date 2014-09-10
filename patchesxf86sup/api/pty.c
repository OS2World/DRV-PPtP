/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de)
 * Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#define INCL_DOS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#include <os2.h>

#include <sys/termio.h>
#include "pty.h"
#include "ptyos2.h"

static int x_ioctl(HFILE fd, int func)
{
	APIRET rc = DosDevIOCtl(fd,XFREE86_PTY, func,
		NULL, 0, NULL,
		NULL, 0, NULL);
	return rc ? -1 : 0;
}

static int u_ioctl(HFILE fd, ULONG data)
{
	APIRET rc = DosDevIOCtl(fd,XFREE86_USER, data,
		NULL, 0, NULL,
		NULL, 0, NULL);
	return rc ? -1 : 0;
}

static int o_ioctl(HFILE fd, int func, void *data, ULONG sz)
{
	ULONG len;

	APIRET rc = DosDevIOCtl(fd,XFREE86_PTY, func,
		NULL, 0, NULL,
		(ULONG*)data, sz, &len);
	return rc ? -1 : 0;
}

static int i_ioctl(HFILE fd, int func, void *data, ULONG sz)
{
	ULONG len;

	APIRET rc = DosDevIOCtl(fd,XFREE86_PTY, func,
		(ULONG*)data, sz, &len,
		NULL, 0, NULL);
	return (rc) ? -1 : 0;
}


#define CTRL(x) (x&037)
#define _POSIX_VDISABLE ((UCHAR)'\377')
UCHAR	ttydefchars[] = {
	CTRL('c'), 034, 0177, CTRL('u'), CTRL('d'),
	_POSIX_VDISABLE, 1, 0, CTRL('z'), CTRL('s'),
	CTRL('q')
};

int ptioctl(int fd, int func, void* data)
{
	struct pt_termios pt;
	struct termio *t;
	int ret,i;

	int cmd = func & 0xff;
	if (func & 0x100)
		func = 0x100;

	switch (func) {
	case 0x100:
		return u_ioctl(fd,cmd);

	case TCGETA:
		ret = o_ioctl(fd,XTY_TIOCGETA,&pt,sizeof(struct pt_termios));
		if (ret) return -1;
		t = (struct termio*)data;
		t->c_iflag = pt.c_iflag;
		t->c_oflag = pt.c_oflag;
		t->c_cflag = pt.c_cflag;
		t->c_lflag = pt.c_lflag;		

		/* attn: this strips the last 3 chars */
		for (i=0; i<NCC; i++)
			t->c_cc[i] = pt.c_cc[i];
		return 0;
	case TCSETA:
	case TCSETAW:
	case TCSETAF:
		t = (struct termio*)data;
		pt.c_iflag = t->c_iflag;
		pt.c_oflag = t->c_oflag;
		pt.c_cflag = t->c_cflag;
		pt.c_lflag = t->c_lflag;

		/* attn: strips the last three chars that are possible */
		for (i=0; i<NCC; i++)
			pt.c_cc[i] = t->c_cc[i];
		if (func==TCSETA) 
			i = XTY_TIOCSETA;
		else if (func==TCSETAW) 
			i = XTY_TIOCSETAW;
		else
			i = XTY_TIOCSETAF;
		return i_ioctl(fd,i,&pt,sizeof(struct pt_termios));
	case TCFLSH:
		return i_ioctl(fd,XTY_TCFLUSH,data,sizeof(ULONG));
	case TCSBRK:
		x_ioctl(fd,XTY_TIOCDRAIN);
		return 0;
	case TCXONC:
		return 0;
	case TIOCEXT:
		return i_ioctl(fd,XTY_TIOCEXT,data,sizeof(ULONG));
	case TIOCPKT:
		return i_ioctl(fd,PTY_TIOCPKT,data,sizeof(ULONG));
	case TIOCUCNTL:
		return i_ioctl(fd,PTY_TIOCUCNTL,data,sizeof(ULONG));
	case TIOCREMOTE:
		return i_ioctl(fd,PTY_TIOCREMOTE,data,sizeof(ULONG));
	case TIOCSETA:
		return i_ioctl(fd,XTY_TIOCSETA,&pt,sizeof(struct pt_termios));
	case TIOCSETAW:
		return i_ioctl(fd,XTY_TIOCSETAW,&pt,sizeof(struct pt_termios));
	case TIOCSETAF:
		return i_ioctl(fd,XTY_TIOCSETAF,&pt,sizeof(struct pt_termios));
	case TIOCGETA:
		return o_ioctl(fd,XTY_TIOCGETA,&pt,sizeof(struct pt_termios));
	case TIOCFLUSH:
		return i_ioctl(fd,XTY_TIOCFLUSH,data,sizeof(ULONG));
	case TIOCCONS:
		return i_ioctl(fd,XTY_TIOCCONS,data,sizeof(ULONG));
	case TIOCDRAIN:
		return x_ioctl(fd,XTY_TIOCDRAIN);
	case TIOCSTART:
		return x_ioctl(fd,XTY_TIOCSTART);
	case TIOCSTI:
		return i_ioctl(fd,XTY_TIOCSTI,data,sizeof(ULONG));
	case TIOCSTOP:
		return x_ioctl(fd,XTY_TIOCSTOP);
	case TIOCSWINSZ:
		return i_ioctl(fd,XTY_TIOCSWINSZ,data,sizeof(struct winsize));
	case TIOCNAME:
		return o_ioctl(fd,XTY_NAME,data,14);
	case TIOCGWINSZ:
		return o_ioctl(fd,XTY_TIOCGWINSZ,data,sizeof(struct winsize));
	case TIOCOUTQ:
		return o_ioctl(fd,XTY_TIOCSTI,data,sizeof(ULONG));
	case TIOCSETC:
		return i_ioctl(fd,XTY_TIOCSETC,data,sizeof(struct tchars));
	case TIOCGETC:
		return o_ioctl(fd,XTY_TIOCGETC,data,sizeof(struct tchars));
	case I_PUSH:
		return 0;
	default:
		return -1;
	}	
}
