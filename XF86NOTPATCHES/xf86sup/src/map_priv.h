/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#ifndef _MAP_PRIV_H_
#define _MAP_PRIV_H_

#include "../api/mapos2.h"

struct mp_tbl {
	ULONG	phys;
	ULONG	vmaddr;
	USHORT	sfnum;
        ULONG   lockhan;
};

#define NMAP 128

extern struct mp_tbl maps[];

extern int map_init(RP);

#endif
