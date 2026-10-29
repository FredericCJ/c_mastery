#include "uart.h"
#ifdef PU_BIG_ENDIAN
int pm_printf(const char *, ...);
int pm_puts(const char *);
#define printf pm_printf
#define puts pm_puts
#else
#include <stdio.h>
#endif
#define CHECK(x)                                      \
    do                                                \
    {                                                 \
        if (!(x))                                     \
        {                                             \
            printf("PU_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                 \
        }                                             \
    } while (0)

struct dg_device
{
    uint32_t source_after, source_reads, sr, cr1, cr3, brr, dma_busy, dma_count,
        dma_flags;
    unsigned gate, synced, reset, bad, dr_reads, writes, dma_stop, log_n,
        log[128];
    uint8_t incoming, wire[8];
    uintptr_t address;
};

static uint32_t rd(void *ctx, unsigned r)
{
    struct dg_device *m = ctx;
    if (m->log_n < 128)
    {
        m->log[m->log_n++] = r;
    }
    if (r == PU_SOURCE)
    {
        return ++m->source_reads >= m->source_after && m->source_after;
    }
    if (r == PU_GATE)
    {
        m->synced = m->gate;
        return m->gate;
    }
    if (r == PU_DMA_GATE)
    {
        return 1;
    }
    if (r == PU_DMA_BUSY)
    {
        return m->dma_busy;
    }
    if (r == PU_DMA_COUNT)
    {
        return m->dma_count;
    }
    if (r == PU_DMA_FLAGS)
    {
        return m->dma_flags;
    }
    if (r == PU_CR1)
    {
        return m->cr1;
    }
    if (r == PU_SR)
    {
        return m->sr;
    }
    if (r == PU_DR)
    {
        m->dr_reads++;
        m->sr &= ~47u;
        return m->incoming;
    }
    return 0;
}

static void wr(void *ctx, unsigned r, uintptr_t v)
{
    struct dg_device *m = ctx;
    if (m->log_n < 128)
    {
        m->log[m->log_n++] = r + 32;
    }
    if (r == PU_GATE)
    {
        m->gate = 1;
        m->synced = 0;
        return;
    }
    if (r == PU_RESET)
    {
        if (!m->synced)
        {
            m->bad++;
        }
        m->reset = (unsigned)v;
        return;
    }
    if (r == PU_DMA_GATE)
    {
        return;
    }
    if (!m->synced || m->reset)
    {
        m->bad++;
    }
    if (r == PU_CR1)
    {
        m->cr1 = (uint32_t)v;
    }
    if (r == PU_CR3)
    {
        m->cr3 = (uint32_t)v;
    }
    if (r == PU_BRR)
    {
        m->brr = (uint32_t)v;
    }
    if (r == PU_SR)
    {
        m->sr &= (uint32_t)v | ~0x360u;
    }
    if (r == PU_DR)
    {
        if (!(m->sr & PU_TXE) || m->writes == 8)
        {
            m->bad++;
        }
        else
        {
            m->wire[m->writes++] = (uint8_t)v;
        }
        m->sr &= ~PU_TC;
    }
    if (r == PU_DMA_ADDRESS)
    {
        m->address = v;
    }
    if (r == PU_DMA_COUNT)
    {
        m->dma_count = (uint32_t)v;
    }
    if (r == PU_DMA_CLEAR)
    {
        m->dma_flags = 0;
    }
    if (r == PU_DMA_CONTROL)
    {
        if (v)
        {
            m->dma_busy = 1;
        }
        else
        {
            m->dma_stop++;
        }
    }
}

int main(void)
{
    struct pu_clock c = {168000000, 1, 1, 4, 2, 0};
    struct pu_rates rates = {0};
    CHECK(pu_rates(&c, &rates) == PU_OK && rates.apb1 == 42000000 &&
          rates.timer1 == 84000000 && rates.timer2 == 168000000);
    c.timpre = 1;
    CHECK(pu_rates(&c, &rates) == PU_OK && rates.timer1 == 168000000);
    c.apb1 = 8;
    CHECK(pu_rates(&c, &rates) == PU_OK && rates.apb1 == 21000000 &&
          rates.timer1 == 84000000);
    c.apb1 = 0;
    CHECK(pu_rates(&c, &rates) == PU_INVALID);
    c = (struct pu_clock){16000000, 1, 1, 1, 1, 0};
    struct dg_device m = {0};
    struct pu_io io = {&m, rd, wr};
    struct pu_driver d = {0};
    uint8_t data[3] = {0, 65, 255};
    CHECK(pu_open(&d, io, &c, 115200, 0, 2) == PU_TIMEOUT && !d.ready &&
          m.writes == 0);
    m.source_reads = 0;
    m.source_after = 2;
    CHECK(pu_open(&d, io, &c, 115200, 0, 2) == PU_OK && m.brr == 139 && !m.bad);
    CHECK(pu_open(&d, io, &c, 115200, 1, 2) == PU_OK && m.brr == 275 &&
          !(m.brr & 8));
    CHECK(pu_submit(&d, io, 2, 1, PU_POLL, data, 3, 0, 20) == PU_STALE);
    m.sr = PU_TXE | PU_TC;
    CHECK(pu_submit(&d, io, 1, 1, PU_IRQ, data, 3, 0, 20) == PU_OK);
    CHECK(pu_step(&d, io, 1, 1) == PU_BUSY && d.memory_owned && d.line_owned &&
          m.writes == 1);
    CHECK(pu_step(&d, io, 1, 2) == PU_BUSY && m.writes == 2);
    CHECK(pu_step(&d, io, 1, 3) == PU_BUSY && !d.memory_owned && d.line_owned &&
          m.writes == 3 && m.wire[0] == 0 && m.wire[2] == 255);
    CHECK(pu_open(&d, io, &c, 115200, 0, 2) == PU_BUSY);
    m.sr = PU_TXE | PU_TC;
    CHECK(pu_step(&d, io, 1, 20) == PU_OK && !d.active);
    CHECK(pu_step(&d, io, 1, 20) == PU_STALE);
    struct pu_rx q = {0};
    struct pu_byte b = {99, 99};
    unsigned ready = 99;
    CHECK(pu_receive(&q, PU_RXNE, 0) == PU_OK &&
          pu_receive(&q, PU_RXNE | PU_ORE, 65) == PU_OK &&
          pu_receive(&q, PU_RXNE, 255) == PU_FULL && q.dropped == 1);
    CHECK(pu_take(&q, &b) == PU_OK && b.value == 0 && b.error == 0);
    CHECK(pu_take(&q, &b) == PU_OK && b.value == 65 && b.error == PU_ORE);
    b.value = 99;
    CHECK(pu_take(&q, &b) == PU_BUSY && b.value == 99 && q.errors == 1);
    q.backpressure = 1;
    q.reserve = 1;
    CHECK(pu_peer_ready(&q, &ready) == PU_OK && ready);
    CHECK(pu_receive(&q, PU_RXNE, 0) == PU_OK);
    CHECK(pu_peer_ready(&q, &ready) == PU_OK && !ready);
    CHECK(pu_receive(&q, PU_RXNE, 65) == PU_OK &&
          pu_receive(&q, PU_RXNE, 255) == PU_FULL && q.dropped == 2);
    CHECK(pu_take(&q, &b) == PU_OK && pu_take(&q, &b) == PU_OK);
    m.sr = PU_PE;
    CHECK(pu_rx_poll(&q, io, 2) == PU_TIMEOUT && m.dr_reads == 0);
    m.sr = PU_RXNE | PU_PE | PU_NF | PU_FE;
    m.incoming = 255;
    CHECK(pu_rx_poll(&q, io, 2) == PU_OK && m.dr_reads == 1 && !(m.sr & 47));
    CHECK(pu_take(&q, &b) == PU_OK && b.value == 255 && b.error == 7);
    m.sr = PU_TXE | PU_TC;
    CHECK(pu_submit(&d, io, 1, 2, PU_DMA, data, 3, UINT32_MAX - 5, 10) ==
              PU_OK &&
          !(m.sr & PU_TC) && m.dma_busy && m.address == (uintptr_t)data);
    CHECK(pu_cancel(&d, io, 2) == PU_OK && d.memory_owned && m.dma_stop == 1);
    m.sr = PU_TXE;
    CHECK(pu_step(&d, io, 2, UINT32_MAX - 4) == PU_BUSY && d.memory_owned);
    m.dma_busy = 0;
    m.dma_count = 0;
    CHECK(pu_step(&d, io, 2, 0) == PU_BUSY && !d.memory_owned && d.line_owned);
    m.sr |= PU_TC;
    CHECK(pu_step(&d, io, 2, 1) == PU_CANCELLED && !d.line_owned);
    CHECK(pu_submit(&d, io, 1, 3, PU_DMA, data, 3, 10, 10) == PU_OK);
    m.dma_count = 0;
    m.dma_busy = 0;
    m.dma_flags = 1;
    CHECK(pu_step(&d, io, 3, 11) == PU_BUSY && !d.memory_owned && d.line_owned);
    CHECK(pu_cancel(&d, io, 3) == PU_OK);
    m.sr |= PU_TC;
    CHECK(pu_step(&d, io, 3, 12) == PU_CANCELLED);
    CHECK(pu_submit(&d, io, 1, 4, PU_DMA, data, 3, 20, 10) == PU_OK);
    CHECK(pu_reset_notice(&d, io) == PU_OK && !d.ready && d.memory_owned);
    m.dma_busy = 0;
    m.sr |= PU_TC;
    CHECK(pu_step(&d, io, 4, 21) == PU_IO);
    c.generation = 2;
    CHECK(pu_open(&d, io, &c, 115200, 0, 2) == PU_OK);
    CHECK(pu_submit(&d, io, 1, 5, PU_POLL, data, 1, 0, 1) == PU_STALE);
    CHECK(pu_submit(&d, io, 2, 5, PU_POLL, data, 1, 0, 1) == PU_OK);
    m.sr = 0;
    CHECK(pu_poll(&d, io, 5, 0, 2) == PU_TIMEOUT && d.active &&
          !d.memory_owned && d.line_owned);
    m.sr = PU_TC;
    CHECK(pu_step(&d, io, 5, 1) == PU_TIMEOUT);
    CHECK(pu_submit(&d, io, 2, 6, PU_IRQ, data, 1, 0, 1) == PU_OK);
    m.sr = 0;
    CHECK(pu_step(&d, io, 6, 1) == PU_BUSY && d.result == PU_TIMEOUT &&
          d.cancel);
    m.sr = PU_TC;
    CHECK(pu_step(&d, io, 6, 2) == PU_TIMEOUT);
    CHECK(pu_deadline_fit(16000000, 139, 3, 260, &ready) == PU_OK && !ready);
    CHECK(pu_deadline_fit(16000000, 139, 3, 261, &ready) == PU_OK && ready);
    CHECK(pu_deadline_fit(8000000, 139, 3, 261, &ready) == PU_OK && !ready);
    CHECK(pu_deadline_fit(8000000, 69, 3, 259, &ready) == PU_OK && ready);
    CHECK(pu_deadline_fit(8000000, 69, 3, 220, &ready) == PU_OK && !ready);
    c.sys_hz = 8000000;
    c.generation = 3;
    CHECK(pu_open(&d, io, &c, 115200, 0, 2) == PU_OK && m.brr == 69);
    CHECK(pu_submit(&d, io, 2, 7, PU_IRQ, data, 1, 0, 1) == PU_STALE);
    d.token = UINT32_MAX;
    CHECK(pu_submit(&d, io, 3, 1, PU_IRQ, data, 1, 0, 1) == PU_STALE);
    q.errors = UINT32_MAX;
    CHECK(pu_receive(&q, PU_RXNE | PU_ORE, 1) == PU_FULL && q.count == 0);
    CHECK(pu_receive(&q, PU_RXNE, 1) == PU_OK &&
          pu_receive(&q, PU_RXNE, 2) == PU_OK);
    q.dropped = UINT32_MAX;
    CHECK(pu_receive(&q, PU_RXNE, 3) == PU_FULL && q.dropped == UINT32_MAX);
    CHECK(!m.bad);
    puts("PU_PASS");
    return 0;
}
