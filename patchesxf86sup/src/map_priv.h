/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#include "../api/mapos2.h"

struct mp_tbl {
	ULONG	phys;
	ULONG	vmaddr;
	USHORT	sfnum;
};

#define NMAP 64

extern struct mp_tbl maps[];

extern int map_init(RP);

