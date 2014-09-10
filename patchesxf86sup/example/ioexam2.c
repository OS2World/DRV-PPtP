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

void thread1(void*);
void ioplthread2(void*);

int main(int argc,char*argv[])
{
	int rc;
	int i;

	/* warn the user */
	fprintf(stderr, "This example will read out VGA registers for demonstration\n");
	fprintf(stderr, "of I/O threads.\n");
	fprintf(stderr, "If you don't have a VGA or SVGA compatible adapter\n");
	fprintf(stderr, "or are not sure, press CTRL-C now. Otherwise\n");
	fprintf(stderr, "proceed with RETURN\n");
	getchar();

	fprintf(stderr, "The example will create three threads, the second of\n");
	fprintf(stderr, "which gains I/O privilege via the fastio$ driver\n");
	fprintf(stderr, "The others just check whether they have IOPL right\n");
	fprintf(stderr, "and suspend. Eventually, thread 1 will try I/O\n");
	fprintf(stderr, "without having I/O privilege and crash the program\n");
	fprintf(stderr, "This is correct. RETURN to proceed\n");
	getchar();

	_beginthread(thread1, NULL, 16384, NULL);
	_beginthread(ioplthread2, NULL, 16384, NULL);

	while (1) {
		fprintf(stderr,"thread 3: PSW=%04X\n",psw());
		sleep(1);
	}
	return 0;
}

void thread1(void* dummy)
{
	int i;
	for (i=0; i<10; i++) {
		fprintf(stderr,"thread 1(%d): PSW=%04X\n",i,psw());
		sleep(1);
	}

	fprintf(stderr,"thread 1(%d): Next activation of thread1 will intentionally cause\n",i);
	fprintf(stderr,"thread 1(%d): a core dump:\n",i); fflush(stderr);
	sleep(1); /* schedule others again */

	fprintf(stderr,"thread 1(%d): PSW=%04X read port=%02X\n",
		i,psw(),c_inb1(0x3cc));
	exit(1);
}

void ioplthread2(void *dummy)
{
	int val;

	while (1) {
		if (io_init1() != 0) {
			fprintf(stderr,"thread 3: cannot raise IOPL\n");
			exit(1);
		}

		fprintf(stderr,"thread 2: raising iopl=3 PSW=%04X\n",psw());
		sleep(1);
		val = c_inb1(0x3cc);	/* do some I/O, causing core if */
					/* thread has no right*/
		fprintf(stderr,"thread 2: read port 0x3cc=%02X, PSW=%04X\n",
			val, psw());
		sleep(1);
		if (io_exit1() != 0) {
			fprintf(stderr,"thread 3: cannot reset IOPL\n");
			exit(1);
		}
		fprintf(stderr,"thread 2: resetting iopl=2 PSW=%04X\n",psw());
		sleep(1);
	}
}
