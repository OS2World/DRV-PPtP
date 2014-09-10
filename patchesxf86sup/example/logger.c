/* Released to the public domain
 * Use this code freely for own developments
 */

/* This program may serve as a receiver for ptyexam. Its only purpose is
 * to echo everything that is received from stdin to stdout. I used it
 * to debug the pty driver before cmd.exe worked.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/fcntl.h>

void hexwrite(FILE *fd, char *buf, int n)
{
	int i;
	fprintf(fd,"Got:\n");
	for (i=0; i<n; i++)
		fprintf(fd," %02X",buf[i]);
	fprintf(fd,"\n");
	fflush(fd);
}

main()
{
	char buf[100];
	int nread,rc;
	FILE *fd = fopen("log.dbg","w");
	if (!fd) {
		fprintf(stderr,"Cannot open log.dbg\n");
		exit(1);
	}

	fprintf(fd,"Log opened, %d\n",isatty(0)); fflush(fd);
/*	fcntl(0,F_SETFL,O_BINARY);*/
	fcntl(0,F_SETFL,O_NDELAY);

	for(;;) {
		memset(buf,0,100);
#ifdef WORKING_CHARREAD
		nread = read(0,buf, 10);
#else
		rc = DosRead(0,buf,sizeof(buf),&nread);
		if (rc) {
			fprintf(fd,"rc=%d\n",rc);
			break;
		}
#endif
		if (buf[0]=='x' && buf[1]=='y') break;
		if (nread<0) {
			fprintf(fd,"nread = %d\n",nread); fflush(fd);
			break;
		}
		if (nread) {
			hexwrite(fd,buf,nread);
			write(1,buf,nread);
		}
	}
	fprintf(fd,"Log closed\n"); fflush(fd);
	fclose(fd);
	exit(0);
}