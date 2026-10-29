#include "physical.h"
#include <limits.h>
#ifndef EP_MUTANT
#define EP_MUTANT 0
#endif
static int time_ok(uint32_t last, uint32_t now)
{
    return now >= last && now <= UINT32_MAX - 30000u;
}

int ep_open(struct ep_tx *s, struct r5_io io, const struct pu_clock *clock,
            uint32_t baud, const struct r5_timing *timing, uint32_t now)
{
    if (!s || !io.uart.read || !io.uart.write || !io.drive || !timing ||
        timing->tick_ns != 1000 || timing->receiver_disabled ||
        timing->guard_ns != 200000 || !time_ok(0, now))
    {
        return EP_INVALID;
    }
    if (s->enabled || s->phase || s->byte.active)
    {
        return EP_BUSY;
    }
    struct r5_delays d;
    struct pu_rates rates;
    if (r5_delays(timing, &d) || pu_rates(clock, &rates) ||
        pu_open(&s->byte, io.uart, clock, baud, 0, 4))
    {
        return EP_INVALID;
    }
    s->io = io;
    s->delays = d;
    s->clock = rates.apb1;
    s->divisor = io.uart.read(io.uart.context, PU_BRR);
    s->last = now;
    s->enabled = 1;
    io.drive(io.context, 0);
    return EP_OK;
}

int ep_fits(const struct ep_tx *s, unsigned n, uint32_t budget, unsigned *out)
{
    if (!s || !s->enabled || !out || !n || n > 96 || !budget || budget > 10000)
    {
        return EP_INVALID;
    }
    /* OVER16 BRR is the integer divider, 8N1. No 64-bit division helper. */
    *out = EP_MUTANT == 1 ? 1
                          : (uint64_t)n * 10u * s->divisor * 1000000u <=
                                (uint64_t)s->clock * budget;
    return EP_OK;
}

int ep_send(struct ep_tx *s, uint32_t token, const uint8_t *p, size_t n,
            uint32_t now, uint32_t budget)
{
    unsigned fits;
    if (!s || !p || !token || !time_ok(s->last, now) || n > 96 ||
        ep_fits(s, (unsigned)n, budget, &fits))
    {
        return EP_INVALID;
    }
    if (s->phase)
    {
        return EP_BUSY;
    }
    if (token <= s->token)
    {
        return EP_STALE;
    }
    if (!fits)
    {
        return EP_TIMEOUT;
    }
    if ((n + 7) / 8 > UINT32_MAX - s->byte.token)
    {
        return EP_FULL;
    }
    for (unsigned i = 0; i < n; i++)
    {
        s->bytes[i] = p[i];
    }
    s->token = token;
    s->count = (unsigned)n;
    s->offset = 0;
    s->cancel = 0;
    s->result = EP_BUSY;
    uint32_t enable = EP_MUTANT == 7
                          ? s->delays.enable
                          : (s->delays.enable > 200 ? s->delays.enable : 200);
    s->last = now;
    s->due = now + enable;
    s->deadline = s->due + budget;
    s->first_deadline = 0;
    s->phase = EP_ENABLE;
    s->io.drive(s->io.context, 1);
    return EP_OK;
}

static int stop(struct ep_tx *s, uint32_t now, unsigned result)
{
    s->result = result;
    s->io.drive(s->io.context, 0);
    s->due = now + (s->delays.disable > s->delays.guard ? s->delays.disable
                                                        : s->delays.guard);
    s->phase = EP_GUARD;
    return EP_BUSY;
}

int ep_cancel(struct ep_tx *s, uint32_t token, uint32_t now)
{
    if (!s || !time_ok(s->last, now))
    {
        return EP_INVALID;
    }
    if (!s->phase || token != s->token)
    {
        return EP_STALE;
    }
    s->last = now;
    s->cancel = 1;
    if (s->phase == EP_ENABLE)
    {
        return stop(s, now, EP_DROP);
    }
    if (s->phase == EP_ACTIVE && s->byte.active)
    {
        (void)pu_cancel(&s->byte, s->io.uart, s->byte.token);
    }
    return EP_BUSY;
}

int ep_step(struct ep_tx *s, uint32_t token, uint32_t now)
{
    if (!s || !time_ok(s->last, now))
    {
        return EP_INVALID;
    }
    if (!s->phase || token != s->token)
    {
        return EP_STALE;
    }
    s->last = now;
    if (s->phase == EP_GUARD)
    {
        if (now < s->due)
        {
            return EP_BUSY;
        }
        s->phase = EP_IDLE;
        return (int)s->result;
    }
    if (s->phase == EP_ENABLE)
    {
        if (now < s->due)
        {
            return EP_BUSY;
        }
        if (now >= s->deadline)
        {
            return stop(s, now, EP_TIMEOUT);
        }
        s->chunk = s->count > 8 ? 8 : s->count;
        if (pu_submit(&s->byte, s->io.uart, s->byte.configured_generation,
                      s->byte.token + 1, PU_POLL, s->bytes, s->chunk, now,
                      s->deadline - now))
        {
            return stop(s, now, EP_DROP);
        }
        s->phase = EP_ACTIVE;
    }
    if (s->first_deadline && s->offset == 0 && s->byte.position == 0 &&
        now > s->first_deadline)
    {
        s->cancel = 1;
        s->result = EP_TIMEOUT;
        (void)pu_cancel(&s->byte, s->io.uart, s->byte.token);
    }
    if (now >= s->deadline && !s->cancel)
    {
        s->cancel = 1;
        s->result = EP_TIMEOUT;
        (void)pu_cancel(&s->byte, s->io.uart, s->byte.token);
    }
    int r = pu_step(&s->byte, s->io.uart, s->byte.token, now);
    if (EP_MUTANT == 2 && s->byte.position == s->byte.count)
    {
        return stop(s, now, EP_OK);
    }
    if (s->byte.active || s->byte.memory_owned || s->byte.line_owned)
    {
        return EP_BUSY;
    }
    if (r != PU_OK || s->cancel)
    {
        return stop(s, now,
                    r == PU_TIMEOUT || s->result == EP_TIMEOUT ? EP_TIMEOUT
                                                               : EP_DROP);
    }
    s->offset += s->chunk;
    if (s->offset == s->count)
    {
        return stop(s, now, EP_OK);
    }
    if (now >= s->deadline)
    {
        return stop(s, now, EP_TIMEOUT);
    }
    s->chunk = s->count - s->offset;
    if (s->chunk > 8)
    {
        s->chunk = 8;
    }
    if (EP_MUTANT == 3)
    {
        s->io.drive(s->io.context, 0);
    }
    if (pu_submit(&s->byte, s->io.uart, s->byte.configured_generation,
                  s->byte.token + 1, PU_POLL, s->bytes + s->offset, s->chunk,
                  now, s->deadline - now))
    {
        return stop(s, now, EP_DROP);
    }
    return EP_BUSY;
}

int ep_poll(struct ep_master *s, uint32_t token, uint32_t now)
{
    if (!s || !time_ok(s->last, now) || now > UINT32_MAX - 60000u)
    {
        return EP_INVALID;
    }
    if (s->polling || s->grant.phase || now < s->grant.turn_due)
    {
        return EP_BUSY;
    }
    if (s->grant.generation == UINT32_MAX)
    {
        return EP_FULL;
    }
    struct edu_frame f = {
        .source = 1, .destination = 2, .kind = 1, .length = 0};
    uint8_t bytes[96];
    size_t n = 0;
    if (edu_frame_encode(&f, bytes, sizeof bytes, &n))
    {
        return EP_INVALID;
    }
    int r = ep_send(&s->tx, token, bytes, n, now, 10000);
    if (r)
    {
        return r;
    }
    s->last = now;
    s->polling = 1;
    s->parser = (struct ef_framer){0};
    return EP_OK;
}

int ep_tick(struct ep_master *s, uint32_t now)
{
    if (!s || !time_ok(s->last, now))
    {
        return EP_INVALID;
    }
    s->last = now;
    if (s->polling)
    {
        int r = ep_step(&s->tx, s->tx.token, now);
        /* The actual final-TC/DE-low transition starts the existing200us guard;
           local guard and peer earliest-start interval are the same interval. */
        if (((EP_MUTANT == 8 && r == EP_OK) ||
             (EP_MUTANT != 8 && s->tx.phase == EP_GUARD)) &&
            s->tx.result == EP_OK && !s->grant.phase)
        {
            if (e7_grant_begin(&s->grant, now, &s->token))
            {
                return EP_INVALID;
            }
        }
        if (r == EP_BUSY)
        {
            return r;
        }
        s->polling = 0;
        if (r)
        {
            return r;
        }
    }
    if (EP_MUTANT == 4 && s->grant.phase == 2 && now > s->grant.start_deadline)
    {
        s->grant.phase = 0;
        return EP_TIMEOUT;
    }
    int r = e7_grant_tick(&s->grant, now);
    if (r == E7_START_TIMEOUT || r == E7_END_TIMEOUT)
    {
        s->parser = (struct ef_framer){0};
        return EP_TIMEOUT;
    }
    return r ? EP_INVALID : EP_OK;
}

int ep_receive(struct ep_master *s, unsigned status, uint8_t value,
               uint32_t now, struct edu_frame *out)
{
    if (!s || !out || status > 255 || !time_ok(s->last, now))
    {
        return EP_INVALID;
    }
    if (!(status & PU_RXNE))
    {
        return EP_BUSY;
    }
    if ((status & 15u) && s->faults == UINT32_MAX)
    {
        return EP_FULL;
    }
    if (s->tx.phase && s->echoes == UINT32_MAX)
    {
        return EP_FULL;
    }
    if (!s->tx.phase && (s->drops == UINT32_MAX || s->delivered == UINT32_MAX))
    {
        return EP_FULL;
    }
    s->last = now;
    if ((status & 15u) && EP_MUTANT != 5)
    {
        s->faults++;
        s->parser = (struct ef_framer){0};
    }
    if (s->tx.phase)
    {
        s->echoes++;
        return EP_DROP;
    }
    if (status & 15u)
    {
        s->drops++;
        return EP_DROP;
    }
    if (!s->grant.phase)
    {
        s->drops++;
        return EP_DROP;
    }
    if (s->grant.phase == 1)
    {
        int r = e7_grant_first(&s->grant, s->token, now);
        if (r)
        {
            s->drops++;
            return EP_DROP;
        }
    }
    if (e7_grant_tick(&s->grant, now))
    {
        s->parser = (struct ef_framer){0};
        s->drops++;
        return EP_DROP;
    }
    struct edu_frame f;
    int r = ef_frame_byte(&s->parser, 1, value, 0, &f);
    if (r != 1)
    {
        return EP_BUSY;
    }
    unsigned deliver = 0;
    if (f.source != 2 ||
        e7_link_admit(f.kind, 1, f.destination, 1, 0, 0, &deliver))
    {
        s->drops++;
        return EP_DROP;
    }
    if (e7_grant_finish(&s->grant, s->token, now))
    {
        s->drops++;
        return EP_DROP;
    }
    if (!deliver)
    {
        return EP_OK;
    }
    s->delivered++;
    *out = f;
    return EP_FRAME;
}

int ep_disconnect(struct ep_master *s, uint32_t now)
{
    if (!s || !time_ok(s->last, now) || s->faults == UINT32_MAX)
    {
        return EP_INVALID;
    }
    s->last = now;
    s->faults++;
    s->parser = (struct ef_framer){0};
    if (s->tx.phase)
    {
        return ep_cancel(&s->tx, s->tx.token, now);
    }
    s->grant.phase = 0;
    return EP_OK;
}

int ep_slave_receive(struct ep_slave *s, unsigned status, uint8_t value,
                     uint32_t now)
{
    if (!s || status > 255 || !time_ok(s->last, now))
    {
        return EP_INVALID;
    }
    if (!(status & PU_RXNE))
    {
        return EP_BUSY;
    }
    s->last = now;
    if (status & 15u)
    {
        s->parser = (struct ef_framer){0};
        return EP_DROP;
    }
    if (s->tx.phase)
    {
        return EP_DROP;
    }
    struct edu_frame f;
    int r = ef_frame_byte(&s->parser, 1, value, 0, &f);
    if (r != 1)
    {
        return EP_BUSY;
    }
    unsigned data = 99;
    if (f.source != 1 || f.kind != 1 ||
        e7_link_admit(f.kind, 2, f.destination, 0, 0, 0, &data))
    {
        return EP_DROP;
    }
    if (s->granted && now <= s->origin + 2200)
    {
        return EP_BUSY;
    }
    s->granted = 1;
    s->origin = now;
    return EP_OK;
}

int ep_reply(struct ep_slave *s, uint32_t token, const struct edu_frame *f,
             uint32_t now)
{
    if (!s || !f || !time_ok(s->last, now) || f->source != 2 ||
        f->destination != 1 || f->kind == 1)
    {
        return EP_INVALID;
    }
    if (EP_MUTANT != 6 && !s->granted)
    {
        return EP_STALE;
    }
    if (now < s->origin + 200)
    {
        return EP_BUSY;
    }
    uint32_t enable = s->tx.delays.enable > 200 ? s->tx.delays.enable : 200;
    if (now + enable > s->origin + 2200)
    {
        s->granted = 0;
        s->last = now;
        return EP_TIMEOUT;
    }
    uint8_t bytes[96];
    size_t n;
    if (edu_frame_encode(f, bytes, sizeof bytes, &n))
    {
        return EP_INVALID;
    }
    int r = ep_send(&s->tx, token, bytes, n, now, 10000);
    if (r)
    {
        return r;
    }
    s->tx.first_deadline = s->origin + 2200;
    s->granted = 0;
    s->last = now;
    return EP_OK;
}

static int sample(struct pu_io io, struct pu_byte *out)
{
    struct pu_rx q = {0};
    int r = pu_rx_poll(&q, io, 1);
    if (r == PU_TIMEOUT)
    {
        return EP_BUSY;
    }
    if (r || pu_take(&q, out))
    {
        return EP_INVALID;
    }
    return EP_OK;
}

int ep_master_poll_rx(struct ep_master *s, uint32_t now, struct edu_frame *out)
{
    if (!s || !out || !s->tx.enabled || !time_ok(s->last, now))
    {
        return EP_INVALID;
    }
    /* Reserve counter headroom before destructive DR acquisition. */
    if (s->faults == UINT32_MAX || s->echoes == UINT32_MAX ||
        s->drops == UINT32_MAX || s->delivered == UINT32_MAX)
    {
        return EP_FULL;
    }
    struct pu_byte b;
    int r = sample(s->tx.io.uart, &b);
    return r ? r : ep_receive(s, PU_RXNE | b.error, b.value, now, out);
}

int ep_slave_poll_rx(struct ep_slave *s, uint32_t now)
{
    if (!s || !s->tx.enabled || !time_ok(s->last, now))
    {
        return EP_INVALID;
    }
    struct pu_byte b;
    int r = sample(s->tx.io.uart, &b);
    return r ? r : ep_slave_receive(s, PU_RXNE | b.error, b.value, now);
}
