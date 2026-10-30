/*
 * Reference implementation of the effects laboratory components.
 *
 * First-principles code: only the exact-width integer and size types of
 * effects.h are used, with no libc allocator, parser or sorter. Storage and
 * attempt behavior arrive through caller-supplied callbacks. Instances are not
 * thread safe; callers serialize access.
 *
 * Mutants (EF_MUTANT, set by the campaign build): 1 lets reset ignore
 * borrows, 2 lets a parser hand out a second view, 3 breaks sort stability.
 */
#include "effects.h"
#ifndef EF_MUTANT
#define EF_MUTANT 0
#endif

#define EF_ATTEMPT_LIMIT 16u
#define EF_ATTEMPT_BUFFER_BYTES 16u
#define EF_SORT_LIMIT 16u

/* Numerical Recipes linear congruential generator: seed * a + c modulo 2^32. */
#define EF_LCG_MULTIPLIER 1664525u
#define EF_LCG_INCREMENT 1013904223u

/* Each byte is at most 255 and there are at most 32 of them: sum <= 8160. */
enum ef_status ef_checksum(const unsigned char *data, size_t length,
                           uint32_t *out)
{
    if (!out || (!data && length) || length > EF_SCRATCH_BYTES)
    {
        return EF_INVALID;
    }
    uint32_t value = 0;
    for (size_t i = 0; i < length; ++i)
    {
        value += data[i];
    }
    *out = value;
    return EF_OK;
}

/*
 * Makes sure the cache owns scratch storage. On allocator failure sets the
 * failure latch and returns EF_NOMEM with the cache otherwise unchanged; on
 * success clears the latch.
 */
static enum ef_status cache_acquire(struct ef_cache *cache)
{
    if (cache->scratch)
    {
        return EF_OK;
    }
    unsigned char *storage =
        cache->allocator.take(cache->allocator.context, EF_SCRATCH_BYTES);
    if (!storage)
    {
        cache->failed = 1;
        return EF_NOMEM;
    }
    cache->scratch = storage;
    cache->failed = 0;
    return EF_OK;
}

enum ef_status ef_cached(struct ef_cache *cache, const unsigned char *data,
                         size_t length, uint32_t *out)
{
    if (!cache || !out || (!data && length) || length > EF_SCRATCH_BYTES ||
        !cache->generation || !cache->allocator.take ||
        !cache->allocator.release)
    {
        return EF_INVALID;
    }
    if (cache->borrows)
    {
        return EF_BUSY;
    }
    if (cache->failed && cache->permanent)
    {
        return EF_FAILED;
    }
    enum ef_status status = cache_acquire(cache);
    if (status != EF_OK)
    {
        return status;
    }
    /* Only the admitted prefix is copied; the suffix is never read back. */
    for (size_t i = 0; i < length; ++i)
    {
        cache->scratch[i] = data[i];
    }
    return ef_checksum(cache->scratch, length, out);
}

enum ef_status ef_cache_reset(struct ef_cache *cache)
{
    if (!cache || !cache->generation || !cache->allocator.release)
    {
        return EF_INVALID;
    }
    if (EF_MUTANT != 1 && cache->borrows)
    {
        return EF_BUSY;
    }
    if (cache->generation == UINT32_MAX)
    {
        return EF_FULL;
    }
    if (cache->scratch)
    {
        cache->allocator.release(cache->allocator.context, cache->scratch);
    }
    cache->scratch = 0;
    cache->failed = 0;
    ++cache->generation;
    return EF_OK;
}

enum ef_status ef_cache_borrow(struct ef_cache *cache,
                               const unsigned char **out)
{
    if (!cache || !out || !cache->scratch)
    {
        return EF_INVALID;
    }
    if (cache->borrows == UINT32_MAX)
    {
        return EF_FULL;
    }
    ++cache->borrows;
    *out = cache->scratch;
    return EF_OK;
}

enum ef_status ef_cache_return(struct ef_cache *cache)
{
    if (!cache || !cache->borrows)
    {
        return EF_INVALID;
    }
    --cache->borrows;
    return EF_OK;
}

static uint32_t pool_slot_bit(unsigned slot)
{
    return UINT32_C(1) << slot;
}

enum ef_status ef_pool_take(struct ef_pool *pool, unsigned char **out)
{
    if (!pool || !out)
    {
        return EF_INVALID;
    }
    if (pool->requests == UINT32_MAX)
    {
        return EF_FULL;
    }
    /* A full pool still counts the request: demand is observable. */
    ++pool->requests;
    for (unsigned slot = 0; slot < EF_POOL_SLOTS; ++slot)
    {
        if (!(pool->used & pool_slot_bit(slot)))
        {
            pool->used |= pool_slot_bit(slot);
            *out = &pool->bytes[slot];
            return EF_OK;
        }
    }
    return EF_FULL;
}

enum ef_status ef_pool_return(struct ef_pool *pool, unsigned index)
{
    if (!pool || index >= EF_POOL_SLOTS || !(pool->used & pool_slot_bit(index)))
    {
        return EF_INVALID;
    }
    pool->used &= ~pool_slot_bit(index);
    return EF_OK;
}

static unsigned char to_upper_ascii(unsigned char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return (unsigned char)(c - 'a' + 'A');
    }
    return c;
}

enum ef_status ef_parse(struct ef_parser *parser, const unsigned char *data,
                        size_t length, struct ef_view *out)
{
    if (!parser || !out || !parser->generation || (!data && length) ||
        length > EF_SCRATCH_BYTES)
    {
        return EF_INVALID;
    }
    if (EF_MUTANT != 2 && parser->borrowed)
    {
        return EF_BUSY;
    }
    /* Reject before any write so a refused call changes nothing. */
    if (parser->cursor > SIZE_MAX - length)
    {
        return EF_FULL;
    }
    for (size_t i = 0; i < length; ++i)
    {
        parser->scratch[i] = to_upper_ascii(data[i]);
    }
    parser->cursor += length;
    parser->borrowed = 1;
    out->owner = parser;
    out->generation = parser->generation;
    out->length = length;
    out->data = parser->scratch;
    return EF_OK;
}

enum ef_status ef_view_release(struct ef_view *view)
{
    if (!view || !view->owner || !view->owner->borrowed ||
        view->generation != view->owner->generation)
    {
        return EF_INVALID;
    }
    view->owner->borrowed = 0;
    view->owner = 0;
    view->data = 0;
    view->length = 0;
    return EF_OK;
}

enum ef_status ef_parser_reset(struct ef_parser *parser, uint32_t seed)
{
    if (!parser || !parser->generation)
    {
        return EF_INVALID;
    }
    if (parser->borrowed)
    {
        return EF_BUSY;
    }
    if (parser->generation == UINT32_MAX)
    {
        return EF_FULL;
    }
    parser->cursor = 0;
    parser->seed = seed;
    ++parser->generation;
    for (unsigned i = 0; i < EF_SCRATCH_BYTES; ++i)
    {
        parser->scratch[i] = 0;
    }
    return EF_OK;
}

uint32_t ef_random(struct ef_parser *parser)
{
    /* uint32_t arithmetic wraps modulo 2^32, which is the recurrence. */
    parser->seed = parser->seed * EF_LCG_MULTIPLIER + EF_LCG_INCREMENT;
    return parser->seed;
}

enum ef_status ef_attempt(ef_callback_fn callback, void *context,
                          unsigned nonblocking, unsigned attempts,
                          unsigned *used)
{
    if (!callback || !used || attempts > EF_ATTEMPT_LIMIT || !nonblocking)
    {
        return EF_INVALID;
    }
    unsigned char buffer[EF_ATTEMPT_BUFFER_BYTES];
    *used = 0;
    while (*used < attempts)
    {
        int result = callback(context, buffer, sizeof buffer);
        ++*used;
        if (result > 0)
        {
            return EF_OK;
        }
        if (result < 0)
        {
            return EF_FAILED;
        }
    }
    return EF_INCOMPLETE;
}

/*
 * Stable insertion sort. The comparison is a > b, never a subtraction, so
 * INT_MIN and INT_MAX cannot overflow. At most 120 comparisons for 16 items.
 */
enum ef_status ef_sort(int *values, size_t count)
{
    if ((!values && count) || count > EF_SORT_LIMIT)
    {
        return EF_INVALID;
    }
    for (size_t i = 1; i < count; ++i)
    {
        int value = values[i];
        size_t hole = i;
        while (hole && values[hole - 1] > value)
        {
            values[hole] = values[hole - 1];
            --hole;
        }
        values[hole] = value;
    }
    return EF_OK;
}

static void record_copy(struct ef_record *to, const struct ef_record *from)
{
    to->key = from->key;
    to->identity = from->identity;
    for (unsigned i = 0; i < EF_RECORD_PAYLOAD_BYTES; ++i)
    {
        to->payload[i] = from->payload[i];
    }
}

/*
 * Whether the record before the hole must move right. Strictly greater keeps
 * equal keys in their original order; mutant 3 uses >= and loses stability.
 */
static int must_shift_right(int earlier_key, int value_key)
{
    if (EF_MUTANT == 3)
    {
        return earlier_key >= value_key;
    }
    return earlier_key > value_key;
}

enum ef_status ef_sort_records(struct ef_record *records, size_t count)
{
    if ((!records && count) || count > EF_SORT_LIMIT)
    {
        return EF_INVALID;
    }
    for (size_t i = 1; i < count; ++i)
    {
        struct ef_record value;
        record_copy(&value, &records[i]);
        size_t hole = i;
        while (hole && must_shift_right(records[hole - 1].key, value.key))
        {
            record_copy(&records[hole], &records[hole - 1]);
            --hole;
        }
        record_copy(&records[hole], &value);
    }
    return EF_OK;
}
