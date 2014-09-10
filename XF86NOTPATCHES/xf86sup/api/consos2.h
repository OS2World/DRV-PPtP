/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#ifndef _CONSOS2_H_
#define _CONSOS2_H_

/* The driver controlled here provides an emulation for a systemwide
 * console device, similar to the one available in Unix systems.
 *
 * The Console device is normally associated to no output unit, but
 * a process can "grab" the console by opening it and sending it a
 * CONS_TIOCCONS ioctl. After that the process owns the console and
 * any attempt to open it again in order to read from it will give a
 * failure.
 *
 * The owning process may read the console and process the incoming data
 * in any way. Other processes may open the console for writing only.
 * 
 * If the owning process terminates or closes the console, it will
 * be "freed", and any other process may grab it.
 *
 * The console driver is participating in IDC (inter device driver 
 * communication), so certain other device drivers may also interact
 * with the driver and possibly grab the console as well. Currently only
 * TTY/PTY devices know the console, but a specialized com device (serial
 * I/O) might also serve as a default console receiver (i.e. a terminal
 * connected to a serial line; a driver that knows the console device does
 * not exist so far, though).
 *
 * The console device knows all TTY/PTY ioctls, but ignores many of them
 * or returns default values for them.
 * It also accepts the following special ioctls.
 */

/* The device driver category */
#define XFREE86_CONS	0x76	/* 'v' */

/* driver functions */
#define CONS_RESERVED0	0x40	/* don't use */
#define CONS_RESERVED1	0x41	/* don't use */
#define CONS_RESERVED2	0x42	/* don't use */
#define CONS_RESERVED3	0x43	/* don't use */
#define CONS_RESERVED4	0x62	/* don't use */
#define CONS_RESERVED5	0x63	/* don't use */

#define CONS_TIOCCONS	0x4d	/* (ULONG) become console */

#define CONS_SELREG	0x6b	/* (struct cons_sel) register select semaphore */
#define CONS_SELARM	0x6c	/* (ULONG) switch on select single shot */
#define CONS_OWNER	0x6d	/* pass 14 byte buffer. Returns: */
				/* "PIDnnnnn\0" if a process owns it */
				/* "\\dev\\XtyXX\0" if a pty has issued TIOCCONS */
				/* "\\dev\\DEVICEXY\0" if other device redirect */
				/* "\0" if unowned */


/* The following ioctls are common to all XF86SUP devices */
#define CONS_NAME	0x60	/* return my name (pass 14 byte buffer) */
				/* will return "\\dev\\console$\0" */
#define CONS_DRVID	0x61	/* returns struct cons_drvid */

#define CONSDRV_MAGIC	0x36384f58	/* 'XF86' */
#define CONSDRV_ID	3

struct cons_drvid {
	ULONG	magic;
	ULONG	drvtype;
	ULONG	version;
};

/* for select() syscall emulation */

/* for CONS_SELREG */
/* set by driver if there was already a semaphore set (collision),
 * means: use the semaphore you get in this field
 */
#define CONSEL_RSEL	1	/* bit 0: rsel field is modified */

struct cons_sel {
	/* set all fields to 0 to read the current setting */
	/* semaphores may be same for different selectors */
	ULONG	rsel;		/* 32 bit event semaphore for select on read */
	ULONG	reserved;	/* don't use */ 
	ULONG	code;		/* bit field, see above */
};

/* for CONS_SELARM */
#define CONSEL_ARMRSEL	1	/* bit 0: set to single shot post rsel */

#endif
