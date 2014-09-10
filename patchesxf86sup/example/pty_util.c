/* This code taken from the book
 * W. Richard Stevens, "Unix Network Programming", Prentice Hall, 1990,
 * pp. 278-280
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>

int readn(int fd, char *ptr, int nbytes)
{
	int nleft, nread;
	nleft = nbytes;
	while (nleft > 0) {
		nread = read(fd,ptr,nleft);
		if (nread < 0)
			return nread;
		else if (nread==0)
			break;
		nleft -= nread;
		ptr += nread;
	}
	return nbytes-nleft;
}

int writen(int fd, char *ptr, int nbytes)
{
	int nleft, nwritten;
	nleft = nbytes;
	while (nleft > 0) {
		nwritten = write(fd,ptr,nleft);
		if (nwritten <= 0)
			return nwritten;
		nleft -= nwritten;
		ptr += nwritten;
	}
	return nbytes - nleft;
}
