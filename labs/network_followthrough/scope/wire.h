#ifndef CMASTERY_NETWORK_SCOPE_WIRE_H
#define CMASTERY_NETWORK_SCOPE_WIRE_H
#include "scope.h"
/* Established, configured peer data path only: fixed IPv4/TCP20-byte headers,
 * ACK or PSH|ACK, exact next sequence, no SYN/FIN/RST/options/reassembly.
 * A complete TCB must validate ACK/window/control state before this adapter.
 * Borrowed Ethernet frame excludes FCS. BUSY retains sequence and queue. */
int nw_ingress(NwRx *, uint32_t *next_sequence, uint16_t peer_port,
               const unsigned char *frame, size_t length);
#endif
