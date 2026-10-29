#include "unicode.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8, "This UTF-8 storage profile requires octets");

static unsigned width(unsigned b)
{
    return b < 0x80                 ? 1
           : b >= 0xc2 && b <= 0xdf ? 2
           : b >= 0xe0 && b <= 0xef ? 3
           : b >= 0xf0 && b <= 0xf4 ? 4
                                    : 0;
}

static int continuation(const struct lu_state *s, unsigned b)
{
    if (b < 0x80 || b > 0xbf)
    {
        return 0;
    }
#ifndef LU_MUTANT_RANGE
    if (s->used == 1 && ((s->bytes[0] == 0xe0 && b < 0xa0) ||
                         (s->bytes[0] == 0xed && b > 0x9f) ||
                         (s->bytes[0] == 0xf0 && b < 0x90) ||
                         (s->bytes[0] == 0xf4 && b > 0x8f)))
    {
        return 0;
    }
#endif
    return 1;
}

static uint32_t scalar(const struct lu_state *s)
{
    unsigned n = width(s->bytes[0]);
    uint32_t u = s->bytes[0] & (n == 1   ? 0x7fu
                                : n == 2 ? 0x1fu
                                : n == 3 ? 0x0fu
                                         : 0x07u);
    for (unsigned i = 1; i < n; ++i)
    {
        u = (u << 6) | (s->bytes[i] & 0x3fu);
    }
    return u;
}

static enum lu_status walk(struct lu_state *s, const unsigned char *b, size_t n,
                           uint32_t *out, size_t cap, enum lu_policy p,
                           size_t *count)
{
    size_t i = 0, k = 0;
    while (i < n)
    {
        uint32_t u = 0;
        int emit = 0;
        unsigned x = b[i];
        if (!s->used)
        {
            unsigned w = width(x);
            if (!w)
            {
                if (p == LU_STRICT)
                {
                    return LU_INVALID;
                }
                u = 0xfffd;
                emit = 1;
                ++i;
            }
            else
            {
                s->bytes[0] = (unsigned char)x;
                s->used = 1;
                ++i;
                if (w == 1)
                {
                    u = x;
                    s->used = 0;
                    emit = 1;
                }
            }
        }
        else if (!continuation(s, x))
        {
            if (p == LU_STRICT)
            {
                return LU_INVALID;
            }
            s->used = 0;
            u = 0xfffd;
            emit =
                1; /* Reprocess the offending byte, never swallow a new starter. */
#ifdef LU_MUTANT_SWALLOW
            ++i;
#endif
        }
        else
        {
            s->bytes[s->used++] = (unsigned char)x;
            ++i;
            if (s->used == width(s->bytes[0]))
            {
                u = scalar(s);
                s->used = 0;
                emit = 1;
            }
        }
        if (emit)
        {
            if (k == cap)
            {
                return LU_FULL;
            }
            if (out)
            {
                out[k] = u;
            }
            ++k;
        }
    }
    *count = k;
    return s->used ? LU_INCOMPLETE : LU_OK;
}

enum lu_status lu_decode(struct lu_state *s, const unsigned char *b, size_t n,
                         uint32_t *out, size_t cap, enum lu_policy p,
                         struct lu_result *r)
{
    if (!s || !r || (!b && n) || (!out && cap) ||
        (p != LU_STRICT && p != LU_REPLACE) || s->used > 3)
    {
        return LU_ARGUMENT;
    }
    struct lu_state trial = *s;
    size_t count = 0;
    enum lu_status status = walk(&trial, b, n, NULL, SIZE_MAX, p, &count);
    if ((status == LU_OK || status == LU_INCOMPLETE) && count > cap)
    {
        status = LU_FULL;
    }
    if (status != LU_OK && status != LU_INCOMPLETE)
    {
#ifdef LU_MUTANT_ROLLBACK
        *s = trial;
#endif
        return status;
    }
    /* Stable disjoint input makes the second pass identical; no fallible
       operation occurs after preflight. O(n), bounded state, no allocation or
       library calls. */
    trial = *s;
    status = walk(&trial, b, n, out, cap, p, &count);
    *s = trial;
    *r = (struct lu_result){n, count};
    return status;
}

enum lu_status lu_finish(struct lu_state *s, uint32_t *out, size_t cap,
                         enum lu_policy p, struct lu_result *r)
{
    if (!s || !r || (!out && cap) || (p != LU_STRICT && p != LU_REPLACE) ||
        s->used > 3)
    {
        return LU_ARGUMENT;
    }
    if (!s->used)
    {
        *r = (struct lu_result){0, 0};
        return LU_OK;
    }
    if (p == LU_STRICT)
    {
        return LU_INVALID;
    }
    if (!cap)
    {
        return LU_FULL;
    }
    out[0] = 0xfffd;
    s->used = 0;
    *r = (struct lu_result){0, 1};
    return LU_OK;
}

static int valid(uint32_t u)
{
    return u <= 0x10ffff && !(u >= 0xd800 && u <= 0xdfff);
}

enum lu_status lu_encode(uint32_t u, unsigned char *out, size_t cap,
                         size_t *written)
{
    if (!written || (!out && cap))
    {
        return LU_ARGUMENT;
    }
    if (!valid(u))
    {
        return LU_INVALID;
    }
    unsigned n = u < 0x80 ? 1 : u < 0x800 ? 2 : u < 0x10000 ? 3 : 4;
    if (cap < n)
    {
        return LU_FULL;
    }
    if (n == 1)
    {
        out[0] = (unsigned char)u;
    }
    else
    {
        uint32_t v = u;
        for (unsigned i = n - 1; i > 0; --i)
        {
            out[i] = (unsigned char)(0x80 | (v & 0x3f));
            v >>= 6;
        }
        out[0] = (unsigned char)((n == 2 ? 0xc0 : n == 3 ? 0xe0 : 0xf0) | v);
    }
    *written = n;
    return LU_OK;
}

enum lu_status lu_utf16(uint32_t u, uint16_t *out, size_t cap, size_t *written)
{
    if (!written || (!out && cap))
    {
        return LU_ARGUMENT;
    }
    if (!valid(u))
    {
        return LU_INVALID;
    }
    unsigned n = u < 0x10000 ? 1 : 2;
    if (cap < n)
    {
        return LU_FULL;
    }
    if (n == 1)
    {
        out[0] = (uint16_t)u;
    }
    else
    {
        u -= 0x10000;
        out[0] = (uint16_t)(0xd800 + (u >> 10));
        out[1] = (uint16_t)(0xdc00 + (u & 0x3ff));
    }
    *written = n;
    return LU_OK;
}
