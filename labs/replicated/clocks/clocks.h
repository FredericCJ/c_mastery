#ifndef CMASTERY_REPLICATED_CLOCKS_H
#define CMASTERY_REPLICATED_CLOCKS_H
#include "replicated.h"
#include "store.h"
enum RpcStatus {
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
enum RpcPolicy { RPC_REJECT, RPC_WAIT_FOR_WALL };
typedef struct {
  uint64_t identity, value;
} RpcEntry;
typedef struct {
  RpcEntry entries[6];
  unsigned count;
} RpcVector;
typedef struct {
  uint64_t uuid, self, lamport;
  RPHLC hlc;
  RpcVector vector;
} RpcClock;
/* All controls/outputs are live/disjoint and externally serialized. Identities
 * are globally unique within the provisioned nonzero UUID. Replacement keeps
 * all historical components; it does not recycle a vector slot. A received
 * clock is trusted causal input, not an authenticated peer certificate. */
int rpc_make(uint64_t uuid, uint64_t identity, RpcClock *out);
int rpc_step(const RpcClock *local, const RpcClock *remote, uint64_t wall,
             uint64_t maximum_ahead, unsigned policy, RpcClock *out);
int rpc_replace(const RpcClock *local, uint64_t fresh_identity, RpcClock *out);
int rpc_order(const RpcVector *a, const RpcVector *b, int *out);
int rpc_encode(const RpcClock *clock, unsigned char out[160]);
int rpc_decode(uint64_t uuid, const unsigned char in[160], RpcClock *out);
/* Exclusive already-open clock store; no escaped clock is published before
 * its durable transition. Errors may persist the new record and poison pages;
 * reopen/load before retry. A successful flush is the durability premise. */
int rpc_save(store *pages, const RpcClock *clock);
int rpc_load(store *pages, uint64_t uuid, RpcClock *out);
int rpc_step_durable(store *pages, const RpcClock *local,
                     const RpcClock *remote, uint64_t wall,
                     uint64_t maximum_ahead, unsigned policy, RpcClock *out);
int rpc_replace_durable(store *pages, const RpcClock *local,
                        uint64_t fresh_identity, RpcClock *out);
#endif
