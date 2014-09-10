/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

int Punmap(ULONG);	/* return 0 if ok */
ULONG Pmap(ULONG,ULONG); /* return !NULL if ok */
int Verify(FPVOID,USHORT,USHORT); /* return 1 if ok */
void SegLimit(USHORT,USHORT far*);
void int3(void);
int P2V(ULONG,USHORT,FPVOID);
int Block(FPVOID,ULONG,USHORT);
int Run(FPVOID);
void SendEvent(USHORT,USHORT);
int SemRequest(FPVOID,ULONG);
int SemClear(FPVOID);
int GetPID(void);
int GetPGRP(void);
int LockSeg(USHORT);
FPVOID AllocBuf(ULONG sz);
int OpenSEV(ULONG);
int CloseSEV(ULONG);
int PostSEV(ULONG);
int ResetSEV(ULONG);
int acquire_gdt(void);
