/* Copyright (C) 1995 by Holger Veit (Holger.Veit@gmd.de) */
/* Use at your own risk! No Warranty! The author is not responsible for
 * any damage or loss of data caused by proper or improper use of this
 * device driver.
 */

/* Attn user: this is only interesting for you if you are writing a
 * device driver. This is the API you can use in your own driver to
 * send a message to the /dev/console$ driver
 */

#ifndef _CONIDC_H_
#define _CONIDC_H_

struct conidcpkt
{
	char far* buf;
	int buflen;
};
typedef struct conidcpkt far* CONIDC;

/* call this once to establish a connection to the console */
void cons_idc_init(void);

/* send a string packet (see above) to the console
 * return RPDONE if ok.
 */
int cons_idc_send(CONIDC data);

#endif
