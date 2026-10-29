#include "mmio.h"
#include <limits.h>
#ifndef PM_MUTANT
#define PM_MUTANT 0
#endif
int pm_access(struct pm_model *m, unsigned off, unsigned width, unsigned wr,
              uint32_t *v)
{
    if (!m || !v || wr > 1 || m->revision < 1 || m->revision > 2 ||
        m->log_count >= 64 || off > PM_BSRR || off % 4 ||
        (width != 32 && !(off == PM_DATA && width == 8)))
    {
        return -1;
    }
    uint32_t value = *v;
    if (wr)
    {
        if (off == PM_CTRL && (value & ~(m->revision == 1 ? 3u : 7u)))
        {
            return -1;
        }
        if (off == PM_DATA && (value > 255 || !(m->control & 1) ||
                               !(m->status & 1) || m->tx_count == UINT_MAX))
        {
            return -1;
        }
        if (off == PM_RTC || (off == PM_STATUS && (value & ~31u)) ||
            (off == PM_ODR && (value & ~65535u)))
        {
            return -1;
        }
    }
    else if (off == PM_BSRR ||
             (off == PM_DATA && (!m->rx_count || m->rx_count > 4)))
    {
        return -1;
    }
    if (wr)
    {
        if (off == PM_CTRL)
        {
            m->control = value;
        }
        else if (off == PM_STATUS)
        {
            m->status &= ~(PM_MUTANT == 1 ? (m->status & ~value) : value);
        }
        else if (off == PM_DATA)
        {
            m->last_tx = (unsigned char)value;
            ++m->tx_count;
            m->status &= ~1u;
        }
        else if (off == PM_ODR)
        {
            m->odr = value;
        }
        else if (off == PM_BSRR)
        {
            m->odr = (m->odr & ~(value >> 16)) | (value & 65535u);
        }
    }
    else
    {
        if (off == PM_CTRL)
        {
            value = m->control;
        }
        else if (off == PM_STATUS)
        {
            value = m->status;
        }
        else if (off == PM_ODR)
        {
            value = m->odr;
        }
        else if (off == PM_RTC)
        {
            value = m->rtc;
            if (PM_MUTANT != 4)
            {
                m->rtc = 0;
            }
        }
        else
        {
            value = m->rx[0];
            unsigned drop = PM_MUTANT == 2 && m->rx_count > 1 ? 2 : 1;
            for (unsigned i = drop; i < m->rx_count; ++i)
            {
                m->rx[i - drop] = m->rx[i];
            }
            m->rx_count -= drop;
        }
        *v = value;
    }
    m->log[m->log_count++] = (struct pm_access){off, width, wr, value};
    return 0;
}

int pm_gpio(struct fw_io io, unsigned set, unsigned clear)
{
    if (!io.write || !io.read || set > 65535 || clear > 65535 || (set & clear))
    {
        return -1;
    }
    if (PM_MUTANT == 3)
    {
        uint32_t v = io.read(io.context, FW_GPIO);
        io.write(io.context, FW_GPIO, (v | set) & ~clear);
    }
    else
    {
        io.write(io.context, FW_GPIO, set | (clear << 16));
    }
    return 0;
}

int pm_begin(struct pm_request *r, unsigned char b, unsigned polls)
{
    if (!r || !polls || r->phase == 1 || r->phase > 3)
    {
        return -1;
    }
    *r = (struct pm_request){1, polls, b};
    return 0;
}

int pm_step(struct pm_request *r, struct pm_model *m)
{
    if (!r || !m || r->phase != 1)
    {
        return -1;
    }
    uint32_t status = 0;
    if (pm_access(m, PM_STATUS, 32, 0, &status))
    {
        return -1;
    }
    if (status & 1)
    {
        uint32_t b = r->byte;
        if (pm_access(m, PM_DATA, 8, 1, &b))
        {
            return -1;
        }
        r->phase = 2;
        return 0;
    }
    if (!--r->remaining)
    {
        r->phase = 3;
        return -2;
    }
    return 1;
}

int pm_filter_init(struct pm_filter *d, unsigned n, uint32_t duration,
                   uint32_t gap)
{
    if (!d || !n || duration >= 0x80000000u ||
        (duration && (!gap || gap >= 0x80000000u)))
    {
        return -1;
    }
    d->stable = 0;
    d->candidate = 0;
    d->count = 0;
    d->events = 0;
    d->initialized = 0;
    d->last = 0;
    d->since = 0;
    d->threshold = n;
    d->duration = duration;
    d->max_gap = gap;
    return 0;
}

int pm_sample(struct pm_filter *d, unsigned b, uint32_t now, unsigned *changed)
{
    if (!d || !changed || b > 1 || !d->threshold)
    {
        return -1;
    }
    uint32_t elapsed = now - d->last;
    if (d->initialized && d->duration && elapsed >= 0x80000000u)
    {
        return -1;
    }
    struct pm_filter t = *d;
    if (!t.initialized || b != t.candidate ||
        (t.duration && elapsed > t.max_gap))
    {
        t.candidate = b;
        t.count = 1;
        t.since = now;
    }
    else if (t.count < t.threshold)
    {
        ++t.count;
    }
    t.initialized = 1;
    t.last = now;
    unsigned ready = t.duration && PM_MUTANT != 5 ? now - t.since >= t.duration
                                                  : t.count >= t.threshold;
    unsigned event = ready && b != t.stable;
    if (event)
    {
        if (t.events == UINT_MAX)
        {
            return -1;
        }
        ++t.events;
        t.stable = b;
    }
    *d = t;
    *changed = event;
    return 0;
}

int pm_line(unsigned od, unsigned out, unsigned low, unsigned pull)
{
    if (od > 1 || out > 1 || low > 1 || pull > 1)
    {
        return -1;
    }
    if (!out || low)
    {
        return !od && out && low ? 3 : 0;
    }
    return !od || pull ? 1 : 2;
}

int pm_irq_event(struct pm_interrupt *q, struct pm_model *m, uint32_t now)
{
    if (!q || !m || !q->armed || q->request.phase != 1 ||
        now - q->last >= 0x80000000u || now - q->start >= 0x80000000u ||
        m->log_count > 61)
    {
        return -1;
    }
    uint32_t status = 0;
    if (pm_access(m, PM_STATUS, 32, 0, &status))
    {
        return -1;
    }
    if (status & 1)
    {
        uint32_t b = q->request.byte;
        if (pm_access(m, PM_DATA, 8, 1, &b))
        {
            return -1;
        }
        q->request.phase = 2;
    }
    else if (now - q->start >= (PM_MUTANT == 7 ? q->limit - 1 : q->limit))
    {
        q->request.phase = 3;
    }
    q->last = now;
    if (q->request.phase != 1)
    {
        uint32_t control = m->control & ~2u;
        if (pm_access(m, PM_CTRL, 32, 1, &control))
        {
            return -1;
        }
        q->armed = 0;
        return q->request.phase == 2 ? 0 : -2;
    }
    return 1;
}

int pm_irq_begin(struct pm_interrupt *q, struct pm_model *m, unsigned char byte,
                 uint32_t now, uint32_t ticks)
{
    if (!q || !m || q->armed || !ticks || ticks >= 0x80000000u ||
        !(m->control & 1) || m->log_count > 60)
    {
        return -1;
    }
    uint32_t control = m->control | 2u;
    if (pm_access(m, PM_CTRL, 32, 1, &control))
    {
        return -1;
    }
    q->request.phase = 1;
    q->request.remaining = 1;
    q->request.byte = byte;
    q->start = now;
    q->last = now;
    q->limit = ticks;
    q->armed = 1;
    /* Recheck after enabling, including readiness that preceded interrupt arming.
     */
    return PM_MUTANT == 6 ? 1 : pm_irq_event(q, m, now);
}
