#include "posted.h"
#ifndef PL_POST_MUTANT
#define PL_POST_MUTANT 0
#endif
int pl_flush(struct pl_posted *m)
{
    if (!m || m->count > 8 || m->head > 7)
    {
        return PL_INVALID;
    }
    while (m->count)
    {
        struct pm_access a = m->pending[m->head];
        uint32_t v = a.value;
        if (pm_access(&m->device, a.offset, a.width, 1, &v))
        {
            return PL_IO;
        }
        m->head = (m->head + 1) % 8;
        m->count--;
        m->delivered++;
    }
    return PL_OK;
}

int pl_post_write(struct pl_posted *m, unsigned off, unsigned width,
                  uint32_t value)
{
    if (!m || m->count > 8 || m->head > 7 || m->immediate > 1)
    {
        return PL_INVALID;
    }
    if ((off != PM_CTRL && off != PM_STATUS && off != PM_DATA) ||
        (width != 32 && !(off == PM_DATA && width == 8)))
    {
        return PL_INVALID;
    }
    if (m->count == 8)
    {
        return PL_BUSY;
    }
    m->pending[(m->head + m->count) % 8] =
        (struct pm_access){off, width, 1, value};
    m->count++;
    return m->immediate ? pl_flush(m) : PL_OK;
}

int pl_post_read(struct pl_posted *m, unsigned off, unsigned width,
                 uint32_t *out)
{
    if (!m || !out)
    {
        return PL_INVALID;
    }
    return pm_access(&m->device, off, width, 0, out) ? PL_IO : PL_OK;
}

int pl_exchange_begin(struct pl_exchange *s, struct pl_posted *m, uint8_t b,
                      unsigned budget)
{
    if (!s || !m || !budget)
    {
        return PL_INVALID;
    }
    if (s->phase == 1 || s->phase == 2 || m->count)
    {
        return PL_BUSY;
    }
    int r = pl_post_write(m, PM_CTRL, 32, 1);
    if (r != PL_OK)
    {
        s->phase = 4;
        return r;
    }
    if (PL_POST_MUTANT != 3 && (r = pl_flush(m)) != PL_OK)
    {
        s->phase = 4;
        return r;
    }
    *s = (struct pl_exchange){1, budget, b};
    return PL_OK;
}

int pl_exchange_step(struct pl_exchange *s, struct pl_posted *m, uint8_t *out)
{
    if (!s || !m || !out)
    {
        return PL_INVALID;
    }
    if (s->phase != 1 && s->phase != 2)
    {
        return PL_STALE;
    }
    uint32_t status = 0, value = 0;
    int r = pl_post_read(m, PM_STATUS, 32, &status);
    if (r != PL_OK)
    {
        s->phase = 4;
        return r;
    }
    if (status & 2)
    {
        r = pl_post_write(m, PM_STATUS, 32,
                          PL_POST_MUTANT == 2 ? status | 2u : 2u);
        if (r == PL_OK)
        {
            r = pl_flush(m);
        }
        s->phase = r == PL_OK ? 3u : 4u;
        return r == PL_OK ? PL_IO : r;
    }
    if (s->phase == 1 && (status & 1))
    {
        r = pl_post_write(m, PM_DATA, 8, s->byte);
        if (r == PL_OK && PL_POST_MUTANT != 3)
        {
            r = pl_flush(m);
        }
        if (r != PL_OK)
        {
            s->phase = 4;
            return r;
        }
        s->phase = 2;
    }
    if (s->phase == 2 && (status & 4))
    {
        r = pl_post_read(m, PM_DATA, 8, &value);
        if (r != PL_OK)
        {
            s->phase = 4;
            return r;
        }
        if (PL_POST_MUTANT == 1)
        {
            r = pl_post_read(m, PM_DATA, 8, &value);
            if (r != PL_OK)
            {
                return r;
            }
        }
        *out = (uint8_t)value;
        s->phase = 3;
        return PL_OK;
    }
    if (!--s->left)
    {
        s->phase = 3;
        return PL_TIMEOUT;
    }
    return PL_BUSY;
}
