#include "effects.h"

enum ef_status ef_checksum(const unsigned char *p, size_t n, uint32_t *out)
{
    (void)p;
    (void)n;
    (void)out;
    return EF_INVALID;
}

enum ef_status ef_cached(struct ef_cache *c, const unsigned char *p, size_t n,
                         uint32_t *out)
{
    (void)c;
    (void)p;
    (void)n;
    (void)out;
    return EF_INVALID;
}

enum ef_status ef_cache_reset(struct ef_cache *c)
{
    (void)c;
    return EF_INVALID;
}

enum ef_status ef_cache_borrow(struct ef_cache *c, const unsigned char **out)
{
    (void)c;
    (void)out;
    return EF_INVALID;
}

enum ef_status ef_cache_return(struct ef_cache *c)
{
    (void)c;
    return EF_INVALID;
}

enum ef_status ef_pool_take(struct ef_pool *p, unsigned char **out)
{
    (void)p;
    (void)out;
    return EF_INVALID;
}

enum ef_status ef_pool_return(struct ef_pool *p, unsigned index)
{
    (void)p;
    (void)index;
    return EF_INVALID;
}

enum ef_status ef_parse(struct ef_parser *p, const unsigned char *d, size_t n,
                        struct ef_view *out)
{
    (void)p;
    (void)d;
    (void)n;
    (void)out;
    return EF_INVALID;
}

enum ef_status ef_view_release(struct ef_view *v)
{
    (void)v;
    return EF_INVALID;
}

enum ef_status ef_parser_reset(struct ef_parser *p, uint32_t seed)
{
    (void)p;
    (void)seed;
    return EF_INVALID;
}

uint32_t ef_random(struct ef_parser *p)
{
    (void)p;
    return 0;
}

enum ef_status ef_attempt(ef_callback_fn f, void *ctx, unsigned nonblocking,
                          unsigned attempts, unsigned *used)
{
    (void)f;
    (void)ctx;
    (void)nonblocking;
    (void)attempts;
    (void)used;
    return EF_INVALID;
}

enum ef_status ef_sort(int *a, size_t n)
{
    (void)a;
    (void)n;
    return EF_INVALID;
}

enum ef_status ef_sort_records(struct ef_record *a, size_t n)
{
    (void)a;
    (void)n;
    return EF_INVALID;
}
