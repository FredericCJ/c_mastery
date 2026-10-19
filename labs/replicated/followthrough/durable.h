#ifndef CMASTERY_REPLICATED_DURABLE_H
#define CMASTERY_REPLICATED_DURABLE_H
#include "replicated.h"
#include "store.h"
#include <stddef.h>
#include <stdint.h>

enum RplStatus {
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
enum RplKind { RPL_SET = 1, RPL_QUERY_RESULT = 2 };
typedef struct {
  unsigned position, key, kind;
  int32_t value;
} RplEvent;
typedef struct {
  uint64_t uuid;
  uint32_t schema;
  store *pages;
  RPLog memory;
  unsigned kinds[16], poisoned;
} RplLog;
typedef struct {
  unsigned position;
  uint32_t present;
  int32_t values[32];
} RplSnapshot;

/* A callback returns a positive accepted prefix <=n, -2 to retry, or <=0 to
 * terminate. call_limit bounds all invocations per page, including retries.
 * Every successful flush persists all prior accepted writes. On failure any
 * subset of accepted bytes may persist; callers must recover. Stable adapter
 * storage/context and external serialization are required. */
typedef struct {
  void *context;
  ptrdiff_t (*read)(void *, size_t, unsigned char *, size_t);
  ptrdiff_t (*write)(void *, size_t, const unsigned char *, size_t);
  int (*flush)(void *);
  size_t extent;
  unsigned call_limit;
} RplBytes;
typedef struct {
  RplBytes bytes;
} RplPages;
int rpl_pages(RplPages *adapter, RplBytes bytes, store_io *out);

/* Supplied outputs/controls/inputs are live and disjoint. A log exclusively
 * borrows an already-open store, initially empty for create; no external store
 * mutations while it is live. Controls and escaped HAMT roots may not be
 * copied/relocated. Open constructs an unpublished index arena before publish.
 * All callbacks must be nonreentrant and may not retain borrowed buffers.
 * Zero UUID/schema is invalid. Only sixteen events; schema equality is exact.
 */
int rpl_create(store *pages, uint64_t uuid, uint32_t schema, RplLog *out);
int rpl_open(store *pages, uint64_t uuid, uint32_t schema, RplLog *out);
int rpl_append(RplLog *log, unsigned kind, unsigned key, int32_t value);
int rpl_get(const RplLog *log, unsigned position, unsigned key, int32_t *out);
int rpl_event_encode(uint64_t uuid, uint32_t schema, RplEvent event,
                     unsigned char out[64]);
int rpl_event_decode(uint64_t uuid, uint32_t schema, const unsigned char in[64],
                     RplEvent *out);
int rpl_snapshot_save(RplLog *log, unsigned position);
int rpl_snapshot_load(RplLog *log, RplSnapshot *out);
int rpl_state(RplLog *log, unsigned use_snapshot, RplSnapshot *out);
#endif
