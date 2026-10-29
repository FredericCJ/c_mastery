#include "sketch.h"
#ifndef SK_MUTANT
#define SK_MUTANT 0
#endif
static int valid(const struct sk_mh_config *c)
{
    unsigned i, j;
    if (c->universe < 1 || c->universe > 16 || c->count < 1 || c->count > 120)
        return 0;
    for (i = 0; i < c->count; i++)
    {
        unsigned seen = 0;
        for (j = 0; j < c->universe; j++)
        {
            unsigned rank = c->rank[i][j];
            if (rank >= c->universe || (seen & (1u << rank)))
                return 0;
            seen |= 1u << rank;
        }
    }
    return 1;
}

static int same(const struct sk_mh_config *a, const struct sk_mh_config *b)
{
    unsigned i, j;
    if (a->universe != b->universe || a->count != b->count)
        return 0;
    for (i = 0; i < a->count; i++)
        for (j = 0; j < a->universe; j++)
            if (a->rank[i][j] != b->rank[i][j])
                return 0;
    return 1;
}

int sk_mh_init(struct sk_mh *s, const struct sk_mh_config *c)
{
    struct sk_mh next = {0};
    unsigned i;
    if (!valid(c))
        return SK_INVALID;
    next.config = *c;
    for (i = 0; i < c->count; i++)
        next.minimum[i] = (uint8_t)c->universe;
    *s = next;
    return SK_OK;
}

int sk_mh_add(struct sk_mh *s, unsigned element)
{
    unsigned i;
    if (element >= s->config.universe)
        return SK_INVALID;
    for (i = 0; i < s->config.count; i++)
    {
        unsigned rank = s->config.rank[i][element];
        if (rank < s->minimum[i])
            s->minimum[i] = (uint8_t)(SK_MUTANT == 1 ? 0 : rank);
    }
    return SK_OK;
}

int sk_mh_similarity(const struct sk_mh *a, const struct sk_mh *b,
                     struct sk_fraction *out)
{
    unsigned i;
    struct sk_fraction value = {0, a->config.count};
    if (!same(&a->config, &b->config))
        return SK_INVALID;
    for (i = 0; i < a->config.count; i++)
        if (a->minimum[i] == b->minimum[i])
            value.numerator++;
    *out = value;
    return SK_OK;
}

int sk_mh_merge(struct sk_mh *dst, const struct sk_mh *src)
{
    unsigned i;
    if (!same(&dst->config, &src->config))
        return SK_INVALID;
    for (i = 0; i < dst->config.count; i++)
        if (src->minimum[i] < dst->minimum[i])
            dst->minimum[i] = src->minimum[i];
    return SK_OK;
}

int sk_mh_bands(const struct sk_mh *a, const struct sk_mh *b, unsigned rows,
                unsigned *out)
{
    unsigned i, j, total = 0;
    if (!same(&a->config, &b->config) || !rows || rows > a->config.count ||
        a->config.count % rows)
        return SK_INVALID;
    /* Empty sets never enter candidate retrieval, even two empty signatures. */
    if (a->minimum[0] == a->config.universe ||
        b->minimum[0] == b->config.universe)
    {
        *out = 0;
        return SK_OK;
    }
    for (i = 0; i < a->config.count; i += rows)
    {
        unsigned equal = 1;
        for (j = 0; j < rows; j++)
            if (a->minimum[i + j] != b->minimum[i + j])
                equal = 0;
        total += SK_MUTANT == 2 ? 1 : equal;
    }
    *out = total;
    return SK_OK;
}
