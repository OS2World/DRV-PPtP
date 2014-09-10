/* Copyright by Holger.Veit@gmd.de
 *
 * This emulates the System V Release 3.2 interface of opening a pty.
 * This is not a real emulation of a clone device, but it is faster
 * than the standard trial&error method of 4.3BSD proposed in
 * Stevens Unix Network Programming pp 601-602
 */

#define INCL_DOS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#include <os2.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include "ptyos2.h"

int grantpt(int fd)
{
	/* no operation */
	return 0;
}

int unlockpt(int fd)
{
	/* no operation */
	return 0;
}

char* ptcname(int fd)
{
	static char name[14];
	ULONG in = 1,len;

	APIRET rc = DosDevIOCtl(fd,XFREE86_PTY, PTMS_GETPTY,
		(ULONG*)&in, sizeof(ULONG), &len,
		(ULONG*)name, 14, &len);
	return rc ? NULL : name;
}

char* ptsname(int fd)
{
	static char name[14];
	ULONG len;

	APIRET rc = DosDevIOCtl(fd,XFREE86_PTY, XTY_NAME,
		NULL, 0, NULL,
		(ULONG*)name, 14, &len);
	if (rc) return NULL;

	/* returns 01234567890 1 */
	/*         /dev/ptyXX\0  */
	name[5] = 't';
	return name;
}

int pty_master()
{
	int pfd,mfd;
	char *ptcn;

	/* open the cloning device */
	if ( (pfd = open("/dev/ptms$",O_RDWR)) < 0)
		return -1;

	/* ask it for a pty name */
	ptcn = ptcname(pfd);
fprintf(stderr,"master=%s\n",ptcn);
	if (ptcn == NULL)
		return -1;

	/* open this pty */
	mfd = open(ptcn,O_RDWR);

	/* close clone */
	close(pfd);

	return mfd;
}

int pty_slave(int mfd)
{
	char *ptsn = ptsname(mfd);
fprintf(stderr,"slave=%s\n",ptsn);

	if (!ptsn)
		return -1;

	return open(ptsn,O_RDWR);
}
