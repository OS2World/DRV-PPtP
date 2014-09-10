/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#include "../api/consos2.h"

#define CON_SZ 16384
extern int conlen;
extern int conpos;
extern char far *conbuf;
extern int conpid;
extern int concnt;
extern ULONG conrsel;
extern ULONG constate;

struct conidcpkt
{
	char far* buf;
	int buflen;
};
typedef struct conidcpkt far* CONIDC;
