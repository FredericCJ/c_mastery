#ifndef CMASTERY_REPLICATED_DURABLE_H
#define CMASTERY_REPLICATED_DURABLE_H
#include "replicated.h"
#include "store.h"
#include <stddef.h>
#include <stdint.h>

enum rpl_status {
  RPL_OK,
  RPL_INVALID,
  RPL_FULL,
  RPL_IO,
  RPL_UNCERTAIN,
  RPL_CORRUPT,
  RPL_STATE,
  RPL_IDENTITY,
  RPL_UNIMPLEMENTED
};
enum rpl_kind { RPL_SET = 1, RPL_QUERY_RESULT = 2 };
struct rpl_event {
  unsigned position, key, kind;
  int32_t value;
};
struct rpl_log {
  uint64_t uuid;
  uint32_t schema;
  store *pages;
  struct rp_log memory;
  unsigned kinds[16], poisoned;
};
struct rpl_snapshot {
  unsigned position;
  uint32_t present;
  int32_t values[32];
};

/* A callback returns a positive accepted prefix <=n, -2 to retry, or <=0 to
 * terminate. call_limit bounds all invocations per page, including retries.
 * Every successful flush persists all prior accepted writes. On failure any
 * subset of accepted bytes may persist; callers must recover. Stable adapter
 * storage/context and external serialization are required. */
struct rpl_bytes {
  void *context;
  ptrdiff_t (*read)(void *, size_t, unsigned char *, size_t);
  ptrdiff_t (*write)(void *, size_t, const unsigned char *, size_t);
  int (*flush)(void *);
  size_t extent;
  unsigned call_limit;
};
struct rpl_pages {
  struct rpl_bytes bytes;
};
int rpl_pages(struct rpl_pages *adapter, struct rpl_bytes bytes, store_io *out);

/* Supplied outputs/controls/inputs are live and disjoint. A log exclusively
 * borrows an already-open store, initially empty for create; no external store
 * mutations while it is live. Controls and escaped HAMT roots may not be
 * copied/relocated. Open constructs an unpublished index arena before publish.
 * All callbacks must be nonreentrant and may not retain borrowed buffers.
 * Zero UUID/schema is invalid. Only sixteen events; schema equality is exact.
 */
int rpl_create(store *pages, uint64_t uuid, uint32_t schema, struct rpl_log *out);
int rpl_open(store *pages, uint64_t uuid, uint32_t schema, struct rpl_log *out);
int rpl_append(struct rpl_log *log, unsigned kind, unsigned key, int32_t value);
int rpl_get(const struct rpl_log *log, unsigned position, unsigned key, int32_t *out);
int rpl_event_encode(uint64_t uuid, uint32_t schema, struct rpl_event event,
                     unsigned char out[64]);
int rpl_event_decode(uint64_t uuid, uint32_t schema, const unsigned char in[64],
                     struct rpl_event *out);
int rpl_snapshot_save(struct rpl_log *log, unsigned position);
int rpl_snapshot_load(struct rpl_log *log, struct rpl_snapshot *out);
int rpl_state(struct rpl_log *log, unsigned use_snapshot, struct rpl_snapshot *out);
#endif
