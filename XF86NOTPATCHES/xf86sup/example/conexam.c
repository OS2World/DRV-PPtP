/* Released to the public domain
 * Use this code freely for own developments
 */

/* This example is a simple sender process that writes to /dev/console.
 * Run this application with 'start conexam'.
 *
 * It will periodically send messages to /dev/console. If there is a
 * reader listening on /dev/console, it can process (usually by printing
 * them) these messages.
 */

#define INCL_DOSPROCESS
#include <os2.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	int fd,i,nwr,rc;
	char buf[200];
	PTIB ptib;
	PPIB ppib;
	PID pid;

	if ((fd=open("/dev/console$",1)) < 0) {
		fputs("!!! Cannot open console device.\n",
			stderr);
		fputs("\tPossibly the XF86SUP device driver is not properly installed.\n",
			stderr);
		exit(1);
	}

	rc = DosGetInfoBlocks(&ptib,&ppib);
	pid = ppib->pib_ulpid;	

	for(i=0; i<200; i++) {
		sprintf(buf,"CONEXAM PID=%d Message ID=%d\n",pid,i);
		nwr = write(fd,buf,strlen(buf));
		if (nwr < 0) {
			fprintf(stderr,
				"!!! Write to console returned status=%d\n",
					nwr);
			break;
		} 
	}
	close(fd);
	exit(0);
	return 0;
}
