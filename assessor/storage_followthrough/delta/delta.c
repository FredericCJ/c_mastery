#include "delta.h"
#include <string.h>
#ifndef SD_MUTANT
#define SD_MUTANT 0
#endif
#define SD_MAGIC UINT32_C(0x53445731)

static uint32_t crc(const uint8_t *p, size_t n)
{
    uint32_t c = UINT32_MAX;
    for (size_t i = 0; i < n; i++)
    {
        c ^= p[i];
        for (unsigned j = 0; j < 8; j++)
        {
            c = (c >> 1) ^ (UINT32_C(0xedb88320) & (0u - (c & 1u)));
        }
    }
    return ~c;
}

static uint32_t r32(const uint8_t *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static uint64_t r64(const uint8_t *p)
{
    return (uint64_t)r32(p) | (uint64_t)r32(p + 4) << 32;
}

static void w32(uint8_t *p, uint32_t x)
{
    for (unsigned i = 0; i < 4; i++)
    {
        p[i] = (uint8_t)(x >> (8 * i));
    }
}

static void w64(uint8_t *p, uint64_t x)
{
    w32(p, (uint32_t)x);
    w32(p + 4, (uint32_t)(x >> 32));
}

static unsigned count(uint32_t mask)
{
    unsigned n = 0;
    while (mask)
    {
        n += mask & 1u;
        mask >>= 1;
    }
    return n;
}

static int io_valid(store_io io)
{
    return io.page_count >= 67 && io.read && io.write && io.flush;
}

static void record(uint8_t p[4096], unsigned kind, uint64_t gen, uint32_t mask,
                   const uint32_t sums[32])
{
    memset(p, 0, 4096);
    w32(p, SD_MAGIC);
    w32(p + 4, kind);
    w64(p + 8, gen);
    w32(p + 16, kind == 1 ? 32 : mask);
    w32(p + 20, kind == 1 ? 0 : count(mask));
    for (unsigned i = 0; i < 32; i++)
    {
        w32(p + 32 + 4 * i, sums[i]);
    }
    w32(p + 4092, crc(p, 4092));
}

static int valid(const uint8_t p[4096], unsigned kind)
{
    if (r32(p) != SD_MAGIC || r32(p + 4) != kind ||
        r32(p + 4092) != crc(p, 4092))
    {
        return 0;
    }
    if (kind == 1 ? (r32(p + 16) != 32 || r32(p + 20) != 0)
                  : (!r32(p + 16) || r32(p + 20) != count(r32(p + 16))))
    {
        return 0;
    }
    for (unsigned i = 24; i < 32; i++)
    {
        if (p[i])
        {
            return 0;
        }
    }
    for (unsigned i = 160; i < 4092; i++)
    {
        if (p[i])
        {
            return 0;
        }
    }
    return 1;
}

static int readp(store_io io, unsigned id, uint8_t p[4096])
{
    return io.read(io.ctx, id, p) ? SD_IO : SD_OK;
}

static int writep(store_io io, unsigned id, const uint8_t p[4096])
{
    return io.write(io.ctx, id, p) ? SD_IO : SD_OK;
}

static int flush(store_io io)
{
    return io.flush(io.ctx) ? SD_IO : SD_OK;
}

int sd_format(store_io io)
{
    if (!io_valid(io))
    {
        return SD_INVALID;
    }
    uint8_t bytes[4096] = {0};
    uint32_t sums[32];
    for (unsigned i = 0; i < 32; i++)
    {
        sums[i] = crc(bytes, 4096);
        if (writep(io, 2 + i, bytes))
        {
            return SD_IO;
        }
    }
    if (writep(io, 66, bytes) || flush(io))
    {
        return SD_IO;
    }
    record(bytes, 1, 0, 0, sums);
    if (writep(io, 0, bytes) || flush(io) || writep(io, 1, bytes) || flush(io))
    {
        return SD_IO;
    }
    return SD_OK;
}

static int install(struct sd_store *s, const uint8_t marker[4096])
{
    uint32_t mask = r32(marker + 16);
    uint64_t generation = r64(marker + 8);
    uint8_t bytes[4096], zero[4096] = {0};
    /* Validate all prospective pages, including unchanged homes, before writes.
     */
    for (unsigned i = 0; i < 32; i++)
    {
        if (readp(s->io, (mask & (UINT32_C(1) << i)) ? 34 + i : 2 + i, bytes))
        {
            return SD_IO;
        }
        if (SD_MUTANT != 3 && crc(bytes, 4096) != r32(marker + 32 + 4 * i))
        {
            return SD_CORRUPT;
        }
    }
    if (SD_MUTANT == 2 && (writep(s->io, 66, zero) || flush(s->io)))
    {
        return SD_IO;
    }
    for (unsigned i = 0; i < 32; i++)
    {
        if (mask & (UINT32_C(1) << i))
        {
            if (readp(s->io, 34 + i, bytes) || writep(s->io, 2 + i, bytes))
            {
                return SD_IO;
            }
        }
    }
    if (flush(s->io))
    {
        return SD_IO;
    }
    uint32_t sums[32];
    for (unsigned i = 0; i < 32; i++)
    {
        sums[i] = r32(marker + 32 + 4 * i);
    }
    record(bytes, 1, generation, 0, sums);
    if (writep(s->io, (unsigned)(generation & 1u), bytes) || flush(s->io) ||
        writep(s->io, 66, zero) || flush(s->io))
    {
        return SD_IO;
    }
    memcpy(s->sums, sums, sizeof sums);
    s->generation = generation;
    s->wal_mask = 0;
    return SD_OK;
}

int sd_open(struct sd_store *s, store_io io, unsigned full)
{
    if (!s || !io_valid(io) || full > 1)
    {
        return SD_INVALID;
    }
    memset(s, 0, sizeof *s);
    s->io = io;
    s->full = full;
    s->poisoned = 1;
    uint8_t a[4096], b[4096], m[4096];
    if (readp(io, 0, a) || readp(io, 1, b) || readp(io, 66, m))
    {
        return SD_IO;
    }
    int va = valid(a, 1), vb = valid(b, 1);
    if (!va && !vb)
    {
        return SD_CORRUPT;
    }
    if (va && vb && r64(a + 8) == r64(b + 8) && memcmp(a, b, 4096))
    {
        return SD_CORRUPT;
    }
    const uint8_t *chosen = !vb || (va && r64(a + 8) >= r64(b + 8)) ? a : b;
    s->generation = r64(chosen + 8);
    for (unsigned i = 0; i < 32; i++)
    {
        s->sums[i] = r32(chosen + 32 + 4 * i);
    }
    if (valid(m, 2) && r64(m + 8) > s->generation)
    {
        if (s->generation == UINT64_MAX || r64(m + 8) != s->generation + 1)
        {
            return SD_CORRUPT;
        }
        int r = install(s, m);
        if (r)
        {
            return r;
        }
    }
    for (unsigned i = 0; i < 32; i++)
    {
        if (readp(io, 2 + i, a))
        {
            return SD_IO;
        }
        if (crc(a, 4096) != s->sums[i])
        {
            return SD_CORRUPT;
        }
    }
    s->poisoned = 0;
    return SD_OK;
}

int sd_read(struct sd_store *s, unsigned page, void *out)
{
    if (!s || page >= 32 || !out)
    {
        return SD_INVALID;
    }
    if (s->poisoned)
    {
        return SD_STATE;
    }
    if (s->active)
    {
        memcpy(out, s->scratch[page], 4096);
        return SD_OK;
    }
    uint8_t bytes[4096];
    int r = readp(s->io,
                  (s->wal_mask & (UINT32_C(1) << page)) ? 34 + page : 2 + page,
                  bytes);
    if (r)
    {
        return r;
    }
    if (crc(bytes, 4096) != s->sums[page])
    {
        return SD_CORRUPT;
    }
    memcpy(out, bytes, 4096);
    return SD_OK;
}

int sd_checkpoint(struct sd_store *s)
{
    if (!s || s->poisoned || s->active)
    {
        return SD_STATE;
    }
    if (!s->wal_mask)
    {
        return SD_OK;
    }
    uint8_t marker[4096];
    int r = readp(s->io, 66, marker);
    if (!r && (!valid(marker, 2) || r64(marker + 8) != s->generation ||
               r32(marker + 16) != s->wal_mask))
    {
        r = SD_CORRUPT;
    }
    if (!r)
    {
        r = install(s, marker);
    }
    if (r)
    {
        s->poisoned = 1;
    }
    return r;
}

int sd_begin(struct sd_store *s)
{
    if (!s || s->poisoned || s->active)
    {
        return SD_STATE;
    }
    int r = sd_checkpoint(s);
    if (r)
    {
        return r;
    }
    for (unsigned i = 0; i < 32; i++)
    {
        uint8_t page[4096];
        r = sd_read(s, i, page);
        if (r)
        {
            return r;
        }
        memcpy(s->scratch[i], page, 4096);
    }
    s->dirty = 0;
    s->active = 1;
    return SD_OK;
}

int sd_write(struct sd_store *s, unsigned page, size_t offset,
             const void *input, size_t n)
{
    if (!s || page >= 32 || offset > 4096 || n > 4096 - offset || (!input && n))
    {
        return SD_INVALID;
    }
    if (s->poisoned || !s->active)
    {
        return SD_STATE;
    }
    if (n && memcmp(s->scratch[page] + offset, input, n))
    {
        memcpy(s->scratch[page] + offset, input, n);
        if (SD_MUTANT != 4 || page != 31)
        {
            s->dirty |= UINT32_C(1) << page;
        }
    }
    return SD_OK;
}

void sd_abort(struct sd_store *s)
{
    if (s && s->active)
    {
        s->active = 0;
        s->dirty = 0;
    }
}

int sd_commit(struct sd_store *s)
{
    if (!s || s->poisoned || !s->active)
    {
        return SD_STATE;
    }
    if (!s->dirty)
    {
        s->active = 0;
        return SD_OK;
    }
    if (s->generation == UINT64_MAX)
    {
        sd_abort(s);
        return SD_FULL;
    }
    uint64_t generation = s->generation + 1;
    uint32_t mask = s->full ? UINT32_MAX : s->dirty;
    uint32_t sums[32];
    for (unsigned i = 0; i < 32; i++)
    {
        sums[i] = crc(s->scratch[i], 4096);
    }
    uint8_t marker[4096], zero[4096] = {0};
    record(marker, 2, generation, mask, sums);
    int r = writep(s->io, 66, zero);
    if (!r)
    {
        r = flush(s->io);
    }
    for (unsigned i = 0; !r && i < 32; i++)
    {
        if (mask & (UINT32_C(1) << i))
        {
            r = writep(s->io, 34 + i, s->scratch[i]);
        }
    }
    if (!r)
    {
        r = flush(s->io);
    }
    if (r)
    {
        sd_abort(s);
        s->poisoned = 1;
        return SD_IO;
    }
    r = writep(s->io, 66, marker);
    if (!r && SD_MUTANT != 1)
    {
        r = flush(s->io);
    }
    if (r)
    {
        sd_abort(s);
        s->poisoned = 1;
        return SD_UNCERTAIN;
    }
    s->active = 0;
    s->dirty = 0;
    s->generation = generation;
    s->wal_mask = mask;
    memcpy(s->sums, sums, sizeof sums);
    return SD_OK;
}
