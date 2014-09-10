/* Released to the public domain
 * Use this code freely for own developments
 */

/* This example was inspired from Stevens' book 'Unix Network Programming'
 * and is a simple implementation of the 'script' utility.
 * See a complete description in file PTYEXAM.C.
 *
 * This example differs from the original PTYEXAM.C in that it uses a
 * select() like mechanism to wait for data becoming available at the 
 * CTRLR read side. This is actually a test for a full implementation of
 * select() in a future version of EMX. See routine select_read.
 *
 * This program uses fork() and therefore requires EMX.
 */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVIOCTL
#define INCL_DOSSEMAPHORES

#include <os2.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <termio.h>
#include <sys/types.h>
#include <sys/fcntl.h>

#include "pty_util.h"
#include "../api/ptyos2.h"

int master_fd = -1;
int slave_fd = -1;
FILE *log = NULL;
static HEV rsem = 0;

#define BUF_SZ	512

void leave(int ex)
{
	if (log) fclose(log);
	if (slave_fd >= 0) close(slave_fd);
	if (rsem) DosCloseEventSem(rsem);
	exit(ex);
}

void error(char *msg)
{
	fprintf(stderr,"ERROR: %s\n",msg);
	leave(4);
}

static int sigeof = 0;

int sig_term()
{
}

int sig_chld()
{
	sigeof = 1;
}

void logwrite(FILE *fd, char *buf, int n, char *str)
{
	int i;
	fprintf(fd,"%s\n",str);
	for (i=0; i<n; i++)
		fprintf(fd,"%c",buf[i]);
	fprintf(fd,"\n");
	fflush(fd);
}

int select_read(int fd)
{
	struct pt_sel sel;
	ULONG len;
	ULONG on;
	APIRET rc;

	/* if no semaphore: get it from the driver */
	if (!rsem) {
		sel.rsel = sel.xsel = sel.code = 0;
		rc = DosDevIOCtl(fd, XFREE86_PTY, XTY_SELREG,
			(PULONG)&sel, sizeof(sel), &len,
			(PULONG)&sel, sizeof(sel), &len);
		if (rc) return -1;

		/* register it for future */
		rsem = sel.rsel;
		if (rsem == 0) return -1;

		/* open the semaphore */
		rc = DosOpenEventSem(NULL,&rsem);
		if (rc) return -1;
	}

	/* arm the select mechanism */
	on = XTYSEL_ARMRSEL;
	rc = DosDevIOCtl(fd, XFREE86_PTY, XTY_SELARM,
		&on, sizeof(ULONG), &len,
		NULL, 0, NULL);
	if (rc) return -1;

	/* wait until driver wakes process up again */
	rc = DosWaitEventSem(rsem, 60000);	/* one minute timeout */
	switch (rc) {
	case ERROR_TIMEOUT:
		logwrite(log, NULL, 0,"timeout select"); fflush(log);
		break;
	case 0:
		break;
	default:
		return -1;
	}

	/* was ok */
	return 0;
}

void pass_all(int fd, int childpid)
{
	int newpid, nread;
	char buf[BUF_SZ+1];

	if ((newpid=fork()) < 0) {
		error("parent1 fork error");
	} else if (newpid==0) {
		/* child: stdin -> fd */
		for (;;) {
			nread = read(0,buf,BUF_SZ);
			if (nread <= 0) {
				break;
			}
			if (writen(fd,buf, nread) != nread)
				error("writen error to pty");
			logwrite(log,buf,nread,"stdin->proc:"); fflush(log);
		}
		kill(getppid(), SIGTERM);
		leave(0);
	}
	/* parent: fd -> stdout */

	sleep(1);
	
	signal(SIGTERM, sig_term);

	for (;;) {
		memset(buf,0,BUF_SZ);

		/* this will suspend I/O via a semaphore until data is avail. */
		if (select_read(fd) < 0)
			break;

		/* in the original program, read will block in the kernel,
		 * until data seen. The driver will capture the read request,
		 * thus suspending the whole thread.
		 */
		if ((nread=read(fd,buf,BUF_SZ)) <= 0)
			break;

		if (nread) {
			if (write(1, buf, nread) != nread)
				error("write error to stdout");
			logwrite(log,buf,nread,"proc->stdout:"); fflush(log);
		}

		if (sigeof) break;

	}

	kill(newpid,SIGTERM);

	return;
}

void exec_shell(int fd, char *cmd)
{
	close(0);
	close(1);
	close(2);
	if (dup(fd) != 0 || dup(fd) != 1 || dup(fd) != 2) {
		error("unable to duplicate stdio in child process");
	}
	close(fd);

	execl(cmd,cmd,NULL);

	/* should not return */
	error("exec failed");
}

static struct termio tty_mode;


int tty_raw(int fd)
{
	struct termio t,t1;
	if (ptioctl(fd, TCGETA, (char*)&t) < 0) {
		error("TCGETA failed");
		return -1;
	}

	tty_mode = t;

	t.c_iflag = IGNCR;
	t.c_oflag = OPOST|ONOCR;
	t.c_lflag &= ~(ISIG|ICANON|ECHO|XCASE);
	t.c_cflag &= ~(CSIZE|PARENB);
	t.c_cflag |= CS8;
	t.c_cc[VMIN] = 1;
	t.c_cc[VTIME] = 1;

	if (ptioctl(fd, TCSETA, (char*)&t) < 0) {
		error("TCSETA failed");
		return -1;
	}

	return 0;
}

int tty_reset(int fd)
{
	if (ptioctl(fd, TCSETA, (char*)&tty_mode) < 0)
		return -1;

	return 0;
}

int main(int argc, char *argv[])
{
	int shellpid, cs;

	if (argc != 3) {
		fprintf(stderr,"Usage: %s logfile cmd\n",argv[0]);
		leave(1);
	}

	if (!isatty(0) || !isatty(1))
		error("stdin and stdout mustn't be redirected");

	if ((log = fopen(argv[1],"w")) == NULL)
		error("cannot open logfile");

	/* open a master pty */
	master_fd = pty_master();
	if (master_fd < 0)
		error("cannot open pty master");

	/* switch the pty to bypass */
/*	if (tty_raw(master_fd) < 0)
		error("cannot set PTY to bypass");
*/
	if ((shellpid=fork()) < 0)
		error("fork failed");
	else if (shellpid==0) {
		/* in child */
		slave_fd = pty_slave(master_fd);
		if (slave_fd < 0)
			error("cannot open pty slave");
		close(master_fd);

		exec_shell(slave_fd,argv[2]);
	}

	/* hope we get a signal if the shell is dead */
	signal(SIGCHLD,sig_chld);

	/* parent */
	pass_all(master_fd,shellpid);

	/* when we return here, ensure that the other side is killed */
	kill(SIGKILL,shellpid);

	leave(0);

}
