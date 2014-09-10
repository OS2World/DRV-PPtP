
/* Released to the public domain
 * Use this code freely for own developments
 */

/* This program uses the new fastio$ device for 32 bit I/O access.
 * It is assumed to be faster than the traditional method of using
 * a 16 bit IOPL DLL, and BTW, it does not need such a DLL
 *
 */

#include <stdio.h>
#define INCL_DOSFILEMGR
#define INCL_DOSDEVIOCTL
#define INCL_ERRORS
#include <os2.h>
#include <stdio.h>

#include "cio2.h"

int main(int argc,char*argv[])
{
	int rc;
	int i;
	int base, port;
	unsigned int regs[19];

	/* initialize the I/O once */

	printf("diagnostic before IOINIT: PSW=%04X\n", psw());

	rc = io_init1();

	printf("diagnostic after IOINIT: PSW=%04X\n", psw());

	if (rc != 0) {
		fprintf(stderr,"Error %d calling io_init\n",rc);
		exit(1);
	}

	/* warn the user */
	fprintf(stderr, "This example will read out the VGA CRT registers\n");
	fprintf(stderr, "If you don't have a VGA or SVGA compatible adapter\n");
	fprintf(stderr, "or are not sure, press CTRL-C now. Otherwise\n");
	fprintf(stderr, "proceed with RETURN\n");
	getchar();

	printf("diagnostic: PSW=%04X\n", psw());

	/* This checks whether the VGA card is in mono or color mode */
	base = c_inb1(0x3cc) & 1;
	printf("\n\nThe VGA card is in %s mode\n", base ? "COLOR" : "MONO");

	/* depending on the mode, the registers are a address 0x3d4 or 0x3b4 */
	port = base ? 0x3d4 : 0x3b4;

	/* read the CRT registers */
	for (i=0; i<0x19; i++) {
		c_outb1(port, i);		/* address the index register */
		regs[i] = c_inb1(port+1) & 0xff;	/* read the data register */
	}

	/* print the result */
	for (i=0; i<0x19; i++)
		printf("CRT Register %2d = 0x%02X\n",i,regs[i]);

	exit(0);
}
