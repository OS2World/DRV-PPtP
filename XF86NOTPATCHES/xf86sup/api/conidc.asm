; **********************************************************************
; Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de)
; Use at your own risk! No Warranty! The author is not responsible for
; any damage or loss of data caused by proper or improper use of this
; device driver.
; **********************************************************************
;
; compile with masm 6.00a
;
; Attn user: this is only for device driver writers.
; You cannot run this code from a user program.
; Use this API to send a message to the console$ driver.
; Of course, the console$ driver as contained in xf86sup.sys needs to be 
; loaded already.
;
; In your code, issue a cons_idc_init(void) once first.
; After that, you may send a character string to console
; Pass a correctly filled packet of type 'struct conidcpkt' to cons_idc_send.
; The buf member of the structure must be storage owned of your own driver.
; If, for some reason, one of the calls fails, nothing will be sent without
; notice.
; Warning: if you send junk data, you may crash the system.
;
; You probably need to modify this file to adapt it to your Device_Help
; entry point. Look for two lines starting with $$$ for changes.
;
;
	PAGE	80,132
	.286p
	TITLE   conidc - API to send message to console$ driver
	NAME	conidc

.XLIST
	INCLUDE devhlp.inc
	INCLUDE devsym.inc
	INCLUDE error.inc
.LIST

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

_DATA		segment
consname	db	"CONSOLE$"
consattach	dw	0,0,0	; attach area, real mode entry (obsolete)
considc		dd	0	; IDC entry point
considcds	dw	0	; IDC DS

		extrn	_Device_Help:dword

_DATA		ends

_TEXT		segment

;
; $$$ Attn: Adjust the following symbol!!!

		public	_cons_idc_init
		public	_cons_idc_send

		assume	cs:_TEXT,ds:DGROUP, es:NOTHING

; initialize IDC to the console device
; call this from C with cons_idc_init();
;
_cons_idc_init	proc	far
		push	bp
		mov	bp,sp
		push	bx
		push	di
		mov	ax, word ptr considc
		or	ax, word ptr considc+2
		or	ax, word ptr considcds
		jnz	ok19		; already set

		mov	bx, offset consname		
		mov	di, offset consattach
		mov	dl, DevHlp_AttachDD
;
; $$$ Attn: Adjust the following call!
		call	ds:[_Device_Help]
		; sets the attach area we use for calling
ok19:		pop	di
		pop	bx		
		pop	bp
		ret
_cons_idc_init	endp

; send a message to the console driver
; int cons_idc_send(CONIDC data)
;
		.386
_cons_idc_send	proc	far
		push	bp
		mov	bp,sp
		push	ds
		push	es
		push	eax
		mov	ax, word ptr considc
		or	ax, word ptr considc+2
		or	ax, word ptr considcds
		jz	err20		; not init: don't send anything

		mov	eax, dword ptr [bp+6]
		push	eax
		push	ds
		pop	es
		mov	ax, considcds
		mov	ds, ax
		call	es:[considc]
		add	sp, 4
		mov	ax, 08100h		; return RPDONE if ok
err20:		pop	eax
		pop	es
		pop	ds
		pop	bp
		ret
_cons_idc_send	endp

_TEXT		ends
		end
