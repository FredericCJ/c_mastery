#include "contract_modes.h"

er_code ecm_init(struct ecm_buffer *b, unsigned char *p, size_t n, size_t r)
{
    (void)b;
    (void)p;
    (void)n;
    (void)r;
    return ER_NOT_IMPLEMENTED;
}

er_code ecm_write(struct ecm_buffer *b, er_mode m, size_t i, unsigned char v)
{
    (void)b;
    (void)m;
    (void)i;
    (void)v;
    return ER_NOT_IMPLEMENTED;
}

er_code ecm_shared_init(struct ecm_shared *s, unsigned char *p, size_t n, int c,
                        size_t r)
{
    (void)s;
    (void)p;
    (void)n;
    (void)c;
    (void)r;
    return ER_NOT_IMPLEMENTED;
}

er_code ecm_shared_write(struct ecm_shared *s, er_mode m, size_t i,
                         unsigned char v)
{
    (void)s;
    (void)m;
    (void)i;
    (void)v;
    return ER_NOT_IMPLEMENTED;
}

er_code ecm_shared_counter(struct ecm_shared *s, er_mode m, int a)
{
    (void)s;
    (void)m;
    (void)a;
    return ER_NOT_IMPLEMENTED;
}

er_code ecm_shared_snapshot(struct ecm_shared *s, struct ecm_snapshot *o)
{
    (void)s;
    (void)o;
    return ER_NOT_IMPLEMENTED;
}

void ecm_shared_destroy(struct ecm_shared *s)
{
    (void)s;
}
