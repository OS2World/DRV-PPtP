/* Released to the public domain
 * Use this code freely for own developments
 */

/* This example was inspired from Stevens' book 'Unix Network Programming'
 * and is a simple implementation of the 'script' utility.
 * 'Script' runs a specified command in text mode and records all inputs
 * entered by the user and outputs to standard output. Usually it is used
 * together with a cmd.exe interpreter or another text mode program to track
 * what a user does.
 *
 * 'Script' is run with two arguments:
 * 	script logfilename command
 * where 'logfilename' is the file receiving the log
 * and 'command' is the text mode program that is executed.
 * You may redirect the log data to /dev/console if you like (you need to start
 * the 'consvr' utility, which is also an example in this directory, first).
 * Note that you cannot use this program to record PM or WinOS/2 activities,
 * it was just meant for text mode applications. Also applications that
 * use video, keyboard, or mouse API calls might not work.
 * Also for simplicity, no argument may be passed to 'command'. Feel free
 * to add this facility; it is not difficult.
 *
 * FYI: If you believe you can do the same without a pty, but a pipe,
 * read Chp. 15 of Stevens' book for a discussion of this
 * (no, it won't work correctly with a pipe).
 *
 * This program uses fork() and therefore requires EMX
 */

#define INCL_DOS
#include <os2.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <termio.h>
#include <sys/types.h>
#include <sys/fcntl.h>

#include "pty_util.h"
#include "ptyos2.h"

int master_fd = -1;
int slave_fd = -1;
FILE *log = NULL;

#define BUF_SZ	512

void leave(int ex)
{
	if (log) fclose(log);
	if (slave_fd >= 0) close(slave_fd);
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

int forkprocess(int fd)
{
	/* note this ioctl will be integrated in a future EMX version
	 * and is then no longer required here.
	 */
	ULONG data=1, len;
	DosDevIOCtl(fd,XFREE86_PTY,XTY_ENADUP,
		&data,sizeof(ULONG),&len, NULL,0,NULL);
	return fork();
}

void pass_all(int fd, int childpid)
{
	int newpid, nread;
	char buf[BUF_SZ+1];

	if ((newpid=forkprocess(fd)) < 0) {
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
		exit(0);
	}
	/* parent: fd -> stdout */

	DosSleep(1000);
	
	signal(SIGTERM, sig_term);

	for (;;) {
		memset(buf,0,BUF_SZ);
		if ((nread=read(fd,buf,BUF_SZ)) <= 0) {
			break;
		}
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

	execl(cmd,cmd,"ttyp0", NULL);

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
	ULONG data=1,len;

	if (argc != 3) {
		fprintf(stderr,"Usage: %s logfile cmd\n",argv[0]);
		exit(1);
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
	if ((shellpid=forkprocess(master_fd)) < 0)
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
