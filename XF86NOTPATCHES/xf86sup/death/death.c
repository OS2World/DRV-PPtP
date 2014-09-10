#include <stdio.h>
#include <stdlib.h>

#define INCL_DOS
#define INCL_ERRORS
#include <os2.h>

#include "death.h"

void usage();

int main(int argc, char *argv[]) 
{
	PID tokill;
	APIRET rc;

	if (argc != 2) usage();

	tokill = atoi(argv[1]);
	if (tokill <= 0 || tokill > 32767)
		usage();

	rc = death(tokill);
	printf("rc=%d\n",rc);
	exit(rc);
}

void usage()
{
	fputs("usage: death pid-to-kill\n",stderr);
	exit(42);
}
	