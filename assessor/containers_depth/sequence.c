/*
 * Reference implementations of the sequence containers: fixed array, strided
 * view, ring buffer, growable vector and the two payload (ownership) vectors.
 *
 * Shared contract (full text in labs/containers_depth/CONTRACTS.md):
 *  - Every container borrows or owns storage the caller keeps alive and at a
 *    stable address; operations are externally serialized.
 *  - A failed operation leaves logical contents, ownership and promised
 *    outputs unchanged.  Arguments that break a precondition the code can
 *    detect return CD_INVALID; exhaustion returns CD_FULL, CD_EMPTY or
 *    CD_NOMEM.
 *  - Sizes are element counts; byte sizes are checked with SIZE_MAX divisions
 *    before any multiplication.
 *
 * The ring and vector functions (ring_valid through cd_vector_destroy) are
 * copied by assessor/algorithm_invariants/instrument.py, which adds
 * observation counters to exact statement spellings; keep their tokens
 * unchanged.
 *
 * CD_MUTANT selects one deliberately wrong variant for the mutation campaign.
 */
#include "containers.h"

#ifndef CD_MUTANT
#define CD_MUTANT 0
#endif

/* Payload limit: cd_clone_batch admits at most this many payloads per call;
 * for cd_payload_append the limit applies to the vector's total count after
 * the append. */
enum
{
    PAYLOAD_LIMIT = 8
};

static int array_valid(const struct cd_array *a)
{
    return a && a->live && a->length <= a->capacity &&
           (!a->capacity || a->data);
}

/* Borrows p[0..cap); p may be NULL only for an empty (cap == 0) owner. */
enum cd_status cd_array_init(struct cd_array *a, int *p, size_t cap)
{
    if (!a || (!p && cap) || cap > SIZE_MAX / sizeof *p)
    {
        return CD_INVALID;
    }
    *a = (struct cd_array){p, 0, cap, 1};
    return CD_OK;
}

/* at == length appends.  Returns CD_FULL (array unchanged) at capacity. */
enum cd_status cd_array_insert(struct cd_array *a, size_t at, const int *value)
{
    if (!array_valid(a) || !value || at > a->length)
    {
        return CD_INVALID;
    }
    if (a->length == a->capacity)
    {
        return CD_FULL;
    }
    /* Read the scalar before shifting: value may alias a moved element. */
    int staged = *value;
    for (size_t i = a->length; i > at; --i)
    {
        a->data[i] = a->data[i - 1];
    }
    a->data[at] = CD_MUTANT == 1 ? *value : staged;
    ++a->length;
    return CD_OK;
}

enum cd_status cd_array_erase(struct cd_array *a, size_t at, int *out)
{
    if (!array_valid(a) || !out || at >= a->length)
    {
        return CD_INVALID;
    }
    int old = a->data[at];
    for (size_t i = at + 1; i < a->length; ++i)
    {
        a->data[i - 1] = a->data[i];
    }
    --a->length;
    *out = old;
    return CD_OK;
}

enum cd_status cd_array_get(const struct cd_array *a, size_t at, int *out)
{
    if (!array_valid(a) || !out || at >= a->length)
    {
        return CD_INVALID;
    }
    *out = a->data[at];
    return CD_OK;
}

/* Clears the control object only; the backing array stays the caller's.  A
 * second destroy, or any use afterwards, returns CD_INVALID. */
enum cd_status cd_array_destroy(struct cd_array *a)
{
    if (!array_valid(a))
    {
        return CD_INVALID;
    }
    a->data = 0;
    a->length = 0;
    a->capacity = 0;
    a->live = 0;
    return CD_OK;
}

/* |stride| as size_t; negating PTRDIFF_MIN directly would overflow. */
static size_t magnitude(ptrdiff_t stride)
{
    return stride < 0 ? (size_t)(-(stride + 1)) + 1 : (size_t)stride;
}

/* Index of element `position` of a view: first +/- position * |stride|.
 * Callers have already proved the walk stays inside [0, extent). */
static size_t view_index(const struct cd_view *v, size_t position)
{
    size_t delta = position * magnitude(v->stride);
    return v->stride < 0 ? v->first - delta : v->first + delta;
}

/* A view is an integer index model: element i is base[first + i * stride].
 * The last index is proved in range by dividing (the remaining distance by
 * |stride|) before any multiplication, so no intermediate can wrap.  An empty
 * view forms no endpoint and only needs first <= extent. */
static int view_valid(const struct cd_view *v)
{
    if (!v || (!v->base && v->extent) || !v->stride ||
        v->extent > SIZE_MAX / sizeof(int))
    {
        return 0;
    }
    if (!v->count)
    {
        return v->first <= v->extent;
    }
    if (v->first >= v->extent)
    {
        return 0;
    }
    size_t step = magnitude(v->stride);
    size_t moves = v->count - 1;
    return v->stride < 0 ? moves <= v->first / step
                         : moves <= (v->extent - 1 - v->first) / step;
}

enum cd_status cd_view_init(struct cd_view *v, const int *p, size_t extent,
                            size_t first, size_t count, ptrdiff_t stride)
{
    if (!v)
    {
        return CD_INVALID;
    }
    struct cd_view candidate = {p, extent, first, count, stride};
    if (!view_valid(&candidate))
    {
        return CD_INVALID;
    }
    *v = candidate;
    return CD_OK;
}

enum cd_status cd_view_get(const struct cd_view *v, size_t position, int *out)
{
    if (!view_valid(v) || !out || position >= v->count)
    {
        return CD_INVALID;
    }
    *out = v->base[view_index(v, position)];
    return CD_OK;
}

/* out may equal v.  An empty subview keeps the source's first index, because
 * a negative-stride endpoint one past the start is not representable. */
enum cd_status cd_view_sub(const struct cd_view *v, size_t start, size_t count,
                           struct cd_view *out)
{
    if (!view_valid(v) || !out || start > v->count || count > v->count - start)
    {
        return CD_INVALID;
    }
    struct cd_view candidate = *v;
    candidate.count = count;
    if (count)
    {
        candidate.first = view_index(v, start);
    }
    *out = candidate;
    return CD_OK;
}

/* Invariants: head < capacity (head == 0 when capacity == 0); count plus the
 * outstanding reservation never exceeds capacity.  head/count rather than
 * head/tail, so a full ring and an empty ring are distinguishable. */
static int ring_valid(const struct cd_ring *r)
{
    return r && (!r->capacity || r->data) && r->count <= r->capacity &&
           (r->capacity ? r->head < r->capacity : r->head == 0) &&
           r->reserved <= r->capacity - r->count;
}

/* Describes n slots starting at index `at` as at most two contiguous spans;
 * the second wraps to slot 0.  Requires n <= capacity. */
static void spans(const struct cd_ring *r, size_t at, size_t n,
                  struct cd_spans *out)
{
    size_t first = n;
    if (n && first > r->capacity - at)
    {
        first = r->capacity - at;
    }
    *out = (struct cd_spans){first ? r->data + at : 0, first,
                             n > first ? r->data : 0, n - first};
}

/* Borrows p[0..cap); p may be NULL only when cap is 0. */
enum cd_status cd_ring_init(struct cd_ring *r, int *p, size_t cap)
{
    if (!r || (!p && cap) || cap > SIZE_MAX / sizeof *p)
    {
        return CD_INVALID;
    }
    *r = (struct cd_ring){p, cap, 0, 0, 0, 0};
    return CD_OK;
}

/* The spans borrow ring storage and are invalidated by any mutation.  Returns
 * CD_BUSY while a reservation is open. */
enum cd_status cd_ring_read(const struct cd_ring *r, struct cd_spans *out)
{
    if (!ring_valid(r) || !out)
    {
        return CD_INVALID;
    }
    if (r->reserving)
    {
        return CD_BUSY;
    }
    spans(r, r->head, r->count, out);
    return CD_OK;
}

/* Exposes n free slots, starting at (head + count) mod capacity, for the
 * caller to fill, and blocks every other ring operation (CD_BUSY) until
 * cd_ring_commit.  The tail is computed by subtraction when head + count would
 * pass capacity, so nothing can overflow.  n larger than the free space
 * returns CD_FULL without opening a reservation. */
enum cd_status cd_ring_reserve(struct cd_ring *r, size_t n,
                               struct cd_spans *out)
{
    if (!ring_valid(r) || !out)
    {
        return CD_INVALID;
    }
    if (r->reserving)
    {
        return CD_BUSY;
    }
    if (n > r->capacity - r->count)
    {
        return CD_FULL;
    }
    size_t tail = r->capacity ? (r->head >= r->capacity - r->count
                                     ? r->head - (r->capacity - r->count)
                                     : r->head + r->count)
                              : 0;
    spans(r, tail, n, out);
    r->reserving = 1;
    r->reserved = n;
    return CD_OK;
}

/* Publishes the first n reserved slots, which the caller must have
 * initialized; n == 0 cancels.  Over-commit returns CD_INVALID and keeps the
 * reservation open. */
enum cd_status cd_ring_commit(struct cd_ring *r, size_t n)
{
    if (!ring_valid(r) || !r->reserving)
    {
        return CD_INVALID;
    }
    if (n > r->reserved && CD_MUTANT != 2)
    {
        return CD_INVALID;
    }
    r->count += n;
    r->reserved = 0;
    r->reserving = 0;
    return CD_OK;
}

/* Strong mode (partial == 0) admits the whole batch or returns CD_FULL with
 * nothing copied.  Partial mode copies the largest fitting prefix.  *written
 * is set only on CD_OK. */
enum cd_status cd_ring_push(struct cd_ring *r, const int *values, size_t n,
                            unsigned partial, size_t *written)
{
    if (!ring_valid(r) || !written || (!values && n) || partial > 1)
    {
        return CD_INVALID;
    }
    if (r->reserving)
    {
        return CD_BUSY;
    }
    size_t amount = n;
    if (amount > r->capacity - r->count)
    {
        if (!partial)
        {
            return CD_FULL;
        }
        amount = r->capacity - r->count;
    }
    struct cd_spans s;
    enum cd_status status = cd_ring_reserve(r, amount, &s);
    if (status != CD_OK)
    {
        return status;
    }
    for (size_t i = 0; i < s.first_count; ++i)
    {
        s.first[i] = values[i];
    }
    for (size_t i = 0; i < s.second_count; ++i)
    {
        s.second[i] = values[s.first_count + i];
    }
    cd_ring_commit(r, amount);
    *written = amount;
    return CD_OK;
}

/* All or none: fewer than n items returns CD_EMPTY and leaves out untouched. */
enum cd_status cd_ring_pop(struct cd_ring *r, int *out, size_t n)
{
    if (!ring_valid(r) || (!out && n))
    {
        return CD_INVALID;
    }
    if (r->reserving)
    {
        return CD_BUSY;
    }
    if (n > r->count)
    {
        return CD_EMPTY;
    }
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = r->data[r->head];
        if (++r->head == r->capacity)
        {
            r->head = 0;
        }
    }
    r->count -= n;
    return CD_OK;
}

static int vector_valid(const struct cd_vector *v)
{
    return v && v->live && v->allocator.take && v->allocator.release &&
           v->length <= v->capacity && (!v->capacity || v->data);
}

/* The vector owns its backing block through the supplied allocator and starts
 * with no storage. */
enum cd_status cd_vector_init(struct cd_vector *v,
                              struct cd_allocator allocator)
{
    if (!v || !allocator.take || !allocator.release)
    {
        return CD_INVALID;
    }
    *v = (struct cd_vector){0, 0, 0, allocator, 0, 1};
    return CD_OK;
}

/* Never shrinks.  Order matters for failure atomicity: acquire the new block,
 * copy, publish pointer and capacity, and only then release the old block.
 * CD_NOMEM leaves the old block, length and capacity intact; CD_FULL (also
 * unchanged) means adding the relocated length to the `copies` counter would
 * overflow size_t.  `copies` counts moved elements. */
enum cd_status cd_vector_reserve(struct cd_vector *v, size_t cap)
{
    if (!vector_valid(v) || cap > SIZE_MAX / sizeof(int))
    {
        return CD_INVALID;
    }
    if (cap <= v->capacity)
    {
        return CD_OK;
    }
    if (v->copies > SIZE_MAX - v->length)
    {
        return CD_FULL;
    }
    size_t visible = v->length;
    if (CD_MUTANT == 3)
    {
        ++v->length;
    }
    int *p = v->allocator.take(v->allocator.context, cap);
    v->length = visible;
    if (!p)
    {
        return CD_NOMEM;
    }
    for (size_t i = 0; i < v->length; ++i)
    {
        p[i] = v->data[i];
    }
    int *old = v->data;
    v->data = p;
    v->capacity = cap;
    v->copies += v->length;
    if (old)
    {
        v->allocator.release(v->allocator.context, old);
    }
    return CD_OK;
}

/* Growth policy: capacity 0 -> 1, then doubling; CD_FULL if doubling would
 * overflow size_t. */
enum cd_status cd_vector_insert(struct cd_vector *v, size_t at, int value)
{
    if (!vector_valid(v) || at > v->length)
    {
        return CD_INVALID;
    }
    if (v->length == v->capacity)
    {
        size_t cap = v->capacity ? v->capacity : 1;
        if (v->capacity)
        {
            if (cap > SIZE_MAX / 2)
            {
                return CD_FULL;
            }
            cap *= 2;
        }
        enum cd_status s = cd_vector_reserve(v, cap);
        if (s != CD_OK)
        {
            return s;
        }
    }
    for (size_t i = v->length; i > at; --i)
    {
        v->data[i] = v->data[i - 1];
    }
    v->data[at] = value;
    ++v->length;
    return CD_OK;
}

enum cd_status cd_vector_erase(struct cd_vector *v, size_t at, int *out)
{
    if (!vector_valid(v) || !out || at >= v->length)
    {
        return CD_INVALID;
    }
    int value = v->data[at];
    for (size_t i = at + 1; i < v->length; ++i)
    {
        if (CD_MUTANT != 8)
        {
            v->data[i - 1] = v->data[i];
        }
    }
    --v->length;
    *out = value;
    return CD_OK;
}

/* Relocates into a block of exactly length elements, or frees the block when
 * the vector is empty.  CD_NOMEM keeps the old block; CD_FULL (also unchanged)
 * means adding the relocated length to the `copies` counter would overflow
 * size_t. */
enum cd_status cd_vector_shrink(struct cd_vector *v)
{
    if (!vector_valid(v))
    {
        return CD_INVALID;
    }
    if (v->length == v->capacity)
    {
        return CD_OK;
    }
    if (v->copies > SIZE_MAX - v->length)
    {
        return CD_FULL;
    }
    int *p = 0;
    if (v->length)
    {
        p = v->allocator.take(v->allocator.context, v->length);
        if (!p)
        {
            return CD_NOMEM;
        }
        for (size_t i = 0; i < v->length; ++i)
        {
            p[i] = v->data[i];
        }
    }
    int *old = v->data;
    v->data = p;
    v->capacity = v->length;
    v->copies += v->length;
    if (old)
    {
        v->allocator.release(v->allocator.context, old);
    }
    return CD_OK;
}

/* Releases the block exactly once; every borrow of the data is invalidated. */
enum cd_status cd_vector_destroy(struct cd_vector *v)
{
    if (!vector_valid(v))
    {
        return CD_INVALID;
    }
    int *old = v->data;
    v->data = 0;
    v->length = v->capacity = 0;
    v->live = 0;
    if (old)
    {
        v->allocator.release(v->allocator.context, old);
    }
    return CD_OK;
}

/* Destroys payloads made[count-1] .. made[0], newest first.  Undoes a partly
 * built clone prefix. */
static void destroy_payloads_newest_first(const struct cd_payload_ops *ops,
                                          struct cd_payload *made, size_t count)
{
    while (count)
    {
        ops->destroy(ops->context, &made[--count]);
    }
}

/* Clones n payloads into scratch first.  If clone k fails, the k successful
 * clones are destroyed in reverse and out is untouched (CD_FAILED).  On
 * success the resources move into out; the scratch copies are not destroyed. */
enum cd_status cd_clone_batch(struct cd_payload *out,
                              const struct cd_payload *input, size_t n,
                              struct cd_payload_ops ops)
{
    if ((!out && n) || (!input && n) || n > PAYLOAD_LIMIT || !ops.clone ||
        !ops.destroy)
    {
        return CD_INVALID;
    }
    struct cd_payload scratch[PAYLOAD_LIMIT];
    for (size_t made = 0; made < n; ++made)
    {
        if (ops.clone(ops.context, &input[made], &scratch[made]))
        {
            destroy_payloads_newest_first(&ops, scratch, made);
            return CD_FAILED;
        }
    }
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = scratch[i];
    }
    return CD_OK;
}

enum cd_status cd_payload_init(struct cd_payload_vector *v,
                               struct cd_payload_allocator a,
                               struct cd_payload_ops ops)
{
    if (!v || !a.take || !a.release || !ops.clone || !ops.destroy)
    {
        return CD_INVALID;
    }
    *v = (struct cd_payload_vector){0, 0, a, ops};
    return CD_OK;
}

/* Builds the replacement block completely before publishing it.  The existing
 * payloads are copied bitwise (ownership moves with them); only the new suffix
 * is cloned.  On a clone failure the new suffix is destroyed, the replacement
 * block released and the vector keeps its old block (CD_FAILED). */
enum cd_status cd_payload_append(struct cd_payload_vector *v,
                                 const struct cd_payload *input, size_t n)
{
    if (!v || (!input && n) || !v->allocator.take || !v->allocator.release ||
        !v->ops.clone || !v->ops.destroy || v->count > PAYLOAD_LIMIT ||
        n > PAYLOAD_LIMIT - v->count)
    {
        return CD_INVALID;
    }
    if (!n)
    {
        return CD_OK;
    }
    struct cd_payload *replacement =
        v->allocator.take(v->allocator.context, v->count + n);
    if (!replacement)
    {
        return CD_NOMEM;
    }
    for (size_t i = 0; i < v->count; ++i)
    {
        replacement[i] = v->data[i];
    }
    struct cd_payload *suffix = replacement + v->count;
    for (size_t made = 0; made < n; ++made)
    {
        if (v->ops.clone(v->ops.context, &input[made], &suffix[made]))
        {
            destroy_payloads_newest_first(&v->ops, suffix, made);
            v->allocator.release(v->allocator.context, replacement);
            return CD_FAILED;
        }
    }
    struct cd_payload *old = v->data;
    v->data = replacement;
    v->count += n;
    if (old)
    {
        v->allocator.release(v->allocator.context, old);
    }
    return CD_OK;
}

/* Destroys every payload newest first, releases the block and disables the
 * allocator, so a repeated destroy is refused.  v->count is decremented
 * before each destroy callback, so a callback that reads the published count
 * sees only the payloads not yet destroyed. */
enum cd_status cd_payload_destroy(struct cd_payload_vector *v)
{
    if (!v || !v->allocator.release || !v->ops.destroy)
    {
        return CD_INVALID;
    }
    while (v->count)
    {
        --v->count;
        v->ops.destroy(v->ops.context, &v->data[v->count]);
    }
    if (v->data)
    {
        v->allocator.release(v->allocator.context, v->data);
    }
    v->data = 0;
    v->allocator.take = 0;
    v->allocator.release = 0;
    return CD_OK;
}
