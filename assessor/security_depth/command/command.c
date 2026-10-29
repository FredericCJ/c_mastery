#include "command.h"

static enum sc_result parse(const unsigned char *p, size_t n,
                            const struct sc_authority *a, int extended,
                            unsigned depth, struct sc_batch *b,
                            struct sc_meter *m, int dry)
{
    unsigned op, count, i, opt = 0, cost;
    size_t offset = 2, expected;
    struct sc_command c = {0};
    enum sc_result r;
    if (n < 2)
    {
        return SC_LENGTH;
    }
    ++m->headers;
    op = p[0];
    count = p[1];
    if (op == 3 && extended)
    {
        size_t position = 2;
        if (depth >= 2)
        {
            return SC_BUDGET;
        }
        if (count > 4)
        {
            return SC_COUNT;
        }
        for (i = 0; i < count; ++i)
        {
            size_t length;
            if (position >= n)
            {
                return SC_LENGTH;
            }
            length = p[position++];
            if (length > n - position)
            {
                return SC_LENGTH;
            }
            r = parse(p + position, length, a, extended, depth + 1, b, m, dry);
            if (r != SC_OK)
            {
                return r;
            }
            position += length;
        }
        return position == n ? SC_OK : SC_LENGTH;
    }
    if (op == 5 && extended)
    {
        if (n != 4)
        {
            return SC_LENGTH;
        }
#ifdef SC_MUTANT_EXPANSION
        if (count > 4)
        {
            count = 4; /* Harmless incorrect clipping, no out-of-range write. */
        }
#endif
        if (count > 4)
        {
            return SC_BUDGET;
        }
        c.opcode = 1;
        c.count = count;
        cost = count ? count : 1;
        if (cost > 4 - b->work || b->count == 4)
        {
            return SC_BUDGET;
        }
        for (i = 0; !dry && i < count; ++i)
        {
            c.values[i] = (uint16_t)((unsigned)p[2] * 256u + p[3]);
            ++m->values;
        }
        b->commands[b->count++] = c;
        b->work += cost;
        return SC_OK;
    }
    if (op != 1 && op != 2 && !(extended && op == 4))
    {
        return SC_OPCODE;
    }
    if (count > 4)
    {
        return SC_COUNT;
    }
    if (op == 4)
    {
        if (n < 4)
        {
            return SC_LENGTH;
        }
        if (p[3] > 1)
        {
            return SC_INVALID;
        }
        opt = p[3];
        offset = 4;
        op = 2; /* Claimed p[2] role is never authority. */
    }
    expected = offset + 2u * count + 2u * opt;
#ifdef SC_MUTANT_TRAILING
    if (n < expected)
#else
    if (n != expected)
#endif
        return SC_LENGTH;
    cost = (count ? count : 1) + opt;
    if (cost > 4 - b->work || b->count == 4)
    {
        return SC_BUDGET;
    }
#ifndef SC_MUTANT_AUTHORITY
    if (op == 2 && (!a->allowed || a->generation == UINT_MAX))
    {
        return SC_DENIED;
    }
#endif
    c.opcode = op;
    c.count = count;
    c.generation = op == 2 ? a->generation : 0;
    for (i = 0; !dry && i < count; ++i)
    {
        c.values[i] = (uint16_t)((unsigned)p[offset + 2 * i] * 256u +
                                 p[offset + 2 * i + 1]);
        ++m->values;
    }
    b->commands[b->count++] = c;
    b->work += cost;
    return SC_OK;
}

enum sc_result sc_decode(const unsigned char *p, size_t n,
                         const struct sc_authority *a, int extended,
                         struct sc_batch *out, struct sc_meter *m)
{
    struct sc_batch b = {0};
    struct sc_meter local = {0};
    unsigned char stable[32];
    size_t i;
    enum sc_result r;
    if (!a || !out || !m || (!p && n))
    {
        return SC_INVALID;
    }
    if (!a->generation || a->allowed > 1)
    {
        return SC_INVALID;
    }
    if (n > 32)
    {
        *m = local;
        return extended ? SC_BUDGET : SC_LENGTH;
    }
    for (i = 0; i < n; ++i)
    {
        stable[i] = p[i];
    }
    local.bytes = (unsigned)n;
    r = parse(stable, n, a, extended, 0, &b, &local, 1);
    if (r == SC_OK)
    {
        struct sc_batch materialized = {0};
        struct sc_meter material_meter = {0};
        r = parse(stable, n, a, extended, 0, &materialized, &material_meter, 0);
        if (r == SC_OK)
        {
            b = materialized;
            local.values = material_meter.values;
        }
    }
    *m = local;
#ifndef SC_MUTANT_PUBLICATION
    if (r == SC_OK)
#endif
        *out = b;
    return r;
}

enum sc_result sc_revoke(struct sc_authority *a)
{
    if (!a || !a->generation || a->allowed > 1)
    {
        return SC_INVALID;
    }
    a->allowed = 0;
    if (a->generation == UINT_MAX)
    {
        return SC_EXHAUSTED;
    }
    ++a->generation;
    return SC_OK;
}

enum sc_result sc_apply(const struct sc_batch *b, const struct sc_authority *a,
                        uint32_t *ordinary, uint32_t *privileged)
{
    uint32_t add = 0, effects = 0;
    unsigned i, j, work = 0;
    if (!b || !a || !ordinary || !privileged || b->count > 4 || b->work > 4 ||
        !a->generation || a->allowed > 1)
    {
        return SC_INVALID;
    }
    for (i = 0; i < b->count; ++i)
    {
        const struct sc_command *c = &b->commands[i];
        if ((c->opcode != 1 && c->opcode != 2) || c->count > 4)
        {
            return SC_INVALID;
        }
        work += c->count ? c->count : 1;
        if (work > 4)
        {
            return SC_BUDGET;
        }
        if (c->opcode == 2)
        {
#ifndef SC_MUTANT_STALE
            if (!a->allowed || a->generation == UINT_MAX ||
                c->generation != a->generation)
            {
                return SC_STALE;
            }
#endif
            ++effects;
        }
        else
        {
            for (j = 0; j < c->count; ++j)
            {
                add += c->values[j];
            }
        }
    }
    if (work > b->work)
    {
        return SC_INVALID;
    }
    if (add > UINT32_MAX - *ordinary || effects > UINT32_MAX - *privileged)
    {
        return SC_EXHAUSTED;
    }
    *ordinary += add;
    *privileged += effects;
    return SC_OK;
}

enum sc_result sc_decimal(const unsigned char *p, size_t n, int zeros,
                          unsigned *out, char canonical[2])
{
    unsigned value = 0;
    size_t i;
    if (!p || !out || !canonical || !n)
    {
        return SC_INVALID;
    }
    if (n > 32)
    {
        return SC_BUDGET;
    }
    if (!zeros && n > 1 && p[0] == '0')
    {
        return SC_INVALID;
    }
    for (i = 0; i < n; ++i)
    {
        unsigned digit;
        if (p[i] < '0' || p[i] > '9')
        {
            return SC_INVALID;
        }
        digit = p[i] - '0';
        if (value > 4 / 10 || digit > 4 - value * 10)
        {
            return SC_COUNT;
        }
        value = value * 10 + digit;
    }
    *out = value;
    canonical[0] = (char)('0' + value);
    canonical[1] = 0;
    return SC_OK;
}

enum sc_result sc_extent(size_t offset, size_t count, size_t width,
                         size_t limit, size_t *out)
{
    if (!out)
    {
        return SC_INVALID;
    }
    if (offset > limit || (width && count
#ifdef SC_MUTANT_EXTENT
                                        >=
#else
                                        >
#endif
                                        (limit - offset) / width))
        return SC_COUNT;
    *out = offset + count * width;
    return SC_OK;
}
