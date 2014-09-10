/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#ifndef _IOOS2_H_
#define _IOOS2_H_

/* This driver provides fast 32 bit access to I/O ports.
 * There is one ioctl available which returns a single 32 bit 
 * GDT selector (call gate) which can be used as an entry point to the 
 * port I/O routines.
 * For speed reasons, NO check is performed in the driver. You can crash
 * the whole system severely by improper application. Also the parameter
 * passing is done via CPU registers rather than a slow stack frame.
 * You probably need an assembler fragment to properly interface to the
 * call.
 */

/* the device driver category */
#define XFREE86_IO	0x76	/* 'v' */

/* available ioctls */
#define IO_RESERVED0	0x40	/* don't use */
#define IO_RESERVED1	0x41	/* don't use */
#define IO_RESERVED2	0x42	/* don't use */
#define IO_RESERVED3	0x43	/* don't use */
#define IO_RESERVED4	0x62	/* don't use */
#define IO_RESERVED5	0x63	/* don't use */

#define IO_GETSEL32	0x64	/* returns an USHORT 32bit call gate selector */

/* This ioctls are common to all XF86SUP devices */
#define IO_NAME		0x60	/* pass 14 byte buffer, returns */
				/* "\\dev\\fastio$\0" */
#define IO_DRVID	0x61	/* return struct io_drvid */



#define IODRV_MAGIC	0x36384f58	/* 'XF86' */
#define IODRV_ID	5

struct io_drvid {
	ULONG	magic;
	ULONG	drvtype;
	ULONG	version;
};

/* The call gate must be entered with an indirect intersegment call;
 * refer to 386/486/Pentium assembly language reference manual.
 *
 * The calling convention is the following:
 * In:
 *    DX = port address
 *    AL,AX,EAX = optional parameter (out port)
 *    BX function code
 * Out:
 *    DX unchanged
 *    AL,AX,EAX = result or unchanged
 *    BX destroyed
 *
 * Functions: (BX)
 *    0  reserved
 *    1  read port byte (DX -> AL)
 *    2  read port word (DX -> AX)
 *    2  read port dword (DX -> EAX)
 *    4  write port byte (DX,AL ->)
 *    5  write port word (DX,AX ->)
 *    6  write port dword (DX,EAX ->)
 *   >6  reserved
 */

#endif
