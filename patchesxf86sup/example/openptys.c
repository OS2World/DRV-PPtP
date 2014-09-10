/* Released to the public domain
 * Use this code freely for own developments
 */

/* This code illustrates both methods supported to open ptys.
 *
 * The first method uses the device /dev/ptms$, which returns the name of 
 * a new pty whenever a PTMS_GETPTY ioctl is called.
 *
 * The second method tries to open ptys, until an open succeeds.
 *
 *
 */

/* enable the following if you have an EMX.DLL which knows how to deal with
 * /dev/pty?? devices correctly
 */
/*#define EMX_KNOWS_PTYS*/

#define INCL_DOS
#define INCL_DOSPROCESS
#include <os2.h>

#include "../api/ptyos2.h"
#include <stdlib.h>
#include <stdio.h>

main()
{
	int i;
	int fdptms;
	int fd1[5], fd2[5];
	int errcode = 1;

	char letter1[] = "pq";
	char letter2[] = "0123456789abcdef";

	int try;

	APIRET rc;
	ULONG in = 1, len;
	char name[14];

	for (i=0; i<5; i++)
		fd1[i] = fd2[i] = -1;

/* first method */

	if ((fdptms=open("/dev/ptms$",0)) <0) {
		fprintf(stderr,"cannot open /dev/ptms$\n");
		goto cleanup;
	}

	for (i=0; i<5; i++) {
		rc = DosDevIOCtl(fdptms, XFREE86_PTY, PTMS_GETPTY,
				 &in, sizeof(ULONG), &len,
				 &name, sizeof(name), &len);
		if (rc) {
			fprintf(stderr,"Error PTMS_GETPTY: rc=%d\n",rc);
			goto cleanup;
		}
		fd1[i] = open(name,0);
		if (fd1[i]==0) {
			fprintf(stderr,"Cannot open pty %s\n",name);
			goto cleanup;
		}
		printf("successfully opened %s, handle =%d\n",name,fd1[i]);
	}

	close (fdptms);

/* to mix things up a bit: */
	close(fd1[3]); fprintf(stderr,"Closing handle %d\n",fd1[3]); fd1[3] = 0; 
	close(fd1[0]); fprintf(stderr,"Closing handle %d\n",fd1[0]); fd1[0] = 0;




/* second method */

	for (i=0; i<5; i++) {
		HFILE fd;
		for (try = 0; try<32; try++) {
			ULONG action;
			sprintf(name, "/dev/pty%c%c",
				try>15 ? letter1[1] : letter1[0],
				letter2[try & 15] );

#ifndef EMX_KNOWS_PTYS
			/* currently EMX open does not set the critical
			 * flag OPEN_FLAGS_FAIL_ON_ERROR, so I need to
			 * bypass open() for the demonstration of the
			 * effect.
			 * What I would need is something like:
			 * #define O_DEVICE 0x2000
			 * fd = open(name,O_RDWR|O_DEVICE)
			 * (of course O_DEVICE passed through in os2\fileio.c
			 */
			if (!DosOpen(name,&fd,&action, 0,
			    FILE_NORMAL,FILE_OPEN,
			    OPEN_FLAGS_FAIL_ON_ERROR |
			      OPEN_SHARE_DENYNONE |
			      OPEN_ACCESS_READWRITE,
			    (PEAOP2)NULL))
				break;
#else
			if (fd = open(name,1))
				break;
#endif
		}
		if (try==32) {
			fprintf(stderr,"No more PTYs\n");
			goto cleanup;
		} else {
			fd2[i] = fd;
			printf("successfully opened %s, handle = %d\n",
				name, fd);
		}
	}

	errcode = 0;

	/* cleanup all */
cleanup:
	for (i=0; i<5; i++) {
		if (fd1[i]>=0) close(fd1[i]);
		if (fd2[i]>=0) close(fd2[i]);
	}

	exit(errcode);
}
