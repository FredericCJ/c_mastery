#include "object_runtime_ext.h"
#include <float.h>
#include <limits.h>
#ifndef ORX_MUTANT
#define ORX_MUTANT 0
#endif

static int same(struct orx_handle a, struct orx_handle b)
{
    return a.slot == b.slot && a.generation == b.generation;
}

static int live(const struct orx_heap *h, struct orx_handle o)
{
    return h && o.slot < 8 && o.generation && o.generation <= 3 &&
           h->entry[o.slot].live &&
           (ORX_MUTANT == 3 || h->entry[o.slot].generation == o.generation);
}

uint32_t orx_binary32(uint32_t b)
{
    return (b & UINT32_C(0x7f800000)) == UINT32_C(0x7f800000) &&
                   (b & UINT32_C(0x007fffff)) && ORX_MUTANT != 1
               ? UINT32_C(0x7fc00000)
               : b;
}

enum or_status orx_value_decode(const unsigned char w[6], struct orx_value *out)
{
    struct orx_value v = {0};
    uint32_t b = 0;
    unsigned i;
    if (!w || !out)
    {
        return OR_INVALID;
    }
    for (i = 1; i < 5; i++)
    {
        b = b * 256 + w[i];
    }
    v.tag = w[0];
    if (v.tag == 1)
    {
        if (w[5])
        {
            return OR_INVALID;
        }
        v.as.integer = b <= INT32_MAX
                           ? (int32_t)b
                           : INT32_MIN + (int32_t)(b - UINT32_C(0x80000000));
        if (ORX_MUTANT == 2)
        {
            v.as.integer = (int32_t)(b & UINT32_C(0x7fffffff));
        }
    }
    else if (v.tag == 2)
    {
        if (w[5])
        {
            return OR_INVALID;
        }
        v.as.binary32 = orx_binary32(b);
    }
    else if (v.tag == 3)
    {
        if (b < 1 || b > 8 || w[5] < 1 || w[5] > 3)
        {
            return OR_INVALID;
        }
        v.as.handle = (struct orx_handle){(uint8_t)(b - 1), w[5]};
    }
    else
    {
        return OR_INVALID;
    }
    *out = v;
    return OR_OK;
}

enum or_status orx_value_encode(struct orx_value v, unsigned char w[6])
{
    unsigned char t[6] = {0};
    uint32_t b;
    unsigned i;
    if (!w)
    {
        return OR_INVALID;
    }
    t[0] = (unsigned char)v.tag;
    if (v.tag == 1)
    {
        b = (uint32_t)v.as.integer;
    }
    else if (v.tag == 2)
    {
        b = orx_binary32(v.as.binary32);
    }
    else if (v.tag == 3)
    {
        if (v.as.handle.slot >= 8 || !v.as.handle.generation ||
            v.as.handle.generation > 3)
        {
            return OR_INVALID;
        }
        b = v.as.handle.slot + 1u;
        t[5] = v.as.handle.generation;
    }
    else
    {
        return OR_INVALID;
    }
    for (i = 4; i; i--)
    {
        t[i] = (unsigned char)(b & 255u);
        b >>= 8;
    }
    for (i = 0; i < 6; i++)
    {
        w[i] = t[i];
    }
    return OR_OK;
}

static int byte_order(void)
{
    double one = 1.0;
    const unsigned char *p = (const unsigned char *)&one;
    uint64_t lo = 0, hi = 0;
    unsigned i;
    if (CHAR_BIT != 8 || sizeof(double) != 8 || FLT_RADIX != 2 ||
        DBL_MANT_DIG != 53 || DBL_MAX_EXP != 1024)
    {
        return -1;
    }
    for (i = 0; i < 8; i++)
    {
        lo |= (uint64_t)p[i] << (8 * i);
        hi = (hi << 8) | p[i];
    }
    if (lo == UINT64_C(0x3ff0000000000000))
    {
        return 0;
    }
    if (hi == UINT64_C(0x3ff0000000000000))
    {
        return 1;
    }
    return -1;
}

enum or_status orx_double_bits(double value, uint64_t *out)
{
    const unsigned char *p = (const unsigned char *)&value;
    uint64_t b = 0;
    int order = byte_order();
    unsigned i;
    if (!out || order < 0)
    {
        return OR_INVALID;
    }
    for (i = 0; i < 8; i++)
    {
        b = (b << 8) | p[order ? i : 7 - i];
    }
    *out = or_number_bits(b);
    return OR_OK;
}

enum or_status orx_bits_double(uint64_t b, double *out)
{
    double value = 0;
    unsigned char *p = (unsigned char *)&value;
    int order = byte_order();
    unsigned i;
    if (!out || order < 0)
    {
        return OR_INVALID;
    }
    b = or_number_bits(b);
    for (i = 0; i < 8; i++)
    {
        p[order ? 7 - i : i] = (unsigned char)(b & 255u);
        b >>= 8;
    }
    *out = value;
    return OR_OK;
}

void orx_heap_init(struct orx_heap *h)
{
    unsigned i;
    if (!h)
    {
        return;
    }
    *h = (struct orx_heap){0};
    h->epoch = 1;
    for (i = 0; i < 8; i++)
    {
        h->entry[i].generation = 1;
    }
    for (i = 0; i < 4; i++)
    {
        h->lease[i].generation = 1;
    }
}

enum or_status orx_new(struct orx_heap *h, const unsigned char *b, size_t n,
                       struct orx_handle *out)
{
    unsigned s, p, i, used = 0;
    struct orx_cell c = {0};
    if (!h || !out || (!b && n) || n > 15 || !h->epoch)
    {
        return OR_INVALID;
    }
    for (s = 0; s < 8; s++)
    {
        if (!h->entry[s].live && h->entry[s].generation <= 3)
        {
            break;
        }
    }
    if (s == 8)
    {
        return OR_FULL;
    }
    for (i = 0; i < 8; i++)
    {
        if (h->entry[i].live)
        {
            used |= 1u << h->entry[i].position;
        }
    }
    for (p = 0; p < 8; p++)
    {
        if (!(used & (1u << p)))
        {
            break;
        }
    }
    if (p == 8)
    {
        return OR_FULL;
    }
    for (i = 0; i < n; i++)
    {
        c.bytes[i] = b[i];
    }
    c.length = (uint8_t)n;
    h->cell[p] = c;
    h->entry[s] =
        (struct orx_entry){1, h->entry[s].generation, (uint8_t)p, 1, 0};
    *out = (struct orx_handle){(uint8_t)s, h->entry[s].generation};
    return OR_OK;
}

enum or_status orx_retain(struct orx_heap *h, struct orx_handle o)
{
    if (!live(h, o))
    {
        return OR_INVALID;
    }
    if (h->entry[o.slot].roots == UINT8_MAX)
    {
        return OR_FULL;
    }
    h->entry[o.slot].roots++;
    return OR_OK;
}

enum or_status orx_release(struct orx_heap *h, struct orx_handle o)
{
    if (!live(h, o))
    {
        return OR_INVALID;
    }
    if (!h->entry[o.slot].roots)
    {
        return OR_STATE;
    }
    h->entry[o.slot].roots--;
    return OR_OK;
}

enum or_status orx_read(const struct orx_heap *h, struct orx_handle o,
                        unsigned char out[15], size_t *n)
{
    unsigned i;
    const struct orx_cell *c;
    if (!out || !n || !live(h, o))
    {
        return OR_INVALID;
    }
    c = &h->cell[h->entry[o.slot].position];
    for (i = 0; i < c->length; i++)
    {
        out[i] = c->bytes[i];
    }
    *n = c->length;
    return OR_OK;
}

enum or_status orx_edge(struct orx_heap *h, struct orx_handle from,
                        unsigned field, struct orx_handle to)
{
    if (!live(h, from) || field >= 2 || (to.generation && !live(h, to)) ||
        (!to.generation && to.slot))
    {
        return OR_INVALID;
    }
    h->cell[h->entry[from.slot].position].edge[field] = to;
    return OR_OK;
}

enum or_status orx_collect(struct orx_heap *h, unsigned *reclaimed)
{
    unsigned seen = 0, last, i, j, n = 0;
    if (!h || !reclaimed || !h->epoch)
    {
        return OR_INVALID;
    }
    if (h->epoch == UINT8_MAX)
    {
        return OR_FULL;
    }
    for (i = 0; i < 8; i++)
    {
        if (h->entry[i].live && (h->entry[i].roots || h->entry[i].pins))
        {
            seen |= 1u << i;
        }
    }
    do
    {
        last = seen;
        for (i = 0; i < 8; i++)
        {
            if (seen & (1u << i))
            {
                for (j = 0; j < 2; j++)
                {
                    struct orx_handle o = h->cell[h->entry[i].position].edge[j];
                    if (o.generation)
                    {
                        if (!live(h, o))
                        {
                            return OR_INVALID;
                        }
                        seen |= 1u << o.slot;
                    }
                }
            }
        }
    } while (last != seen);
    for (i = 0; i < 8; i++)
    {
        if (h->entry[i].live && !(seen & (1u << i)))
        {
            h->cell[h->entry[i].position] = (struct orx_cell){0};
            h->entry[i].live = 0;
            h->entry[i].roots = h->entry[i].pins = 0;
            h->entry[i].generation++;
            n++;
        }
    }
    h->epoch++;
    *reclaimed = n;
    return OR_OK;
}

enum or_status orx_pack(struct orx_heap *h, unsigned *moved)
{
    struct orx_cell next[8] = {0};
    uint8_t pos[8] = {0};
    unsigned pinned = 0, used = 0, n = 0, i, p;
    if (!h || !moved || !h->epoch)
    {
        return OR_INVALID;
    }
    for (i = 0; i < 8; i++)
    {
        if (h->entry[i].live && h->entry[i].pins && ORX_MUTANT != 4)
        {
            p = h->entry[i].position;
            pinned |= 1u << p;
            pos[i] = (uint8_t)p;
        }
    }
    used = pinned;
    for (i = 0; i < 8; i++)
    {
        if (h->entry[i].live && !(pinned & (1u << h->entry[i].position)))
        {
            for (p = 0; p < 8; p++)
            {
                if (!(used & (1u << p)))
                {
                    break;
                }
            }
            next[p] = h->cell[h->entry[i].position];
            pos[i] = (uint8_t)p;
            used |= 1u << p;
            n += (p != h->entry[i].position);
        }
    }
    /* Never even rewrite a pinned cell: a foreign worker may read its bytes. */
    for (p = 0; p < 8; p++)
    {
        if (!(pinned & (1u << p)))
        {
            h->cell[p] = next[p];
        }
    }
    for (i = 0; i < 8; i++)
    {
        if (h->entry[i].live)
        {
            h->entry[i].position = pos[i];
        }
    }
    *moved = n;
    return OR_OK;
}

enum or_status orx_import(struct orx_heap *h, struct orx_value value,
                          const struct orx_grant *grant, struct orx_handle *out)
{
    enum or_status s;
    if (!out || value.tag != 3 || !live(h, value.as.handle))
    {
        return OR_INVALID;
    }
    if (ORX_MUTANT != 9 && (!grant || (ORX_MUTANT != 10 && grant->heap != h) ||
                            !same(grant->object, value.as.handle)))
    {
        return OR_INVALID;
    }
    s = orx_retain(h, value.as.handle);
    if (s != OR_OK)
    {
        return s;
    }
    *out = value.as.handle;
    return OR_OK;
}

static struct orx_lease *lease(struct orx_ticket t)
{
    struct orx_lease *l;
    if (!t.heap || t.slot >= 4 || !t.generation || t.generation > 3)
    {
        return NULL;
    }
    l = &t.heap->lease[t.slot];
    if (!l->active || (ORX_MUTANT != 8 && l->generation != t.generation))
    {
        return NULL;
    }
    return l;
}

enum or_status orx_foreign_begin(struct orx_heap *h, struct orx_handle o,
                                 uint64_t who, struct orx_ticket *ticket,
                                 const unsigned char **bytes, size_t *length)
{
    unsigned i;
    struct orx_lease *l;
    struct orx_cell *c;
    if (!ticket || !bytes || !length || !live(h, o))
    {
        return OR_INVALID;
    }
    if (h->entry[o.slot].pins == UINT8_MAX)
    {
        return OR_FULL;
    }
    for (i = 0; i < 4; i++)
    {
        if (!h->lease[i].active && h->lease[i].generation <= 3)
        {
            break;
        }
    }
    if (i == 4)
    {
        return OR_FULL;
    }
    l = &h->lease[i];
    c = &h->cell[h->entry[o.slot].position];
    if (ow_pin_init(&l->pin, c->bytes, who) != OW_OK ||
        ow_pin_submit(&l->pin, c->bytes, who) != OW_OK)
    {
        return OR_INVALID;
    }
    l->object = o;
    l->active = 1;
    h->entry[o.slot].pins++;
    *ticket = (struct orx_ticket){h, (uint8_t)i, l->generation};
    *bytes = c->bytes;
    *length = c->length;
    return OR_OK;
}

enum or_status orx_foreign_cancel(struct orx_ticket t, uint64_t who)
{
    struct orx_lease *l = lease(t);
    if (!l || ow_pin_cancel(&l->pin, who) != OW_OK)
    {
        return OR_INVALID;
    }
    if (ORX_MUTANT == 7 && t.heap->entry[l->object.slot].pins)
    {
        t.heap->entry[l->object.slot].pins--;
    }
    return OR_OK;
}

enum or_status orx_foreign_finish(struct orx_ticket t, uint64_t who)
{
    struct orx_lease *l = lease(t);
    if (!l || ow_pin_complete(&l->pin, who) != OW_OK)
    {
        return OR_INVALID;
    }
    if (t.heap->entry[l->object.slot].pins)
    {
        t.heap->entry[l->object.slot].pins--;
    }
    l->active = 0;
    l->generation++;
    return OR_OK;
}

static unsigned hash(const unsigned char *p, size_t n)
{
    unsigned h = 0;
    size_t i;
    for (i = 0; i < n; i++)
    {
        h = (h * 33u + p[i]) & 255u;
    }
    return h;
}

static int32_t code(struct orx_handle h)
{
    return (int32_t)(h.generation * 8u + h.slot + 1u);
}

static struct orx_handle decode(int32_t c)
{
    return (struct orx_handle){(uint8_t)((c - 1) % 8), (uint8_t)((c - 1) / 8)};
}

static void place(struct orx_intern *t, int32_t *b, unsigned cap,
                  struct orx_handle o)
{
    const struct orx_cell *c = &t->heap->cell[t->heap->entry[o.slot].position];
    unsigned p = hash(c->bytes, c->length) % cap;
    while (b[p])
    {
        p = (p + 1) % cap;
    }
    b[p] = code(o);
}

enum or_status orx_intern_init(struct orx_intern *t, struct orx_heap *h,
                               struct v4_allocator a)
{
    if (!t || !h || !h->epoch || !a.take || !a.release)
    {
        return OR_INVALID;
    }
    *t = (struct orx_intern){h, a, NULL, 0, 0, h->epoch};
    return OR_OK;
}

enum or_status orx_intern_reserve(struct orx_intern *t, unsigned capacity)
{
    int32_t *b, *old;
    unsigned i;
    if (!t || !t->heap || t->observed_epoch != t->heap->epoch ||
        (capacity != 4 && capacity != 8 && capacity != 16))
    {
        return OR_INVALID;
    }
    if (capacity <= t->capacity)
    {
        return OR_OK;
    }
    b = t->allocator.take(t->allocator.context, capacity);
    if (!b)
    {
        return OR_FULL;
    }
    for (i = 0; i < capacity; i++)
    {
        b[i] = 0;
    }
    for (i = 0; i < t->capacity; i++)
    {
        if (t->bucket[i])
        {
            place(t, b, capacity, decode(t->bucket[i]));
        }
    }
    old = t->bucket;
    t->bucket = b;
    t->capacity = capacity;
    if (old)
    {
        t->allocator.release(t->allocator.context, old);
    }
    return OR_OK;
}

enum or_status orx_intern_get(struct orx_intern *t, const unsigned char *b,
                              size_t n, struct orx_handle *out)
{
    unsigned p, i, s;
    struct orx_handle o;
    enum or_status status;
    int32_t *candidate = NULL, *old;
    unsigned cap;
    if (!t || !out || (!b && n) || n > 15 || !t->heap ||
        t->observed_epoch != t->heap->epoch)
    {
        return OR_INVALID;
    }
    if (t->capacity)
    {
        p = hash(b, n) % t->capacity;
        for (i = 0; i < t->capacity && t->bucket[p];
             i++, p = (p + 1) % t->capacity)
        {
            const struct orx_cell *c;
            o = decode(t->bucket[p]);
            if (!live(t->heap, o))
            {
                return OR_INVALID;
            }
            c = &t->heap->cell[t->heap->entry[o.slot].position];
            if (c->length != n)
            {
                continue;
            }
            for (s = 0; s < n && c->bytes[s] == b[s]; s++)
            {
            }
            if (s == n || ORX_MUTANT == 5)
            {
                status = orx_retain(t->heap, o);
                if (status != OR_OK)
                {
                    return status;
                }
                *out = o;
                return OR_OK;
            }
        }
    }
    cap = t->capacity;
    if (!cap)
    {
        cap = 4;
    }
    else if (t->used + 1 > cap / 2)
    {
        cap *= 2;
    }
    if (cap > 16)
    {
        return OR_FULL;
    }
    /* Allocate and rehash privately; a later heap failure still rolls everything
     * back. */
    if (cap != t->capacity)
    {
        candidate = t->allocator.take(t->allocator.context, cap);
        if (!candidate)
        {
            return OR_FULL;
        }
        for (i = 0; i < cap; i++)
        {
            candidate[i] = 0;
        }
        for (i = 0; i < t->capacity; i++)
        {
            if (t->bucket[i])
            {
                place(t, candidate, cap, decode(t->bucket[i]));
            }
        }
    }
    status = orx_new(t->heap, b, n, &o);
    if (status != OR_OK)
    {
        if (candidate && ORX_MUTANT != 6)
        {
            t->allocator.release(t->allocator.context, candidate);
        }
        return status;
    }
    if (candidate)
    {
        old = t->bucket;
        t->bucket = candidate;
        t->capacity = cap;
        if (old)
        {
            t->allocator.release(t->allocator.context, old);
        }
    }
    place(t, t->bucket, t->capacity, o);
    t->used++;
    *out = o;
    return OR_OK;
}

enum or_status orx_intern_sweep(struct orx_intern *t, unsigned *removed)
{
    struct orx_handle keep[8];
    unsigned i, n = 0, before;
    if (!t || !removed || !t->heap)
    {
        return OR_INVALID;
    }
    before = t->used;
    for (i = 0; i < t->capacity; i++)
    {
        if (t->bucket[i])
        {
            struct orx_handle o = decode(t->bucket[i]);
            if (live(t->heap, o))
            {
                keep[n++] = o;
            }
            t->bucket[i] = 0;
        }
    }
    for (i = 0; i < n; i++)
    {
        place(t, t->bucket, t->capacity, keep[i]);
    }
    t->used = n;
    t->observed_epoch = t->heap->epoch;
    *removed = before - n;
    return OR_OK;
}

enum or_status orx_intern_close(struct orx_intern *t)
{
    if (!t || !t->heap)
    {
        return OR_INVALID;
    }
    if (t->bucket)
    {
        t->allocator.release(t->allocator.context, t->bucket);
    }
    *t = (struct orx_intern){0};
    return OR_OK;
}
