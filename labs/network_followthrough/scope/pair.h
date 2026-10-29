#ifndef CMASTERY_NETWORK_SCOPE_PAIR_H
#define CMASTERY_NETWORK_SCOPE_PAIR_H
#include "com_net.h"
#include "scope.h"
/* Fixed same-link peer; two explicit remote ports. This adapter reuses the
 * historical packet stack, including its documented lower-send/window gaps.
 * Changing admission from one to two is not full TCP/IP conformance. */
struct nw_pair {
  struct net_stack flows[2];
  uint16_t ports[2];
  unsigned capacity;
  uint32_t epoch;
};
int nw_pair_init(struct nw_pair *, unsigned capacity, uint32_t epoch,
                 uint16_t first_port, uint16_t second_port);
int nw_pair_receive(struct nw_pair *, const void *, size_t, uint64_t, struct net_packet *);
#endif
