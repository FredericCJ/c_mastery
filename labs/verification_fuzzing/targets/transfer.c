#include "transfer.h"

int vt_mul(uint32_t a, uint32_t b, uint32_t *out)
{
    (void)a;
    (void)b;
    (void)out;
    return 0;
}

int vt_narrow(int64_t a, uint16_t *out)
{
    (void)a;
    (void)out;
    return 0;
}

size_t vt_desc(const struct vt_record *a, size_t n, int32_t k)
{
    (void)a;
    (void)n;
    (void)k;
    return 0;
}

int vt_offer(struct vt_queue *q, const void *p, vt_prepare_fn f, vt_notify_fn g,
             void *c)
{
    (void)q;
    (void)p;
    (void)f;
    (void)g;
    (void)c;
    return 0;
}

int vt_remove(struct vt_queue *q)
{
    (void)q;
    return 0;
}

void vt_reset(struct vt_stream *s)
{
    (void)s;
}

int vt_feed(struct vt_stream *s, const unsigned char *p, size_t n)
{
    (void)s;
    (void)p;
    (void)n;
    return 0;
}

int vt_finish(const struct vt_stream *s, uint32_t *out)
{
    (void)s;
    (void)out;
    return 0;
}

unsigned vt_violation(const struct vt_event *a, size_t n)
{
    (void)a;
    (void)n;
    return 0;
}

size_t vt_shrink(struct vt_event *a, size_t n)
{
    (void)a;
    return n;
}
