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
#	void _c_outb1(short port,char data)
#
#
	.globl	_c_outb1
	.align	4, 0x90
_c_outb1:
	movl	4(%esp), %edx		# get port
	movb	8(%esp), %al		# get data
	outb	%al,%dx
	ret


# performs fast output of a word to an I/O port 
# this routine is intended to be called from gcc C code
#
# Calling convention:
#	void _c_outw1(short port,short data)
#
#
	.globl	_c_outw1
	.align	4, 0x90
_c_outw1:
	movl	4(%esp), %edx		# get port
	movw	8(%esp), %ax		# get data
	outw	%ax,%dx
	ret

# performs fast output of a longword to an I/O port 
# this routine is intended to be called from gcc C code
#
# Calling convention:
#	void _c_outl1(short port,long data)
#
#
	.globl	_c_outl1
	.align	4, 0x90
_c_outl1:
	movl	4(%esp), %edx		# get port
	movl	8(%esp), %eax		# get data
	outl	%eax,%dx
	ret

# performs fast input of a byte from an I/O port 
# this routine is intended to be called from gcc C code
#
# Calling convention:
#	char _c_inb1(short port)
#
#
	.globl _c_inb1
	.align	4, 0x90
_c_inb1:
	movl	4(%esp), %edx		# get port
	inb	%dx,%al
	andl	$0xff, %eax
	ret

# performs fast input of a word from an I/O port 
# this routine is intended to be called from gcc C code
#
# Calling convention:
#	short _c_inw1(short port)
#
#
	.globl _c_inw1
	.align	4, 0x90
_c_inw1:
	movl	4(%esp), %edx		# get port
	inw	%dx,%ax
	andl	$0xffff,%eax		# mask out word
	ret

# performs fast input of a longword from an I/O port 
# this routine is intended to be called from gcc C code
#
# Calling convention:
#	lomg _c_inl1(short port)
#
#
	.globl _c_inl1
	.align	4, 0x90
_c_inl1:
	movl	4(%esp), %edx		# get port
	inl	%dx,%eax
	ret

#------------------------------------------------------------------------------

# Initialize I/O access via the driver. 
# You *must* call this routine once for each *thread* that wants to do
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
#	int io_init1(void)
# Return:
#	0 if successful
#	standard APIRET return code if error
#
	.text
	.align	4, 0x90
devname:
	.ascii	"/dev/fastio$\0"

	.globl	_io_init1
	.align	4, 0x90
_io_init1:
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

				# now use this function to raise the IOPL
	movl	$13,%ebx	# special function code
	.byte	0xff, 0x1d	# call intersegment indirect 16:32
	.long	ioentry

	# thread should now be running at IOPL=3

	xorl	%eax,%eax	# return code = 0
	leave			# clean stack frame
	ret			# exit

	.text
	.globl	_io_exit1
	.align	4, 0x90
_io_exit1:
	push	%ebp
	movl	%esp, %ebp	# stackframe, I am accustomed to this :-)

	movw	gdt, %ax	# check if io_init was called once
	orw	%ax,%ax
	jz	exerr		# no gdt entry, so process cannot be at IOPL=3
				# through this mechanism

	movl	$14,%ebx	# function code to disable iopl
	.byte	0xff, 0x1d	# call intersegment indirect 16:32
	.long	ioentry
	
	# process should now be at IOPL=3 again
	xorl	%eax,%eax	# ok, return code = 0
	leave
	ret
exerr:	xorl	%eax, %eax	# not ok, return code = ffffffff
	decl	%eax
	leave
	ret

# for diagnostic only

	.globl	_psw
	.align	4, 0x90
_psw:	pushf			# get the current PSW
	popl	%eax		# into EAX
	ret
