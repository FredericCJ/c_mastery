#include "core.h"

int md_write(unsigned char *arena, size_t capacity, size_t offset,
             const unsigned char *input, size_t n)
{
    return 0;
}

int md_extent(uint32_t count, uint32_t stride, uint32_t limit, uint32_t *out)
{
    return 0;
}

int md_read(const struct md_arena *arena, uint32_t generation, unsigned index,
            unsigned char *out)
{
    return 0;
}

int md_authorize(const struct md_arena *arena, unsigned principal,
                 uint32_t generation, unsigned begin, unsigned length)
{
    return 0;
}

int md_dispatch(unsigned operation, unsigned grant, unsigned *sentinel)
{
    return 0;
}

int md_log_copy(const char *input, size_t n, char *output, size_t capacity)
{
    return 0;
}

int md_release(struct md_owner *owner)
{
    return 0;
}

int md_equal(const unsigned char *a, const unsigned char *b, size_t n,
             size_t *steps)
{
    return 0;
}
