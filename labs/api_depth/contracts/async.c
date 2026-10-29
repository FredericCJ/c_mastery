/* Intentional learner scaffold. Complete according to ASYNC.md. */
#include "async.h"

void ac_operation_init(struct ac_operation *o)
{
    (void)o;
}

enum ac_async_status ac_submit_v2(struct ac_operation *o,
                                  const unsigned char *p, size_t n,
                                  struct ac_allocate a, ac_terminal_fn cb,
                                  void *ctx, unsigned in, uint32_t *t)
{
    (void)o;
    (void)p;
    (void)n;
    (void)a;
    (void)cb;
    (void)ctx;
    (void)in;
    (void)t;
    return ACX_UNIMPLEMENTED;
}

enum ac_async_status ac_cancel_v2(struct ac_operation *o, uint32_t t)
{
    (void)o;
    (void)t;
    return ACX_UNIMPLEMENTED;
}

enum ac_async_status ac_commit_v2(struct ac_operation *o, uint32_t t)
{
    (void)o;
    (void)t;
    return ACX_UNIMPLEMENTED;
}

enum ac_async_status ac_finish_v2(struct ac_operation *o, uint32_t t,
                                  unsigned a)
{
    (void)o;
    (void)t;
    (void)a;
    return ACX_UNIMPLEMENTED;
}

const unsigned char *ac_box_data(const struct ac_box *b, size_t *n)
{
    (void)b;
    (void)n;
    return 0;
}

void ac_box_drop(struct ac_box *b)
{
    (void)b;
}
