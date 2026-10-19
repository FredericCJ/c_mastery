#ifndef CMASTERY_REPLICATED_WIRE_DEPTH_H
#define CMASTERY_REPLICATED_WIRE_DEPTH_H
#include "replicated.h"
#include "stream.h"
enum {
  RPW_OK,
  RPW_INVALID,
  RPW_LIMIT,
  RPW_REQUIRED,
  RPW_FULL,
  RPW_MORE,
  RPW_STATE,
  RPW_UNIMPLEMENTED
};
/* kind0 unsigned value, kind1 opaque bytes, kind2 child arena index.
   Tags3/4 require kind2;1/2/5/6 require0;7 requires1;8..15 allow0/1. */
typedef struct {
  unsigned tag, kind;
  uint32_t value;
  unsigned length;
  unsigned char bytes[32];
} RpwField;
typedef struct {
  unsigned count;
  RpwField fields[8];
} RpwRecord;
typedef struct {
  unsigned count;
  RpwRecord records[16];
} RpwDoc;
typedef struct {
  unsigned depth, bytes, fields;
  uint32_t supported;
} RpwLimits;
int rpw_decode(const unsigned char *, size_t, RpwLimits, RpwDoc *);
int rpw_encode(const RpwDoc *, RpwLimits, unsigned char *, size_t, size_t *);
/* Serialized live disjoint native objects; outputs unchanged on refusal.
   update changes only root ID/delta and preserves unknown values/bytes. */
int rpw_update(RpwDoc *, RpwLimits, uint32_t, int32_t);
int rpw_project(const RpwDoc *, RpwLimits, RPMessage *);
/* Canonical bit representation experiment; does not evaluate native doubles.
   All NaNs -> positive quiet NaN, both zeros -> +0, other bits preserved. */
uint64_t rpw_binary64(uint64_t);
int rpw_utf8(const unsigned char *, size_t);
typedef struct {
  RfParser parser;
  RpwLimits limits;
  unsigned used, phase;
  unsigned char bytes[256];
} RpwStream;
int rpw_stream_init(RpwStream *, RpwLimits);
/* Owned partial bytes, no retained input borrow. Phase0 collecting,1 done,
   2 failed,3 cancelled. consumed reports actual input advancement even when
   an admitted stream fails; output only on complete empty-frame terminator. */
int rpw_feed(RpwStream *, const unsigned char *, size_t, size_t *, RpwDoc *);
int rpw_eof(RpwStream *);
int rpw_cancel(RpwStream *);
int rpw_frame(const RpwDoc *, RpwLimits, unsigned char *, size_t, size_t *);
#endif
