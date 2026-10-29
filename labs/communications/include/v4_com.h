#ifndef CMASTERY_V4_COM_H
#define CMASTERY_V4_COM_H
#include "com_edu7.h"
#include "com_net.h"
struct v4_reassembly {
  struct edu_rx rx;
  uint64_t last, due;
  unsigned granted;
};
/* grant is explicit link-service authority; 100-tick absolute assembly lifetime
   starts with the first accepted fragment and is not renewed by duplicates.
   Timeout discards only incomplete
   state, never a delivered message or duplicate-suppression identity. */
int v4_reassembly(struct v4_reassembly *, const struct edu_fragment *, struct edu_fragment *,
                  uint64_t now, unsigned grant);
struct v4_arp {
  uint32_t peer;
  uint8_t bytes[64];
  size_t length;
  uint64_t due;
  unsigned pending, retries, resolved;
};
int v4_arp_queue(struct v4_arp *, uint32_t, const uint8_t *, size_t, uint64_t);
/* 1 ARP request,2 retained payload ready,-1 exhausted,0 waiting. */
int v4_arp_poll(struct v4_arp *, uint64_t);
int v4_arp_resolve(struct v4_arp *, uint32_t);
typedef int (*v4_packet_sink_fn)(void *, const struct net_packet *);
int v4_tcp_delivery(struct net_stack *, uint64_t, v4_packet_sink_fn, void *,
                    unsigned *drops);
/* Abstract M_CAN driver adapter: offsets are educational register IDs.
   Actual H745 message-RAM reachability/cache/clock and cable evidence remain.
 */
struct v4_can_io {
  void *ctx;
  uint32_t (*read)(void *, unsigned);
  void (*write)(void *, unsigned, uint32_t);
};
int v4_can_init(struct v4_can_io, uint32_t nominal, uint32_t data, unsigned polls);
int v4_can_loopback(struct v4_can_io, const uint8_t *, size_t, uint8_t *, size_t,
                    unsigned polls);
#endif
