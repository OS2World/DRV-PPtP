/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#ifndef _CIO2_H_
#define _CIO2_H_

extern int io_init1();
extern int io_exit1();

extern char c_inb1(short);
extern short c_inw1(short);
extern long c_inl1(short);
extern void c_outb1(short,char);
extern void c_outw1(short,short);
extern void c_outl1(short,long);
extern int psw();

#endif
