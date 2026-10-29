#include "membership.h"
#include "hashing.h"
#ifndef RPM_MUTATION
#define RPM_MUTATION 0
#endif
static uint32_t pos(const struct rpm_counter *s, unsigned i)
{
    return s->blocks[i / 3].positive[i % 3];
}

static uint32_t neg(const struct rpm_counter *s, unsigned i)
{
    return s->blocks[i / 3].negative[i % 3];
}

static void set(struct rpm_counter *s, unsigned i, uint64_t id, uint32_t p,
                uint32_t n)
{
    s->ids[i] = id;
    s->blocks[i / 3].positive[i % 3] = p;
    s->blocks[i / 3].negative[i % 3] = n;
}

static unsigned find(const struct rpm_counter *s, uint64_t id)
{
    unsigned i;
    for (i = 0; i < s->count && s->ids[i] != id; ++i)
    {
    }
    return i;
}

static int valid(const struct rpm_counter *s)
{
    unsigned i;
    if (!s || !s->uuid || !s->self || s->count < 1 || s->count > 6)
    {
        return 0;
    }
    for (i = 0; i < s->count; ++i)
    {
        if (!s->ids[i] || (i && s->ids[i - 1] >= s->ids[i]))
        {
            return 0;
        }
    }
    for (; i < 6; ++i)
    {
        if (s->ids[i] || pos(s, i) || neg(s, i))
        {
            return 0;
        }
    }
    return find(s, s->self) < s->count;
}

static int insert(struct rpm_counter *s, uint64_t id)
{
    unsigned i, j;
    if (s->count == 6)
    {
        return RPM_FULL;
    }
    for (i = 0; i < s->count && s->ids[i] < id; ++i)
    {
    }
    if (i < s->count && s->ids[i] == id)
    {
        return RPM_IDENTITY;
    }
    for (j = s->count; j > i; --j)
    {
        set(s, j, s->ids[j - 1], pos(s, j - 1), neg(s, j - 1));
    }
    set(s, i, id, 0, 0);
    ++s->count;
    return RPM_OK;
}

int rpm_make(uint64_t uuid, uint64_t self, struct rpm_counter *out)
{
    struct rpm_counter s = {0};
    if (!out || !uuid || !self)
    {
        return RPM_INVALID;
    }
    s.uuid = uuid;
    s.self = self;
    s.ids[0] = self;
    s.count = 1;
    *out = s;
    return RPM_OK;
}

int rpm_add(const struct rpm_counter *s, int negative, uint32_t amount,
            struct rpm_counter *out)
{
    struct rpm_counter t;
    unsigned i;
    int r;
    if (!out || !valid(s))
    {
        return RPM_INVALID;
    }
    t = *s;
    i = find(s, s->self);
    r = rp_pn_add(&t.blocks[i / 3], i % 3, negative, amount);
    if (r != RP_OK)
    {
        return r == RP_RANGE ? RPM_RANGE : RPM_INVALID;
    }
    *out = t;
    return RPM_OK;
}

int rpm_replace(const struct rpm_counter *s, uint64_t fresh,
                struct rpm_counter *out)
{
    struct rpm_counter t;
    int r;
    if (!out || !valid(s) || !fresh)
    {
        return RPM_INVALID;
    }
    if (find(s, fresh) < s->count)
    {
        return RPM_IDENTITY;
    }
    t = *s;
    if (RPM_MUTATION == 1)
    {
        unsigned i = find(&t, t.self);
        set(&t, i, fresh, 0, 0);
        t.self = fresh;
        *out = t;
        return RPM_OK;
    }
    r = insert(&t, fresh);
    if (r != RPM_OK)
    {
        return r;
    }
    t.self = fresh;
    *out = t;
    return RPM_OK;
}

int rpm_merge(const struct rpm_counter *s, const struct rpm_counter *other,
              struct rpm_counter *out)
{
    struct rpm_counter t;
    unsigned i, j;
    int r;
    if (!out || !valid(s) || !valid(other))
    {
        return RPM_INVALID;
    }
    if (s->uuid != other->uuid)
    {
        return RPM_IDENTITY;
    }
    t = *s;
    for (i = 0; i < other->count; ++i)
    {
        j = find(&t, other->ids[i]);
        if (j == t.count)
        {
            r = insert(&t, other->ids[i]);
            if (r != RPM_OK)
            {
                return r;
            }
            j = find(&t, other->ids[i]);
        }
        if (RPM_MUTATION == 2)
        {
            if (pos(other, i) > UINT32_MAX - pos(&t, j) ||
                neg(other, i) > UINT32_MAX - neg(&t, j))
            {
                return RPM_RANGE;
            }
            set(&t, j, t.ids[j], pos(&t, j) + pos(other, i),
                neg(&t, j) + neg(other, i));
        }
        else
        {
            struct rp_pn delta = {{0}, {0}};
            delta.positive[j % 3] = pos(other, i);
            delta.negative[j % 3] = neg(other, i);
            rp_pn_merge(&t.blocks[j / 3], &delta);
        }
    }
    *out = t;
    return RPM_OK;
}

int rpm_value(const struct rpm_counter *s, int64_t *out)
{
    if (!out || !valid(s))
    {
        return RPM_INVALID;
    }
    *out = rp_pn_value(&s->blocks[0]) + rp_pn_value(&s->blocks[1]);
    return RPM_OK;
}

static void put64(unsigned char *p, uint64_t v)
{
    unsigned i;
    for (i = 0; i < 8; ++i)
    {
        p[i] = (unsigned char)(v >> (56 - 8 * i));
    }
}

static void put32(unsigned char *p, uint32_t v)
{
    unsigned i;
    for (i = 0; i < 4; ++i)
    {
        p[i] = (unsigned char)(v >> (24 - 8 * i));
    }
}

static uint64_t get64(const unsigned char *p)
{
    uint64_t v = 0;
    unsigned i;
    for (i = 0; i < 8; ++i)
    {
        v = v << 8 | p[i];
    }
    return v;
}

static uint32_t get32(const unsigned char *p)
{
    uint32_t v = 0;
    unsigned i;
    for (i = 0; i < 4; ++i)
    {
        v = v << 8 | p[i];
    }
    return v;
}

int rpm_encode(const struct rpm_counter *s, unsigned char out[160])
{
    unsigned char b[160] = {0};
    unsigned i;
    if (!out || !valid(s))
    {
        return RPM_INVALID;
    }
    b[0] = 'R';
    b[1] = 'P';
    b[2] = 'N';
    b[3] = '1';
    b[4] = 1;
    b[5] = (unsigned char)s->count;
    put64(b + 8, s->uuid);
    put64(b + 16, s->self);
    for (i = 0; i < s->count; ++i)
    {
        put64(b + 32 + 16 * i, s->ids[i]);
        put32(b + 40 + 16 * i, pos(s, i));
        put32(b + 44 + 16 * i, neg(s, i));
    }
    (void)rph_hash(b, 128, b + 128);
    for (i = 0; i < 160; ++i)
    {
        out[i] = b[i];
    }
    return RPM_OK;
}

int rpm_decode(const unsigned char b[160], struct rpm_counter *out)
{
    struct rpm_counter t = {0};
    unsigned char sum[32];
    unsigned i;
    if (!b || !out)
    {
        return RPM_INVALID;
    }
    if (b[0] != 'R' || b[1] != 'P' || b[2] != 'N' || b[3] != '1' || b[4] != 1 ||
        b[5] < 1 || b[5] > 6 || b[6] || b[7])
    {
        return RPM_CORRUPT;
    }
    for (i = 24; i < 32; ++i)
    {
        if (b[i] && RPM_MUTATION != 9)
        {
            return RPM_CORRUPT;
        }
    }
    (void)rph_hash(b, 128, sum);
    if (RPM_MUTATION != 3 && !rph_equal(sum, b + 128))
    {
        return RPM_CORRUPT;
    }
    t.uuid = get64(b + 8);
    t.self = get64(b + 16);
    t.count = b[5];
    for (i = 0; i < 6; ++i)
    {
        set(&t, i, get64(b + 32 + 16 * i), get32(b + 40 + 16 * i),
            get32(b + 44 + 16 * i));
    }
    if (!valid(&t))
    {
        return RPM_CORRUPT;
    }
    *out = t;
    return RPM_OK;
}

int rpm_seal(const struct rpm_counter *s, const unsigned char key[32],
             unsigned char out[64])
{
    unsigned char b[64] = {0};
    unsigned i, j;
    if (!out || !key || !valid(s))
    {
        return RPM_INVALID;
    }
    b[0] = 'R';
    b[1] = 'P';
    b[2] = 'A';
    b[3] = '1';
    put64(b + 8, s->uuid);
    put64(b + 16, s->self);
    j = find(s, s->self);
    put32(b + 24, pos(s, j));
    put32(b + 28, neg(s, j));
    (void)rph_hmac(key, 32, b, 32, b + 32);
    for (i = 0; i < 64; ++i)
    {
        out[i] = b[i];
    }
    return RPM_OK;
}

int rpm_admit(const struct rpm_counter *s, const struct rpm_permit *p,
              const unsigned char b[64], struct rpm_counter *out)
{
    unsigned char tag[32];
    struct rpm_counter t;
    uint64_t id;
    uint32_t positive, negative;
    unsigned i, j;
    int r;
    if (!out || !valid(s) || !p || !b || !p->uuid || !p->writer ||
        p->active > 1)
    {
        return RPM_INVALID;
    }
    if (b[0] != 'R' || b[1] != 'P' || b[2] != 'A' || b[3] != '1')
    {
        return RPM_INVALID;
    }
    for (i = 4; i < 8; ++i)
    {
        if (b[i])
        {
            return RPM_INVALID;
        }
    }
    id = get64(b + 16);
    if (p->uuid != s->uuid || get64(b + 8) != s->uuid)
    {
        return RPM_IDENTITY;
    }
    if (RPM_MUTATION != 4 && id != p->writer)
    {
        return RPM_IDENTITY;
    }
    if (!id)
    {
        return RPM_IDENTITY;
    }
    if (RPM_MUTATION != 8 && !p->active)
    {
        return RPM_POLICY;
    }
    (void)rph_hmac(p->key, 32, b, 32, tag);
    if (RPM_MUTATION != 5 && !rph_equal(tag, b + 32))
    {
        return RPM_AUTH;
    }
    positive = get32(b + 24);
    negative = get32(b + 28);
    if (RPM_MUTATION != 6 &&
        (positive > p->positive_limit || negative > p->negative_limit))
    {
        return RPM_POLICY;
    }
    t = *s;
    j = find(&t, id);
    if (j == t.count)
    {
        r = insert(&t, id);
        if (r != RPM_OK)
        {
            return r;
        }
        j = find(&t, id);
    }
    if (RPM_MUTATION == 7)
    {
        set(&t, j, id, positive, negative);
    }
    else
    {
        struct rp_pn delta = {{0}, {0}};
        delta.positive[j % 3] = positive;
        delta.negative[j % 3] = negative;
        rp_pn_merge(&t.blocks[j / 3], &delta);
    }
    *out = t;
    return RPM_OK;
}
