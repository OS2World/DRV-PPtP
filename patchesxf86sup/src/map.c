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
#include "..\api\conidc.h"
#include "map_priv.h"
#include "util.h"

/*
 * find_map:
 *   search for the entry point in the mapping table that belongs to the
 *   same file id and has the same 'vmaddr', return index or -1
 */
static int find_map(USHORT sf, ULONG vmaddr)
{
	int i;
	for (i=0; i<NMAP; i++) {
	if (maps[i].sfnum == sf && maps[i].vmaddr == vmaddr)
		return i;
	}
	return -1;
}

/*
 * find_empty:
 *   find an empty slot in the mapping table, return index or -1
 */
static int find_empty(void)
{
	int i;
	for (i=0; i<NMAP; i++) {
		if (maps[i].sfnum==-1 && maps[i].vmaddr==0)
			return i;
	}
	return -1;
}

typedef struct xf86_pmap_param PARAM;
typedef struct xf86_pmap_data DATA;
typedef struct xf86_pmap_readrom ROMPARAM;

/*
 * map_ioctl:
 *   process the supported ioctls
 */
static int map_ioctl(RP rp)
{
	PARAM far *p = (PARAM far*)IOPARAM(rp);
	DATA far *d;
	int idx;
	ULONG addr;

	/* only accept class XFREE86_PMAP */
	if (IOCAT(rp) != XFREE86_PMAP)
		return RP_EBAD;

	switch (IOFUNC(rp)) {
	case PMAP_MAP:
		/* test: Parameter packet is readable and has correct size
		 *       get a free slot
		 */
		if (Verify(p,sizeof(PARAM),0)==0 &&
		    (idx = find_empty()) != -1) {

			/* map the requested region */
			addr = Pmap(p->u.physaddr,p->size);

			/* was okay? */
			if (addr) {

				/* get the return packet of ioctl */
				d = (DATA far*)IODATA(rp);

				/* writable ? */
				if (Verify(d,sizeof(DATA),1)==0) {

					/* set the mapping address in
					 * process address space
					 */
					d->addr = addr;
					d->sel = 0;	

					/* registrate stuff in mapping table */
					maps[idx].phys = p->u.physaddr;
					maps[idx].vmaddr = addr;
					maps[idx].sfnum = IOSFN(rp);
					return RPDONE;
				}
			}
		}
		/* come here for error condition */
		break;
	case PMAP_UNMAP:
		/* test: parameter packet readable and correct size
		 *       entry found in table
		 *       unmapping okay?
		 */
		if (Verify(p,sizeof(PARAM),0)==0 &&
		    (idx = find_map(IOSFN(rp),p->u.vmaddr)) != -1 &&
		    Punmap(p->u.vmaddr)==0) {

			/* remove entry from table */
			maps[idx].phys = 0;
			maps[idx].vmaddr = 0;
			maps[idx].sfnum = -1;
			return RPDONE;
		}
		/* arrive here for error condition */
		break;
	case PMAP_NAME:
		if (Verify(d,14,1) == 0) {
			bmove((char far*)d,"\\dev\\pmap$\0",11);
			return RPDONE;
		}
		break;
	case PMAP_DRVID:
		return drvid(d,IODLEN(rp),PMAPDRV_ID);
	case PMAP_READROM:	/* testcfg.sys replacement function */
		if (Verify(p,sizeof(ROMPARAM),0) == 0) {
			ROMPARAM far *p1 = (ROMPARAM far *)p;
			USHORT n = p1->numbytes;
			ULONG ad = p1->physaddr + n;
			if (p1->command == 0 &&
			    p1->physaddr >= 0xc0000 &&
			    p1->physaddr <= 0xfffff &&
			    ad >= 0xc0000 &&
			    ad <= 0xfffff) {
				char far *d1 = (char far*)IODATA(rp);
				if (Verify(d1,n,1) == 0) {
					char far *addr;
					if (P2V(p1->physaddr,n,&addr)==0) {
						bmove(d1,addr,n);
						/*UnPhysToVirt == NOOP in OS/2 2.x */
						return RPDONE;
					}
				}
			}
		}
		break;
	default:
		return RP_EBAD;
	}

	/* break from case */
	return RP_EINVAL;
}

/*
 * map_close:
 *   unmap all areas that were occupied by process
 */
static int map_close(RP rp)
{
	/* get file id */
	USHORT sfnum = rp->pk.opncls.sfnum;
	int i;

	for (i=0; i<NMAP; i++) {
		/* unmap the entry with this file id */
		if (maps[i].sfnum == sfnum) {
			Punmap(maps[i].vmaddr);

			/* and clear the slot */
			maps[i].sfnum = -1;
			maps[i].phys =
			maps[i].vmaddr = 0;
		}
	}
	return RPDONE;
}

/*
 * map_shutdown:
 *   unmap everything
 */
static int map_shutdown(void)
{
	int i;
	for (i=0; i<NMAP; i++) {
		if (maps[i].sfnum != -1) {
			Punmap(maps[i].vmaddr);
			maps[i].sfnum = -1;
			maps[i].phys =
			maps[i].vmaddr = 0;
		}
	}
	return RPDONE;
}

/* map_open: just send a warning to console */
int map_open(void)
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
	bmove(buf+26," is accessing /dev/pmap$\r\n\0",27);

	pkt.buf = buf;
	pkt.buflen = 52;

	cons_idc_init();
	cons_idc_send(&pkt);

	return RPDONE;
}

/*
 * map_strategy:
 *   open always succeeds, I/O always fails, other functions
 *   perform their function
 */
int mapstrategy(RP rp)
{
	switch (rp->rp_cmd) {
	case CMDInit:
		return map_init(rp);
	case CMDClose:
		return map_close(rp);
	case CMDOpen:
		return map_open();
	case CMDShutdown:
		return map_shutdown();
	case CMDGenIOCTL:
		return map_ioctl(rp);
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
