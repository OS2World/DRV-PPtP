#define INCL_BASE
#define INCL_DOSEXCEPTIONS
#include <os2.h>
#include <stdio.h>

ULONG sighndlr (PEXCEPTIONREPORTRECORD pERepRec,
		PEXCEPTIONREGISTRATIONRECORD pERegRec,
		PCONTEXTRECORD pCtxRec,
		PVOID p);

int main(int argc,char*argv[])
{
	EXCEPTIONREGISTRATIONRECORD xcpt = { 0, &sighndlr };

	/* make me immortile */
	DosError(FERR_DISABLEEXCEPTION|FERR_DISABLEHARDERR);
	DosSetExceptionHandler(&xcpt);

	printf("My PID is %d, kill me!\n",getpid());

	while(1) { sleep(0); }
}


ULONG sighndlr (PEXCEPTIONREPORTRECORD pERepRec,
		PEXCEPTIONREGISTRATIONRECORD pERegRec,
		PCONTEXTRECORD pCtxRec,
		PVOID p)
{
	return XCPT_CONTINUE_EXECUTION;
}
