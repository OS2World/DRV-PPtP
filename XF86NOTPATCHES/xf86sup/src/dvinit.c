/* Copyright (C) 1995,1996 by Holger Veit (Holger.Veit@gmd.de) */
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
char once_msg[] = "\r\nCopyright (C)1995, 1996 by Holger.Veit@gmd.de\r\n";
char dbg_msg[] = "WARNING: DEBUG MODE ENABLED\r\n";
/*char fp17_msg[] = "WARNING: Kernel is not FP17 or later; some functions unavailable\r\n";*/

#ifdef INTRO
char map_msg[] = "\tMem mapper OK (/dev/pmap$)\r\n";
char io_msg[] = "\tIO access device OK (/dev/fastio$)\r\n";
char con_msg[] = "\tConsole mux OK (/dev/console)\r\n";
char pty_msg[] = "\t32 PTYs OK (/dev/[pt]ty[pq][0-f])\r\n";
#endif

extern char done_init;
extern void DiscardProc(void);
extern char dbgflag;
extern USHORT kversion;

/* ALL DATA DECLARED AFTER THIS LINE WILL BE DISCARDED */
char DiscardData = 0;

void read_args(char far* cmd)
{
	register char far* p;

	dbgflag = 0;
	for (p = cmd; *p && *p != ' '; p++);

	for (; *p && *p==' '; p++);
	while (*p) {
		/* don't enable this devil */
		if (*p== '/') {
			if (p[1]=='6' && p[2]=='6' && *++p=='6')
				dbgflag = 1;
			else if (p[1]=='H' || p[1]=='h') {
				histbufsz = atoi(&p[2]);
				if (histbufsz & 0x8000)
					histbufsz = 32767;
				else if (histbufsz < 80 && histbufsz != 0) 
					histbufsz = 80;
			}
		} 
		p++;
	}
}

int init_once(RP rp)
{
#if 0
	HFILE kfd;
	USHORT rc,action,nread;
	ULONG np;
	char rbuf[100];
#endif

	switch (done_init) {
	case 2:	/* fatal error */
		return 2;
	case 0:	/* must init */
		Device_Help = rp->pk.init.devhlp;

		/* read cmd line params */
		read_args(rp->pk.init.args);

		done_init = 1;	

#if 0
		/* check the version of the kernel */
		rc = DosOpen("\\os2krnl",&kfd,&action,0,0,0x01,0x33c0,0);
		if (rc==0) {
			rc = DosChgFilePtr(kfd,0l,2,&np);
			np -= 100l;
			rc = DosChgFilePtr(kfd,np,0,&np);
			if (rc==0) {
				rc = DosRead(kfd,rbuf,100,&nread);
				if (rc==0) {
					int i;
					for (i=0; i<nread; i++) {
						if (rbuf[i]=='@') {
							if (rbuf[i+1]=='#' &&
							    rbuf[i+2]=='I' &&
							    rbuf[i+3]=='B' &&
							    rbuf[i+4]=='M') {
								kversion = (rbuf[i+6] & 0xf) <<4;
								kversion |= (rbuf[i+8] & 0xf);
								kversion <<= 4;
								kversion |= (rbuf[i+9] & 0xf);
								kversion <<= 4;
								kversion |= (rbuf[i+10] & 0xf);
								break;
							}
						}
					}

				}
			}
			DosClose(kfd);
		}
#endif

		DosPutMessage(1,sizeof(version_msg)-1,version_msg);
		DosPutMessage(1,sizeof(once_msg)-1,once_msg);
		if (dbgflag) {
			DosPutMessage(1,sizeof(dbg_msg)-1,dbg_msg);
			int3();
		}

#if 0
		if (kversion < 0x8241) {
			DosPutMessage(1,sizeof(fp17_msg)-1,fp17_msg);
		}
#endif
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
		maps[i].vmaddr = 
                maps[i].lockhan = 0;
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
	return RPDONE;
}

int pts_init(void) 
{
	return RPDONE;
}

void DiscardProc(void) {}

