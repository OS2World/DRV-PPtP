; **********************************************************************
; Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de)
; Use at your own risk! No Warranty! The author is not responsible for
; any damage or loss of data caused by proper or improper use of this
; device driver.
; **********************************************************************
	PAGE	80,132
	.286p
	TITLE   xfree86.sys - XFREE86 SUPPORT DRIVER
	NAME	xfree86
.XLIST
	INCLUDE devhlp.inc
	INCLUDE devsym.inc
	INCLUDE error.inc
.LIST

; **********************************************************************
; SEGMENTS
; **********************************************************************

; These are the standard segments
_DATA   segment word public 'DATA'
_DATA   ends
CONST   segment word public 'CONST'
CONST   ends
_BSS	segment word public 'BSS'
_BSS	ends

DGROUP  GROUP   CONST, _BSS, _DATA

_TEXT   segment word public 'CODE'
_TEXT   ends

; **********************************************************************
; Macros
; **********************************************************************

; declares a device driver header
Header macro lbl, next, attr, strat, intr, name, pcs, pds, rcs, rds, cap
	?Header macro f,sz,val
		.errnz ($ - lbl - offset f)
		sz val
	endm
	public  lbl
	lbl	label byte
	?Header SDevNext	dd	<next>
	?Header SDevAtt	 	dw	<attr>
	?Header SDevStrat	dw	<strat>
	?Header SDevInt	 	dw	<intr>
	?Header SDevName	db	<name>
	?Header SDevProtCS	dw	<pcs>
	?Header SDevProtDS	dw	<pds>
	?Header SDevRealCS	dw	<rcs>
	?Header SDevRealDS	dw	<rds>
	?Header SDevCaps	dd	<cap or 0000000000000001b>
	.errnz ($ - lbl - size SysDev3)
endm

_DATA	   segment

; **********************************************************************
; header and attribute of PMAP$ device
; **********************************************************************
MAP_ATTR	equ	DEV_CHAR_DEV or DEV_30 or DEVLEV_3
Header MapDev   <offset IoDev>,MAP_ATTR,MapStrat,0,'PMAP$   ',0,0,0,0,0

; **********************************************************************
; header and attribute of FASTIO$ device
; **********************************************************************
IO_ATTR	equ	DEV_CHAR_DEV or DEV_30 or DEVLEV_3
Header IoDev   <offset ptyp0>,IO_ATTR,IOStrat,0,'FASTIO$ ',0,0,0,0,0

; **********************************************************************
; headers of PTY master part
; **********************************************************************
PTY_ATTR	equ	DEV_CHAR_DEV or DEV_NON_IBM or DEV_30 or DEVLEV_3

; This is somewhat ugly: To get 32 ptys, you need 32*2 device headers
Header ptyp0	<offset ptyp1>, PTY_ATTR,PtyStrat0,0,'PTYP0   ',0,0,0,0,0
Header ptyp1	<offset ptyp2>, PTY_ATTR,PtyStrat1,0,'PTYP1   ',0,0,0,0,0
Header ptyp2	<offset ptyp3>, PTY_ATTR,PtyStrat2,0,'PTYP2   ',0,0,0,0,0
Header ptyp3	<offset ptyp4>, PTY_ATTR,PtyStrat3,0,'PTYP3   ',0,0,0,0,0
Header ptyp4	<offset ptyp5>, PTY_ATTR,PtyStrat4,0,'PTYP4   ',0,0,0,0,0
Header ptyp5	<offset ptyp6>, PTY_ATTR,PtyStrat5,0,'PTYP5   ',0,0,0,0,0
Header ptyp6	<offset ptyp7>, PTY_ATTR,PtyStrat6,0,'PTYP6   ',0,0,0,0,0
Header ptyp7	<offset ptyp8>, PTY_ATTR,PtyStrat7,0,'PTYP7   ',0,0,0,0,0
Header ptyp8	<offset ptyp9>, PTY_ATTR,PtyStrat8,0,'PTYP8   ',0,0,0,0,0
Header ptyp9	<offset ptypa>, PTY_ATTR,PtyStrat9,0,'PTYP9   ',0,0,0,0,0
Header ptypa	<offset ptypb>, PTY_ATTR,PtyStrata,0,'PTYPA   ',0,0,0,0,0
Header ptypb	<offset ptypc>, PTY_ATTR,PtyStratb,0,'PTYPB   ',0,0,0,0,0
Header ptypc	<offset ptypd>, PTY_ATTR,PtyStratc,0,'PTYPC   ',0,0,0,0,0
Header ptypd	<offset ptype>, PTY_ATTR,PtyStratd,0,'PTYPD   ',0,0,0,0,0
Header ptype	<offset ptypf>, PTY_ATTR,PtyStrate,0,'PTYPE   ',0,0,0,0,0
Header ptypf	<offset ptyq0>, PTY_ATTR,PtyStratf,0,'PTYPF   ',0,0,0,0,0
Header ptyq0	<offset ptyq1>, PTY_ATTR,PtyStra10,0,'PTYQ0   ',0,0,0,0,0
Header ptyq1	<offset ptyq2>, PTY_ATTR,PtyStra11,0,'PTYQ1   ',0,0,0,0,0
Header ptyq2	<offset ptyq3>, PTY_ATTR,PtyStra12,0,'PTYQ2   ',0,0,0,0,0
Header ptyq3	<offset ptyq4>, PTY_ATTR,PtyStra13,0,'PTYQ3   ',0,0,0,0,0
Header ptyq4	<offset ptyq5>, PTY_ATTR,PtyStra14,0,'PTYQ4   ',0,0,0,0,0
Header ptyq5	<offset ptyq6>, PTY_ATTR,PtyStra15,0,'PTYQ5   ',0,0,0,0,0
Header ptyq6	<offset ptyq7>, PTY_ATTR,PtyStra16,0,'PTYQ6   ',0,0,0,0,0
Header ptyq7	<offset ptyq8>, PTY_ATTR,PtyStra17,0,'PTYQ7   ',0,0,0,0,0
Header ptyq8	<offset ptyq9>, PTY_ATTR,PtyStra18,0,'PTYQ8   ',0,0,0,0,0
Header ptyq9	<offset ptyqa>, PTY_ATTR,PtyStra19,0,'PTYQ9   ',0,0,0,0,0
Header ptyqa	<offset ptyqb>, PTY_ATTR,PtyStra1a,0,'PTYQA   ',0,0,0,0,0
Header ptyqb	<offset ptyqc>, PTY_ATTR,PtyStra1b,0,'PTYQB   ',0,0,0,0,0
Header ptyqc	<offset ptyqd>, PTY_ATTR,PtyStra1c,0,'PTYQC   ',0,0,0,0,0
Header ptyqd	<offset ptyqe>, PTY_ATTR,PtyStra1d,0,'PTYQD   ',0,0,0,0,0
Header ptyqe	<offset ptyqf>, PTY_ATTR,PtyStra1e,0,'PTYQE   ',0,0,0,0,0
Header ptyqf	<offset ttyp0>, PTY_ATTR,PtyStra1f,0,'PTYQF   ',0,0,0,0,0

; **********************************************************************
; headers of pty slave parts
; **********************************************************************
TTY_ATTR	equ	DEV_CHAR_DEV or DEV_NON_IBM or DEV_30 or DEVLEV_3

Header ttyp0	<offset ttyp1>, TTY_ATTR,TtyStrat0,0,'TTYP0   ',0,0,0,0,0
Header ttyp1	<offset ttyp2>, TTY_ATTR,TtyStrat1,0,'TTYP1   ',0,0,0,0,0
Header ttyp2	<offset ttyp3>, TTY_ATTR,TtyStrat2,0,'TTYP2   ',0,0,0,0,0
Header ttyp3	<offset ttyp4>, TTY_ATTR,TtyStrat3,0,'TTYP3   ',0,0,0,0,0
Header ttyp4	<offset ttyp5>, TTY_ATTR,TtyStrat4,0,'TTYP4   ',0,0,0,0,0
Header ttyp5	<offset ttyp6>, TTY_ATTR,TtyStrat5,0,'TTYP5   ',0,0,0,0,0
Header ttyp6	<offset ttyp7>, TTY_ATTR,TtyStrat6,0,'TTYP6   ',0,0,0,0,0
Header ttyp7	<offset ttyp8>, TTY_ATTR,TtyStrat7,0,'TTYP7   ',0,0,0,0,0
Header ttyp8	<offset ttyp9>, TTY_ATTR,TtyStrat8,0,'TTYP8   ',0,0,0,0,0
Header ttyp9	<offset ttypa>, TTY_ATTR,TtyStrat9,0,'TTYP9   ',0,0,0,0,0
Header ttypa	<offset ttypb>, TTY_ATTR,TtyStrata,0,'TTYPA   ',0,0,0,0,0
Header ttypb	<offset ttypc>, TTY_ATTR,TtyStratb,0,'TTYPB   ',0,0,0,0,0
Header ttypc	<offset ttypd>, TTY_ATTR,TtyStratc,0,'TTYPC   ',0,0,0,0,0
Header ttypd	<offset ttype>, TTY_ATTR,TtyStratd,0,'TTYPD   ',0,0,0,0,0
Header ttype	<offset ttypf>, TTY_ATTR,TtyStrate,0,'TTYPE   ',0,0,0,0,0
Header ttypf	<offset ttyq0>, TTY_ATTR,TtyStratf,0,'TTYPF   ',0,0,0,0,0
Header ttyq0	<offset ttyq1>, TTY_ATTR,TtyStra10,0,'TTYQ0   ',0,0,0,0,0
Header ttyq1	<offset ttyq2>, TTY_ATTR,TtyStra11,0,'TTYQ1   ',0,0,0,0,0
Header ttyq2	<offset ttyq3>, TTY_ATTR,TtyStra12,0,'TTYQ2   ',0,0,0,0,0
Header ttyq3	<offset ttyq4>, TTY_ATTR,TtyStra13,0,'TTYQ3   ',0,0,0,0,0
Header ttyq4	<offset ttyq5>, TTY_ATTR,TtyStra14,0,'TTYQ4   ',0,0,0,0,0
Header ttyq5	<offset ttyq6>, TTY_ATTR,TtyStra15,0,'TTYQ5   ',0,0,0,0,0
Header ttyq6	<offset ttyq7>, TTY_ATTR,TtyStra16,0,'TTYQ6   ',0,0,0,0,0
Header ttyq7	<offset ttyq8>, TTY_ATTR,TtyStra17,0,'TTYQ7   ',0,0,0,0,0
Header ttyq8	<offset ttyq9>, TTY_ATTR,TtyStra18,0,'TTYQ8   ',0,0,0,0,0
Header ttyq9	<offset ttyqa>, TTY_ATTR,TtyStra19,0,'TTYQ9   ',0,0,0,0,0
Header ttyqa	<offset ttyqb>, TTY_ATTR,TtyStra1a,0,'TTYQA   ',0,0,0,0,0
Header ttyqb	<offset ttyqc>, TTY_ATTR,TtyStra1b,0,'TTYQB   ',0,0,0,0,0
Header ttyqc	<offset ttyqd>, TTY_ATTR,TtyStra1c,0,'TTYQC   ',0,0,0,0,0
Header ttyqd	<offset ttyqe>, TTY_ATTR,TtyStra1d,0,'TTYQD   ',0,0,0,0,0
Header ttyqe	<offset ttyqf>, TTY_ATTR,TtyStra1e,0,'TTYQE   ',0,0,0,0,0
Header ttyqf	<offset ptms>,TTY_ATTR,TtyStra1f,0,'TTYQF   ',0,0,0,0,0

; **********************************************************************
; header of the PTY clone helper device
; **********************************************************************
Header ptms	<offset console>, PTY_ATTR,PtmsStrat,0,'PTMS$   ',0,0,0,0,0

; **********************************************************************
; header of console device
; **********************************************************************
CON_ATTR	equ	DEV_CHAR_DEV or DEV_NON_IBM or DEV_30 or DEVLEV_3 or DEV_IOCTL
;Header console  <offset null>,  CON_ATTR,ConStrat,_conidc,'CONSOLE ',0,0,0,0,0
Header console  -1,  CON_ATTR,ConStrat,_conidc,'CONSOLE$',0,0,0,0,0

; **********************************************************************
; header of null device
; **********************************************************************
;NUL_ATTR	equ	DEV_CHAR_DEV or DEV_30 or DEVLEV_3

;Header null	-1,		NUL_ATTR,NulStrat,0,'NULL    ',0,0,0,0,0

; **********************************************************************
; public DATA declarations
; **********************************************************************
		public  _Device_Help
		public  _qmutex
		public	_dbgflag
		public	_io_gdt32

_Device_Help	dd	0	; 16:16 indirect address of Device Helpers 
Paddr		dd	0	; temp save for Pmap and ResetSEV
Psize		dd	0	; temp save for Pmap
_qmutex	 dd	33 dup (0)	; mutex semaphores for tty driver and con driver
_dbgflag db	0		; flag to enable int3 trapping
_io_gdt32	dw	0	; 32 bit call gate for I/O
gdthelper	dw	0	; helper selector for accessing GDT
gdtsave		dq	0	; save for GDT register

_DATA		ends

; **********************************************************************
; public TEXT routines, called from C code
; **********************************************************************
_TEXT		segment
		assume cs:_TEXT, ds:DGROUP, es:NOTHING, ss:DGROUP

		extrn   _mapstrategy:far
		extrn   _ptsstrategy:far
		extrn   _ptmsstrategy:far
		extrn   _ptcstrategy:far
		extrn   _constrategy:far
		extrn   _conidc:far
;		extrn   _nullstrategy:far
		extrn   _iostrategy:far

		public  __acrtused
		public  MapStrat
		public  PtyStrat
		public  TtyStrat
		public  ConStrat
		public	IOStrat
;		public  NulStrat

; **********************************************************************
; Startup routine for MAP driver
; **********************************************************************

MapStrat	proc	far
__acrtused:
		push	es
		push	bx
		call	_mapstrategy	; int mapstrategy(RP rp)
		pop	bx
		pop	es
		mov	word ptr es:[bx+3], ax	; return value
		ret
MapStrat	endp

; **********************************************************************
; Startup routine for FASTIO driver
; **********************************************************************

IOStrat	proc	far
		push	es
		push	bx
		call	_iostrategy	; int iostrategy(RP rp)
		pop	bx
		pop	es
		mov	word ptr es:[bx+3], ax	; return status
		ret
IOStrat		endp

; **********************************************************************
; Startup routine for the PTY driver, 32 supported
; **********************************************************************

PtyStrat0	proc	far
		push	0
		jmp	short PtyStrat
PtyStrat1:	push	1
		jmp	short PtyStrat
PtyStrat2:	push	2
		jmp	short PtyStrat
PtyStrat3:	push	3
		jmp	short PtyStrat
PtyStrat4:	push	4
		jmp	short PtyStrat
PtyStrat5:	push	5
		jmp	short PtyStrat
PtyStrat6:	push	6
		jmp	short PtyStrat
PtyStrat7:	push	7
		jmp	short PtyStrat
PtyStrat8:	push	8
		jmp	short PtyStrat
PtyStrat9:	push	9
		jmp	short PtyStrat
PtyStrata:	push	10
		jmp	short PtyStrat
PtyStratb:	push	11
		jmp	short PtyStrat
PtyStratc:	push	12
		jmp	short PtyStrat
PtyStratd:	push	13
		jmp	short PtyStrat
PtyStrate:	push	14
		jmp	short PtyStrat
PtyStratf:	push	15
		jmp	short PtyStrat
PtyStra10:	push	16
		jmp	short PtyStrat
PtyStra11:	push	17
		jmp	short PtyStrat
PtyStra12:	push	18
		jmp	short PtyStrat
PtyStra13:	push	19
		jmp	short PtyStrat
PtyStra14:	push	20
		jmp	short PtyStrat
PtyStra15:	push	21
		jmp	short PtyStrat
PtyStra16:	push	22
		jmp	short PtyStrat
PtyStra17:	push	23
		jmp	short PtyStrat
PtyStra18:	push	24
		jmp	short PtyStrat
PtyStra19:	push	25
		jmp	short PtyStrat
PtyStra1a:	push	26
		jmp	short PtyStrat
PtyStra1b:	push	27
		jmp	short PtyStrat
PtyStra1c:	push	28
		jmp	short PtyStrat
PtyStra1d:	push	29
		jmp	short PtyStrat
PtyStra1e:	push	30
		jmp	short PtyStrat
PtyStra1f:	push	31
		jmp	short PtyStrat
PtmsStrat:	push	32
PtyStrat:	push	es
		push	bx
		call	_ptcstrategy	; int ptcstrategy(RP rp, int devnr)
		pop	bx
		pop	es
		add	sp,2		; discard devnr
		mov	word ptr es:[bx+3], ax	; return status
		ret
PtyStrat0	endp

; **********************************************************************
; Startup routine for the TTY driver, 32 supported
; **********************************************************************

TtyStrat0	proc	far
		push	0
		jmp	short TtyStrat
TtyStrat1:	push	1
		jmp	short TtyStrat
TtyStrat2:	push	2
		jmp	short TtyStrat
TtyStrat3:	push	3
		jmp	short TtyStrat
TtyStrat4:	push	4
		jmp	short TtyStrat
TtyStrat5:	push	5
		jmp	short TtyStrat
TtyStrat6:	push	6
		jmp	short TtyStrat
TtyStrat7:	push	7
		jmp	short TtyStrat
TtyStrat8:	push	8
		jmp	short TtyStrat
TtyStrat9:	push	9
		jmp	short TtyStrat
TtyStrata:	push	10
		jmp	short TtyStrat
TtyStratb:	push	11
		jmp	short TtyStrat
TtyStratc:	push	12
		jmp	short TtyStrat
TtyStratd:	push	13
		jmp	short TtyStrat
TtyStrate:	push	14
		jmp	short TtyStrat
TtyStratf:	push	15
		jmp	short TtyStrat
TtyStra10:	push	16
		jmp	short TtyStrat
TtyStra11:	push	17
		jmp	short TtyStrat
TtyStra12:	push	18
		jmp	short TtyStrat
TtyStra13:	push	19
		jmp	short TtyStrat
TtyStra14:	push	20
		jmp	short TtyStrat
TtyStra15:	push	21
		jmp	short TtyStrat
TtyStra16:	push	22
		jmp	short TtyStrat
TtyStra17:	push	23
		jmp	short TtyStrat
TtyStra18:	push	24
		jmp	short TtyStrat
TtyStra19:	push	25
		jmp	short TtyStrat
TtyStra1a:	push	26
		jmp	short TtyStrat
TtyStra1b:	push	27
		jmp	short TtyStrat
TtyStra1c:	push	28
		jmp	short TtyStrat
TtyStra1d:	push	29
		jmp	short TtyStrat
TtyStra1e:	push	30
		jmp	short TtyStrat
TtyStra1f:	push	31
TtyStrat:	push	es
		push	bx
		call	_ptsstrategy	; int ptsstrategy(RP rp, int devnr)
		pop	bx
		pop	es
		add	sp,2		; discard devnr
		mov	word ptr es:[bx+3], ax	; return status
		ret
TtyStrat0	endp

; **********************************************************************
; Startup routine for the CONSOLE driver
; **********************************************************************

ConStrat	proc	far
		push	es
		push	bx
		call	_constrategy	; int constrategy(RP rp, int devnr)
		pop	bx
		pop	es
		mov	word ptr es:[bx+3], ax	; return status
		ret
ConStrat	endp

; **********************************************************************
; Startup routine for the NULL driver
; **********************************************************************

;NulStrat	proc	far
;		push	es
;		push	bx
;		call	_nullstrategy	; int nullstrategy(RP rp)
;		pop	bx
;		pop	es
;		mov	word ptr es:[bx+3], ax	; return status
;		ret
;NulStrat	endp

; **********************************************************************
; Utility Device helpers
; **********************************************************************

		public  _Punmap
		public  _Pmap
		public  _Verify
		public  _int3
		public  _SegLimit
		public  _P2V
		public  _Block
		public  _Run
		public  _SendEvent
		public  _SemRequest
		public  _SemClear
		public  _GetPID
		public  _LockSeg
		public	_AllocBuf
		public	_GetPGRP
		public	_OpenSEV
		public	_CloseSEV
		public	_PostSEV
		public	_ResetSEV
		public	_io_call
		public	_acquire_gdt

		.386p

; generate an int3 trap for the kernel debugger. This function is
; activated by the cmdline option /666. The use of the "number of the beast"
; is intentional: if you don't have a kernel debugger running, this function
; will severely crash and possibly damage your system.
;
_int3		proc	far
		assume cs:_TEXT, ds:DGROUP
		test	byte ptr [_dbgflag], 1	; if dbgflag is enabled
		jz	intex
		int	3			; perform an int3
intex:		ret
_int3		endp

; int linfoseg(USHORT [bp+6])
; return a 16 bit value in the current local process info segment,
; argument is the index into the segment
; returns 0 if function failed.
;
linfoseg	proc	far
		push	bp
		mov	bp,sp
		push	cx		; save some registers
		push	es
		push	bx
		push	si
		mov	al,2		; select local info seg
		xor	cx,cx		; unused
		mov	dl, DevHlp_GetDOSVar
		call	[_Device_Help]	; call function
		jc	err15		; if carry set, error
		mov	es,ax
		les	bx, es:[bx]	; address of local info seg ->es:bx
		mov	si, word ptr [bp+6]	; get index
		mov	ax, es:[bx+si]	; load word
		jmp	short ok15	; exit ok
err15:		xor	ax,ax		; exit error
ok15:		pop	si
		pop	bx
		pop	es
		pop	cx
		pop	bp
		ret
linfoseg	endp

; int Pmap(DWORD [bp+6], DWORD [bp+10])
; Create a memory window into physical memory usable by a 32 bit process
; arg1 = real physical base address of window
; arg2 = size of window
; returns a usable 0:32 pointer in process address space
; or 0L, if error
;
_Pmap		proc	far
		assume cs:_TEXT, ds:DGROUP
		push	bp
		mov	bp, sp
		push	esi			; save some regs
		push	ecx

		mov	eax, dword ptr [bp+6]	; save base address
		mov	Paddr, eax
		mov	eax, dword ptr [bp+10]	; save size
		mov	Psize, eax
		mov	ax, ds			; make a linear address
		mov	esi, offset Paddr	; from the address
		mov	dl, DevHlp_VirtToLin
		call	[_Device_Help]
		jc	err0			; if carry set fail

		mov	edi, eax		; address requested
		mov	eax, 00000030h		; phys mem, in process space
		mov	ecx, Psize		; length of segment
		mov	dl, DevHlp_VMAlloc	; map address
		call	[_Device_Help]
		jc	err0			; if carry set fail

		mov	edx, eax		; convert EAX
		shr	edx, 16			; to DX:AX
		jmp	short ok0		; exit ok
err0:		xor	ax,ax			; exit error, return 0L
		xor	dx,dx
ok0:		pop	ecx
		pop	esi
		pop	bp
		ret
_Pmap		endp

; int Punmap(DWORD [bp+6])
; unmap a previously allocated address, argument is address
; return -1, if error
; return 0, if successful
;
_Punmap	 	proc	far
		assume  cs:_TEXT, ds:DGROUP
		push	bp
		mov	bp,sp
		push	dx			; save regs

		mov	eax, dword ptr [bp+6]	; get address
		mov	dl, DevHlp_VMFree	; free it
		call	[_Device_Help]
		jc	err1			; if carry ,error
		xor	ax,ax			; exit ok
		jmp	short ok1
err1:		or	ax,0ffffh		; exit error
ok1:		pop	dx
		pop	bp
		ret
_Punmap		endp

; int Verify(FPVOID [bp+6], int [bp+10], char [bp+12])
; check whether a segment is accessible, arg1 = base address, arg2 = size,
; arg3 = type (0=R/O, 1=R/W)
; return 0, if sucessful
; return !=0 if error
;
_Verify		proc	far
		assume  cs:_TEXT, ds:DGROUP
		push	bp
		mov	bp,sp
		push	dx			; save regs
		push	cx
		push	di

		mov	ax,word ptr [bp+8]	; selector
		mov	cx,word ptr [bp+10]	; size
		mov	di,word ptr [bp+6]	; offset
		mov	dh,byte ptr [bp+12]	; type of segment (R/O, R/W)
		mov	dl,DevHlp_VerifyAccess
		call	[_Device_Help]		; check memory
		jc	err4			; carry set if error

		xor	ax,ax			; exit ok
		jmp	ok4
err4:		or	ax,1			; exit error
ok4:		pop	di
		pop	cx
		pop	dx
		pop	bp
		ret
_Verify		endp

; int SegLimit(int [bp+6], int* [bp+8])
; put the segment limit of the given selector in arg1 into address arg2
; never fails, returns 0
;
_SegLimit	proc	far
		push	bp
		mov	bp,sp

		push	es			; save regs
		push	bx
		push	di

		mov	ax, word ptr [bp+6]	; get selector
		;lsl	bx, ax			; load segment limit
		db	0fh,03h,0d8h		; circumvent assembler bug
		les	di, dword ptr [bp+8]	; get address
		mov	word ptr es:[di],bx	; store limit
		xor	ax,ax			; return ok

		pop	di
		pop	bx
		pop	es
		pop	bp
		ret
_SegLimit	endp

; int P2V(ULONG [bp+6], int [bp+10], FPVOID [bp+12])
; convert an address segment in arg1, size arg2 into a usable 16:16 address
; returned in arg3
; return 0 if ok, -1 if error
;
_P2V		proc	far
		push	bp
		mov	bp,sp
		push	ds			; save regs
		push	es
		push	bx
		push	cx
		push	dx
		push	si

		mov	bx, word ptr [bp+6]	; address to be converted
		mov	ax, word ptr [bp+8]
		mov	cx, word ptr [bp+10]	; size of segment
		mov	dh, 0			; return in DS:SI
		mov	dl, DevHlp_PhysToVirt	; convert
		call	[_Device_Help]
		jc	err7			; if carry set, error

		mov	ax, ds
		les	bx, dword ptr [bp+12]	; get addr of return var
		mov	word ptr es:[bx],si	; save addr
		mov	word ptr es:[bx+2],ax
		xor	ax,ax
		jmp	short ok7		; return ok
err7:		or	ax,0ffffh		; return error
ok7:		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	es
		pop	ds
		pop	bp
		ret
_P2V		endp

; int Block(ULONG [bp+6], ULONG [bp+10], char [bp+14])
; block a DD thread on ID arg1 for timeout arg2, interruptable if arg3=0
; DD returns if resumed by Run call or interrupted
; returns 0 if resumed normally
; 1 if timeout
; 2 if awaken asynchronously
;
_Block		proc	far
		push	bp
		mov	bp,sp
		push	bx			; save regs
		push	cx
		push	dx
		push	di

		mov	bx, word ptr [bp+6]	; block id
		mov	ax, word ptr [bp+8]
		mov	cx, word ptr [bp+10]	; block timeout
		mov	di, word ptr [bp+12]
		mov	dh, byte ptr [bp+14]	; interruptible flag
		mov	dl, DevHlp_ProcBlock
		call	[_Device_Help]		; block now
		jnc	xrun			; carry clear if normal run
		jnz	xtimeo			; C=1, Z=1 if timeout
		mov	ax,2			; asynch wakeup exit
		jmp	short blk
xrun:		xor	ax,ax			; normal resume exit
		jmp	short blk
xtimeo:		mov	ax,1			; timeout exit
blk:		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	bp
		ret
_Block		endp

; void Run(ULONG [bp+6])
; Run a blocked DD thread, arg1=block id
; returns no error status
;
_Run		proc	far
		push	bp
		mov	bp,sp
		push	bx			; save regs
		push	dx
		mov	bx, word ptr [bp+6]	; block ID
		mov	ax, word ptr [bp+8]
		mov	dl, DevHlp_ProcRun	; run now
		call	[_Device_Help]
		pop	dx
		pop	bx
		pop	bp
		ret
_Run		endp

; int SendEvent(char [bp+6], int [bp+8])
; send an event (arg1) with optional argument arg2 to current process
; returns no status
;
_SendEvent	proc	far
		push	bp
		mov	bp,sp
		push	bx			; save regs
		push	dx
		mov	ah, byte ptr [bp+6]	; event #
		mov	bx, word ptr [bp+8]	; optional arg
		mov	dl, DevHlp_SendEvent	; send it
		call	[_Device_Help]
		pop	dx
		pop	bx
		pop	bp
		ret
_SendEvent	endp

; int SemRequest(ULONG* sema, ULONG timeout)
; request a mutex semaphore, wait for given timeout
; returns 0 if semaphore owned
; returns != 0 if error
;
_SemRequest	proc	far
		push	bp
		mov	bp,sp
		push	bx			; save regs
		push	cx
		push	dx
		push	di
		mov	bx, word ptr [bp+6]	; address of semaphore
		mov	ax, word ptr [bp+8]
		mov	cx, word ptr [bp+10]	; timeout
		mov	di, word ptr [bp+12]
		mov	dl, DevHlp_SemRequest	; acquire semaphore
		call	[_Device_Help]
		jc	err9			; error if carry set
		xor	ax,ax			; return 0, if ok
err9:		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	bp
		ret
_SemRequest	endp

; int SemClear(ULONG* sema)
; release an owned semaphore
; returns 0 if ok 
; != 0 if error
;
_SemClear	proc	far
		push	bp
		mov	bp,sp
		push	bx			; save regs
		push	dx
		mov	bx, word ptr [bp+6]	; address of semaphore
		mov	ax, word ptr [bp+8]
		mov	dl, DevHlp_SemClear	; release it
		call	[_Device_Help]
		jc	err10			; carry set if error
		xor	ax,ax			; return 0 if ok
err10:		pop	dx
		pop	bx
		pop	bp
		ret
_SemClear	endp

; int GetPID()
; return the process ID of current process
;
_GetPID	 proc	far
		push	bp
		mov	bp,sp
		push	0		; process ID is offset 0 in
		call	linfoseg	; local info segment
		add	sp,2
		pop	bp
		ret
_GetPID		endp

; int LockSeg(USHORT [bp+6])
; lock a segment in memory to make it unswappable
; pass selector of segment
; returns 0 if ok
; -1 if failure
;
_LockSeg	proc	far
		push	bp
		mov	bp,sp
		push	bx			; save regs
		push	dx
		mov	ax, word ptr [bp+6]	; selector of segment
		xor	bl, bl			; wait flag
		mov	bh, 1			; lock type
		mov	dl, DevHlp_Lock		; perform it
		call	[_Device_Help]
		jc	err12			; carry set if error

		xor	ax,ax			; exit 0 if ok
		jmp	short ok12
err12:		or	ax,0ffffh		; exit error 
ok12:		pop	dx
		pop	bx
		pop	bp
		ret
_LockSeg	endp

; PFVOID* AllocBuf(ULONG [bp+6])
; Allocate a chunk of dynamic memory of size arg1
; return address if ok
; return 0:0 if error
;
_AllocBuf	proc	far
		push	bp
		mov	bp,sp
		push	ecx			; save regs
		push	edi

		mov	ecx, dword ptr [bp+6]	; get size of segment

		; Watchout: you have the rare opportunity to see something
		; undocumented explained: Bit 7 is marked as reserved in the
		; DD doc, but setting it will cause a GDT selector becoming
		; allocated for the requested memory segment. The SEL:OFF
		; pair corresponding to the address returned in EAX is
		; returned in ECX, thus the GDT selector is in the high 16 bits
		; of ECX.

		mov	eax, 00000083h		; fixed, <16M, allocate GDT
		mov	edi, 0ffffffffh		; unused
		mov	dl, DevHlp_VMAlloc	; allocate memory
		call	[_Device_Help]
		jc	err0			; error if carry set

		mov	ax,cx			; move ECX -> dx:ax
		shr	ecx,16
		mov	dx,cx
		jmp	short ok13		; exit ok
err13:		xor	ax,ax			; exit error return 0:0
		xor	dx,dx
ok13:		pop	edi
		pop	ecx
		pop	bp
		ret
_AllocBuf	endp

; int GetPGRP()
; return Session ID of current process (process group)
;
_GetPGRP	proc	far
		push	bp
		mov	bp,sp
		push	8			; session id is index 8
		call	linfoseg		; in local info segment
		add	sp,2
		pop	bp
		ret
_GetPGRP	endp

; int OpenSEV(ULONG semhandle)
; Opens a 32 bit event semaphore
; return -1 if failure, 0 if ok
;
_OpenSEV	proc	far
		push	bp
		mov	bp,sp
		mov	eax, dword ptr [bp+6]	; get handle
		mov	dl, DevHlp_OpenEventSem
		call	[_Device_Help]
		jc	err14
		xor	ax,ax
		jmp	short ok14
err14:		mov	ax,0ffffh
ok14:		pop	bp
		ret
_OpenSEV	endp

; int CloseSEV(ULONG semhandle)
; Close a 32 bit event semaphore
; return -1 if error, 0 if ok
; 
_CloseSEV	proc	far
		push	bp
		mov	bp,sp
		mov	eax, dword ptr [bp+6]	; get handle
		mov	dl, DevHlp_CloseEventSem
		call	[_Device_Help]
		jc	err18
		xor	ax,ax
		jmp	short ok18
err18:		mov	ax,0ffffh
ok18:		pop	bp
		ret
_CloseSEV	endp

; int PostSEV(ULONG semhandle)
; Post a 32 bit event semaphore
; return 0 if ok or already posted, -1 if other error
;
_PostSEV	proc	far
		push	bp
		mov	bp,sp
		mov	eax, dword ptr [bp+6]	; get handle
		mov	dl, DevHlp_PostEventSem
		call	[_Device_Help]
		jnc	ok16
		cmp	eax, 299	; error code ALREADY_POSTED
		je	ok16
		mov	ax,0ffffh
		jmp	short ret16
ok16:		xor	ax,ax
ret16:		pop	bp
		ret
_PostSEV	endp

; int ResetSEV(ULONG handle)
; Reset a 32 bit event semaphore
; return 0 if ok or already reset, -1 if error
;
_ResetSEV	proc	far
		push	bp
		mov	bp,sp
		push	edi
		push	esi
		mov	ax, ds			; make a linear address
		mov	esi, offset Paddr	; from the tmp address
		mov	dl, DevHlp_VirtToLin
		call	[_Device_Help]
		jc	err17a			; if carry set fail
		mov	edi, eax		; address for returning count

		mov	eax, dword ptr [bp+6]	; get handle
		mov	dl, DevHlp_ResetEventSem
		call	[_Device_Help]
		jnc	ok17
		cmp	eax, 300	; error code ALREADY_RESET
		je	ok17
err17a:		mov	ax,0ffffh
		jmp	short err17
ok17:		xor	ax,ax
err17:		pop	esi
		pop	edi
		pop	bp
		ret
_ResetSEV	endp

; int acquire_gdt()
; This routine is the worst hack this driver contains, and you should study
; it, but avoid it under all circumstances.
;
; This routine must be explained in detail:
; We rely on the fact that there is a DevHlp function named DevHlp_DynamicAPI
; and will remain available in the future.
; Dynamic API will setup a callgate to a routine in the driver. This is the 
; theory. What actually happens is that a very large thunking and parameter
; checking routine is entered first, and then indirectly calls the driver
; routine. What we do here is pretty much foul: we calculate the offset of
; this selector in the GDT, and change it to point to the driver routine
; directly. Therefore this routine must be run in ring 0 mode.
;
; I expect that this way is the fastest possible way in OS/2 to execute
; R0 code.
;
		.386p
_acquire_gdt	proc	far
		pusha					; remark we push only
							; 16 bit regs, but
							; use 32 bit regs:
							; no problem, context
							; save is on our side
		mov	ax, word ptr [_io_gdt32]	; get selector
		or	ax,ax
		jnz	aexit				; if we didn't have one
							; make one

		xor	ax, ax
		mov	word ptr [_io_gdt32], ax	; clear gdt save
		mov	word ptr [gdthelper], ax	; helper

		push	ds
		pop	es				; ES:DI = addr of
		mov	di, offset _io_gdt32		; _io_gdt32
		mov	cx, 2				; two selectors
		mov	dl, DevHlp_AllocGDTSelector	; get GDT selectors
		call	[_Device_Help]
		jc	aexit				; exit if failed

		sgdt	fword ptr [gdtsave]		; access the GDT ptr
		mov	ebx, dword ptr [gdtsave+2]	; get lin addr of GDT
		movzx	eax, word ptr [_io_gdt32]	; build offset into table
		and	eax, 0fffffff8h			; mask away DPL
		add	ebx, eax			; build address

		mov	ax, word ptr [gdthelper]	; sel to map to
		mov	ecx, 08h			; only one entry
		mov	dl, DevHlp_LinToGDTSelector	
		call	[_Device_Help]
		jc	aexit0				; if failed exit

		mov	ax, word ptr [gdthelper]
		mov	es, ax				; build address to GDT
		xor	bx, bx

		mov	word ptr es:[bx], offset _io_call ; fix address off
		mov	word ptr es:[bx+2], cs		; fix address sel
		mov	word ptr es:[bx+4], 0ec00h	; a r0 386 call gate
		mov	word ptr es:[bx+6], 0000h	; high offset

		mov	dl, DevHlp_FreeGDTSelector	; free gdthelper
		call	[_Device_Help]
		jnc	short aexit

aexit0:		xor	ax,ax				; clear selector
		mov	word ptr [_io_gdt32], ax
aexit:
		popa			; restore all registers
		mov	ax, word ptr [_io_gdt32]
		ret
_acquire_gdt	endp

; This is the entry point to the io handler. In order to make it as
; fast as possible, this is written in assembler with passing data in
; registers
; Calling convention:
; In:
;    DX = port
;    AL,AX,EAX = data when port write
;    BX = function code
; Out:
;    DX = unchanged
;    Al,AX,EAX = return value or unchanged
;    Bx = destroyed
;
		.386p

retfd		macro
		db	066h, 0cbh	; 32 bit return
		endm

_io_call	proc	far
		assume	cs:_TEXT, ds:NOTHING, es:NOTHING
		and	bx,15		; only allow 0-15
		add	bx,bx		; build address
		add	bx,offset iotbl	
		jmp	cs:[bx]		; indirect jump

iotbl:		dw	iofret		; 0 reserved
		dw	iof1		; 1 inb
		dw	iof2		; 2 inw
		dw	iof3		; 3 inl
		dw	iof4		; 4 outb
		dw	iof5		; 5 outw
		dw	iof6		; 6 outl
		dw	iofret		; 7 reserved	(insb)

		dw	iofret		; 8 reserved	(insw)
		dw	iofret		; 9 reserved	(insl)
		dw	iofret		; 10 reserved	(outsb)
		dw	iofret		; 11 reserved	(outsw)
		dw	iofret		; 12 reserved	(outsl)
		dw	iofr3		; 13 iopl=3
		dw	iofr2		; 14 iopl=2
		dw	iofret		; 15 reserved

; Note: be aware that this code is called via a 386 call gate, so
; the return adresses stored follow the 32 bit convention
; even if the base segment, which this code is in, is a FAR 16:16 segment.
;
iof1:		in	al,dx		; read byte
iofret:		retfd			; NOP exit 
iof2:		in	ax,dx		; read word
		retfd
iof3:		in	eax,dx		; read dword
		retfd
iof4:		out	dx,al		; write byte
		retfd
iof5:		out	dx,ax		; write word
		retfd
iof6:		out	dx,eax		; write dword
		retfd

iofr3:		pushf
		pop	ax
		or	ax, 0011000000000000b
		push	ax
		popf
		retfd
iofr2:		pushf
		pop	ax
		or	ax, 0010000000000000b
		and	ax, 1110111111111111b
		push	ax
		popf
		retfd
_io_call	endp

_TEXT		ends
		end
