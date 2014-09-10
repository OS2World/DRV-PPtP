/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */


#include <os2.h>
#include "rp_priv.h"
#include "map_priv.h"
#include "tty_priv.h"
#include "con_priv.h"

extern FPFUNCTION	Device_Help;	/* the device helper routine */

struct mp_tbl	maps[NMAP] = { {0l,0l,0}, };
TTY		ttys[NPTY] = { 0, };
TTY		constty = 0;
int		conpid = 0;
int		conlen = 0;
int		conpos = 0;
int		concnt = 0;
ULONG		constate = 0;
ULONG		conrsel = 0;	
char far*	buf1 = 0;
char far*	buf2 = 0;
char		buf3[CON_SZ] = { 0, };
char far*	conbuf = &buf3[0];

char hexcvt[] ="0123456789abcdef";
int p10[] = { 10000,1000,100,10,1 };
char done_init = 0;
