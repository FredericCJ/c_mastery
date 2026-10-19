#ifndef CM_DIAGNOSTIC_HOST_H
#define CM_DIAGNOSTIC_HOST_H
#include "diagnostic.h"
/* POSIX host-only. Caller owns a connected nonblocking local datagram socket.
 */
int dg_udp_receive(int, DGSession *, DGMessage *);
#endif
