#ifndef CMASTERY_QUERY_FOLLOW_H
#define CMASTERY_QUERY_FOLLOW_H
#include "coroutines.h"

struct cf_scan
{
    const struct co_row *rows;
    size_t count, index;
    unsigned closed;
};

struct cf_volcano
{
    struct cf_scan scan;
    size_t scan_calls, predicate_calls;
};

int cf_volcano_open(struct cf_volcano *, const struct co_row *, size_t);
int cf_volcano_next(struct cf_volcano *, unsigned *, uint32_t *);
int cf_volcano_close(struct cf_volcano *);

struct cf_batch
{
    const struct co_row *rows;
    size_t capacity, available, committed, scan;
    unsigned key, active, kept, complete, closed, eof;
    uint32_t sum;
};

int cf_batch_open(struct cf_batch *, const struct co_row *, size_t);
int cf_batch_poll(struct cf_batch *, size_t available, unsigned eof,
                  unsigned sink_ready, unsigned *, uint32_t *);
int cf_batch_cancel(struct cf_batch *);
int cf_order(const struct co_row *, size_t, unsigned eof, struct co_row *,
             size_t, size_t *);
#endif
