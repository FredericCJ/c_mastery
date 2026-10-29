#include "lc.h"
#include "api_depth.h"
#include "lc_private.h"
#include <stdint.h>
#include <stdlib.h>
#ifndef LC_MUTANT
#define LC_MUTANT 0
#endif
struct lc_buffer
{
#if LC_REPRESENTATION == 2
    unsigned private_revision;
#endif
    struct api_buffer bytes;
};

struct lc_operation
{
    lc_done_fn done;
    void *context;
    unsigned state;
};
#ifdef LC_STATIC_HELPER
unsigned lc_private_adjust(unsigned);
#endif
unsigned lc_release(void)
{
#ifdef LC_STATIC_HELPER
    return lc_private_adjust(LC_RELEASE);
#else
    return LC_RELEASE;
#endif
}

enum lc_status lc_info(unsigned abi, unsigned char *out, size_t n)
{
    if (!out)
    {
        return LC_INVALID;
    }
    if (abi != 1 || (LC_RELEASE == 1 ? n != 16 : n < 16))
    {
        return LC_VERSION;
    }
    unsigned char temp[16] = {'L',
                              'C',
                              1,
                              LC_RELEASE,
                              sizeof(void *),
                              sizeof(size_t),
                              _Alignof(size_t),
                              0};
    for (size_t i = 0; i < 16; i++)
    {
        out[i] = temp[i];
    }
    return LC_OK;
}

struct lc_buffer *lc_create(size_t capacity_bytes)
{
    if (capacity_bytes > (LC_RELEASE == 1 ? 8u : 16u))
    {
        return NULL;
    }
    struct lc_buffer *p = malloc(sizeof *p);
    if (!p)
    {
        return NULL;
    }
    p->bytes = (struct api_buffer){{0}, 0, capacity_bytes};
#if LC_REPRESENTATION == 2
    p->private_revision = 2;
#endif
    return p;
}

enum lc_status lc_append(struct lc_buffer *p, const unsigned char *s, size_t n)
{
    if (!p)
    {
        return LC_INVALID;
    }
    /* Capacity admission precedes any publication; failure snapshots expose a
       write-before-check defect even when the reported status looks correct. */
    if (LC_MUTANT == 1 && n > p->bytes.capacity - p->bytes.length)
    {
        p->bytes.length = 0;
    }
    enum api_status r = api_append(&p->bytes, s, n);
    return r == API_OK ? LC_OK : r == API_FULL ? LC_FULL : LC_INVALID;
}

const unsigned char *lc_view(const struct lc_buffer *p, size_t *n)
{
    if (!p || !n)
    {
        return NULL;
    }
    *n = p->bytes.length;
    return p->bytes.data;
}

enum lc_status lc_copy(const struct lc_buffer *p, unsigned char *out,
                       size_t capacity, size_t *written)
{
    if (!p || !written || (!out && p->bytes.length))
    {
        return LC_INVALID;
    }
    if (capacity < p->bytes.length)
    {
        return LC_FULL;
    }
    for (size_t i = 0; i < p->bytes.length; i++)
    {
        out[i] = p->bytes.data[i];
    }
    *written = p->bytes.length;
    return LC_OK;
}

enum lc_status lc_extent(struct lc_elements n, size_t width,
                         struct lc_bytes available, struct lc_bytes *needed)
{
    if (!needed || !width)
    {
        return LC_INVALID;
    }
    if (n.value > SIZE_MAX / width || n.value * width > available.value)
    {
        return LC_FULL;
    }
    needed->value = n.value * width;
    return LC_OK;
}

void lc_release_buffer(struct lc_buffer *p)
{
    free(p);
}

enum lc_status lc_sync(lc_done_fn done, void *ctx)
{
    if (!done)
    {
        return LC_INVALID;
    }
    done(ctx, LC_OK);
    return LC_OK;
}

struct lc_operation *lc_start(lc_done_fn done, void *ctx)
{
    if (!done)
    {
        return NULL;
    }
    struct lc_operation *p = malloc(sizeof *p);
    if (!p)
    {
        return NULL;
    }
    *p = (struct lc_operation){done, ctx, 0};
    return p;
}

enum lc_status lc_cancel(struct lc_operation *p)
{
    if (!p)
    {
        return LC_INVALID;
    }
    if (p->state == 2)
    {
        return LC_OK;
    }
    p->state = 1;
    if (LC_MUTANT == 2)
    {
        p->state = 2;
    }
    return LC_PENDING;
}

enum lc_status lc_poll(struct lc_operation *p)
{
    if (!p)
    {
        return LC_INVALID;
    }
    if (p->state == 1)
    {
        return LC_PENDING;
    }
    if (p->state == 2)
    {
        return LC_OK;
    }
    p->state = 2;
    p->done(p->context, LC_OK);
    return LC_OK;
}

enum lc_status lc_ack(struct lc_operation *p)
{
    if (!p)
    {
        return LC_INVALID;
    }
    if (p->state == 2)
    {
        return LC_OK;
    }
    if (p->state != 1)
    {
        return LC_BUSY;
    }
    /* Only the acknowledged, serialized driver transition establishes that no
       later callback can access the context. Requesting cancellation cannot. */
    p->state = 2;
    p->done(p->context, LC_CANCELLED);
    return LC_OK;
}

enum lc_status lc_release_operation(struct lc_operation *p)
{
    if (!p)
    {
        return LC_OK;
    }
    if (p->state != 2)
    {
        return LC_BUSY;
    }
    free(p);
    return LC_OK;
}
