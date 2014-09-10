/* Copyright (C) 1995, 1996 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

#ifndef _RP_PRIV_H_
#define _RP_PRIV_H_

typedef int (pascal far *FPFUNCTION)();
typedef void far *FPVOID;
typedef ULONG	PHYS;

struct rpkt
{
	UCHAR	rp_len;
	UCHAR	rp_unit;
	UCHAR	rp_cmd;
	USHORT	rp_stat;
	UCHAR	_reserved_[4];
	struct rpkt far* rp_link;
	union {
		UCHAR	buf[19];
		struct {
			UCHAR		unit;
			FPFUNCTION	devhlp;
			char far*	args;
			UCHAR		drv;
		} init;
		struct {
			UCHAR		unit;
			USHORT		cs;
			USHORT		ds;
			FPVOID		bpb;
		} initexit;
		struct {
			UCHAR		media;
			PHYS		buf;
			USHORT		cnt;
			ULONG		start;
			USHORT		sfnum;
		} rdwr;
		struct {
			UCHAR		buf;
		} ndr;
		struct {
			USHORT		sfnum;
		} opncls;
		struct {
			UCHAR		cat;
			UCHAR		func;
			FPVOID		param;
			FPVOID		data;
			USHORT		sfnum;
			USHORT		plen;
			USHORT		dlen;
		} ioctl2;
	} pk;
};
typedef struct rpkt far* RP;

#define RPLINK(rp) rp->rp_link
#define RPSTAT(rp) rp->rp_stat
#define RWCNT(rp) rp->pk.rdwr.cnt
#define RWBUF(rp) rp->pk.rdwr.buf
#define RWNBUF(rp) rp->pk.ndr.buf
#define IOCAT(rp) rp->pk.ioctl2.cat
#define IOFUNC(rp) rp->pk.ioctl2.func
#define IOPARAM(rp) rp->pk.ioctl2.param
#define IODATA(rp) rp->pk.ioctl2.data
#define IOPLEN(rp) rp->pk.ioctl2.plen
#define IODLEN(rp) rp->pk.ioctl2.dlen
#define IOSFN(rp) rp->pk.ioctl2.sfnum

#define RPERR	0x8000
#define RPDEV	0x4000
#define RPBUSY	0x0200
#define RPDONE	0x0100

#define RP_EBAD		RPDONE|RPERR|ERROR_I24_BAD_COMMAND
#define RP_EINVAL	RPDONE|RPERR|ERROR_I24_INVALID_PARAMETER
#define RP_EREAD	RPDONE|RPERR|ERROR_I24_READ_FAULT
#define RP_EINTR	RPDONE|RPERR|ERROR_I24_CHAR_CALL_INTERRUPTED
#define RP_EUSE		RPDONE|RPERR|ERROR_I24_DEVICE_IN_USE
#define RP_ERDY		RPDONE|RPERR|ERROR_I24_NOT_READY
#define RP_EGEN		RPDONE|RPERR|ERROR_I24_GEN_FAILURE
#define RP_EWRITE	RPDONE|RPERR|ERROR_I24_WRITE_FAULT

#endif
