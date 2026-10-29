#include "transfer.h"
#include <limits.h>
#ifndef VT_MUTANT
#define VT_MUTANT 0
#endif
int vt_mul(uint32_t a, uint32_t b, uint32_t *out)
{
    if (!out || (b && a > UINT32_MAX / b && VT_MUTANT != 1))
        return 0;
    *out = a * b;
    return 1;
}

int vt_narrow(int64_t a, uint16_t *out)
{
    if (!out || a < 0 || a > UINT16_MAX)
        return 0;
    *out = (uint16_t)a;
    return 1;
}

size_t vt_desc(const struct vt_record *a, size_t n, int32_t k)
{
    size_t l = 0, h = n;
    while (l < h)
    {
        size_t m = l + (h - l) / 2;
        if (a[m].key > k || (VT_MUTANT == 2 && a[m].key == k))
            l = m + 1;
        else
            h = m;
    }
    return l;
}

int vt_offer(struct vt_queue *q, const void *p, vt_prepare_fn prepare,
             vt_notify_fn notify, void *ctx)
{
    if (!q || q->busy || q->n >= 3 || !p || !prepare || !notify)
        return 0;
    q->busy = 1;
    int ok = prepare(ctx, p);
    if (ok)
    {
        unsigned i = q->n;
        q->payload[i] = p;
        q->notify[i] = notify;
        q->context[i] = ctx;
        ++q->n;
    }
    q->busy = 0;
    return ok != 0;
}

int vt_remove(struct vt_queue *q)
{
    if (!q || q->busy || !q->n)
        return 0;
    const void *p = q->payload[0];
    vt_notify_fn fn = q->notify[0];
    void *ctx = q->context[0];
    --q->n;
    for (unsigned i = 0; i < q->n; i++)
    {
        q->payload[i] = q->payload[i + 1];
        q->notify[i] = q->notify[i + 1];
        q->context[i] = q->context[i + 1];
    }
    q->busy = 1;
    fn(ctx, p);
    if (VT_MUTANT == 3)
        fn(ctx, p);
    q->busy = 0;
    return 1;
}

void vt_reset(struct vt_stream *s)
{
    s->n = 0;
    s->rejected = 0;
}

int vt_feed(struct vt_stream *s, const unsigned char *p, size_t n)
{
    if (!s || (!p && n) || s->rejected)
        return 0;
    for (size_t i = 0; i < n; i++)
    {
        if (s->n >= 18 || (s->n && s->n >= (size_t)s->bytes[0] + 2) ||
            (!s->n && p[i] > 16))
        {
            s->rejected = 1;
            return 0;
        }
        s->bytes[s->n++] = p[i];
    }
    if (VT_MUTANT == 4 && n == 1)
        s->rejected = 1;
    return !s->rejected;
}

int vt_finish(const struct vt_stream *s, uint32_t *out)
{
    if (!s || !out || s->rejected || s->n < 2 ||
        s->n != (size_t)s->bytes[0] + 2)
        return 0;
    unsigned x = 0;
    uint32_t h = 0;
    for (size_t i = 1; i + 1 < s->n; i++)
    {
        x ^= s->bytes[i];
        h = h * 33 + s->bytes[i];
    }
    if (x != s->bytes[s->n - 1])
        return 0;
    *out = h;
    return 1;
}

unsigned vt_violation(const struct vt_event *a, size_t n)
{
    unsigned submitted = 0, cancelled = 0, complete = 0;
    for (size_t i = 0; i < n; i++)
    {
        switch (a[i].op)
        {
            case 0:
                submitted = 1;
                break;
            case 1:
                if (submitted && !complete)
                    cancelled = 1;
                break;
            case 2:
                if (submitted)
                    complete = 1;
                break;
            case 3:
                if (submitted && cancelled && !complete)
                    return a[i].id;
                break;
            default:
                break;
        }
    }
    return 0;
}

size_t vt_shrink(struct vt_event *a, size_t n)
{
    if (n > 64)
        return n;
    unsigned signature = vt_violation(a, n);
    if (!signature)
        return n;
    size_t i = 0;
    while (i < n)
    {
        struct vt_event candidate[64];
        size_t k = 0;
        for (size_t j = 0; j < n; j++)
            if (j != i)
                candidate[k++] = a[j];
        unsigned other = vt_violation(candidate, k);
        if (other && (VT_MUTANT == 5 || other == signature))
        {
            for (size_t j = 0; j < k; j++)
                a[j] = candidate[j];
            n = k;
            i = 0;
        }
        else
            i++;
    }
    return n;
}
