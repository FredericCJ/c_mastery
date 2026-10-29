#include "transfer.h"
#include <limits.h>
_Static_assert(CHAR_BIT == 8 && INT_MIN <= -32768,
               "explicit octet and edge profile");

void ct_init(struct ct_stream *s)
{
    if (s)
        *s = (struct ct_stream){0};
}

static enum ct_status fail(struct ct_stream *s, enum ct_status e)
{
    s->state = 2;
    return e;
}

enum ct_status ct_feed(struct ct_stream *s, const unsigned char *p, size_t n,
                       unsigned final, const struct ag_certificate *c,
                       struct cd_csr *out)
{
    if (!s || !out || (n && !p) || final > 1)
        return CT_INVALID;
    if (s->state)
        return CT_STATE;
    if (n > sizeof s->bytes - s->used)
        return fail(s, CT_INVALID);
    for (size_t i = 0; i < n; ++i)
        s->bytes[s->used++] = p[i];
    if (s->used >= 6)
    {
        const unsigned char *b = s->bytes;
        if (b[0] != 0x47 || b[1] != 0x43 || b[2] != 1 || !b[3] || b[3] > 8 ||
            b[4] > 24 || b[5] >= b[3])
            return fail(s, CT_INVALID);
        s->expected = 6 + 4u * b[4];
        if (s->used > s->expected)
            return fail(s, CT_INVALID);
    }
#ifndef CT_MUTANT_EARLY
    if (!final)
        return CT_MORE;
#else
    if (!final && (!s->expected || s->used < s->expected))
        return CT_MORE;
#endif
    if (s->used < 6 || s->used != s->expected)
        return fail(s, CT_TRUNCATED);
    if (!c)
        return fail(s, CT_INVALID);
    struct cd_edge edges[24];
    unsigned count = s->bytes[4];
    for (unsigned i = 0; i < count; ++i)
    {
        const unsigned char *b = s->bytes + 6 + 4 * i;
        unsigned w = (unsigned)b[2] * 256u + b[3];
        int value = w < 32768u ? (int)w : (int)(w - 32768u) - 32768;
#ifdef CT_MUTANT_UNSIGNED
        value = (int)w;
#endif
        edges[i] = (struct cd_edge){b[0], b[1], value};
    }
    struct cd_csr graph;
    if (cd_csr_build(&graph, s->bytes[3], edges, count, 1) != CD_OK)
        return fail(s, CT_INVALID);
    if (ag_certificate(&graph, s->bytes[5], c) != AG_OK)
        return fail(s, CT_CERTIFICATE);
    *out = graph;
    s->state = 1;
    return CT_OK;
}
