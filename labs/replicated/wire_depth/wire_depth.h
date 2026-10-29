#ifndef CMASTERY_REPLICATED_WIRE_DEPTH_H
#define CMASTERY_REPLICATED_WIRE_DEPTH_H
#include "replicated.h"
#include "stream.h"

enum
{
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
struct rpw_field
{
    unsigned tag, kind;
    uint32_t value;
    unsigned length;
    unsigned char bytes[32];
};

struct rpw_record
{
    unsigned count;
    struct rpw_field fields[8];
};

struct rpw_doc
{
    unsigned count;
    struct rpw_record records[16];
};

struct rpw_limits
{
    unsigned depth, bytes, fields;
    uint32_t supported;
};

int rpw_decode(const unsigned char *, size_t, struct rpw_limits,
               struct rpw_doc *);
int rpw_encode(const struct rpw_doc *, struct rpw_limits, unsigned char *,
               size_t, size_t *);
/* Serialized live disjoint native objects; outputs unchanged on refusal.
   update changes only root ID/delta and preserves unknown values/bytes. */
int rpw_update(struct rpw_doc *, struct rpw_limits, uint32_t, int32_t);
int rpw_project(const struct rpw_doc *, struct rpw_limits, struct rp_message *);
/* Canonical bit representation experiment; does not evaluate native doubles.
   All NaNs -> positive quiet NaN, both zeros -> +0, other bits preserved. */
uint64_t rpw_binary64(uint64_t);
int rpw_utf8(const unsigned char *, size_t);

struct rpw_stream
{
    struct rf_parser parser;
    struct rpw_limits limits;
    unsigned used, phase;
    unsigned char bytes[256];
};

int rpw_stream_init(struct rpw_stream *, struct rpw_limits);
/* Owned partial bytes, no retained input borrow. Phase0 collecting,1 done,
   2 failed,3 cancelled. consumed reports actual input advancement even when
   an admitted stream fails; output only on complete empty-frame terminator. */
int rpw_feed(struct rpw_stream *, const unsigned char *, size_t, size_t *,
             struct rpw_doc *);
int rpw_eof(struct rpw_stream *);
int rpw_cancel(struct rpw_stream *);
int rpw_frame(const struct rpw_doc *, struct rpw_limits, unsigned char *,
              size_t, size_t *);
#endif
