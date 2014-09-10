/* ppp_fcs.h ... header file for PPP-HDLC FCS
 *               C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: ppp_fcs.h,v 1.2 1997/12/13 23:40:53 cananian Exp $
 */

#define PPPINITFCS16    0xffff  /* Initial FCS value */
#define PPPGOODFCS16    0xf0b8  /* Good final FCS value */

#include <db.h>

u_int16_t pppfcs16(u_int16_t fcs, void *cp, int len);
