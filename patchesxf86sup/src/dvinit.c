/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

/* uncomment if you want one info line for each driver installed */
/*#define INTRO*/

#define INCL_ERRORS
#define INCL_DOS
#include <os2.h>
#include <devcmd.h>
#include <string.h>
#include "rp_priv.h"
#include "dh_priv.h"
#include "map_priv.h"
#include "tty_priv.h"
#include "con_priv.h"
#include "io_priv.h"
#include "version.h"

extern FPFUNCTION Device_Help;

char version_msg[] = VERSION;
char once_msg[] = "\r\nCopyright (C)1995 by Holger.Veit@gmd.de\r\n";
char dbg_msg[] = "\tWARNING: DEBUG MODE ENABLED\r\n";

#ifdef INTRO
char map_msg[] = "\tMem mapper OK (/dev/pmap$)\r\n";
char io_msg[]  = "\tIO access device OK (/dev/fastio$)\r\n";
char con_msg[] = "\tConsole mux OK (/dev/console)\r\n";
char pty_msg[] = "\t32 PTYs OK (/dev/[pt]ty[pq][0-f])\r\n";
#endif

extern char done_init;
extern void DiscardProc(void);
extern char far* buf1;
extern char far* buf2;
extern char dbgflag;

/* ALL DATA DECLARED AFTER THIS LINE WILL BE DISCARDED */
char DiscardData = 0;

void read_args(char far* cmd)
{
	register char far* p;

	dbgflag = 0;
	for (p = cmd; *p && *p != ' '; p++);
	for (; *p==' '; p++);

	/* don't enable this devil */
	if (*p== '/' && *++p=='6' && *++p=='6' && *++p=='6')
		dbgflag = 1;
}

int init_once(RP rp)
{
	switch (done_init) {
	case 2:	/* fatal error */
		return 2;
	case 0:	/* must init */
		Device_Help = rp->pk.init.devhlp;

		/* read cmd line params */
		read_args(rp->pk.init.args);

		done_init = 1;	

		/* allocate memory */
		buf1 = AllocBuf(32l * sizeof(struct tty));
		buf2 = AllocBuf(32l * sizeof(struct tty));

		if (!buf1 || !buf2)
		{
			/* fatal */
			rp->pk.initexit.cs = 0;
			rp->pk.initexit.ds = 0;
			done_init = 2;
			return 2;
		}

		DosPutMessage(1, sizeof(version_msg) - 1, version_msg);
		DosPutMessage(1, sizeof(once_msg) - 1, once_msg);
		if (dbgflag)
		{
			DosPutMessage(1, sizeof(dbg_msg) - 1, dbg_msg);
			int3();
		}

		/* fall thru */
	case 1: /* ok */
#define GETSEL(ptr) ((USHORT)(((ULONG)((void far*)ptr)>>16)&0xffff))
		SegLimit(GETSEL(DiscardProc),&rp->pk.initexit.cs);
		SegLimit(GETSEL(&DiscardData),&rp->pk.initexit.ds);
	}
	return 1;
}

int map_init(RP rp)
{
	int i;
	if (init_once(rp) == 2) return RP_EGEN;
	for (i=0; i<NMAP; i++) {
		maps[i].sfnum = -1;
		maps[i].phys =
		maps[i].vmaddr = 0;
	}

#ifdef INTRO
	DosPutMessage(1,sizeof(map_msg)-1,map_msg);
#endif
	return RPDONE;
}

int io_init(RP rp)
{
	if (init_once(rp)==2) return RP_EGEN;
	io_gdt32 = 0;

#ifdef INTRO
	DosPutMessage(1,sizeof(io_msg)-1,io_msg);
#endif
	return RPDONE;
}

int con_init(RP rp)
{
	if (init_once(rp)==2) return RP_EGEN;

	conpos = 0;
	conlen = 0;
	conpid = 0;
	constty = 0;
	conrsel = 0;
	constate = 0;

#ifdef INTRO
	DosPutMessage(1,sizeof(con_msg)-1,con_msg);
#endif
	return RPDONE;
}

int ptc_init(RP rp)
{
	int i;
	TTY tp;
	char far *ptr = buf1;

	if (init_once(rp) == 2)
	  return RP_EGEN;
	
	constty = 0;
	
	for (i=0; i<NPTY; i++)
	{
		ptr = i<16 ? buf1 : buf2;
		tp = (TTY) (ptr + ( (i%16) * sizeof(struct tty) ));
		ttys[i]   = tp;
		tp->state = 0;
		tp->send  = tp->ucntl = 0;
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
	}
#ifdef INTRO
	DosPutMessage(1,sizeof(pty_msg)-1,pty_msg);
#endif
	return RPDONE;
}

int pts_init(void)
{
	return RPDONE;
}

void DiscardProc(void) {}
