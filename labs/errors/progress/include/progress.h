#ifndef CMASTERY_EPT_H
#define CMASTERY_EPT_H
#include "errors.h"
#include "stream.h"

enum
{
    EPT_OK,
    EPT_INVALID,
    EPT_RANGE,
    EPT_BUSY,
    EPT_AGAIN,
    EPT_IO,
    EPT_CANCELLED,
    EPT_UNCERTAIN,
    EPT_DONE,
    EPT_STALE,
    EPT_CONTRACT,
    EPT_UNIMPLEMENTED
};

enum
{
    EPT_FLUSH_OK,
    EPT_FLUSH_IO,
    EPT_FLUSH_UNKNOWN
};

struct ept_flush_result
{
    size_t prefix;
    uint32_t epoch;
    unsigned code;
};
typedef struct ept_flush_result (*ept_flush_fn)(void *, size_t, size_t,
                                                uint32_t);

struct ept_stream
{
    struct sf_stream frame;
    struct sf_handle base;
    size_t durable;
    uint32_t epoch;
    unsigned active, state, uncertain;
};

struct ept_handle
{
    struct ept_stream *owner;
    uint32_t generation;
};

struct ept_observation
{
    size_t accepted, durable;
    uint32_t epoch;
    unsigned state, uncertain;
};

/* Fresh storage only; live, noncopied control remains at the same address.
   All calls externally serialized; callbacks synchronous and nonreentrant. */
void ept_init(struct ept_stream *);
unsigned ept_start(struct ept_stream *, const unsigned char *, size_t,
                   struct ept_handle *);
unsigned ept_step(struct ept_handle, er_sink, void *, struct ept_observation *);
unsigned ept_cancel(struct ept_handle, struct ept_observation *);
unsigned ept_checkpoint(struct ept_handle, ept_flush_fn, void *,
                        struct ept_observation *);
/* Trusted backend authority after known flush, not a proof from readable bytes. */
unsigned ept_reconcile(struct ept_handle, size_t, uint32_t, unsigned,
                       struct ept_observation *);
unsigned ept_observe(struct ept_handle, struct ept_observation *);
unsigned ept_release(struct ept_handle, unsigned, unsigned);
#endif
