/* Released to the public domain
 * Use this code freely for own developments
 */

/* This example is a simple receiver process for the /dev/console device.
 * It creates an OS/2 window which will display messages that are sent to
 * the /dev/console window.
 * Run this application with 'start consvr'.
 * Afterwards run one of the applications 'conexam' or 'ptyexam -l' to
 * test the console device.
 *
 * In order to keep the example most universal, this example program uses
 * the Dos* API rather than standard C library functions, which would be
 * also possible.
 */

#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR
#include <os2.h>
#include <stdio.h>
#include <ctype.h>
#include "../api/consos2.h"

int main(int argc, char *argv[])
{
	char buf[200];
	APIRET rc;
	ULONG action,arg,argsz, nread;
	HFILE fd;

	/* open device */
	rc = DosOpen((PSZ)"\\dev\\console$", &fd, &action, 0,
		FILE_NORMAL, FILE_OPEN, 
		OPEN_ACCESS_READWRITE|OPEN_SHARE_DENYNONE, (PEAOP2)NULL);
	if (rc) {
		fprintf(stderr,
			"!!! Cannot open console device, rc=%d\n",
			rc);
		fputs("Probably the XF86SUP device driver was not properly installed.\n",
			stderr);
		exit(1);
	}

	/* grab the console */
	arg = 1;
	rc = DosDevIOCtl(fd, XFREE86_CONS, CONS_TIOCCONS,
		&arg, sizeof(arg),&argsz,
		NULL, 0, NULL);
	if (rc) {
		fprintf(stderr,"!!! Cannot grab console, status = %d\n",rc);
		fputs("Possibly another process currently owns the console\n",
			stderr);
		DosClose(fd);
		exit(2);
	}

	rc = DosDevIOCtl(fd, XFREE86_CONS, CONS_OWNER,
		NULL, 0, NULL,
		(PULONG)&buf, sizeof(buf),&argsz);
	if (rc) {
		fprintf(stderr,"!!! Cannot get console owner, status = %d\n",rc);
		fputs("Possibly another process currently owns the console\n",
			stderr);
		DosClose(fd);
		exit(2);
	}
	printf("Owner of console is: %s\n",buf);

	for(;;) {
		/* read some chars */
		memset(buf,0,200);
		rc = DosRead(fd,(PVOID)buf,199,&nread);
		if (rc) {
			fprintf(stderr,
				"!!! Read from console returned status=%d\n",
				rc);
			break;
		}
		if (nread) {
			fputs(buf,stdout);
			if (nread < 199) {
				/* wait some time */
				DosSleep(50); /* 50 msec */
			}
		}
	}
	DosClose(fd);
	exit(0);
	return 0;
}
