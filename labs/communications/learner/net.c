#include "com_net.h"
#include <string.h>
/* Buildable exercises: every operation still needs implementation. */
uint16_t net_checksum(const void *v, size_t n, uint32_t sum) { return 0; }
void net_init(NetStack *s, uint32_t isn) {
  (void)isn;
  memset(s, 0, sizeof *s);
}
void net_link_down(NetStack *s) { return; }
void net_rtt(NetStack *s, uint32_t r) { return; }
int net_receive(NetStack *s, const void *v, size_t n, uint64_t now,
                NetPacket *o) {
  return -1;
}
int net_tick(NetStack *s, uint64_t now, NetPacket *o) { return -1; }
