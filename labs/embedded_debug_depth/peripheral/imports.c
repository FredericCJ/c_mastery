/* Original scripted ports exercise actual supplied producer implementations.
   Numeric target addresses are model keys, never hosted pointers. */
#include "uart.h"
#include "spi.h"
#include "dma.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                      \
    do                                                \
    {                                                 \
        if (!(x))                                     \
        {                                             \
            printf("DP_CHECK:%u:%s\n", __LINE__, #x); \
            return 1;                                 \
        }                                             \
    } while (0)

struct du_registers
{
    uint32_t r[32];
    unsigned writes, reads, gate_seen, bad_sequence;
};

static uint32_t ur(void *v, unsigned reg)
{
    struct du_registers *m = v;
    if (reg == PU_GATE)
        m->gate_seen = 1;
    return m->r[reg];
}

static void uw(void *v, unsigned reg, uintptr_t value)
{
    struct du_registers *m = v;
    if (reg == PU_RESET && !m->gate_seen)
        m->bad_sequence = 1;
    m->r[reg] = (uint32_t)value;
    if (reg == PU_DR)
    {
        m->writes++;
        m->r[PU_SR] &= ~(PU_TXE | PU_TC);
    }
}

static uint32_t sr(void *v, unsigned reg)
{
    struct du_registers *m = v;
    uint32_t r = m->r[reg];
    if (reg == PS_DR)
    {
        m->reads++;
        m->r[PS_SR] &= ~PS_RXNE;
    }
    return r;
}

static void sw(void *v, unsigned reg, uintptr_t value)
{
    struct du_registers *m = v;
    m->r[reg] = (uint32_t)value;
    if (reg == PS_DR)
    {
        m->writes++;
        m->r[PS_SR] = PS_BSY;
    }
}

static int serial(void)
{
    struct du_registers m = {0};
    m.r[PU_SOURCE] = 1;
    m.r[PU_SR] = PU_TXE | PU_TC;
    struct pu_io io = {&m, ur, uw};
    struct pu_clock c = {16000000, 1, 1, 1, 1, 0};
    struct pu_driver u = {0};
    uint8_t tx[2] = {0x35, 0xa7};
    CHECK(pu_open(&u, io, &c, 115200, 0, 2) == PU_OK && !m.bad_sequence);
    CHECK(pu_submit(&u, io, 1, 1, PU_POLL, tx, 2, 0, 100) == PU_OK);
    CHECK(pu_step(&u, io, 1, 1) == PU_BUSY && m.writes == 1 && u.line_owned);
    m.r[PU_SR] = PU_TXE;
    CHECK(pu_step(&u, io, 1, 2) == PU_BUSY && m.writes == 2 &&
          !u.memory_owned && u.line_owned);
    m.r[PU_SR] = PU_TXE;
    CHECK(pu_step(&u, io, 1, 3) == PU_BUSY && u.line_owned);
    m.r[PU_SR] |= PU_TC;
    CHECK(pu_step(&u, io, 1, 4) == PU_OK && !u.line_owned);
    CHECK(pu_submit(&u, io, 2, 2, PU_POLL, tx, 2, 5, 100) == PU_STALE);
    struct du_registers sm = {0};
    sm.r[PS_SR] = PS_TXE;
    struct ps_io si = {&sm, sr, sw};
    struct ps_spi s = {0};
    uint8_t rx[2] = {0};
    CHECK(ps_open(&s, si, &c, 0, 3) == PS_OK);
    CHECK(ps_begin(&s, si, 1, 1, tx, rx, 2, 0, 0, 100, 0, 1) == PS_OK);
    CHECK(ps_step(&s, si, 1, 0) == PS_BUSY && sm.writes == 1);
    sm.r[PS_SR] = PS_TXE | PS_RXNE;
    sm.r[PS_DR] = 0x81;
    CHECK(ps_step(&s, si, 1, 1) == PS_BUSY && sm.reads == 1 && rx[0] == 0x81 &&
          sm.writes == 2);
    sm.r[PS_SR] = PS_TXE | PS_RXNE | PS_BSY;
    sm.r[PS_DR] = 0x42;
    CHECK(ps_step(&s, si, 1, 2) == PS_BUSY && sm.reads == 2 && rx[1] == 0x42 &&
          s.cs_owned);
    sm.r[PS_SR] = PS_TXE;
    CHECK(ps_step(&s, si, 1, 3) == PS_BUSY && s.cs_owned);
    CHECK(ps_step(&s, si, 1, 4) == PS_OK && !s.cs_owned);
    return 0;
}

struct du_memory
{
    struct pd_snapshot x;
    unsigned stop, enable, clean, invalidate, release, acquire;
    uint32_t source, destination;
    unsigned cached;
    unsigned char cpu_source[32], memory_source[32], cpu_dest[32],
        memory_dest[32];
    unsigned dest_valid, dest_dirty;
};

static int configure(void *v, const struct pd_extent *e, uint32_t token)
{
    struct du_memory *m = v;
    m->source = e->source;
    m->destination = e->destination;
    m->x = (struct pd_snapshot){token, 0, e->bytes / e->width, 0, 1};
    return 1;
}

static void enable(void *v)
{
    struct du_memory *m = v;
    m->enable++;
    m->x.enabled = 1;
}

static void stop(void *v)
{
    struct du_memory *m = v;
    m->stop++;
}

static struct pd_snapshot snapshot(void *v)
{
    return ((struct du_memory *)v)->x;
}

static void visibility(void *v, unsigned op, uint32_t address, uint32_t bytes)
{
    struct du_memory *m = v;
    (void)bytes;
    if (op == PD_RELEASE)
        m->release++;
    if (op == PD_ACQUIRE)
        m->acquire++;
    if (op == PD_CLEAN)
    {
        m->clean++;
        if (address == m->source)
            memcpy(m->memory_source, m->cpu_source, 32);
        else if (address == m->destination && m->dest_dirty)
        {
            memcpy(m->memory_dest, m->cpu_dest, 32);
            m->dest_dirty = 0;
        }
    }
    if (op == PD_INVALIDATE)
    {
        m->invalidate++;
        m->dest_valid = 0;
        m->dest_dirty = 0;
    }
}

static struct pd_io dma_io(struct du_memory *m)
{
    return (struct pd_io){m, configure, enable, stop, snapshot, visibility};
}

static int dma(unsigned cached)
{
    struct du_memory m = {0};
    m.cached = cached;
    for (unsigned i = 0; i < 32; i++)
    {
        m.cpu_source[i] = (unsigned char)(i + 1);
        m.cpu_dest[i] = 99;
        m.memory_dest[i] = 7;
    }
    m.dest_valid = m.dest_dirty = 1;
    uint32_t base = cached ? 0x30040040u : 0x20000000u;
    struct pd_extent e = {base, base + 64, 32, 32, 32, 1};
    struct pd_transfer s = {0};
    struct pd_io io = dma_io(&m);
    struct pd_result r = {99, 99, 99, 99};
    CHECK(pd_submit(&s, io, cached ? PD_H745 : PD_F439, cached, &e, 1, 1) ==
              PD_OK &&
          s.phase == 1);
    CHECK(m.release == 1 && m.clean == (cached ? 2u : 0u) &&
          m.invalidate == cached);
    if (!cached)
    {
        memcpy(m.memory_source, m.cpu_source, 32);
        memcpy(m.memory_dest, m.cpu_dest, 32);
    }
    /* A modeled speculative clean refill is allowed while CPU payload access is
       forbidden. Post-invalidation must discard it before owner reads. */
    memcpy(m.cpu_dest, m.memory_dest, 32);
    m.dest_valid = 1;
    memcpy(m.memory_dest, m.memory_source, 12);
    m.x.remaining = 20;
    CHECK(pd_cancel(&s, io, 1) == PD_BUSY && m.stop == 1);
    CHECK(pd_finalize(&s, io, 1, &r) == PD_BUSY && r.token == 99);
    m.x.enabled = 0;
    m.x.fifo_empty = 0;
    CHECK(pd_poll(&s, io, 1) == PD_BUSY && s.phase == 2);
    m.x.fifo_empty = 1;
    CHECK(pd_poll(&s, io, 1) == PD_BUSY);
    m.x.flags = PD_TC;
    CHECK(pd_poll(&s, io, 1) == PD_OK);
    CHECK(pd_finalize(&s, io, 1, &r) == PD_OK && r.status == PD_CANCELLED &&
          r.complete_bytes == 12 && r.possibly_written_bytes == 12);
    if (!cached || !m.dest_valid)
        memcpy(m.cpu_dest, m.memory_dest, 32);
    CHECK(m.cpu_dest[0] == 1 && m.cpu_dest[11] == 12 && m.cpu_dest[12] == 99);
    CHECK(pd_finalize(&s, io, 1, &r) == PD_STALE);
    CHECK(pd_submit(&s, io, cached ? PD_H745 : PD_F439, cached, &e, 1, 2) ==
          PD_OK);
    m.x = (struct pd_snapshot){1, 0, 0, PD_TC, 1};
    CHECK(pd_poll(&s, io, 2) == PD_BUSY && s.phase == 1);
    CHECK(pd_poll(&s, io, 1) == PD_STALE);
    m.x.token = 2;
    CHECK(pd_poll(&s, io, 2) == PD_OK);
    CHECK(pd_cancel(&s, io, 2) == PD_BUSY && s.phase == 3);
    CHECK(pd_finalize(&s, io, 2, &r) == PD_OK && r.status == PD_OK &&
          r.complete_bytes == 32);
    CHECK(pd_finalize(&s, io, 2, &r) == PD_STALE);
    e.source = 0x10000000;
    uint32_t total = 123;
    CHECK(pd_validate(PD_F439, 0, &e, 1, &total) == PD_INVALID && total == 123);
    return 0;
}

int main(void)
{
    if (serial() || dma(0) || dma(1))
        return 1;
    puts("DP_PASS:actual-provider-composition");
    return 0;
}
