#include "firmware.h"
#include <limits.h>
#ifndef CF_MUTANT
#define CF_MUTANT 0
#endif
int cf_init(struct cf_firmware *f)
{
    if (!f)
        return 0;
    struct cf_firmware initial = {0};
    *f = initial;
    return cs_init(&f->core, 3, 1000) == CS_OK && cpf_ring_init(&f->samples, 4);
}

enum cf_status cf_boot(struct cf_firmware *f, int (*ready)(void *),
                       void *context, unsigned attempts)
{
    if (!f || f->state != CF_BOOT || !ready || !attempts || attempts > 32)
        return CF_INVALID;
    for (unsigned i = 0; i < attempts; ++i)
        if (ready(context))
        {
            f->state = CF_RUNNING;
            return CF_OK;
        }
    f->state = CF_SAFE_STOP;
    f->first_fault = 1;
    f->actuator = 0;
    return CF_FAILED;
}

enum cf_status cf_acquire(struct cf_firmware *f, unsigned success,
                          const struct cpf_sample *sample)
{
    if (!f || f->state != CF_RUNNING || success > 1)
        return CF_INVALID;
    if (!success)
        return CF_PENDING;
    if (!sample)
        return CF_INVALID;
    return cpf_irq_push(&f->samples, *sample) ? CF_OK : CF_FULL;
}

enum cf_status cf_foreground(struct cf_firmware *f, int authorized)
{
    if (!f || f->state != CF_RUNNING)
        return CF_INVALID;
    struct cpf_sample sample;
    if (!cpf_take(&f->samples, &sample))
        return CF_PENDING;
    unsigned char bytes[42];
    size_t n = 0, used = 0;
    if (cpf_frame(&sample, bytes, sizeof bytes, &n) != CP_OK)
        return CF_FAILED;
    enum cs_status s = cs_feed(&f->core, 0, bytes, n, &used, authorized, 0);
    if (used != n)
        return CF_FAILED;
    return s == CS_FRAME    ? CF_OK
           : s == CS_DENIED ? CF_DENIED
           : s == CS_FULL   ? CF_FULL
                            : CF_FAILED;
}

enum cf_status cf_command(struct cf_firmware *f, const struct cp_message *m,
                          int authorized)
{
    if (!f || !m || f->state != CF_RUNNING || m->length != 1 ||
        m->payload[0] > 1)
        return CF_INVALID;
    if (!authorized && CF_MUTANT != 1)
        return CF_DENIED;
    if (f->command_valid)
    {
        if (m->sequence == f->last_command && m->payload[0] == f->last_value)
            return CF_REPLAY;
        if (m->sequence <= f->last_command)
            return CF_DENIED;
    }
    enum cs_status s = cs_offer(&f->core, m, 1, 0, 0);
    if (s == CS_FULL)
        return CF_FULL;
    if (s != CS_FRAME)
        return CF_FAILED;
    f->command_valid = 1;
    f->last_command = m->sequence;
    f->last_value = m->payload[0];
    f->actuator = m->payload[0];
    return CF_OK;
}

enum cf_status cf_dma_begin(struct cf_firmware *f, struct pd_io io,
                            uint32_t source, uint32_t destination,
                            uint32_t token)
{
    if (!f || f->state != CF_RUNNING || f->dma_token)
        return CF_INVALID;
    struct pd_extent e = {source, destination, 8, 64, 64, 1};
    int s = pd_submit(&f->transfer, io, PD_F439, 0, &e, 1, token);
    if (s != PD_OK)
        return s == PD_BUSY ? CF_PENDING : CF_INVALID;
    f->dma_token = token;
    return CF_OK;
}

enum cf_status cf_dma_cancel(struct cf_firmware *f, struct pd_io io,
                             uint32_t token)
{
    if (!f || token != f->dma_token || !token)
        return CF_INVALID;
    int s = pd_cancel(&f->transfer, io, token);
    if (CF_MUTANT == 2)
        f->dma_token = 0;
    return s == PD_BUSY ? CF_PENDING : CF_INVALID;
}

enum cf_status cf_dma_finish(struct cf_firmware *f, struct pd_io io,
                             uint32_t token)
{
    if (!f || token != f->dma_token || !token)
        return CF_INVALID;
    if (pd_poll(&f->transfer, io, token) != PD_OK)
        return CF_PENDING;
    struct pd_result result;
    if (pd_finalize(&f->transfer, io, token, &result) != PD_OK)
        return CF_FAILED;
    f->dma_token = 0;
    if (result.status != PD_OK || result.complete_bytes != 8 ||
        f->state != CF_RUNNING)
        return CF_FAILED;
    struct cpf_sample sample = {0};
    for (unsigned i = 0; i < 4; ++i)
        sample.sequence |= (uint32_t)f->dma_destination[i] << (8 * i);
    sample.value = (uint16_t)((unsigned)f->dma_destination[4] |
                              ((unsigned)f->dma_destination[5] << 8));
    sample.tick = (uint16_t)((unsigned)f->dma_destination[6] |
                             ((unsigned)f->dma_destination[7] << 8));
    return cf_acquire(f, 1, &sample);
}

unsigned cf_watchdog(struct cf_firmware *f, unsigned epoch_mask)
{
    if (!f || f->state != CF_RUNNING || epoch_mask > 3)
        return 0;
    unsigned feed = CF_MUTANT == 3 ? !!epoch_mask : epoch_mask == 3;
    if (feed && f->feeds < UINT_MAX)
        ++f->feeds;
    return feed;
}

void cf_fault(struct cf_firmware *f, uint32_t cause)
{
    if (!f || !cause)
        return;
    if (!f->first_fault)
        f->first_fault = cause;
    f->actuator = 0;
    (void)cs_close(&f->core, 1);
    if (f->state == CF_SAFE_STOP)
        return;
    if (f->attempts == 2)
        f->state = CF_SAFE_STOP;
    else
    {
        ++f->attempts;
        f->state = CF_RECOVERING;
    }
}

enum cf_status cf_recover(struct cf_firmware *f)
{
    if (!f || f->state != CF_RECOVERING)
        return CF_INVALID;
    if (f->dma_token || f->core.borrowed || f->core.has_active)
        return CF_PENDING;
    f->prior_accepted += f->core.accepted;
    f->prior_delivered += f->core.delivered;
    f->prior_discarded += f->core.discarded;
    /* A caller-established quiescent acquisition/notification boundary is
     * required before reinitializing either producer; this is not a timeout. */
    if (cs_init(&f->core, 3, 1000) != CS_OK || !cpf_ring_init(&f->samples, 4))
        return CF_FAILED;
    f->state = CF_RUNNING;
    return CF_OK;
}

int cf_budget(uint32_t limit, uint32_t extra, uint32_t *total,
              uint32_t *remaining)
{
    if (!total || !remaining || extra > UINT32_MAX - 1896u)
        return 0;
    uint32_t n = 1896u + extra;
    if (n > limit)
        return 0;
    *total = n;
    *remaining = limit - n;
    return 1;
}

static uint32_t slot_crc(const struct cf_slot *s)
{
    unsigned char bytes[14];
    for (unsigned i = 0; i < 8; ++i)
        bytes[i] = s->payload[i];
    bytes[8] = (unsigned char)s->generation;
    bytes[9] = (unsigned char)(s->generation >> 8);
    for (unsigned i = 0; i < 4; ++i)
        bytes[10 + i] = (unsigned char)(s->compatible >> (8 * i));
    return cp_crc32(bytes, sizeof bytes);
}

void cf_candidate(struct cf_slot *s, uint16_t generation, unsigned cut,
                  unsigned compatible, unsigned trusted, unsigned parts)
{
    struct cf_slot empty = {0};
    *s = empty;
    if (cut >= 1)
        for (unsigned i = 0; i < 8; ++i)
            s->payload[i] = (unsigned char)(0xa5u + i);
    if (cut >= 2)
    {
        s->generation = generation;
        s->compatible = compatible;
    }
    if (cut >= 3)
        s->crc = slot_crc(s);
    if (cut >= 4)
        s->verified = !!trusted;
    if (cut >= 5)
    {
        if (parts >= 1)
            s->commit_a = UINT32_C(0x29a7c45d);
        if (parts >= 2)
            s->commit_b = ~UINT32_C(0x29a7c45d);
    }
}

static int valid(const struct cf_slot *s)
{
    return s->compatible == 1 && (s->verified == 1 || CF_MUTANT == 4) &&
           s->crc == slot_crc(s) && s->commit_a == UINT32_C(0x29a7c45d) &&
           s->commit_b == ~UINT32_C(0x29a7c45d);
}

int cf_select(const struct cf_slot *a, const struct cf_slot *b, unsigned *out)
{
    if (!a || !b || !out)
        return 0;
    int av = valid(a), bv = valid(b);
    if (!av && !bv)
        return 0;
    if (!bv)
    {
        *out = 0;
        return 1;
    }
    if (!av)
    {
        *out = 1;
        return 1;
    }
    uint16_t distance = (uint16_t)(b->generation - a->generation);
    if (!distance || distance == 32768)
        return 0;
    *out = distance < 32768;
    return 1;
}

static uint32_t crash_crc(uint32_t sequence, uint32_t cause)
{
    unsigned char bytes[8];
    for (unsigned i = 0; i < 4; ++i)
    {
        bytes[i] = (unsigned char)(sequence >> (8 * i));
        bytes[4 + i] = (unsigned char)(cause >> (8 * i));
    }
    return cp_crc32(bytes, 8);
}

void cf_crash(struct cf_crash *r, uint32_t sequence, uint32_t cause,
              unsigned cut)
{
    struct cf_crash empty = {0};
    *r = empty;
    if (cut >= 1)
        r->sequence = sequence;
    if (cut >= 2)
        r->cause = cause;
    if (cut >= 3)
        r->check = crash_crc(sequence, cause);
    if (cut >= 4)
        r->commit = UINT32_C(0xc735e14a);
}

static int crash_valid(const struct cf_crash *r)
{
    return r->sequence && r->cause && r->commit == UINT32_C(0xc735e14a) &&
           r->check == crash_crc(r->sequence, r->cause);
}

int cf_crash_select(const struct cf_crash *a, const struct cf_crash *b,
                    unsigned *out)
{
    if (!a || !b || !out)
        return 0;
    int av = crash_valid(a), bv = crash_valid(b);
    if (!av && !bv)
        return 0;
    if (av && bv && a->sequence == b->sequence)
        return 0;
    *out = bv && (!av || b->sequence > a->sequence);
    return 1;
}

int cf_copy_request(const unsigned char *base, size_t length, size_t offset,
                    size_t count, unsigned char out[8])
{
    if (!base || !out || !count || count > 8 || offset > length ||
        count > length - offset)
        return 0;
    for (size_t i = 0; i < count; ++i)
        out[i] = base[offset + i];
    return 1;
}

int cf_elapsed(uint16_t start, uint16_t end, unsigned wraps, uint32_t *out)
{
    if (!out || wraps > 4 || (!wraps && end < start))
        return 0;
    *out = (uint32_t)wraps * 65536u + end - start;
    return 1;
}

unsigned cf_status_read(struct cf_register *r)
{
    unsigned observed = r->flags;
    r->flags &= ~4u;
    return observed;
}

void cf_status_write(struct cf_register *r, unsigned value)
{
    r->flags &= ~(value & 3u);
}

int cf_status_ack(struct cf_register *r, unsigned mask)
{
    if (!r || !mask || (mask & ~3u))
        return 0;
    cf_status_write(r, CF_MUTANT == 5 ? cf_status_read(r) | mask : mask);
    return 1;
}
