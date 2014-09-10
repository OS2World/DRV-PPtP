/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#ifndef _CIO_H_
#define _CIO_H_

extern int io_init();
extern int io_exit();

extern char c_inb(short);
extern short c_inw(short);
extern long c_inl(short);
extern void c_outb(short,char);
extern void c_outw(short,short);
extern void c_outl(short,long);

#endif
