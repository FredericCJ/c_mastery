#ifndef NI_IP_H
#define NI_IP_H
#include "systems.h"
#include <stddef.h>
#include <stdint.h>
enum { NI_OK, NI_WAIT, NI_INVALID, NI_BUSY, NI_CONFLICT, NI_STUB };
enum { NI_FREE, NI_ACTIVE, NI_READY, NI_POISON };
enum { NI_STRICT, NI_IDENTICAL };
typedef struct {
  uint32_t local, peer;
} NiConfig;
typedef struct {
  const unsigned char *header, *payload;
  size_t length;
  uint32_t source, destination;
  unsigned id, offset, more, protocol;
} NiView;
typedef struct {
  unsigned char bytes[1500];
  size_t length;
} NiPacket;
typedef struct {
  uint32_t source, destination;
  unsigned id, protocol, state;
  uint64_t expiry;
  unsigned total, have_last, first, covered, high, nonfinal;
  unsigned char header[20], bytes[1480], bitmap[185];
} NiSlot;
typedef struct {
  NiConfig config;
  uint64_t last, timeout;
  unsigned slots, policy;
  NiSlot slot[2];
} NiReassembly;
/* Valid spans do not overlap output or state. All calls externally serialized.
 * Configuration is immutable during a session. See CONTRACTS.md. */
int ni_parse(const NiConfig *, const unsigned char *, size_t, unsigned,
             NiView *);
int ni_echo(const NiConfig *, const unsigned char *, size_t, NiPacket *);
int ni_init(NiReassembly *, NiConfig, unsigned, unsigned, uint64_t, uint64_t);
int ni_tick(NiReassembly *, uint64_t);
/* Active wire adapter uses expire, not tick: copies up to two timeout reports.
 * Caller supplies two disjoint packets; count is published last. */
int ni_expire(NiReassembly *, uint64_t, NiPacket[2], unsigned *);
int ni_receive(NiReassembly *, const unsigned char *, size_t, uint64_t,
               unsigned *);
int ni_take(NiReassembly *, unsigned, NiPacket *);
#endif
