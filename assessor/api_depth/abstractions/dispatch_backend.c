#include "dispatch.h"
#include <stdint.h>

static ptrdiff_t accept(struct ad_sink *s, const unsigned char *p, size_t n,
                        size_t quota)
{
    if (!s || (!p && n) || s->length > 66 || n > 66 - s->length ||
        s->calls == SIZE_MAX || n > SIZE_MAX - s->offered)
    {
        return -1;
    }
    ++s->calls;
    s->offered += n;
    if (s->mode == 1)
    {
        s->mode = 0;
        return -2;
    }
    if (s->mode == 2)
    {
        return 0;
    }
    if (s->mode == 3)
    {
        return -1;
    }
    if (s->mode == 4)
    {
        return (ptrdiff_t)n +
               1; /* deliberately invalid callback contract, no out-of-bounds access */
    }
    if (n > quota)
    {
        n = quota;
    }
#ifdef AD_MUTANT_BYTE
    for (size_t i = 0; i < n; ++i)
    {
        s->bytes[s->length++] = (unsigned char)(p[i] ^ 1u);
    }
#else
    for (size_t i = 0; i < n; ++i)
    {
        s->bytes[s->length++] = p[i];
    }
#endif
    return (ptrdiff_t)n;
}

ptrdiff_t ad_one(struct ad_sink *s, const unsigned char *p, size_t n)
{
    return accept(s, p, n, 1);
}

ptrdiff_t ad_four(struct ad_sink *s, const unsigned char *p, size_t n)
{
    return accept(s, p, n, 4);
}

ptrdiff_t ad_all(struct ad_sink *s, const unsigned char *p, size_t n)
{
    return accept(s, p, n, 66);
}

const struct ad_ops *ad_operations(unsigned kind)
{
    static const struct ad_ops ops[3] = {
        {1, 0, ad_one}, {1, 1, ad_four}, {1, 2, ad_all}};
    return kind < 3 ? ops + kind : 0;
}
