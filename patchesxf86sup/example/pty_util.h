/* This file declares the pty utility functions, that were described in
 * Stevens' book Unix Network Programming. See pty_util.c for more details.
 */

#ifndef _PTY_UTIL_H_
#define _PTY_UTIL_H_

extern int readn(int,char*,int);
extern int writen(int,char*,int);

#endif
