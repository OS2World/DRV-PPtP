#include  <stdio.h>
#include  <fcntl.h>

void main()
{
  int fd;
  int len;
  fd = open("/dev/ttyp0", O_RDWR);
  if (fd < 0)
    printf("Konnte ttyp0 nicht oeffnen\n");

  len = write(fd, "Hallo", 5);
  printf("%d bytes geschrieben\n", len);
  close(fd);
}
