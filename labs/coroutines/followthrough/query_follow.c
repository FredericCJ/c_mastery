#include "query_follow.h"

int cf_volcano_open(struct cf_volcano *a, const struct co_row *b, size_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return CO_UNIMPLEMENTED;
}

int cf_volcano_next(struct cf_volcano *a, unsigned *b, uint32_t *c)
{
    (void)a;
    (void)b;
    (void)c;
    return CO_UNIMPLEMENTED;
}

int cf_volcano_close(struct cf_volcano *a)
{
    (void)a;
    return CO_UNIMPLEMENTED;
}

int cf_batch_open(struct cf_batch *a, const struct co_row *b, size_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return CO_UNIMPLEMENTED;
}

int cf_batch_poll(struct cf_batch *a, size_t b, unsigned c, unsigned d,
                  unsigned *e, uint32_t *f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return CO_UNIMPLEMENTED;
}

int cf_batch_cancel(struct cf_batch *a)
{
    (void)a;
    return CO_UNIMPLEMENTED;
}

int cf_order(const struct co_row *a, size_t b, unsigned c, struct co_row *d,
             size_t e, size_t *f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return CO_UNIMPLEMENTED;
}
