#ifndef CMASTERY_REPLICATED_DELTAS_H
#define CMASTERY_REPLICATED_DELTAS_H
#include "replicated.h"
#include "store.h"

enum
{
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
struct rpd_set
{
    struct rp_set first, second;
};

struct rpd_state
{
    uint64_t uuid, epoch, sequence;
    unsigned self;
    struct rpd_set set;
};

struct rpd_mark
{
    uint64_t epoch, upto;
};

struct rpd_entry
{
    uint64_t sequence;
    struct rpd_set delta;
};

struct rpd_replica
{
    store *pages;
    struct rpd_state state;
    struct rpd_entry history[4];
    unsigned count;
    struct rpd_mark acked[3], sent[3], received[3];
};

struct rpd_packet
{
    uint64_t uuid, sender_epoch, target_epoch, start, end;
    unsigned sender, target, full;
    struct rpd_set body;
};

struct rpd_ack
{
    uint64_t uuid, from_epoch, to_epoch, upto;
    unsigned from, to;
};

int rpd_set_add(const struct rpd_set *state, unsigned writer, unsigned value,
                struct rpd_set *out, struct rpd_set *delta);
int rpd_set_remove(const struct rpd_set *state, unsigned value,
                   struct rpd_set *out, struct rpd_set *delta);
int rpd_set_join(const struct rpd_set *state, const struct rpd_set *delta,
                 struct rpd_set *out);
int rpd_contains(const struct rpd_set *state, unsigned value);
int rpd_encode(const struct rpd_state *state, unsigned char out[128]);
int rpd_decode(const unsigned char wire[128], struct rpd_state *out);
int rpd_create(store *pages, uint64_t uuid, unsigned self,
               struct rpd_replica *out);
/* Open increments and commits transport epoch before returning authority.
   Old controls are retired; CRDT tag counters and sequence never reset. */
int rpd_open(store *pages, uint64_t uuid, unsigned self,
             struct rpd_replica *out);
int rpd_local(struct rpd_replica *replica, unsigned add, unsigned value);
/* peer_epoch is supplied by an identified current peer session. */
int rpd_send(struct rpd_replica *replica, unsigned peer, uint64_t peer_epoch,
             struct rpd_packet *out);
int rpd_receive(struct rpd_replica *replica, const struct rpd_packet *packet,
                struct rpd_ack *out);
int rpd_ack(struct rpd_replica *replica, uint64_t current_peer_epoch,
            const struct rpd_ack *ack);
#endif
