#include  <stdio.h>
#include  <fcntl.h>

void main()
{
  int  fd = open("/dev/ptyp0", O_RDWR);

  while (1)
  {
    char buffer[256];
    int  len;

    len = read(fd, buffer, 256);
    write(1, buffer, len);
  }
}
