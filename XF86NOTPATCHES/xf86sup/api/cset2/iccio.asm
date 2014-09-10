; **********************************************************************
; Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de)
; Use at your own risk! No Warranty! The author is not responsible for
; any damage or loss of data caused by proper or improper use of this
; device driver.
; **********************************************************************
;
; compile with ALP
;
	TITLE	ICCIO.ASM
	.386
	.387
CODE32	SEGMENT DWORD USE32 PUBLIC 'CODE'
CODE32	ENDS
DATA32	SEGMENT DWORD USE32 PUBLIC 'DATA'
DATA32	ENDS
CONST32	SEGMENT DWORD USE32 PUBLIC 'CONST'
CONST32	ENDS
BSS32	SEGMENT DWORD USE32 PUBLIC 'BSS'
BSS32	ENDS
DGROUP	GROUP CONST32, BSS32, DATA32
	ASSUME	CS:FLAT, DS:FLAT, SS:FLAT, ES:FLAT
DATA32	SEGMENT
DATA32	ENDS
BSS32	SEGMENT
BSS32	ENDS
CONST32	SEGMENT
CONST32	ENDS

DATA32	SEGMENT
ioentry 	DWORD	0
gdt		WORD	0
DATA32	ENDS

CODE32	SEGMENT

	ALIGN 04H

; performs fast output of a byte to an I/O port 
; this routine is intended to be called from icc C code
;
; Calling convention:
;	void c_outb(short port,char data)
;
;
	PUBLIC	c_outb
c_outb	PROC
	MOV	EDX, DWORD PTR [ESP+4]	; get port
	MOV	AL, BYTE PTR [ESP+8]	; get data
	PUSH	EBX			; save register
	MOV	EBX, 4			; function code 4 = write byte
	CALL	FWORD PTR [ioentry]	; call intersegment indirect 16:32
	POP	EBX
	RET
c_outb	ENDP

; performs fast output of a word to an I/O port 
; this routine is intended to be called from icc C code
;
; Calling convention:
;	void c_outw(short port,short data)
;
;
	ALIGN	04H

	PUBLIC	c_outw
c_outw	PROC
	MOV	EDX, DWORD PTR [ESP+4]	; get port
	MOV	AL, BYTE PTR [ESP+8]	; get data
	PUSH	EBX			; save register
	MOV	EBX, 5			; function code 5 = write word
	CALL	FWORD PTR [ioentry]	; call intersegment indirect 16:32
	POP	EBX
	RET
c_outw	ENDP

; performs fast output of a dword to an I/O port 
; this routine is intended to be called from icc C code
;
; Calling convention:
;	void c_outl(short port,long data)
;
;
	ALIGN	04H

	PUBLIC	c_outl
c_outl	PROC
	MOV	EDX, DWORD PTR [ESP+4]	; get port
	MOV	AL, BYTE PTR [ESP+8]	; get data
	PUSH	EBX			; save register
	MOV	EBX, 6			; function code 6 = write dword
	CALL	FWORD PTR [ioentry]	; call intersegment indirect 16:32
	POP	EBX
	RET
c_outl	ENDP

; performs fast input of a byte from an I/O port 
; this routine is intended to be called from gcc C code
;
; Calling convention:
;	char c_inb(short port)
;
;
	ALIGN	04H
	PUBLIC	c_inb
c_inb	PROC
	MOV	EDX, DWORD PTR [ESP+4]	; get port number
	PUSH	EBX			; save register
	MOV	EBX, 1			; function code 1 = read byte
	CALL	FWORD PTR [ioentry]	; call intersegment indirect 16:32
	AND	EAX, 000000ffH		; mask out required byte
	POP	EBX			; restore register
	RET
c_inb	ENDP

; performs fast input of a word from an I/O port 
; this routine is intended to be called from gcc C code
;
; Calling convention:
;	short c_inw(short port)
;
;
	ALIGN	04H
	PUBLIC	c_inw
c_inw	PROC
	MOV	EDX, DWORD PTR [ESP+4]	; get port number
	PUSH	EBX			; save register
	MOV	EBX, 2			; function code 2 = read short
	CALL	FWORD PTR [ioentry]	; call intersegment indirect 16:32
	AND	EAX, 0000ffffH		; mask out required byte
	POP	EBX			; restore register
	RET
c_inw	ENDP

; performs fast input of a dword from an I/O port 
; this routine is intended to be called from gcc C code
;
; Calling convention:
;	long c_inl(short port)
;
;
	ALIGN	04H
	PUBLIC	c_inl
c_inl	PROC
	MOV	EDX, DWORD PTR [ESP+4]	; get port number
	PUSH	EBX			; save register
	MOV	EBX, 3			; function code 3 = read long
	CALL	FWORD PTR [ioentry]	; call intersegment indirect 16:32
	POP	EBX			; restore register
	RET
c_inl	ENDP


;------------------------------------------------------------------------------

; performs fast output of a byte to an I/O port 
; this routine is intended to be called from assembler code
; note there is no stack frame, however 8 byte stack space is required
;
; calling convention:
;	MOV	EDX, portnr 
;	MOV	AL, data
;	CALL	a_outb
;
;
	ALIGN	04H
	PUBLIC	a_outb
a_outb	PROC
	PUSH	EBX			; save register
	MOV	EBX, 4			; function code 4 = write byte
	CALL	FWORD PTR [ioentry]	; call intersegment indirect 16:32
	POP	EBX			; restore bx
	RET
a_outb	ENDP

; performs fast output of a word to an I/O port 
; this routine is intended to be called from assembler code
; note there is no stack frame, however 8 byte stack space is required
;
; calling convention:
;	MOV	EDX, portnr 
;	MOV	AX, data
;	CALL	a_outw
;
;
	ALIGN	04H
	PUBLIC	a_outw
a_outw	PROC
	PUSH	EBX			; save register
	MOV	EBX, 5			; function code 5 = write word
	CALL	FWORD PTR [ioentry]	; call intersegment indirect 16:32
	POP	EBX			; restore bx
	RET
a_outw	ENDP

; performs fast output of a long to an I/O port 
; this routine is intended to be called from assembler code
; note there is no stack frame, however 8 byte stack space is required
;
; calling convention:
;	MOV	EDX, portnr 
;	MOV	EAX, data
;	CALL	a_outl
;
;
	ALIGN	04H
	PUBLIC	a_outl
a_outl	PROC
	PUSH	EBX			; save register
	MOV	EBX, 6			; function code 6 = write long
	CALL	FWORD PTR [ioentry]	; call intersegment indirect 16:32
	POP	EBX			; restore bx
	RET
a_outl	ENDP

; performs fast input of a byte from an I/O port 
; this routine is intended to be called from assembler code
; note there is no stack frame, however 8 byte stack space is required
;
; calling convention:
;	MOV	EDX, portnr 
;	CALL	a_inb
;	;data in AL
;
	ALIGN	04H
	PUBLIC	a_inb
a_inb	PROC
	PUSH	EBX			; save register
	MOV	EBX, 1			; function code 1 = read byte
	CALL	FWORD PTR [ioentry]	; call intersegment indirect 16:32
	AND	EAX, 000000FFh		; mask byte
	POP	EBX			; restore register
	RET
a_inb	ENDP


; performs fast input of a word from an I/O port 
; this routine is intended to be called from assembler code
; note there is no stack frame, however 8 byte stack space is required
;
; calling convention:
;	MOV	EDX, portnr 
;	CALL	a_inb
;	;data in AX
;
	ALIGN	04H
	PUBLIC	a_inw
a_inw	PROC
	PUSH	EBX			; save register
	MOV	EBX, 2			; function code 2 = read word
	CALL	FWORD PTR [ioentry]	; call intersegment indirect 16:32
	AND	EAX, 0000FFFFh		; mask byte
	POP	EBX			; restore register
	RET
a_inw	ENDP

; performs fast input of a dword from an I/O port 
; this routine is intended to be called from assembler code
; note there is no stack frame, however 8 byte stack space is required
;
; calling convention:
;	MOV	EDX, portnr 
;	CALL	a_inl
;	;data in EAX
;
	ALIGN	04H
	PUBLIC	a_inl
a_inl	PROC
	PUSH	EBX			; save register
	MOV	EBX, 3			; function code 3 = read dword
	CALL	FWORD PTR [ioentry]	; call intersegment indirect 16:32
	POP	EBX			; restore register
	RET
a_inl	ENDP

CODE32	ENDS

;------------------------------------------------------------------------------

; Initialize I/O access via the driver. 
; You *must* call this routine once for each executable that wants to do
; I/O.
;
; The routine is mainly equivalent to a C routine performing the 
; following (but no need to add another file):
;	DosOpen("/dev/fastio$", read, nonexclusive)
;	DosDevIOCtl(device, XFREE86_IO, IO_GETSEL32)
;	selector -> ioentry+4
;	DosClose(device)
;
; Calling convention:
;	int io_init(void)
; Return:
;	0 if successful
;	standard APIRET return code if error
;

CONST32	SEGMENT
	ALIGN	04H
devname:
	DB	"/dev/fastio$",0
CONST32	ENDS

CODE32	SEGMENT
	PUBLIC	io_init
	EXTRN	DosOpen:PROC
	EXTRN	DosClose:PROC
	EXTRN	DosDevIOCtl:PROC
io_init	PROC
	PUSH	EBP
	MOV	EBP, ESP	; standard stack frame
	SUB	ESP, 16		; reserve memory
				; -16 = len arg of DosDevIOCtl
				; -12 = action arg of DosOpen
				; -8 = fd arg of DosOpen
				; -2 = short GDT selector arg
	PUSH	0		; (PEAOP2)NULL
	PUSH	66		; OPEN_ACCESS_READWRITE|OPEN_SHARE_DENYNONE
	PUSH	1		; FILE_OPEN
	PUSH	0		; FILE_NORMAL
	PUSH	0		; initial size
	LEA	EAX, [EBP-12]	; Adress of 'action' arg
	PUSH	EAX
	LEA	EAX, [EBP-8]	; Address of 'fd' arg
	PUSH	EAX
	PUSH	OFFSET devname
	CALL	DosOpen		; call DosOpen
	ADD	ESP, 32		; cleanup stack frame
	CMP	EAX, 0		; is return code zero?
	JE	goon		; yes, proceed
	LEAVE			; no return error
	RET

	ALIGN	04H
goon:
	LEA	EAX, [EBP-16]	; address of 'len' arg of DosDevIOCtl
	PUSH	EAX
	PUSH	2		; sizeof(short)
	LEA	EAX, [EBP-2]	; address to return the GDT selector
	PUSH	EAX
	PUSH	0		; no parameter len
	PUSH	0		; no parameter size
	PUSH	0		; no parameter address
	PUSH	100		; function code IO_GETSEL32
	PUSH	118		; category code XFREE6_IO
	MOV	EAX, [EBP-8]	; file handle
	PUSH	EAX
	CALL	DosDevIOCtl	; perform ioctl
	ADD	ESP, 36		; cleanup stack
	CMP	EAX, 0		; is return code = 0?
	JE	ok		; yes, proceed
	PUSH	EAX		; was error, save error code
	MOV	EAX, [EBP-8]	; file handle
	PUSH	EAX
	CALL	DosClose	; close device
	ADD	ESP, 4		; clean stack
	POP	EAX		; get error code
	LEAVE			; return error
	RET
	
	ALIGN	04H
ok:
	MOV	EAX,[EBP-8]	; file handle
	PUSH	EAX		; do normal close
	CALL	DosClose
	ADD	ESP, 4		; clean stack

	MOV	AX, WORD PTR [EBP-2]	; load gdt selector
	MOV	gdt, AX		; store in ioentry address selector part
	XOR	EAX, EAX	; eax = 0
	MOV	DWORD PTR [ioentry], EAX ; clear ioentry offset part
				; return code = 0 (in %eax)
	LEAVE			; clean stack frame
	RET			; exit
io_init	ENDP

; just for symmetry, does nothing

	ALIGN	04H
	PUBLIC	io_exit
io_exit	PROC
	XOR	EAX,EAX
	RET
io_exit	ENDP

	ALIGN	04H
	PUBLIC	int03

int03	PROC
	INT	3
	RET
int03	ENDP

CODE32	ENDS
	END
