/* pptp_gre.h -- encapsulate PPP in PPTP-GRE.
 *               Handle the IP Protocol 47 portion of PPTP.
 *               C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: pptp_gre.h,v 1.2 1997/12/14 06:49:37 cananian Exp $
 */

#include <db.h>

/*void pptp_gre_copy(u_int16_t call_id, u_int16_t peer_call_id,
		   char *pty, char *inetaddr);*/
void pptp_gre_copy(u_int16_t call_id, u_int16_t peer_call_id,
		   int pty_fd, struct in_addr inetaddr);
