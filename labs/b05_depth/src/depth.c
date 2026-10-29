#include "b05.h"

int b5_verify(const struct b5_instruction *p, size_t n, unsigned *r)
{
    (void)p;
    (void)n;
    (void)r;
    return B5_UNIMPLEMENTED;
}

uint32_t b5_checksum(uint32_t g, uint32_t v)
{
    (void)g;
    (void)v;
    return 0;
}

int b5_recover(uint32_t g, uint32_t b, const struct b5_redo *r, size_t n,
               uint32_t *out)
{
    (void)g;
    (void)b;
    (void)r;
    (void)n;
    (void)out;
    return B5_UNIMPLEMENTED;
}

int b5_copy_to_user(struct b5_page p[4], size_t a, const unsigned char *b,
                    size_t n)
{
    (void)p;
    (void)a;
    (void)b;
    (void)n;
    return B5_UNIMPLEMENTED;
}

int b5_timer_arm(struct b5_timer *t, uint32_t n, uint32_t d)
{
    (void)t;
    (void)n;
    (void)d;
    return B5_UNIMPLEMENTED;
}

int b5_timer_poll(struct b5_timer *t, uint32_t n, int *r)
{
    (void)t;
    (void)n;
    (void)r;
    return B5_UNIMPLEMENTED;
}

int b5_response(uint32_t c, uint32_t b, uint32_t d, const struct b5_task *h,
                size_t n, uint32_t *r)
{
    (void)c;
    (void)b;
    (void)d;
    (void)h;
    (void)n;
    (void)r;
    return B5_UNIMPLEMENTED;
}

int b5_read_enter(struct b5_grace *g, unsigned i, uint32_t *r)
{
    (void)g;
    (void)i;
    (void)r;
    return B5_UNIMPLEMENTED;
}

int b5_read_exit(struct b5_grace *g, unsigned i, uint32_t t)
{
    (void)g;
    (void)i;
    (void)t;
    return B5_UNIMPLEMENTED;
}

int b5_grace_begin(struct b5_grace *g)
{
    (void)g;
    return B5_UNIMPLEMENTED;
}

int b5_grace_done(struct b5_grace *g, int *r)
{
    (void)g;
    (void)r;
    return B5_UNIMPLEMENTED;
}

int b5_request(struct b5_request *r, unsigned o, uint32_t g, unsigned v)
{
    (void)r;
    (void)o;
    (void)g;
    (void)v;
    return B5_UNIMPLEMENTED;
}
