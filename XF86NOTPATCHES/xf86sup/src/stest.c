#include <stdio.h>
#define INCL_DOSMEMMGR
#include <os2.h>
#include "mapos2.h"

char pmap_dev[] = "PMAP$";

main()
{
	PVOID addr;
	APIRET rc;
	HFILE pmap;
	ULONG action,len;
	struct xf86_pmap_param par;
	struct xf86_pmap_data dta;

	rc = DosAllocMem(&addr,65536l,fPERM);
	if (rc) {
		fprintf(stderr,"Error DosAlloc, rc=%d\n",rc);
		exit(1);
	}

	fprintf(stderr,"Adress allocated=%lx\n",(ULONG)addr);

	rc = DosOpen(pmap_dev,&pmap,&action, 0, FILE_NORMAL, FILE_OPEN,
                OPEN_ACCESS_READWRITE|OPEN_SHARE_DENYNONE, (PEAOP2)NULL);
	if (rc) {
		fprintf(stderr,"Error DosOpen, rc=%d\n",rc);
		exit(1);
	}

        /* map device */
        par.u.vmaddr = (ULONG)addr;
        par.size = 65536l;
        rc = DosDevIOCtl(pmap, XFREE86_PMAP, PMAP_LOCKBUF,
                (ULONG*)&par, sizeof(par), &len,
                (ULONG*)&dta, sizeof(dta), &len);
        if (rc) {
                fprintf(stderr,"Error ioctl: %d\n", rc);
                DosClose(pmap);
                exit(1);
        }

	fprintf(stderr,"vaddr=%lx, paddr=%lx\n",
		addr, dta.addr);

	DosClose(pmap);
	exit(0);
}
