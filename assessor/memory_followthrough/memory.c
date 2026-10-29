#include "memory.h"
#include <limits.h>

static int span(size_t backing, struct mw_span s)
{
    return s.offset <= backing && s.capacity <= backing - s.offset &&
           s.initialized <= s.capacity;
}

static int overlap(size_t a, size_t an, size_t b, size_t bn)
{
    return an && bn && a < b + bn && b < a + an;
}

enum mw_status mw_fill(unsigned char *p, size_t backing, struct mw_span s,
                       ptrdiff_t index, size_t n, unsigned char value)
{
    if (!p || !span(backing, s) || index < 0)
        return MW_INPUT;
    size_t i = (size_t)index;
    if (i > s.capacity || n > s.capacity - i)
        return MW_EXTENT;
    for (size_t j = 0; j < n; j++)
        p[s.offset + i + j] = value;
    return MW_OK;
}

enum mw_status mw_scan(const unsigned char *p, size_t backing, struct mw_span s,
                       size_t *out)
{
    if (!p || !out || !span(backing, s))
        return MW_INPUT;
    for (size_t i = 0; i < s.initialized; i++)
        if (!p[s.offset + i])
        {
            *out = i;
            return MW_OK;
        }
    return s.initialized < s.capacity ? MW_INIT : MW_NUL;
}

enum mw_status mw_string(unsigned char *p, size_t backing, struct mw_span dst,
                         struct mw_span src, size_t n, size_t *out)
{
    if (!p || !out || !span(backing, dst) || !span(backing, src))
        return MW_INPUT;
    if (n > src.initialized)
        return MW_INIT;
    if (n >= dst.capacity)
        return MW_EXTENT;
    if (overlap(dst.offset, n + 1, src.offset, n))
        return MW_OVERLAP;
    for (size_t i = 0; i < n; i++)
        if (!p[src.offset + i])
            return MW_NUL;
    for (size_t i = 0; i < n; i++)
        p[dst.offset + i] = p[src.offset + i];
    p[dst.offset + n] = 0;
    *out = n;
    return MW_OK;
}

enum mw_status mw_move(unsigned char *p, size_t backing, struct mw_span dst,
                       struct mw_span src, size_t n, size_t *out)
{
    if (!p || !out || !span(backing, dst) || !span(backing, src))
        return MW_INPUT;
    if (n > src.initialized)
        return MW_INIT;
    if (n > dst.capacity)
        return MW_EXTENT;
    if (dst.offset > src.offset)
    {
        for (size_t i = n; i > 0; i--)
            p[dst.offset + i - 1] = p[src.offset + i - 1];
    }
    else
        for (size_t i = 0; i < n; i++)
            p[dst.offset + i] = p[src.offset + i];
    *out = n;
    return MW_OK;
}

enum mw_status mw_size(size_t count, size_t stride, size_t header,
                       size_t terminator, size_t alignment, size_t limit,
                       size_t *out)
{
    if (!out || !stride || stride > limit || count > limit || header > limit ||
        terminator > limit)
        return MW_INPUT;
    if (count > limit / stride)
        return MW_MULTIPLY;
    size_t n;
    if (f_checked_size(count, stride, 0, &n) != F_OK)
        return MW_MULTIPLY;
    if (header > limit - n)
        return MW_ADD;
    n += header;
    if (terminator > limit - n)
        return MW_ADD;
    n += terminator;
    if (!n)
        return MW_ZERO;
    if (!alignment || (alignment & (alignment - 1)))
        return MW_ALIGN;
    size_t padding = (alignment - n % alignment) % alignment;
    if (padding > limit - n)
        return MW_ALIGN;
    *out = n + padding;
    return MW_OK;
}

enum mw_status mw_signed_size(intmax_t count, size_t stride, size_t header,
                              size_t *out)
{
    if (count < 0 || (uintmax_t)count > SIZE_MAX)
        return MW_INPUT;
    return mw_size((size_t)count, stride, header, 0, 1, SIZE_MAX, out);
}

static int add(size_t a, size_t b, size_t *out)
{
    if (b > SIZE_MAX - a)
        return 0;
    *out = a + b;
    return 1;
}

static int align(size_t n, size_t alignment, size_t *out)
{
    size_t padding = (alignment - n % alignment) % alignment;
    return add(n, padding, out);
}

enum mw_status mw_layout(size_t region, size_t alignment, size_t data,
                         size_t bss, size_t retained, size_t heap, size_t guard,
                         size_t stack, struct mw_layout *out)
{
    if (!out || !alignment || (alignment & (alignment - 1)) || stack > region)
        return MW_INPUT;
    struct mw_layout r = {0};
    r.data = (struct mw_interval){0, data};
    if (!align(data, alignment, &r.bss.low) ||
        !add(r.bss.low, bss, &r.bss.high) ||
        !align(r.bss.high, alignment, &r.retained.low) ||
        !add(r.retained.low, retained, &r.retained.high))
        return MW_ADD;
    r.fixed = r.retained.high;
    if (!align(r.fixed, alignment, &r.heap.low) ||
        !add(r.heap.low, heap, &r.heap.high) ||
        !align(r.heap.high, alignment, &r.guard.low) ||
        !add(r.guard.low, guard, &r.guard.high))
        return MW_ADD;
    r.stack = (struct mw_interval){region - stack, region};
    r.fixed_fits = r.fixed <= region;
    r.reservation_fits =
        r.guard.high <= r.stack.low && r.stack.low % alignment == 0;
    r.gap = r.heap.high <= r.stack.low ? r.stack.low - r.heap.high : 0;
    r.flash_bytes = data;
    *out = r;
    return MW_OK;
}

static int interval(size_t size, struct mw_interval r)
{
    return r.low <= r.high && r.high <= size;
}

enum mw_status mw_startup(unsigned char *dst, size_t n,
                          const unsigned char *load, size_t ln,
                          struct mw_interval data, struct mw_interval bss,
                          struct mw_interval retained)
{
    if (!dst || (!load && ln) || !interval(n, data) || !interval(n, bss) ||
        !interval(n, retained) || data.high - data.low != ln)
        return MW_INPUT;
    if (overlap(data.low, ln, bss.low, bss.high - bss.low) ||
        overlap(data.low, ln, retained.low, retained.high - retained.low) ||
        overlap(bss.low, bss.high - bss.low, retained.low,
                retained.high - retained.low))
        return MW_OVERLAP; /* load belongs to a separate immutable image under this API. */
    for (size_t i = 0; i < ln; i++)
        dst[data.low + i] = load[i];
    for (size_t i = bss.low; i < bss.high; i++)
        dst[i] = 0;
    return MW_OK;
}

static int allocator(struct mw_allocator a)
{
    return a.take && a.resize && a.drop;
}

enum mw_status mw_owner(struct mw_owner *o, struct mw_allocator a, size_t n)
{
    if (!o || o->data || o->generation || o->retired || !allocator(a) || !n)
        return MW_INPUT;
    unsigned char *p = a.take(a.context, n);
    if (!p)
        return MW_NOMEM;
    for (size_t i = 0; i < n; i++)
        p[i] = 0;
    *o = (struct mw_owner){p, n, 1, 0, 0, a};
    return MW_OK;
}

enum mw_status mw_resize(struct mw_owner *o, size_t n, unsigned copying)
{
    if (!o || !o->data || !n || copying > 1 || !allocator(o->allocator))
        return MW_INPUT;
    if (o->pins)
        return MW_BUSY;
    if (o->generation == UINT32_MAX)
        return MW_RETIRED;
    size_t old_size = o->size;
    unsigned char *next;
    if (copying)
    {
        next = o->allocator.take(o->allocator.context, n);
        if (!next)
            return MW_NOMEM;
        size_t kept = n < old_size ? n : old_size;
        for (size_t i = 0; i < kept; i++)
            next[i] = o->data[i];
        for (size_t i = kept; i < n; i++)
            next[i] = 0;
        o->allocator.drop(o->allocator.context, o->data);
    }
    else
    {
        next = o->allocator.resize(o->allocator.context, o->data, n);
        if (!next)
            return MW_NOMEM;
        for (size_t i = old_size; i < n; i++)
            next[i] = 0;
    }
    o->data = next;
    o->size = n;
    o->generation++;
    return MW_OK;
}

enum mw_status mw_read(const struct mw_owner *o, struct mw_borrow b,
                       unsigned char *out)
{
    if (!o || !out || !o->data || b.generation != o->generation)
        return MW_STALE;
    if (b.offset >= o->size)
        return MW_EXTENT;
    *out = o->data[b.offset];
    return MW_OK;
}

enum mw_status mw_drop(struct mw_owner *o, uint32_t generation)
{
    if (!o)
        return MW_INPUT;
    if (!o->data)
        return MW_STALE;
    if (generation != o->generation)
        return MW_STALE;
    if (o->pins)
        return MW_BUSY;
    o->allocator.drop(o->allocator.context, o->data);
    o->data = NULL;
    o->size = 0;
    if (o->generation == UINT32_MAX)
        o->retired = 1;
    else
        o->generation++;
    return MW_OK;
}

enum mw_status mw_record(struct mw_record *out, struct mw_allocator a,
                         const unsigned char *src, size_t n, unsigned mode,
                         unsigned version, unsigned optional, unsigned fail)
{
    if (!out || out->payload || !allocator(a) || (!src && n) || n > 16 ||
        mode > 1 || version < 1 || version > 2 || optional > 255 || fail > 5)
        return MW_INPUT;
    struct mw_record r = {0};
    r.allocator = a;
    if (fail == 1)
        return MW_INIT;
    r.length = (uint32_t)n;
    if (fail == 2)
        return MW_INIT;
    r.mode = mode;
    if (n)
    {
        r.payload = a.take(a.context, n);
        if (!r.payload)
            return MW_NOMEM;
    }
    if (fail == 3)
    {
        if (r.payload)
            a.drop(a.context, r.payload);
        return MW_INIT;
    }
    for (size_t i = 0; i < n; i++)
        r.payload[i] = src[i];
    r.tag = 1;
    if (fail == 4)
    {
        if (r.payload)
            a.drop(a.context, r.payload);
        return MW_INIT;
    }
    r.version = version;
    r.optional = version == 2 ? optional : 0;
    if (fail == 5)
    {
        if (r.payload)
            a.drop(a.context, r.payload);
        return MW_INIT;
    }
    *out = r;
    return MW_OK;
}

enum mw_status mw_serialize(const struct mw_record *r, unsigned char *out,
                            size_t capacity, size_t *length)
{
    if (!r || !out || !length || r->tag != 1 || r->length > 16 ||
        (r->length && !r->payload) || r->mode > 1 || r->version < 1 ||
        r->version > 2 || r->optional > 255)
        return MW_INPUT;
    size_t n = 3 + r->length + (r->version == 2 ? 1 : 0);
    if (n > capacity)
        return MW_EXTENT;
    out[0] = (unsigned char)r->version;
    out[1] = (unsigned char)r->length;
    out[2] = (unsigned char)r->mode;
    for (size_t i = 0; i < r->length; i++)
        out[3 + i] = r->payload[i];
    if (r->version == 2)
        out[3 + r->length] = (unsigned char)r->optional;
    *length = n;
    return MW_OK;
}

void mw_record_drop(struct mw_record *r)
{
    if (r && r->tag == 1)
    {
        if (r->payload)
            r->allocator.drop(r->allocator.context, r->payload);
        *r = (struct mw_record){0};
    }
}

enum mw_status mw_tree(unsigned depth, unsigned breadth, unsigned *work,
                       size_t capacity, unsigned *out, size_t *peak)
{
    if (!work || !out || !peak || depth > 6 || breadth > 1 || !capacity)
        return MW_INPUT;
    size_t head = 0, count = 1, high = 1;
    unsigned result = 0;
    work[0] = depth;
    while (count)
    {
        unsigned d;
        if (breadth)
        {
            d = work[head];
            head = (head + 1) % capacity;
            count--;
        }
        else
            d = work[--count];
        if (!d)
        {
            result++;
            continue;
        }
        if (capacity - count < 2)
            return MW_EXTENT;
        if (breadth)
        {
            work[(head + count) % capacity] = d - 1;
            count++;
            work[(head + count) % capacity] = d - 1;
            count++;
        }
        else
        {
            work[count++] = d - 1;
            work[count++] = d - 1;
        }
        if (count > high)
            high = count;
    }
    *out = result;
    *peak = high;
    return MW_OK;
}

static struct mw_operation *ticket(struct mw_async *a, struct mw_ticket t)
{
    if (!a || t.slot >= 2)
        return NULL;
    struct mw_operation *o = &a->slots[t.slot];
    return o->started && o->generation == t.generation &&
                   o->request == t.request
               ? o
               : NULL;
}

void mw_async_init(struct mw_async *a)
{
    if (a)
    {
        *a = (struct mw_async){0};
        a->slots[0].generation = 1;
        a->slots[1].generation = 1;
    }
}

enum mw_status mw_submit(struct mw_async *a, uint32_t request,
                         struct mw_ticket *out)
{
    if (!a || !out || !request)
        return MW_INPUT;
    for (unsigned i = 0; i < 2; i++)
    {
        struct mw_operation *o = &a->slots[i];
        if (o->started || o->retired)
            continue;
        uint32_t generation = o->generation;
        *o = (struct mw_operation){0};
        o->generation = generation;
        o->operation.generation = generation - 1;
        uint32_t issued = 0;
        if (f_submit(&o->operation, &issued) != F_OK || issued != generation)
            return MW_RETIRED;
        o->started = 1;
        o->submit_active = 1;
        o->request = request;
        *out = (struct mw_ticket){i, generation, request};
        return MW_OK;
    }
    return MW_BUSY;
}

enum mw_status mw_submit_return(struct mw_async *a, struct mw_ticket t)
{
    struct mw_operation *o = ticket(a, t);
    if (!o)
        return MW_STALE;
    if (!o->submit_active)
        return MW_STALE;
    o->submit_active = 0;
    return MW_OK;
}

enum mw_status mw_cancel(struct mw_async *a, struct mw_ticket t)
{
    struct mw_operation *o = ticket(a, t);
    if (!o)
        return MW_STALE;
    return f_cancel(&o->operation, t.generation) == F_PENDING ? MW_OK
                                                              : MW_STALE;
}

enum mw_status mw_terminal(struct mw_async *a, struct mw_ticket t)
{
    struct mw_operation *o = ticket(a, t);
    if (!o)
        return MW_STALE;
    if (f_complete(&o->operation, t.generation) != F_OK)
        return MW_STALE;
    o->callback_pending = 1;
    a->results++;
    return MW_OK;
}

enum mw_status mw_callback_begin(struct mw_async *a, struct mw_ticket t,
                                 unsigned scratch)
{
    struct mw_operation *o = ticket(a, t);
    if (!o)
        return MW_STALE;
    if (scratch > 1)
        return MW_INPUT;
    unsigned bit = 1u << scratch;
    if (!o->callback_pending || o->callback_active || (a->scratch_mask & bit))
        return MW_BUSY;
    o->scratch = scratch;
    o->callback_pending = 0;
    o->callback_active = 1;
    a->scratch_mask |= bit;
    a->callbacks++;
    if (a->callbacks > a->peak_callbacks)
        a->peak_callbacks = a->callbacks;
    return MW_OK;
}

enum mw_status mw_callback_end(struct mw_async *a, struct mw_ticket t)
{
    struct mw_operation *o = ticket(a, t);
    if (!o)
        return MW_STALE;
    if (!o->callback_active)
        return MW_STALE;
    a->scratch_mask &= ~(1u << o->scratch);
    a->callbacks--;
    o->callback_active = 0;
    return MW_OK;
}

enum mw_status mw_reclaim(struct mw_async *a, struct mw_ticket t)
{
    struct mw_operation *o = ticket(a, t);
    if (!o)
        return MW_STALE;
    if (o->operation.state != F_DONE || o->submit_active ||
        o->callback_pending || o->callback_active)
        return MW_BUSY;
    o->started = 0;
    if (o->generation == UINT32_MAX)
        o->retired = 1;
    else
        o->generation++;
    return MW_OK;
}
