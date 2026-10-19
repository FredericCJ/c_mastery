#ifndef CMASTERY_REPLICATED_DELTAS_H
#define CMASTERY_REPLICATED_DELTAS_H
#include "replicated.h"
#include "store.h"
enum {
  RPD_OK,
  RPD_INVALID,
  RPD_FULL,
  RPD_RANGE,
  RPD_STATE,
  RPD_IO,
  RPD_UNCERTAIN,
  RPD_CORRUPT,
  RPD_IDENTITY,
  RPD_STALE,
  RPD_CAUSAL,
  RPD_EMPTY,
  RPD_UNIMPLEMENTED
};
/* Writers0/1 use first's actual RPSet slots; writer2 uses second.slot0.
   second.slot1 is permanently unused. Tags never recycle. */
typedef struct {
  RPSet first, second;
} RpdSet;
typedef struct {
  uint64_t uuid, epoch, sequence;
  unsigned self;
  RpdSet set;
} RpdState;
typedef struct {
  uint64_t epoch, upto;
} RpdMark;
typedef struct {
  uint64_t sequence;
  RpdSet delta;
} RpdEntry;
typedef struct {
  store *pages;
  RpdState state;
  RpdEntry history[4];
  unsigned count;
  RpdMark acked[3], sent[3], received[3];
} RpdReplica;
typedef struct {
  uint64_t uuid, sender_epoch, target_epoch, start, end;
  unsigned sender, target, full;
  RpdSet body;
} RpdPacket;
typedef struct {
  uint64_t uuid, from_epoch, to_epoch, upto;
  unsigned from, to;
} RpdAck;
int rpd_set_add(const RpdSet *state, unsigned writer, unsigned value,
                RpdSet *out, RpdSet *delta);
int rpd_set_remove(const RpdSet *state, unsigned value, RpdSet *out,
                   RpdSet *delta);
int rpd_set_join(const RpdSet *state, const RpdSet *delta, RpdSet *out);
int rpd_contains(const RpdSet *state, unsigned value);
int rpd_encode(const RpdState *state, unsigned char out[128]);
int rpd_decode(const unsigned char wire[128], RpdState *out);
int rpd_create(store *pages, uint64_t uuid, unsigned self, RpdReplica *out);
/* Open increments and commits transport epoch before returning authority.
   Old controls are retired; CRDT tag counters and sequence never reset. */
int rpd_open(store *pages, uint64_t uuid, unsigned self, RpdReplica *out);
int rpd_local(RpdReplica *replica, unsigned add, unsigned value);
/* peer_epoch is supplied by an identified current peer session. */
int rpd_send(RpdReplica *replica, unsigned peer, uint64_t peer_epoch,
             RpdPacket *out);
int rpd_receive(RpdReplica *replica, const RpdPacket *packet, RpdAck *out);
int rpd_ack(RpdReplica *replica, uint64_t current_peer_epoch,
            const RpdAck *ack);
#endif
