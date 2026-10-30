#include "effects.h"

/*
 * Learner scaffold: none of these components is implemented yet. Every
 * function returns EF_INVALID (ef_random returns 0) and ignores its
 * arguments. Replace the bodies using effects.h and CONTRACTS.md.
 */

enum ef_status ef_checksum(const unsigned char *data, size_t length,
                           uint32_t *out)
{
    (void)data;
    (void)length;
    (void)out;
    return EF_INVALID;
}

enum ef_status ef_cached(struct ef_cache *cache, const unsigned char *data,
                         size_t length, uint32_t *out)
{
    (void)cache;
    (void)data;
    (void)length;
    (void)out;
    return EF_INVALID;
}

enum ef_status ef_cache_reset(struct ef_cache *cache)
{
    (void)cache;
    return EF_INVALID;
}

enum ef_status ef_cache_borrow(struct ef_cache *cache,
                               const unsigned char **out)
{
    (void)cache;
    (void)out;
    return EF_INVALID;
}

enum ef_status ef_cache_return(struct ef_cache *cache)
{
    (void)cache;
    return EF_INVALID;
}

enum ef_status ef_pool_take(struct ef_pool *pool, unsigned char **out)
{
    (void)pool;
    (void)out;
    return EF_INVALID;
}

enum ef_status ef_pool_return(struct ef_pool *pool, unsigned index)
{
    (void)pool;
    (void)index;
    return EF_INVALID;
}

enum ef_status ef_parse(struct ef_parser *parser, const unsigned char *data,
                        size_t length, struct ef_view *out)
{
    (void)parser;
    (void)data;
    (void)length;
    (void)out;
    return EF_INVALID;
}

enum ef_status ef_view_release(struct ef_view *view)
{
    (void)view;
    return EF_INVALID;
}

enum ef_status ef_parser_reset(struct ef_parser *parser, uint32_t seed)
{
    (void)parser;
    (void)seed;
    return EF_INVALID;
}

uint32_t ef_random(struct ef_parser *parser)
{
    (void)parser;
    return 0;
}

enum ef_status ef_attempt(ef_callback_fn callback, void *context,
                          unsigned nonblocking, unsigned attempts,
                          unsigned *used)
{
    (void)callback;
    (void)context;
    (void)nonblocking;
    (void)attempts;
    (void)used;
    return EF_INVALID;
}

enum ef_status ef_sort(int *values, size_t count)
{
    (void)values;
    (void)count;
    return EF_INVALID;
}

enum ef_status ef_sort_records(struct ef_record *records, size_t count)
{
    (void)records;
    (void)count;
    return EF_INVALID;
}
