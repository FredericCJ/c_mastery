#include "medium.h"
#include <string.h>
#ifndef DM_MUTANT
#define DM_MUTANT 0
#endif
void dm_init(struct d_medium *m)
{
    memset(m, 0, sizeof *m);
}

int dm_arm(struct d_medium *m, unsigned cut, unsigned mode, unsigned lie,
           uint64_t low, uint64_t high)
{
    if (!m || m->dead || cut > 128 || mode > DM_SUBSET || lie > 128 ||
        (high & ~((UINT64_C(1) << (DM_PAGES - 64)) - 1)))
        return 1;
    m->event = 0;
    m->cut = cut;
    m->mode = mode;
    m->lie_flush = lie;
    m->subset[0] = low;
    m->subset[1] = high;
    memset(m->trace, 0, sizeof m->trace);
    return 0;
}

void dm_reboot(struct d_medium *m)
{
    memset(m->dirty, 0, sizeof m->dirty);
    m->event = m->cut = m->lie_flush = m->read_cut = 0;
    m->dead = m->short_read = 0;
}

static void all(struct d_medium *m)
{
    for (unsigned p = DM_PAGES; p > 0; p--)
        if (m->dirty[p - 1])
        {
            memcpy(m->stable[p - 1], m->pending[p - 1], 4096);
            m->dirty[p - 1] = 0;
        }
}

static int boundary(struct d_medium *m, unsigned page, int writing)
{
    m->trace[m->event] = (struct dm_event){writing ? 1u : 2u, page};
    m->event++;
    if (!m->cut || m->event != m->cut)
        return 0;
    if (m->mode == DM_ALL)
        all(m);
    if (m->mode == DM_EVEN || m->mode == DM_ODD || m->mode == DM_SUBSET)
        for (unsigned p = DM_PAGES; p > 0; p--)
        {
            unsigned id = p - 1;
            int selected = m->mode == DM_SUBSET
                               ? (int)((m->subset[id / 64] >> (id % 64)) & 1u)
                               : (id % 2 == (m->mode == DM_ODD ? 1u : 0u));
            if (m->dirty[id] && selected)
                memcpy(m->stable[id], m->pending[id], 4096);
        }
    if (writing &&
        (m->mode == DM_PREFIX || m->mode == DM_SUFFIX || m->mode == DM_MIDDLE))
    {
        size_t offset = m->mode == DM_PREFIX   ? 0
                        : m->mode == DM_SUFFIX ? 2048
                                               : 512;
        size_t count = m->mode == DM_MIDDLE ? 512 : 2048;
        memcpy(m->stable[page] + offset, m->pending[page] + offset, count);
    }
    m->dead = 1;
    return 1;
}

int dm_read(void *ctx, uint32_t page, void *out)
{
    struct d_medium *m = ctx;
    if (m->dead || page >= DM_PAGES || m->reads == UINT64_MAX)
        return 1;
    m->reads++;
    if (m->read_cut && m->reads == m->read_cut)
    {
        if (m->short_read)
            memcpy(out, m->dirty[page] ? m->pending[page] : m->stable[page],
                   17);
        return 1;
    }
    memcpy(out, m->dirty[page] ? m->pending[page] : m->stable[page], 4096);
    return 0;
}

int dm_write(void *ctx, uint32_t page, const void *bytes)
{
    struct d_medium *m = ctx;
    if (m->dead || page >= DM_PAGES || m->writes == UINT64_MAX ||
        m->event == 128)
        return 1;
    m->writes++;
    memcpy(m->pending[page], bytes, 4096);
    m->dirty[page] = 1;
    return boundary(m, page, 1);
}

int dm_flush(void *ctx)
{
    struct d_medium *m = ctx;
    if (m->dead || m->flushes == UINT64_MAX || m->event == 128)
        return 1;
    m->flushes++;
    if (boundary(m, 0, 0))
        return 1;
    if (m->event == m->lie_flush || DM_MUTANT == 1)
        return 0;
    all(m);
    return 0;
}

store_io dm_io(struct d_medium *m)
{
    return (store_io){m, dm_read, dm_write, dm_flush, DM_PAGES};
}
