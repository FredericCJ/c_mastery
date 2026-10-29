#ifndef CMASTERY_API_STREAM_H
#define CMASTERY_API_STREAM_H
#include <stddef.h>
#include <stdint.h>

enum
{
    SF_OK,
    SF_INVALID,
    SF_RANGE,
    SF_STATE,
    SF_PENDING,
    SF_IO,
    SF_DONE,
    SF_CANCELLED,
    SF_UNIMPLEMENTED
};

typedef ptrdiff_t (*sf_write_fn)(void *, const unsigned char *, size_t);

struct sf_stream
{
    unsigned char bytes[66];
    size_t length, offset;
    uint32_t generation;
    unsigned state;
};

struct sf_handle
{
    struct sf_stream *owner;
    uint32_t generation;
};

void sf_init(struct sf_stream *);
int sf_start(struct sf_stream *, const unsigned char *, size_t,
             struct sf_handle *);
int sf_pump(struct sf_handle, sf_write_fn, void *, unsigned, size_t *);
int sf_cancel(struct sf_handle);
#endif
