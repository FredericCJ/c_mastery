#include "codec.h"
#include <limits.h>
#include <string.h>
#ifndef SFC_MUTANT
#define SFC_MUTANT 0
#endif
_Static_assert(CHAR_BIT == 8, "page octets");
#define MAGIC UINT32_C(0x43535433)

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | (uint16_t)p[1] << 8);
}

static uint32_t rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static uint64_t rd64(const uint8_t *p)
{
    return (uint64_t)rd32(p) | ((uint64_t)rd32(p + 4) << 32);
}

static void wr16(uint8_t *p, uint16_t x)
{
    p[0] = (uint8_t)x;
    p[1] = (uint8_t)(x >> 8);
}

static void wr32(uint8_t *p, uint32_t x)
{
    for (unsigned i = 0; i < 4; i++)
        p[i] = (uint8_t)(x >> (8 * i));
}

static void wr64(uint8_t *p, uint64_t x)
{
    wr32(p, (uint32_t)x);
    wr32(p + 4, (uint32_t)(x >> 32));
}

static uint32_t crc(const uint8_t *b)
{
    uint32_t c = UINT32_MAX;
    for (unsigned i = 0; i < 4092; i++)
    {
        c ^= b[i];
        for (unsigned j = 0; j < 8; j++)
            c = (c >> 1) ^ (UINT32_C(0xedb88320) & (0u - (c & 1u)));
    }
    return ~c;
}

static int order(const uint8_t *a, unsigned an, const uint8_t *b, unsigned bn)
{
    unsigned n = an < bn ? an : bn;
    for (unsigned i = 0; i < n; i++)
    {
        if (a[i] < b[i])
            return -1;
        if (a[i] > b[i])
            return 1;
    }
    return an < bn ? -1 : an > bn ? 1 : 0;
}

static int extent(unsigned off, unsigned n)
{
    return off >= 128 && off <= 4092 && n <= 4092 - off;
}

int sf_leaf_decode(uint32_t id, uint64_t gen, const uint8_t *b, size_t length,
                   store_node *out)
{
    if (!b || !out || length != 4096 || id == 0 || id >= 32)
        return STORE_INVALID;
    if (rd32(b) != MAGIC || rd32(b + 4) != (SFC_MUTANT == 4 ? 2u : 3u) ||
        rd64(b + 8) != gen || rd32(b + 16) != id || b[20] != 1 || b[21] > 7 ||
        b[22] || b[23] || rd32(b + 24) >= 32 || rd32(b + 28) != 128 ||
        rd32(b + 4092) != crc(b))
        return STORE_CORRUPT;
    store_node n = {0};
    n.leaf = 1;
    n.n = b[21];
    n.next = rd32(b + 24);
    unsigned starts[14], sizes[14], used = 0;
    for (unsigned i = 0; i < n.n; i++)
    {
        unsigned off = 32 + i * 8, k = rd16(b + off), kl = rd16(b + off + 2),
                 v = rd16(b + off + 4), vl = rd16(b + off + 6);
        if (!kl || kl > 16 || vl > 64 || !extent(k, kl) || !extent(v, vl))
            return STORE_CORRUPT;
        starts[used] = k;
        sizes[used++] = kl;
        starts[used] = v;
        sizes[used++] = vl;
    }
    for (unsigned i = 0; i < used; i++)
        for (unsigned j = 0; j < i; j++)
            if (SFC_MUTANT != 1 && sizes[i] && sizes[j] &&
                starts[i] < starts[j] + sizes[j] &&
                starts[j] < starts[i] + sizes[i])
                return STORE_CORRUPT;
    for (unsigned i = 32 + n.n * 8; i < 128; i++)
        if (b[i])
            return STORE_CORRUPT;
    for (unsigned i = 128; i < 4092; i++)
    {
        int live = 0;
        for (unsigned j = 0; j < used; j++)
            if (i >= starts[j] && i - starts[j] < sizes[j])
                live = 1;
        if (!live && b[i])
            return STORE_CORRUPT;
    }
    for (unsigned i = 0; i < n.n; i++)
    {
        n.klen[i] = (uint8_t)sizes[2 * i];
        n.vlen[i] = (uint8_t)sizes[2 * i + 1];
        memcpy(n.key[i], b + starts[2 * i], n.klen[i]);
        memcpy(n.value[i], b + starts[2 * i + 1], n.vlen[i]);
        if (SFC_MUTANT == 3)
            *out = n;
        if (i && order(n.key[i - 1], n.klen[i - 1], n.key[i], n.klen[i]) >= 0)
            return STORE_CORRUPT;
    }
    *out = n;
    return STORE_OK;
}

int sf_leaf_encode(uint32_t id, uint64_t gen, const store_node *n,
                   uint8_t out[STORE_PAGE_SIZE])
{
    if (!n || !out || !id || id >= 32 || n->leaf != 1 || n->n > 7 ||
        n->next >= 32)
        return STORE_INVALID;
    for (unsigned i = 0; i < n->n; i++)
        if (!n->klen[i] || n->klen[i] > 16 || n->vlen[i] > 64 ||
            (i &&
             order(n->key[i - 1], n->klen[i - 1], n->key[i], n->klen[i]) >= 0))
            return STORE_INVALID;
    uint8_t b[4096] = {0};
    wr32(b, MAGIC);
    wr32(b + 4, 3);
    wr64(b + 8, gen);
    wr32(b + 16, id);
    b[20] = 1;
    b[21] = n->n;
    wr32(b + 24, n->next);
    wr32(b + 28, 128);
    unsigned at = 128;
    for (unsigned i = 0; i < n->n; i++)
    {
        unsigned t = 32 + i * 8;
        wr16(b + t, (uint16_t)at);
        wr16(b + t + 2, n->klen[i]);
        memcpy(b + at, n->key[i], n->klen[i]);
        at += n->klen[i];
        wr16(b + t + 4, (uint16_t)at);
        wr16(b + t + 6, n->vlen[i]);
        memcpy(b + at, n->value[i], n->vlen[i]);
        at += n->vlen[i];
    }
    if (SFC_MUTANT == 2)
        b[8] = (uint8_t)(gen >> 56);
    wr32(b + 4092, crc(b));
    memcpy(out, b, sizeof b);
    return STORE_OK;
}

int sf_leaf_upgrade(uint32_t id, uint64_t gen, const uint8_t *b, size_t n,
                    uint8_t out[STORE_PAGE_SIZE])
{
    if (!b || !out || n != 4096)
        return STORE_INVALID;
    store_node node;
    int r = store_page_decode(id, gen, b, &node);
    return r ? r : sf_leaf_encode(id, gen, &node, out);
}

int sf_leaf_downgrade(uint32_t id, uint64_t gen, const uint8_t *b, size_t n,
                      uint8_t out[STORE_PAGE_SIZE])
{
    store_node node;
    int r = sf_leaf_decode(id, gen, b, n, &node);
    return r ? r : store_page_encode(id, gen, &node, out);
}

int sf_format_fresh(struct sf_factory f, struct sf_image *out)
{
    if (!out || !f.acquire || !f.discard)
        return STORE_INVALID;
    struct sf_image candidate = {0};
    if (!f.acquire(f.context, &candidate))
        return STORE_FULL;
    int r = store_format(candidate.io);
    store verified;
    if (r == STORE_OK)
        r = store_open(&verified, candidate.io);
    if (r != STORE_OK)
    {
        if (SFC_MUTANT != 5)
            f.discard(f.context, candidate);
        return r;
    }
    *out = candidate;
    return STORE_OK;
}
