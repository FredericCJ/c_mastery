#include "lc.h"

unsigned lc_release(void)
{
    return 0;
}

enum lc_status lc_info(unsigned a, unsigned char *b, size_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return LC_UNIMPLEMENTED;
}

struct lc_buffer *lc_create(size_t n)
{
    (void)n;
    return 0;
}

enum lc_status lc_append(struct lc_buffer *p, const unsigned char *s, size_t n)
{
    (void)p;
    (void)s;
    (void)n;
    return LC_UNIMPLEMENTED;
}

const unsigned char *lc_view(const struct lc_buffer *p, size_t *n)
{
    (void)p;
    (void)n;
    return 0;
}

enum lc_status lc_copy(const struct lc_buffer *p, unsigned char *s, size_t n,
                       size_t *w)
{
    (void)p;
    (void)s;
    (void)n;
    (void)w;
    return LC_UNIMPLEMENTED;
}

enum lc_status lc_extent(struct lc_elements e, size_t w, struct lc_bytes b,
                         struct lc_bytes *o)
{
    (void)e;
    (void)w;
    (void)b;
    (void)o;
    return LC_UNIMPLEMENTED;
}

void lc_release_buffer(struct lc_buffer *p)
{
    (void)p;
}

enum lc_status lc_sync(lc_done_fn d, void *p)
{
    (void)d;
    (void)p;
    return LC_UNIMPLEMENTED;
}

struct lc_operation *lc_start(lc_done_fn d, void *p)
{
    (void)d;
    (void)p;
    return 0;
}

enum lc_status lc_cancel(struct lc_operation *p)
{
    (void)p;
    return LC_UNIMPLEMENTED;
}

enum lc_status lc_poll(struct lc_operation *p)
{
    (void)p;
    return LC_UNIMPLEMENTED;
}

enum lc_status lc_ack(struct lc_operation *p)
{
    (void)p;
    return LC_UNIMPLEMENTED;
}

enum lc_status lc_release_operation(struct lc_operation *p)
{
    (void)p;
    return LC_UNIMPLEMENTED;
}
