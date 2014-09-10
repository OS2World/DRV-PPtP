	.file	"gccio"

.data
ioentry:
	.long	0
gdt:
	.word	0

	.text
# performs fast output of a byte to an I/O port 
# this routine is intended to be called from gcc C code
#
# Calling convention:
#	void _c_outb(short port,char data)
#
#
	.globl	_c_outb
	.align	4, 0x90
_c_outb:
	movl	4(%esp), %edx		# get port
	movb	8(%esp), %al		# get data
	pushl	%ebx			# save register
	movw	$4, %bx			# function code 4 = write byte
	.byte	0xff, 0x1d		# call intersegment indirect 16:32
	.long	ioentry
	popl	%ebx			# restore bx
	ret


# performs fast output of a word to an I/O port 
# this routine is intended to be called from gcc C code
#
# Calling convention:
#	void _c_outw(short port,short data)
#
#
	.globl	_c_outw
	.align	4, 0x90
_c_outw:
	movl	4(%esp), %edx		# get port
	movw	8(%esp), %ax		# get data
	pushl	%ebx			# save register
	movw	$5, %bx			# function code 5 = write word
	.byte	0xff, 0x1d		# call intersegment indirect 16:32
	.long	ioentry
	popl	%ebx			# restore bx
	ret

# performs fast output of a longword to an I/O port 
# this routine is intended to be called from gcc C code
#
# Calling convention:
#	void _c_outl(short port,long data)
#
#
	.globl	_c_outl
	.align	4, 0x90
_c_outl:
	movl	4(%esp), %edx		# get port
	movl	8(%esp), %eax		# get data
	pushl	%ebx			# save register
	movw	$6, %bx			# function code 6 = write longword
	.byte	0xff, 0x1d		# call intersegment indirect 16:32
	.long	ioentry
	popl	%ebx			# restore bx
	ret

# performs fast input of a byte from an I/O port 
# this routine is intended to be called from gcc C code
#
# Calling convention:
#	char _c_inb(short port)
#
#
	.globl _c_inb
	.align	4, 0x90
_c_inb:
	movl	4(%esp), %edx		# get port
	pushl	%ebx			# save register
	movw	$1, %bx			# function code 1 = read byte
	.byte	0xff, 0x1d		# call intersegment indirect 16:32
	.long	ioentry
	andl	$0xff, %eax		# mask out required byte
	popl	%ebx			# restore register
	ret

# performs fast input of a word from an I/O port 
# this routine is intended to be called from gcc C code
#
# Calling convention:
#	short _c_inw(short port)
#
#
	.globl _c_inw
	.align	4, 0x90
_c_inw:
	movl	4(%esp), %edx		# get port
	pushl	%ebx			# save register
	movw	$2, %bx			# function code 2 = read word
	.byte	0xff, 0x1d		# call intersegment indirect 16:32
	.long	ioentry
	andl	$0xffff,%eax		# mask out word
	popl	%ebx			# restore register
	ret

# performs fast input of a longword from an I/O port 
# this routine is intended to be called from gcc C code
#
# Calling convention:
#	lomg _c_inl(short port)
#
#
	.globl _c_inl
	.align	4, 0x90
_c_inl:
	movl	4(%esp), %edx		# get port
	pushl	%ebx			# save register
	movw	$3, %bx			# function code 3 = read longword
	.byte	0xff, 0x1d		# call intersegment indirect 16:32
	.long	ioentry
	popl	%ebx			# restore register
	ret


#------------------------------------------------------------------------------

# performs fast output of a byte to an I/O port 
# this routine is intended to be called from gas assembler code
# note there is no stack frame, however 8 byte stack space is required
#
# calling convention:
#	movl	$portnr, %edx 
#	movb	$data %al
#	call	a_outb
#
#
	.globl	a_outb
	.align	4, 0x90
a_outb:
	pushl	%ebx			# save register
	movw	$4, %bx			# function code 4 = write byte
	.byte	0xff, 0x1d		# call intersegment indirect 16:32
	.long	ioentry
	popl	%ebx			# restore bx
	ret

# performs fast output of a word to an I/O port 
# this routine is intended to be called from gas assembler code
# note there is no stack frame, however 8 byte stack space is required
#
# calling convention:
#	movl	$portnr, %edx 
#	movw	$data %ax
#	call	a_outw
#
#
	.globl	a_outw
	.align	4, 0x90
a_outw:
	pushl	%ebx			# save register
	movw	$5, %bx			# function code 5 = write word
	.byte	0xff, 0x1d		# call intersegment indirect 16:32
	.long	ioentry
	popl	%ebx			# restore bx
	ret

# performs fast output of a longword to an I/O port 
# this routine is intended to be called from gas assembler code
# note there is no stack frame, however 8 byte stack space is required
#
# calling convention:
#	movl	$portnr, %edx 
#	movl	$data %eax
#	call	a_outl
#
#
	.globl	a_outl
	.align	4, 0x90
a_outl:
	pushl	%ebx			# save register
	movw	$6, %bx			# function code 6 = write longword
	.byte	0xff, 0x1d		# call intersegment indirect 16:32
	.long	ioentry
	popl	%ebx			# restore bx
	ret

# performs fast input of a byte from an I/O port 
# this routine is intended to be called from gas assembler code
# note there is no stack frame, however 8 byte stack space is required
#
# calling convention:
#	movl	$portnr, %edx 
#	call	a_inb
#	#data in %al
#
	.globl a_inb
	.align	4, 0x90
a_inb:
	pushl	%ebx			# save register
	movw	$1, %bx			# function code 1 = read byte
	.byte	0xff, 0x1d		# call intersegment indirect 16:32
	.long	ioentry
	andl	$0xff, %eax		# mask byte
	popl	%ebx			# restore register
	leave				# return
	ret

# performs fast input of a word from an I/O port 
# this routine is intended to be called from gas assembler code
# note there is no stack frame, however 8 byte stack space is required
#
# calling convention:
#	movl	$portnr, %edx 
#	call	a_inw
#	#data in %ax
#
	.globl a_inw
	.align	4, 0x90
a_inw:
	pushl	%ebx			# save register
	movw	$2, %bx			# function code 1 = read word
	.byte	0xff, 0x1d		# call intersegment indirect 16:32
	.long	ioentry
	andl	$0xffff, %eax		# mask word
	popl	%ebx			# restore register
	leave				# return
	ret

# performs fast input of a longword from an I/O port 
# this routine is intended to be called from gas assembler code
# note there is no stack frame, however 8 byte stack space is required
#
# calling convention:
#	movl	$portnr, %edx 
#	call	a_inl
#	#data in %eax
#
	.globl a_inl
	.align	4, 0x90
a_inl:
	pushl	%ebx			# save register
	movw	$3, %bx			# function code 1 = read byte
	.byte	0xff, 0x1d		# call intersegment indirect 16:32
	.long	ioentry
	popl	%ebx			# restore register
	leave				# return
	ret

#------------------------------------------------------------------------------

# Initialize I/O access via the driver. 
# You *must* call this routine once for each executable that wants to do
# I/O.
#
# The routine is mainly equivalent to a C routine performing the 
# following (but no need to add another file):
#	DosOpen("/dev/fastio$", read, nonexclusive)
#	DosDevIOCtl(device, XFREE86_IO, IO_GETSEL32)
#	selector -> ioentry+4
#	DosClose(device)
#
# Calling convention:
#	int io_init(void)
# Return:
#	0 if successful
#	standard APIRET return code if error
#
	.text
	.align	4, 0x90
devname:
	.ascii	"/dev/fastio$\0"

	.globl	_io_init
	.align	4, 0x90
_io_init:
	pushl	%ebp
	movl	%esp, %ebp	# standard stack frame
	subl	$16, %esp	# reserve memory
				# -16 = len arg of DosDevIOCtl
				# -12 = action arg of DosOpen
				# -8 = fd arg of DosOpen
				# -2 = short GDT selector arg
	pushl	$0		# (PEAOP2)NULL
	pushl	$66		# OPEN_ACCESS_READWRITE|OPEN_SHARE_DENYNONE
	pushl	$1		# FILE_OPEN
	pushl	$0		# FILE_NORMAL
	pushl	$0		# initial size
	leal	-12(%ebp), %eax	# Adress of 'action' arg
	pushl	%eax
	leal	-8(%ebp), %eax	# Address of 'fd' arg
	pushl	%eax
	pushl	$devname
	call	_DosOpen	# call DosOpen
	addl	$32, %esp	# cleanup stack frame
	cmpl	$0, %eax	# is return code zero?
	je	goon		# yes, proceed
	leave			# no return error
	ret
	.align	4,0x90
goon:
	leal	-16(%ebp), %eax	# address of 'len' arg of DosDevIOCtl
	pushl	%eax
	pushl	$2		# sizeof(short)
	leal	-2(%ebp), %eax	# address to return the GDT selector
	pushl	%eax
	pushl	$0		# no parameter len
	pushl	$0		# no parameter size
	pushl	$0		# no parameter address
	pushl	$100		# function code IO_GETSEL32
	pushl	$118		# category code XFREE6_IO
	movl	-8(%ebp), %eax	# file handle
	pushl	%eax
	call	_DosDevIOCtl	# perform ioctl
	addl	$36, %esp	# cleanup stack
	cmpl	$0, %eax	# is return code = 0?
	je	ok		# yes, proceed
	pushl	%eax		# was error, save error code
	movl	-8(%ebp), %eax	# file handle
	pushl	%eax
	call	_DosClose	# close device
	addl	$4,%esp		# clean stack
	popl	%eax		# get error code
	leave			# return error
	ret
	.align	4,0x90
ok:
	movl	-8(%ebp), %eax	# file handle
	pushl	%eax		# do normal close
	call	_DosClose
	addl	$4, %esp	# clean stack

	movw	-2(%ebp), %ax	# load gdt selector
	movw	%ax, gdt	# store in ioentry address selector part
	xorl	%eax, %eax	# eax = 0
	movl	%eax, ioentry	# clear ioentry offset part
				# return code = 0 (in %eax)
	leave			# clean stack frame
	ret			# exit

# just for symmetry, does nothing

	.globl	_io_exit
	.align	4, 0x90
_io_exit:
	xorl	%eax,%eax
	ret

	.globl	_int03
_int03:
	int3
	ret
