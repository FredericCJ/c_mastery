#include "recovery.h"
#ifndef DU_MUTANT
#define DU_MUTANT 0
#endif
static uint32_t get(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static void put(unsigned char *p, uint32_t x)
{
    for (unsigned i = 0; i < 4; i++)
        p[i] = (unsigned char)(x >> (i * 8));
}

uint32_t du_crc(const unsigned char *p, size_t n)
{
    uint32_t c = UINT32_MAX;
    for (size_t i = 0; i < n; i++)
    {
        c ^= p[i];
        for (unsigned b = 0; b < 8; b++)
            c = (c >> 1) ^ ((0u - (c & 1u)) & 0xedb88320u);
    }
    return ~c;
}

static int shape(const struct du_image *i)
{
    return i && i->version && i->generation && i->extent >= 10 &&
           i->extent <= 2048 && i->base <= UINT32_MAX - i->extent &&
           !(i->base & 511u) && (i->entry & 1u) &&
           (i->entry & ~1u) >= i->base + 8 &&
           (i->entry & ~1u) <= i->base + i->extent - 2 &&
           i->stack > 0x20000000u && i->stack <= 0x20020000u &&
           !(i->stack & 7u) && i->clock;
}

int du_encode(const struct du_image *i, const unsigned char *p, size_t n,
              struct du_slot *out)
{
    if (!shape(i) || !p || !out || n != i->extent || n > 24 ||
        get(p) != i->stack || get(p + 4) != i->entry)
        return DU_INVALID;
    struct du_slot r = {{0}};
    put(r.bytes, 0x31554944);
    put(r.bytes + 4, i->version);
    put(r.bytes + 8, i->extent);
    put(r.bytes + 12, i->base);
    put(r.bytes + 16, i->entry);
    put(r.bytes + 20, i->stack);
    put(r.bytes + 24, i->clock);
    for (size_t k = 0; k < n; k++)
        r.bytes[28 + k] = p[k];
    put(r.bytes + 52, i->generation);
    put(r.bytes + 56, du_crc(r.bytes, 56));
    put(r.bytes + 60, 0xc04d17ed);
    *out = r;
    return DU_OK;
}

int du_decode(const struct du_slot *s, struct du_image *out)
{
    if (!s || !out || get(s->bytes) != 0x31554944 ||
        get(s->bytes + 60) != 0xc04d17ed ||
        get(s->bytes + 56) != du_crc(s->bytes, 56))
        return DU_INVALID;
    struct du_image i = {get(s->bytes + 4),  get(s->bytes + 8),
                         get(s->bytes + 12), get(s->bytes + 16),
                         get(s->bytes + 20), get(s->bytes + 24),
                         get(s->bytes + 52)};
    if (!shape(&i) || i.extent > 24 || get(s->bytes + 28) != i.stack ||
        get(s->bytes + 32) != i.entry)
        return DU_INVALID;
    for (unsigned k = i.extent; k < 24; k++)
        if (s->bytes[28 + k])
            return DU_INVALID;
    *out = i;
    return DU_OK;
}

int du_select(const struct du_slot *s, size_t n, uint32_t floor, uint32_t max,
              uint32_t clock, unsigned *out)
{
    if (!s || !out || !n || n > 3 || floor > max || !clock)
        return DU_INVALID;
    unsigned found = 0, pick = 0;
    struct du_image best = {0};
    for (unsigned k = 0; k < n; k++)
    {
        struct du_image i;
        if (du_decode(&s[k], &i) != DU_OK || i.version < floor ||
            i.version > max || i.clock != clock)
            continue;
        if (found && i.version == best.version)
        {
            for (unsigned b = 0; b < 64; b++)
                if (s[k].bytes[b] != s[pick].bytes[b])
                    return DU_CONFLICT;
        }
        if (!found || i.version > best.version)
        {
            found = 1;
            pick = k;
            best = i;
        }
    }
    if (!found)
        return DU_ABSENT;
    *out = pick;
    return DU_OK;
}

int du_install(struct du_slot s[2], unsigned active, const unsigned sector[2],
               const struct du_slot *next, unsigned complete, uint32_t mask)
{
    struct du_image old, newer;
    if (!s || active > 1 || !sector || !next || complete > 16 ||
        du_decode(&s[active], &old) || du_decode(next, &newer) ||
        newer.version <= old.version || newer.generation <= old.generation ||
        (DU_MUTANT != 1 && sector[0] == sector[1]))
        return DU_INVALID;
    unsigned other = 1 - active;
    if (sector[0] == sector[1])
        for (unsigned j = 0; j < 64; j++)
            s[active].bytes[j] = 255;
    for (unsigned j = 0; j < 64; j++)
        s[other].bytes[j] = 255;
    unsigned limit = complete * 4;
    for (unsigned j = 0; j < limit; j++)
        s[other].bytes[j] = next->bytes[j];
    if (complete < 16)
        for (unsigned b = 0; b < 32; b++)
            if (mask & (UINT32_C(1) << b))
                s[other].bytes[limit + b / 8] &=
                    (unsigned char)(next->bytes[limit + b / 8] |
                                    (unsigned char)~(1u << (b % 8)));
    return complete == 16 ? DU_OK : DU_INTERRUPTED;
}

int du_prepare(const struct du_image *i, const struct du_machine *m,
               struct du_plan *out)
{
    if (!shape(i) || !m || !out || m->privileged_thread != 1 ||
        m->active_exception || m->dma_owned || m->peripheral_owned ||
        (DU_MUTANT != 2 && m->clock != i->clock))
        return DU_INVALID;
    *out = (struct du_plan){i->base, i->stack, i->entry};
    return DU_OK;
}

int du_retry_initialize(struct du_retry *r)
{
    if (!r)
        return DU_INVALID;
    for (unsigned i = 0; i < 48; i++)
        ((unsigned char *)r)[i] = 255;
    return lce_record(r->records[0], 1, 0) ? DU_OK : DU_INVALID;
}

int du_retry_state(const struct du_retry *r, uint32_t limit, uint32_t *out)
{
    if (!r || !out || !limit)
        return DU_INVALID;
    unsigned pick;
    uint32_t seq, value;
    if (!lce_choose(r->records[0], r->records[1], &pick) ||
        !lce_valid(r->records[pick], &seq, &value) || value > limit)
        return DU_STOP;
    *out = value;
    return value == limit ? DU_STOP : DU_OK;
}

int du_retry_take(struct du_retry *r, uint32_t limit, unsigned cut,
                  uint32_t *out)
{
    if (!r || !out || cut > 24 || !limit)
        return DU_INVALID;
    unsigned pick;
    uint32_t seq, value;
    if (!lce_choose(r->records[0], r->records[1], &pick) ||
        !lce_valid(r->records[pick], &seq, &value) || value >= limit ||
        seq == UINT32_MAX)
        return DU_STOP;
    unsigned char next[24];
    if (!lce_record(next, seq + 1, value + 1))
        return DU_STOP;
    unsigned other = 1 - pick;
    for (unsigned i = 0; i < 24; i++)
        r->records[other][i] = 255;
    for (unsigned i = 0; i < cut; i++)
        r->records[other][i] = next[i];
    if (DU_MUTANT != 3 && cut != 24)
        return DU_INTERRUPTED;
    *out = value + 1;
    return DU_OK;
}

int du_effect(struct du_recovery *r, unsigned value)
{
    if (!r || value > 1)
        return DU_INVALID;
    if (r->failed)
        return DU_STOP;
    if (value && r->emitted == UINT32_MAX)
        return DU_STOP;
    r->output = value;
    if (value)
        r->emitted++;
    return DU_OK;
}

int du_fault(struct du_recovery *r, uint32_t reason, uint32_t step,
             int (*log)(void *, uint32_t), void *ctx)
{
    if (!r || !reason)
        return DU_INVALID;
    if (r->failed)
    {
        if (!r->secondary)
            r->secondary = reason;
        (void)lce_fault_begin(&r->record);
        return DU_STOP;
    }
    r->failed = 1;
    r->first = reason;
    r->detection_step = step;
    if (!lce_fault_begin(&r->record))
    {
        if (!r->secondary)
            r->secondary = 0x100;
        return DU_STOP;
    }
    int status = log ? log(ctx, reason) : 1;
    if (status != 1 && !r->secondary)
        r->secondary = 0x101;
    if (DU_MUTANT == 4 && r->secondary)
        r->first = r->secondary;
    (void)lce_fault_commit(&r->record, r->first, step);
    return DU_OK;
}

int du_contain(struct du_recovery *r, uint32_t step, int (*safe)(void *),
               void *ctx)
{
    if (!r || !safe || !r->failed || step < r->detection_step)
        return DU_INVALID;
    if (r->contained)
        return DU_OK;
    if (safe(ctx) != 1)
        return DU_STOP;
    r->output = 0;
    r->containment_step = step;
    r->contained = 1;
    if (DU_MUTANT == 5)
        r->emitted = 0;
    return DU_OK;
}

int du_recovery_input(struct du_recovery *r, const unsigned char *p,
                      size_t offered, size_t reported, unsigned status,
                      uint32_t step)
{
    if (!r)
        return DU_INVALID;
    if (status > 2 || reported > offered || (status && reported))
    {
        (void)du_fault(r, 11, step, 0, 0);
        return DU_INVALID;
    }
    if (status)
    {
        (void)du_fault(r, status == 1 ? 12 : 13, step, 0, 0);
        return DU_STOP;
    }
    if (!p || offered > 64 || reported < 2 || p[0] > reported - 1)
    {
        (void)du_fault(r, 14, step, 0, 0);
        return DU_INVALID;
    }
    return r->failed ? DU_STOP : DU_OK;
}
