#include "fault.h"

void df_init(struct df_capture *c)
{
    atomic_init(&c->state, 0);
    atomic_init(&c->secondary, 0);
    c->record = (struct df_record){0};
}

int df_capture(struct df_capture *c, const struct df_input *i,
               void (*h)(void *), void *x)
{
    (void)c;
    (void)i;
    (void)h;
    (void)x;
    return DF_UNIMPLEMENTED;
}

int df_read(const struct df_capture *c, struct df_record *r, unsigned *s)
{
    (void)c;
    (void)r;
    (void)s;
    return DF_UNIMPLEMENTED;
}

int df_flags(const struct df_record *r, struct df_flags *f)
{
    (void)r;
    (void)f;
    return DF_UNIMPLEMENTED;
}

int df_access(const struct df_record *r, const unsigned char i[32], uint32_t b,
              const unsigned char *c, size_t n, struct df_access *a)
{
    (void)r;
    (void)i;
    (void)b;
    (void)c;
    (void)n;
    (void)a;
    return DF_UNIMPLEMENTED;
}

int df_refault(unsigned k, unsigned i, unsigned m, unsigned *o)
{
    (void)k;
    (void)i;
    (void)m;
    (void)o;
    return DF_UNIMPLEMENTED;
}

int df_watermark(const unsigned char *s, size_t n, unsigned char f, size_t *u)
{
    (void)s;
    (void)n;
    (void)f;
    (void)u;
    return DF_UNIMPLEMENTED;
}

int df_canary(const unsigned char *p, size_t n, unsigned char v, unsigned *o)
{
    (void)p;
    (void)n;
    (void)v;
    (void)o;
    return DF_UNIMPLEMENTED;
}

int df_budget(const struct df_live_frame *f, size_t n, uint32_t a, unsigned r,
              unsigned o, struct df_budget *b)
{
    (void)f;
    (void)n;
    (void)a;
    (void)r;
    (void)o;
    (void)b;
    return DF_UNIMPLEMENTED;
}
