#ifndef CMASTERY_REPLICATED_CLOCKS_H
#define CMASTERY_REPLICATED_CLOCKS_H
#include "replicated.h"
#include "store.h"
enum rpc_status {
  RPC_OK,
  RPC_INVALID,
  RPC_FULL,
  RPC_RANGE,
  RPC_IDENTITY,
  RPC_SKEW,
  RPC_WAIT,
  RPC_IO,
  RPC_UNCERTAIN,
  RPC_CORRUPT,
  RPC_STATE,
  RPC_UNIMPLEMENTED
};
enum rpc_policy { RPC_REJECT, RPC_WAIT_FOR_WALL };
struct rpc_entry {
  uint64_t identity, value;
};
struct rpc_vector {
  struct rpc_entry entries[6];
  unsigned count;
};
struct rpc_clock {
  uint64_t uuid, self, lamport;
  struct rp_hlc hlc;
  struct rpc_vector vector;
};
/* All controls/outputs are live/disjoint and externally serialized. Identities
 * are globally unique within the provisioned nonzero UUID. Replacement keeps
 * all historical components; it does not recycle a vector slot. A received
 * clock is trusted causal input, not an authenticated peer certificate. */
int rpc_make(uint64_t uuid, uint64_t identity, struct rpc_clock *out);
int rpc_step(const struct rpc_clock *local, const struct rpc_clock *remote, uint64_t wall,
             uint64_t maximum_ahead, unsigned policy, struct rpc_clock *out);
int rpc_replace(const struct rpc_clock *local, uint64_t fresh_identity, struct rpc_clock *out);
int rpc_order(const struct rpc_vector *a, const struct rpc_vector *b, int *out);
int rpc_encode(const struct rpc_clock *clock, unsigned char out[160]);
int rpc_decode(uint64_t uuid, const unsigned char in[160], struct rpc_clock *out);
/* Exclusive already-open clock store; no escaped clock is published before
 * its durable transition. Errors may persist the new record and poison pages;
 * reopen/load before retry. A successful flush is the durability premise. */
int rpc_save(store *pages, const struct rpc_clock *clock);
int rpc_load(store *pages, uint64_t uuid, struct rpc_clock *out);
int rpc_step_durable(store *pages, const struct rpc_clock *local,
                     const struct rpc_clock *remote, uint64_t wall,
                     uint64_t maximum_ahead, unsigned policy, struct rpc_clock *out);
int rpc_replace_durable(store *pages, const struct rpc_clock *local,
                        uint64_t fresh_identity, struct rpc_clock *out);
#endif
