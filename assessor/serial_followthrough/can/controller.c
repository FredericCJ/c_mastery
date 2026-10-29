#include "can.h"
#include <limits.h>
#ifndef CN_CONTROLLER_MUTANT
#define CN_CONTROLLER_MUTANT 0
#endif
static uint32_t readreg(struct cn_controller *c, unsigned r)
{
    return c->io.read(c->io.context, r);
}

static void writereg(struct cn_controller *c, unsigned r, uint32_t v)
{
    c->io.write(c->io.context, r, v);
}

static int valid(const struct cn_controller *c)
{
    return c && c->io.read && c->io.write;
}

static unsigned flags(const struct cn_controller *c)
{
    return 0x10000u | (c->nart << 4) | (c->locked << 3);
}

int cn_begin(struct cn_controller *c, struct cn_io io,
             const struct sd_timing *t, unsigned nart, unsigned locked,
             unsigned limit)
{
    if (!c || !io.read || !io.write || !t || c->state != CN_COLD || nart > 1 ||
        locked > 1 || !limit || limit > 1000000u)
    {
        return CN_INVALID;
    }
    if (!t->prescaler || t->prescaler > 1024 || !t->bs1 || t->bs1 > 16 ||
        !t->bs2 || t->bs2 > 8 || !t->sjw || t->sjw > 4 || t->sjw > t->bs2)
    {
        return CN_INVALID;
    }
    if (t->btr !=
        ((uint32_t)(t->prescaler - 1) | ((uint32_t)(t->bs1 - 1) << 16) |
         ((uint32_t)(t->bs2 - 1) << 20) | ((uint32_t)(t->sjw - 1) << 24)))
    {
        return CN_INVALID;
    }
    *c = (struct cn_controller){0};
    c->io = io;
    c->state = CN_ENTERING;
    c->nart = nart;
    c->locked = locked;
    c->remaining = limit;
    c->limit = limit;
    c->btr = t->btr;
    writereg(c, CN_IER, 0);
    writereg(c, CN_MCR, flags(c) | 1u);
    return CN_MORE;
}

int cn_control(struct cn_controller *c)
{
    if (!valid(c))
    {
        return CN_INVALID;
    }
    if (c->state == CN_RUNNING)
    {
        return CN_OK;
    }
    if (c->state == CN_CONTROL_FAILED)
    {
        return CN_UNAVAILABLE;
    }
    if (c->state != CN_ENTERING && c->state != CN_SYNCHRONIZING)
    {
        return CN_INVALID;
    }
    uint32_t msr = readreg(c, CN_MSR);
    if (c->state == CN_ENTERING && (msr & 1u))
    {
        /* Dedicated CAN1 owns all filter banks. Preserve reserved FMR reset bits and
           partition; bank0 is owned by CAN1 only with CAN2SB>=1. */
        uint32_t fmr = readreg(c, CN_FMR);
        if (((fmr >> 8) & 63u) == 0)
        {
            c->state = CN_CONTROL_FAILED;
            return CN_UNAVAILABLE;
        }
        writereg(c, CN_BTR, c->btr);
        writereg(c, CN_FMR, fmr | 1u);
        writereg(c, CN_FA1R, 0);
        writereg(c, CN_FM1R, 0);
        writereg(c, CN_FS1R, 1);
        writereg(c, CN_FFA1R, 0);
        writereg(c, CN_F0R1, 0x120u << 21);
        writereg(c, CN_F0R2, (0x7f0u << 21) | 6u);
        writereg(c, CN_FA1R, 1);
        writereg(c, CN_FMR, fmr & ~1u);
        writereg(c, CN_MCR, flags(c));
        c->state = CN_SYNCHRONIZING;
        c->remaining = c->limit;
        return CN_MORE;
    }
    if (c->state == CN_SYNCHRONIZING && !(msr & 3u) &&
        !(readreg(c, CN_ESR) & 4u))
    {
        if (!(readreg(c, CN_TSR) & (1u << 26)))
        {
            c->state = CN_CONTROL_FAILED;
            return CN_UNAVAILABLE;
        }
        c->state = CN_RUNNING;
        c->recovery = 0;
        return CN_OK;
    }
    if (--c->remaining == 0)
    {
        c->state = CN_CONTROL_FAILED;
        return CN_UNAVAILABLE;
    }
    return CN_MORE;
}

int cn_recover(struct cn_controller *c, unsigned limit)
{
    if (!valid(c) || !limit || limit > 1000000u)
    {
        return CN_INVALID;
    }
    if (c->tx_state)
    {
        return CN_BUSY;
    }
    if (c->state != CN_RUNNING && c->state != CN_CONTROL_FAILED)
    {
        return CN_BUSY;
    }
    c->recovery = 1;
    c->remaining = c->limit = limit;
    c->state = CN_ENTERING;
    writereg(c, CN_MCR, flags(c) | 1u);
    return CN_MORE;
}

static int frame_valid(const struct sd_can *f)
{
    return f && f->extended <= 1 && f->remote <= 1 && f->length <= 8 &&
           f->identifier <= (f->extended ? 0x1fffffffu : 0x7ffu);
}

int cn_send(struct cn_controller *c, const struct sd_can *f, uint32_t *token)
{
    if (!valid(c) || !frame_valid(f) || !token)
    {
        return CN_INVALID;
    }
    if (c->state != CN_RUNNING || readreg(c, CN_ESR) & 4u)
    {
        return CN_UNAVAILABLE;
    }
    if (c->tx_state || !(readreg(c, CN_TSR) & (1u << 26)))
    {
        return CN_BUSY;
    }
    if (c->last_token == UINT32_MAX)
    {
        return CN_UNAVAILABLE;
    }
    uint32_t low = 0, high = 0,
             id = f->extended ? (f->identifier << 3) | 4u : f->identifier << 21;
    for (unsigned i = 0; i < f->length && !f->remote; ++i)
    {
        if (i < 4)
        {
            low |= (uint32_t)f->data[i] << (8 * i);
        }
        else
        {
            high |= (uint32_t)f->data[i] << (8 * (i - 4));
        }
    }
    writereg(c, CN_TSR, 1u);
    writereg(c, CN_TDT0R, f->length);
    writereg(c, CN_TDL0R, low);
    writereg(c, CN_TDH0R, high);
    c->token = ++c->last_token;
    c->tx_state = 1;
    c->cancelling = 0;
    writereg(c, CN_TI0R, id | (f->remote << 1) | 1u);
    *token = c->token;
    return CN_OK;
}

int cn_tx_poll(struct cn_controller *c, uint32_t token)
{
    if (!valid(c) || !token || token != c->token)
    {
        return CN_INVALID;
    }
    if (c->tx_state == 2)
    {
        return CN_OK;
    }
    if (c->tx_state != 1)
    {
        return CN_INVALID;
    }
    uint32_t tsr = readreg(c, CN_TSR);
    if (CN_CONTROLLER_MUTANT == 1 && (tsr & 4u))
    {
        c->tx_state = 2;
        c->result = CN_LOST;
        return CN_OK;
    }
    if ((tsr & 1u) && (tsr & (1u << 26)))
    {
        c->result = (tsr & 2u)   ? CN_SENT
                    : (tsr & 8u) ? CN_TX_ERROR
                    : (tsr & 4u) ? CN_LOST
                                 : CN_ABORTED;
        c->tx_state = 2;
        writereg(c, CN_TSR, 1u);
        return CN_OK;
    }
    return (readreg(c, CN_ESR) & 4u) ? CN_UNAVAILABLE : CN_MORE;
}

int cn_cancel(struct cn_controller *c, uint32_t token)
{
    if (!valid(c) || !token || token != c->token)
    {
        return CN_INVALID;
    }
    if (c->tx_state != 1)
    {
        return CN_BUSY;
    }
    if (!c->cancelling)
    {
        writereg(c, CN_TSR, 1u << 7);
        c->cancelling = 1;
    }
    if (CN_CONTROLLER_MUTANT == 2)
    {
        c->tx_state = 2;
        c->result = CN_ABORTED;
    }
    return CN_MORE;
}

int cn_take(struct cn_controller *c, uint32_t token, unsigned *result)
{
    if (!valid(c) || !result || !token || token != c->token)
    {
        return CN_INVALID;
    }
    if (c->tx_state != 2)
    {
        return CN_BUSY;
    }
    *result = c->result;
    c->tx_state = 0;
    c->token = 0;
    c->cancelling = 0;
    return CN_OK;
}

int cn_receive(struct cn_controller *c, struct cn_rx *out)
{
    if (!valid(c) || !out)
    {
        return CN_INVALID;
    }
    if (c->state != CN_RUNNING)
    {
        return CN_UNAVAILABLE;
    }
    uint32_t status = readreg(c, CN_RF0R);
    if (!(status & 3u))
    {
        return CN_MORE;
    }
    uint32_t id = readreg(c, CN_RI0R), dt = readreg(c, CN_RDT0R),
             low = readreg(c, CN_RDL0R), high = readreg(c, CN_RDH0R);
    struct cn_rx r = {0};
    r.frame.extended = (uint8_t)((id >> 2) & 1u);
    r.frame.remote = (uint8_t)((id >> 1) & 1u);
    r.frame.identifier = r.frame.extended ? id >> 3 : id >> 21;
    r.frame.length = (uint8_t)(dt & 15u);
    r.overrun = (status >> 4) & 1u;
    r.filter = (dt >> 8) & 255u;
    r.timestamp = dt >> 16;
    if (r.frame.length <= 8 && !r.frame.remote)
    {
        for (unsigned i = 0; i < r.frame.length; ++i)
        {
            r.frame.data[i] = (uint8_t)((i < 4 ? low : high) >> (8 * (i % 4)));
        }
    }
    /* Release exactly once even when a wider controller contract supplies a DLC
       outside this exercise's stricter 0..8 representation. */
    writereg(c, CN_RF0R, CN_CONTROLLER_MUTANT == 3 ? status | 32u : 32u);
    if (r.frame.length > 8)
    {
        return CN_ERROR;
    }
    *out = r;
    return CN_FRAME;
}

int cn_ack_overrun(struct cn_controller *c)
{
    if (!valid(c))
    {
        return CN_INVALID;
    }
    if (c->state != CN_RUNNING)
    {
        return CN_UNAVAILABLE;
    }
    writereg(c, CN_RF0R, 16u);
    return CN_OK;
}

int cn_queue_put(struct cn_queue *q, const struct cn_rx *r)
{
    if (!q || !r || q->count > 3)
    {
        return CN_INVALID;
    }
    if (q->count == 3)
    {
        if (q->loss == UINT_MAX)
        {
            return CN_UNAVAILABLE;
        }
        for (unsigned i = 1; i < 3; ++i)
        {
            q->item[i - 1] = q->item[i];
        }
        --q->count;
        ++q->loss;
    }
    q->item[q->count++] = *r;
    return CN_OK;
}

int cn_queue_get(struct cn_queue *q, struct cn_rx *r)
{
    if (!q || !r || q->count > 3)
    {
        return CN_INVALID;
    }
    if (!q->count)
    {
        return CN_MORE;
    }
    *r = q->item[0];
    for (unsigned i = 1; i < q->count; ++i)
    {
        q->item[i - 1] = q->item[i];
    }
    --q->count;
    return CN_FRAME;
}
