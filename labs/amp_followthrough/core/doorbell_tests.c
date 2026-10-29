#include "doorbell.h"
#include <stdio.h>
#include <string.h>
#define C(x)                                                        \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "AF_DOOR_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                               \
        }                                                           \
    } while (0)

struct af_model
{
    uint32_t enable[2], pending[2], held;
    unsigned reads, writes, releases, step, inject, put;
    struct af_ring *q;
    struct af_door d;
};

static void tick(struct af_model *m)
{
    if (m->q && m->step++ == m->inject)
    {
        struct af_message v = {
            .epoch = 1, .sequence = 99, .length = 1, .bytes = {42}};
        m->put = af_batch(m->q, 1, &v, 1) == AF_OK;
        m->pending[m->d.core == 4] |= UINT32_C(1) << m->d.incoming;
    }
}

static uint32_t rd(void *ctx, uint32_t a)
{
    struct af_model *m = ctx;
    tick(m);
    m->reads++;
    if (a >= 0x58026480u && a < 0x58026500u)
    {
        return m->held;
    }
    if (a == 0x58026500u || a == 0x58026510u)
    {
        return m->enable[(a - 0x58026500u) / 16];
    }
    return 0;
}

static void wr(void *ctx, uint32_t a, uint32_t v)
{
    struct af_model *m = ctx;
    tick(m);
    m->writes++;
    if (a >= 0x58026400u && a < 0x58026480u)
    {
        m->releases++;
        m->held = v;
    }
    else if (a == 0x58026500u || a == 0x58026510u)
    {
        m->enable[(a - 0x58026500u) / 16] = v;
    }
    else if (a == 0x58026504u || a == 0x58026514u)
    {
        m->pending[(a - 0x58026504u) / 16] &= ~v;
    }
}

static void fence(void *ctx)
{
    tick(ctx);
}

int main(void)
{
    for (unsigned core = 4; core <= 7; core += 3)
    {
        for (unsigned sem = 0; sem < 4; sem++)
        {
            struct af_model m = {0};
            struct af_door d = {core, sem, sem + 4};
            struct af_registers io = {&m, rd, wr, fence};
            m.held = 0x80000000u | ((core == 7 ? 1u : 3u) << 8);
            C(af_ring_door(io, d) == AF_BUSY && m.releases == 0);
            m.held = 0x80000000u | ((core == 7 ? 3u : 1u) << 8);
            C(af_ring_door(io, d) == AF_OK && m.releases == 1);
            m.held |= 5;
            C(af_ring_door(io, d) == AF_BUSY && m.releases == 1);
            C(af_mask_door(io, d, 1) == AF_OK);
            C(m.enable[core == 4] == (1u << d.incoming) &&
              m.enable[core == 7] == 0);
        }
    }
    for (unsigned when = 0; when < 10; when++)
    {
        struct af_ring q;
        af_ring_init(&q, 0);
        struct af_model m = {.q = &q, .inject = when, .d = {4, 1, 0}};
        struct af_registers io = {&m, rd, wr, fence};
        unsigned done = 0, again = 0;
        C(af_service_door(io, m.d, &q, 1, 2, &done, &again) == AF_OK);
        /* Inject also after the final software recheck, representing the next
           producer release; its unmasked pending interrupt is the wake source. */
        if (!m.put)
        {
            while (!m.put)
            {
                tick(&m);
            }
        }
        C(done == 1 || again || (m.pending[1] & m.enable[1]));
        C(done + af_queued(&q) == 1);
    }
    struct af_ring q;
    af_ring_init(&q, 0);
    struct af_message v[3] = {{.epoch = 1, .sequence = 1},
                              {.epoch = 1, .sequence = 2},
                              {.epoch = 1, .sequence = 3}};
    C(af_batch(&q, 1, v, 3) == AF_OK);
    struct af_model m = {.pending = {1, 0}};
    struct af_registers io = {&m, rd, wr, fence};
    unsigned done, again;
    C(af_service_door(io, (struct af_door){7, 1, 0}, &q, 1, 2, &done, &again) ==
          AF_OK &&
      done == 2 && again);
    C(af_service_door(io, (struct af_door){7, 1, 0}, &q, 1, 2, &done, &again) ==
          AF_OK &&
      done == 1 && !again);
    puts(
        "AF_PASS:doorbell owner, IRQ bank, coalescing, budget, rearm boundaries");
    return 0;
}
