#ifndef CMASTERY_NETWORK_SCOPE_PAIR_H
#define CMASTERY_NETWORK_SCOPE_PAIR_H
#include "com_net.h"
#include "scope.h"
/* Fixed same-link peer; two explicit remote ports. This adapter reuses the
 * historical packet stack, including its documented lower-send/window gaps.
 * Changing admission from one to two is not full TCP/IP conformance. */
typedef struct {
  NetStack flows[2];
  uint16_t ports[2];
  unsigned capacity;
  uint32_t epoch;
} NwPair;
int nw_pair_init(NwPair *, unsigned capacity, uint32_t epoch,
                 uint16_t first_port, uint16_t second_port);
int nw_pair_receive(NwPair *, const void *, size_t, uint64_t, NetPacket *);
#endif
