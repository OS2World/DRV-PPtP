/* Released to the public domain
 * Use this code freely for own developments
 */

/* This example maps the color video memory to an address, and
 * writes "HELLO" directly into the video memory by
 * access via PMAP$.
 * Note that similar functionality as shown here is also available through
 * the Vio* calls and the screen01$ device driver. The slight but important
 * difference between these and PMAP$ is, that PMAP allows ANY PHYSICAL
 * MEMORY to be mapped in the process address space, not just video
 * base memory like the others. It is just that the demonstration is
 * most easily shown with video memory.
 * Handle the feature PMAP$ provides with great care, as improper use may
 * severely damage your system (no kidding!).
 *
 *
 *
 * This program must be compiled with EMX/GCC.
 *
 * THIS PROGRAM WORKS ONLY IN FULL SCREEN MODE ON SYSTEMS WHICH HAVE
 * A VGA COMPATIBLE BOARD IN COLOR MODE
 */

#include <stdio.h>
#define INCL_DOSFILEMGR
#define INCL_DOSDEVIOCTL
#define INCL_ERRORS
#include <os2.h>

#include "mapos2.h"

char pmap_dev[] = "PMAP$";

static USHORT hello[] = {
	0xf248,
	0xe365,
	0xd46c,
	0xc56c,
	0xb66f
};

int main(int argc, char *argv[])
{
	HFILE pmap;
	ULONG action, len;
	APIRET rc;

	struct xf86_pmap_param par;
	struct xf86_pmap_data dta;
	USHORT *video_ptr;
	int i;

	/* open device */
	rc = DosOpen(pmap_dev,&pmap,&action, 0, FILE_NORMAL, FILE_OPEN,
		OPEN_ACCESS_READWRITE|OPEN_SHARE_DENYNONE, (PEAOP2)NULL);
	if (rc != 0) {
		fprintf(stderr, "The PMAP$ Device could not be opened\n");
		fprintf(stderr, "Verify that you have XFREE86.SYS installed\n");
		exit(1);
	}
	/* map device */
	par.u.physaddr = 0xb8000;
	par.size = 2048;
	rc = DosDevIOCtl(pmap, XFREE86_PMAP, PMAP_MAP,
		(ULONG*)&par, sizeof(par), &len,
		(ULONG*)&dta, sizeof(dta), &len);
	if (rc) {
		fprintf(stderr,"Error PMAP_MAP: %d\n", rc);
		DosClose(pmap);
		exit(1);
	}

fprintf(stderr,"Address = %lx\n",dta.addr);

	/* now write a string in the middle of line 12 */
	video_ptr = ((USHORT*)dta.addr)+12*80+35;
	for (i=0; i<5; i++)
		video_ptr[i] = hello[i];

	/* unmap device */
	par.u.physaddr = (ULONG)dta.addr;
	par.size = 0;
	rc = DosDevIOCtl(pmap, XFREE86_PMAP, PMAP_UNMAP,
		(ULONG*)&par, sizeof(par), &len,
		NULL, 0, NULL);

	if (rc) {
		fprintf(stderr,"Error PMAP_UNMAP: %d\n", rc);
		DosClose(pmap);
		exit(1);
	}

	/* Close the device */
	DosClose(pmap);

	exit(0);
}