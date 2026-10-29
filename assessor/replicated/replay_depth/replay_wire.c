#include "hashing.h"
#include "replay_depth.h"
#include <string.h>
#ifndef RPR_MUTATION
#define RPR_MUTATION 0
#endif
static void put(unsigned char *p, uint64_t v, unsigned n)
{
    if (RPR_MUTATION == 11)
    {
        for (unsigned i = 0; i < n; ++i)
        {
            p[i] = (unsigned char)v;
            v >>= 8;
        }
        return;
    }
    while (n)
    {
        p[--n] = (unsigned char)v;
        v >>= 8;
    }
}

static uint64_t get(const unsigned char *p, unsigned n)
{
    uint64_t v = 0;
    while (n--)
    {
        v = (v << 8) | *p++;
    }
    return v;
}

static void state_put(unsigned char *p, struct rp_state s)
{
    put(p, s.tick, 4);
    put(p + 4, s.position, 4);
    put(p + 8, s.score, 4);
    put(p + 12, s.random, 4);
}

static struct rp_state state_get(const unsigned char *p)
{
    return (struct rp_state){(unsigned)get(p, 4), (unsigned)get(p + 4, 4),
                             (unsigned)get(p + 8, 4), (uint32_t)get(p + 12, 4)};
}

int rpr_encode(const struct rpr_session *s, unsigned char out[512])
{
    unsigned char b[512] = {0};
    unsigned i;
    if (!out || !rpr_invariant(s))
    {
        return RPR_INVALID;
    }
    memcpy(b, "RPR1", 4);
    b[4] = 1;
    b[5] = (unsigned char)s->horizon;
    b[6] = (unsigned char)s->head;
    b[7] = (unsigned char)s->confirmed;
    put(b + 8, s->timeline, 8);
    put(b + 16, s->revision, 4);
    b[20] = (unsigned char)s->initial[0].position;
    b[21] = (unsigned char)s->initial[1].position;
    put(b + 22, s->initial[0].score, 2);
    put(b + 24, s->initial[1].score, 2);
    put(b + 28, s->initial[0].random, 4);
    put(b + 32, s->initial[1].random, 4);
    for (i = 0; i < s->head; ++i)
    {
        b[36 + i] = (unsigned char)(s->local[i] + 1);
        b[44 + i] = (unsigned char)(s->remote[i] + 1);
        b[52 + i] = (unsigned char)s->known[i];
    }
    put(b + 60, s->effects.next, 4);
    b[64] = (unsigned char)s->effects.pending;
    if (s->effects.pending)
    {
        put(b + 68, rp_zigzag(s->effects.effect.value), 4);
        put(b + 72, s->effects.effect.id, 4);
    }
    for (i = 0; i <= s->head; ++i)
    {
        state_put(b + 80 + 32 * i, s->snapshots[i][0]);
        state_put(b + 96 + 32 * i, s->snapshots[i][1]);
    }
    if (rph_hash(b, 480, b + 480) != RPH_OK)
    {
        return RPR_STATE;
    }
    memcpy(out, b, 512);
    return RPR_OK;
}

int rpr_decode(uint64_t timeline, const unsigned char in[512],
               struct rpr_session *out)
{
    struct rpr_session s = {0};
    unsigned char canonical[512], digest[32];
    unsigned i;
    if (!timeline || !in || !out)
    {
        return RPR_INVALID;
    }
    if (memcmp(in, "RPR1", 4) || in[4] != 1 || get(in + 8, 8) != timeline ||
        in[6] > 8)
    {
        return RPR_CORRUPT;
    }
    if (rph_hash(in, 480, digest) != RPH_OK ||
        (RPR_MUTATION != 3 && !rph_equal(digest, in + 480)))
    {
        return RPR_CORRUPT;
    }
    s.timeline = timeline;
    s.horizon = in[5];
    s.head = in[6];
    s.confirmed = in[7];
    s.revision = (uint32_t)get(in + 16, 4);
    s.initial[0] = (struct rp_state){0, in[20], (unsigned)get(in + 22, 2),
                                     (uint32_t)get(in + 28, 4)};
    s.initial[1] = (struct rp_state){0, in[21], (unsigned)get(in + 24, 2),
                                     (uint32_t)get(in + 32, 4)};
    for (i = 0; i < s.head; ++i)
    {
        s.local[i] = (int)in[36 + i] - 1;
        s.remote[i] = (int)in[44 + i] - 1;
        s.known[i] = in[52 + i];
    }
    s.effects.next = (uint32_t)get(in + 60, 4);
    s.effects.pending = in[64];
    s.effects.effect.value = rp_unzigzag((uint32_t)get(in + 68, 4));
    s.effects.effect.id = (uint32_t)get(in + 72, 4);
    for (i = 0; i <= s.head; ++i)
    {
        s.snapshots[i][0] = state_get(in + 80 + 32 * i);
        s.snapshots[i][1] = state_get(in + 96 + 32 * i);
    }
    if (!rpr_invariant(&s) || rpr_encode(&s, canonical))
    {
        return RPR_CORRUPT;
    }
    if (RPR_MUTATION != 3 && memcmp(canonical, in, 512))
    {
        return RPR_CORRUPT;
    }
    *out = s;
    return RPR_OK;
}

int rpr_input_encode(struct rpr_input input, unsigned char out[16])
{
    unsigned char b[16] = {0};
    if (!out || !input.timeline || input.tick < 1 || input.tick > 8 ||
        input.movement < -1 || input.movement > 1)
    {
        return RPR_INVALID;
    }
    memcpy(b, "RPI1", 4);
    put(b + 4, input.timeline, 8);
    b[12] = (unsigned char)input.tick;
    b[13] = (unsigned char)(input.movement + 1);
    memcpy(out, b, 16);
    return RPR_OK;
}

int rpr_input_decode(const unsigned char in[16], struct rpr_input *out)
{
    struct rpr_input x;
    if (!in || !out)
    {
        return RPR_INVALID;
    }
    if (memcmp(in, "RPI1", 4) || (RPR_MUTATION != 6 && (in[14] || in[15])) ||
        !get(in + 4, 8) || !in[12] || in[12] > 8 || in[13] > 2)
    {
        return RPR_CORRUPT;
    }
    x = (struct rpr_input){get(in + 4, 8), in[12], (int)in[13] - 1};
    *out = x;
    return RPR_OK;
}
