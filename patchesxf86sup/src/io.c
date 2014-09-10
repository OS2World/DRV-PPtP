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
#include "io_priv.h"
#include "..\api\conidc.h"
#include "util.h"
#include <stdio.h>

extern int io_init(RP);
extern void io_call(void);

/*
 * io_ioctl:
 *   process the supported ioctls
 */
static int io_ioctl(RP rp)
{
	void far *d = IODATA(rp);
	int dlen = IODLEN(rp);  

	/* only accept class XFREE86_IO */
	if (IOCAT(rp) != XFREE86_IO)
		return RP_EINVAL;

	switch (IOFUNC(rp)) {
	case IO_GETSEL32:
		if (Verify(d,sizeof(USHORT),1) == 0 &&
		    (!dlen || dlen==sizeof(USHORT)) &&
		    io_gdt32) {
			*(USHORT far*)d = io_gdt32;
			return RPDONE;
		}
		break;
	case IO_NAME:
		return drvname(d, dlen, "\\dev\\fastio$\0\0");
	case IO_DRVID:
		return drvid(d, dlen, IODRV_ID);
	default:
		return RP_EBAD;
	}

	/* break from case */
	return RP_EINVAL;
}

/*
 * Open routine
 */
int io_open(void)
{
	char buf[60];
	struct conidcpkt pkt;
	int pid;

	/* notify the console of this attempt */
	bmove(buf,"Warning: process PID=00000\0",27);
	/* 00000 at char pos 21 */
	pid = GetPID();
	i2ascii(buf+21,pid);
	/* end is at char pos 26 */
	bmove(buf+26," is accessing /dev/fastio$\r\n\0",29);

	pkt.buf = buf;
	pkt.buflen = 54;

	cons_idc_init();
	cons_idc_send(&pkt);

	return acquire_gdt() ? RPDONE : RP_EGEN;
}

/*
 * io_strategy:
 *   open, close always succeed, I/O always fail, ioctl performs a function
 */
int iostrategy(RP rp)
{
	switch (rp->rp_cmd) {
	case CMDInit:
		return io_init(rp);

	case CMDOpen:
		return io_open();

	case CMDClose:
	case CMDShutdown:
		return RPDONE;

	case CMDGenIOCTL:
		return io_ioctl(rp);

	case CMDINPUT:
	case CMDOUTPUT:
		RWCNT(rp) = 0;
		return RPDONE;

	case CMDOUTPUTV:
	case CMDInputF:
	case CMDOutputF:
	case CMDInputS:
	case CMDOutputS:
	default:
		return RP_EBAD;
	}	
}
