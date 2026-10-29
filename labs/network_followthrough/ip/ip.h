#ifndef NI_IP_H
#define NI_IP_H
#include "systems.h"
#include <stddef.h>
#include <stdint.h>
enum { NI_OK, NI_WAIT, NI_INVALID, NI_BUSY, NI_CONFLICT, NI_STUB };
enum { NI_FREE, NI_ACTIVE, NI_READY, NI_POISON };
enum { NI_STRICT, NI_IDENTICAL };
struct ni_config {
  uint32_t local, peer;
};
struct ni_view {
  const unsigned char *header, *payload;
  size_t length;
  uint32_t source, destination;
  unsigned id, offset, more, protocol;
};
struct ni_packet {
  unsigned char bytes[1500];
  size_t length;
};
struct ni_slot {
  uint32_t source, destination;
  unsigned id, protocol, state;
  uint64_t expiry;
  unsigned total, have_last, first, covered, high, nonfinal;
  unsigned char header[20], bytes[1480], bitmap[185];
};
struct ni_reassembly {
  struct ni_config config;
  uint64_t last, timeout;
  unsigned slots, policy;
  struct ni_slot slot[2];
};
/* Valid spans do not overlap output or state. All calls externally serialized.
 * Configuration is immutable during a session. See CONTRACTS.md. */
int ni_parse(const struct ni_config *, const unsigned char *, size_t, unsigned,
             struct ni_view *);
int ni_echo(const struct ni_config *, const unsigned char *, size_t, struct ni_packet *);
int ni_init(struct ni_reassembly *, struct ni_config, unsigned, unsigned, uint64_t, uint64_t);
int ni_tick(struct ni_reassembly *, uint64_t);
/* Active wire adapter uses expire, not tick: copies up to two timeout reports.
 * Caller supplies two disjoint packets; count is published last. */
int ni_expire(struct ni_reassembly *, uint64_t, struct ni_packet[2], unsigned *);
int ni_receive(struct ni_reassembly *, const unsigned char *, size_t, uint64_t,
               unsigned *);
int ni_take(struct ni_reassembly *, unsigned, struct ni_packet *);
#endif
