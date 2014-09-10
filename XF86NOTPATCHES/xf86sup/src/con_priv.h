/* Copyright (C) 1995, 1996 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#ifndef _CON_PRIV_H_
#define _CON_PRIV_H_

#include "../api/consos2.h"
#include "../api/conidc.h"

#define CON_SZ 16384
extern int conlen;
extern int conpos;
extern char far *conbuf;
extern int conpid;
extern int concnt;
extern ULONG conrsel;
extern ULONG constate;

#endif
