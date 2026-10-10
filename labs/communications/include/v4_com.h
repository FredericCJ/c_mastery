#ifndef CMASTERY_V4_COM_H
#define CMASTERY_V4_COM_H
#include "com_edu7.h"
#include "com_net.h"
typedef struct {
  EduRx rx;
  uint64_t last, due;
  unsigned granted;
} V4Reassembly;
/* grant is explicit link-service authority; 100-tick absolute assembly lifetime
   starts with the first accepted fragment and is not renewed by duplicates.
   Timeout discards only incomplete
   state, never a delivered message or duplicate-suppression identity. */
int v4_reassembly(V4Reassembly *, const EduFragment *, EduFragment *,
                  uint64_t now, unsigned grant);
typedef struct {
  uint32_t peer;
  uint8_t bytes[64];
  size_t length;
  uint64_t due;
  unsigned pending, retries, resolved;
} V4Arp;
int v4_arp_queue(V4Arp *, uint32_t, const uint8_t *, size_t, uint64_t);
/* 1 ARP request,2 retained payload ready,-1 exhausted,0 waiting. */
int v4_arp_poll(V4Arp *, uint64_t);
int v4_arp_resolve(V4Arp *, uint32_t);
typedef int (*V4PacketSink)(void *, const NetPacket *);
int v4_tcp_delivery(NetStack *, uint64_t, V4PacketSink, void *,
                    unsigned *drops);
/* Abstract M_CAN driver adapter: offsets are educational register IDs.
   Actual H745 message-RAM reachability/cache/clock and cable evidence remain.
 */
typedef struct {
  void *ctx;
  uint32_t (*read)(void *, unsigned);
  void (*write)(void *, unsigned, uint32_t);
} V4CanIo;
int v4_can_init(V4CanIo, uint32_t nominal, uint32_t data, unsigned polls);
int v4_can_loopback(V4CanIo, const uint8_t *, size_t, uint8_t *, size_t,
                    unsigned polls);
#endif
