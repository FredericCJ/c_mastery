#include "spi.h"
#ifdef PS_BIG_ENDIAN
int pm_printf(const char *, ...);
int pm_puts(const char *);
#define printf pm_printf
#define puts pm_puts
#else
#include <stdio.h>
#endif
#define CHECK(x)                                         \
    do                                                   \
    {                                                    \
        if (!(x))                                        \
        {                                                \
            printf("PS_CHECK:%u\n", (unsigned)__LINE__); \
            return 1;                                    \
        }                                                \
    } while (0)

struct dg_model
{
    uint32_t r[19];
    unsigned reads, writes, dr_reads, dr_writes, stop_requests, instant_dma;
    uint8_t sent[16];
    unsigned order[128], n;
};

static uint32_t read_reg(void *v, unsigned r)
{
    struct dg_model *m = v;
    ++m->reads;
    if (r == PS_DR)
    {
        ++m->dr_reads;
        m->r[PS_SR] &= ~PS_RXNE;
    }
    return m->r[r];
}

static void write_reg(void *v, unsigned r, uintptr_t x)
{
    struct dg_model *m = v;
    ++m->writes;
    m->order[m->n++] = r;
    if (r == PS_DR)
    {
        m->sent[m->dr_writes++] = (uint8_t)x;
        m->r[PS_SR] = PS_BSY;
    }
    if ((r == PS_TX_EN || r == PS_RX_EN) && !x)
    {
        ++m->stop_requests;
        return;
    }
    m->r[r] = (uint32_t)x;
    if (r == PS_RX_START)
    {
        m->r[PS_RX_EN] = 1;
    }
    if (r == PS_TX_START)
    {
        m->r[PS_TX_EN] = 1;
    }
    if (r == PS_TX_START && m->instant_dma)
    {
        m->r[PS_TX_EN] = m->r[PS_RX_EN] = m->r[PS_TX_COUNT] =
            m->r[PS_RX_COUNT] = 0;
        m->r[PS_TX_FLAGS] = m->r[PS_RX_FLAGS] = 1;
        m->r[PS_SR] = PS_TXE | PS_BSY;
    }
    if (r == PS_RESET)
    {
        m->r[PS_SR] = PS_TXE;
    }
}

int main(void)
{
    struct pu_clock c = {16000000, 1, 1, 1, 1, 0};
    uint8_t tx[3] = {0x9f, 0, 0}, rx[3] = {0xcc, 0xcc, 0xcc}, cmd[3], data[3];
    uint16_t value = 777;
    struct ps_spi s = {0};
    struct dg_model m = {0};
    struct ps_io io = {&m, read_reg, write_reg};
    for (unsigned mode = 0; mode < 4; mode++)
    {
        CHECK(ps_open(&s, io, &c, mode, 3) == PS_OK);
        CHECK(m.r[PS_CONFIG] == (0x31cu | mode));
    }
    CHECK(ps_begin(&s, io, 1, 1, tx, rx, 3, 0, UINT32_MAX - 3u, 100, 2, 3) ==
              PS_OK &&
          !m.r[PS_CS]);
    CHECK(ps_step(&s, io, 1, UINT32_MAX - 2u) == PS_BUSY && !m.dr_writes);
    m.r[PS_SR] = PS_TXE;
    CHECK(ps_step(&s, io, 1, UINT32_MAX - 1u) == PS_BUSY && m.dr_writes == 1 &&
          s.sent == 1);
    CHECK(ps_step(&s, io, 1, UINT32_MAX) == PS_BUSY && m.dr_writes == 1);
    for (unsigned i = 0; i < 3; i++)
    {
        m.r[PS_SR] = PS_TXE | PS_RXNE | PS_BSY;
        m.r[PS_DR] = (uint8_t[]){0, 239, 64}[i];
        CHECK(ps_step(&s, io, 1, i) == PS_BUSY);
    }
    CHECK(s.received == 3 && m.dr_reads == 3 && m.dr_writes == 3 &&
          !s.tx_owned && !s.rx_owned && s.cs_owned && s.phase == 2);
    CHECK(rx[0] == 0 && rx[1] == 239 && rx[2] == 64 && m.sent[0] == 0x9f);
    m.r[PS_SR] = PS_TXE;
    CHECK(ps_step(&s, io, 1, 3) == PS_BUSY && s.phase == 3 && !m.r[PS_CS]);
    CHECK(ps_step(&s, io, 1, 5) == PS_BUSY);
    CHECK(ps_step(&s, io, 1, 6) == PS_OK && m.r[PS_CS]);
    CHECK(ps_step(&s, io, 1, 7) == PS_STALE);
    CHECK(ps_begin(&s, io, 1, 1, tx, rx, 3, 0, 7, 20, 0, 0) == PS_STALE);
    CHECK(ps_begin(&s, io, 2, 2, tx, rx, 3, 0, 7, 20, 0, 0) == PS_STALE);
    CHECK(ps_begin(&s, io, 1, 2, tx, rx, 3, 1, 7, 20, 0, 2) == PS_OK);
    CHECK(ps_step(&s, io, 2, 7) == PS_BUSY && m.r[PS_RX_EN] && m.r[PS_TX_EN]);
    {
        unsigned rx_start = 0, tx_start = 0;
        for (unsigned i = 0; i < m.n; i++)
        {
            if (m.order[i] == PS_RX_START)
            {
                rx_start = i;
            }
            if (m.order[i] == PS_TX_START)
            {
                tx_start = i;
            }
        }
        CHECK(rx_start < tx_start);
    }
    CHECK(ps_cancel(&s, io, 2) == PS_BUSY && s.tx_owned && s.rx_owned &&
          s.cs_owned && m.stop_requests == 2);
    m.r[PS_TX_EN] = 0;
    CHECK(ps_step(&s, io, 2, 8) == PS_BUSY && !s.tx_owned && s.rx_owned &&
          s.cs_owned);
    m.r[PS_RX_EN] = 0;
    m.r[PS_SR] = PS_BSY;
    CHECK(ps_step(&s, io, 2, 9) == PS_BUSY && !s.rx_owned && s.cs_owned);
    m.r[PS_SR] = PS_TXE;
    CHECK(ps_step(&s, io, 2, 10) == PS_BUSY);
    CHECK(ps_step(&s, io, 2, 12) == PS_CANCELLED && !s.cs_owned);
    CHECK(ps_begin(&s, io, 1, 3, tx, rx, 3, 0, 13, 5, 0, 3) == PS_OK);
    m.r[PS_SR] = 0;
    CHECK(ps_step(&s, io, 3, 18) == PS_BUSY && s.result == PS_TIMEOUT &&
          s.cs_owned);
    m.r[PS_SR] = PS_TXE;
    CHECK(ps_step(&s, io, 3, 19) == PS_BUSY && s.phase == 3);
    CHECK(ps_step(&s, io, 3, 20) == PS_BUSY && s.phase == 3);
    CHECK(ps_step(&s, io, 3, 22) == PS_TIMEOUT);
    CHECK(ps_begin(&s, io, 1, 4, tx, rx, 3, 0, 23, 10, 0, 2) == PS_OK);
    m.r[PS_SR] = PS_MODF;
    CHECK(ps_step(&s, io, 4, 24) == PS_BUSY && s.result == PS_IO && s.cs_owned);
    CHECK(ps_abort_reset(&s, io, 4, 24) == PS_BUSY && !s.ready && s.phase == 3);
    CHECK(ps_step(&s, io, 4, 26) == PS_IO && !s.cs_owned);
    CHECK(ps_command(0, 31, cmd) == PS_OK && cmd[0] == 0x9f && cmd[1] == 0 &&
          cmd[2] == 0);
    data[0] = 0xa5;
    data[1] = 0xef;
    data[2] = 0x40;
    CHECK(ps_decode(0, data, &value) == PS_OK && value == 0xef40);
    data[0] = 0;
    CHECK(ps_decode(0, data, &value) == PS_PROTOCOL && value == 0xef40);
    CHECK(ps_command(1, 1, cmd) == PS_OK && cmd[0] == 0xc1);
    data[0] = 0xa6;
    data[1] = 0x40;
    data[2] = 0xef;
    CHECK(ps_decode(1, data, &value) == PS_OK && value == 0xef40);

    /* Each wait has a distinct prefix, without inventing successful bytes. */
    for (unsigned cut = 0; cut < 3; cut++)
    {
        struct ps_spi q = {0};
        struct dg_model n = {0};
        struct ps_io qi = {&n, read_reg, write_reg};
        uint8_t tr[3] = {99, 99, 99};
        CHECK(ps_open(&q, qi, &c, 0, 3) == PS_OK);
        CHECK(ps_begin(&q, qi, 1, 1, tx, tr, 3, 0, 0, 8, 0, 1) == PS_OK);
        if (cut)
        {
            n.r[PS_SR] = PS_TXE;
            CHECK(ps_step(&q, qi, 1, 0) == PS_BUSY);
        }
        if (cut == 2)
        {
            for (unsigned i = 0; i < 3; i++)
            {
                n.r[PS_SR] = PS_TXE | PS_RXNE | PS_BSY;
                n.r[PS_DR] = 10 + i;
                CHECK(ps_step(&q, qi, 1, 1 + i) == PS_BUSY);
            }
        }
        CHECK(ps_step(&q, qi, 1, 8) == PS_BUSY && q.result == PS_TIMEOUT);
        CHECK(q.sent == (cut == 0   ? 0
                         : cut == 1 ? 1
                                    : 3) &&
              q.received == (cut == 2 ? 3 : 0));
        CHECK(q.cs_owned && tr[0] == (cut == 2 ? 10 : 99));
        n.r[PS_SR] = PS_TXE;
        CHECK(ps_step(&q, qi, 1, 9) == PS_BUSY);
        CHECK(ps_step(&q, qi, 1, 10) == PS_TIMEOUT);
    }
    {
        struct ps_spi q = {0};
        struct dg_model n = {0};
        struct ps_io qi = {&n, read_reg, write_reg};
        uint16_t words[3], back[3] = {0x1100, 0x22ef, 0x3340};
        CHECK(ps_pack(tx, words, 3) == PS_OK && words[0] == 0x9f &&
              words[1] == 0);
        CHECK(ps_unpack(back, rx, 3) == PS_OK && rx[0] == 0 && rx[1] == 239 &&
              rx[2] == 64);
        CHECK(ps_pack(tx, words, 0) == PS_INVALID &&
              ps_unpack(back, rx, 17) == PS_INVALID);
        CHECK(ps_open(&q, qi, &c, 0, 3) == PS_OK);
        CHECK(ps_begin(&q, qi, 1, 1, tx, rx, 3, 1, 0, 10, 0, 2) == PS_OK);
        CHECK(ps_step(&q, qi, 1, 0) == PS_BUSY);
        n.r[PS_SR] = PS_RXNE | PS_BSY;
        CHECK(ps_cancel(&q, qi, 1) == PS_BUSY);
        CHECK(ps_step(&q, qi, 1, 1) == PS_BUSY && !n.dr_reads);
        CHECK(ps_abort_reset(&q, qi, 1, 2) == PS_BUSY && q.ready && q.cs_owned);
        n.r[PS_RX_EN] = n.r[PS_TX_EN] = 0;
        CHECK(ps_abort_reset(&q, qi, 1, 5) == PS_BUSY && !q.ready &&
              q.due == 7);
        CHECK(ps_step(&q, qi, 1, 6) == PS_BUSY);
        CHECK(ps_step(&q, qi, 1, 7) == PS_CANCELLED);
    }

    {
        struct ps_spi q = {0};
        struct dg_model n = {0};
        struct ps_io qi = {&n, read_reg, write_reg};
        CHECK(ps_open(&q, qi, &c, 0, 3) == PS_OK);
        CHECK(ps_begin(&q, qi, 1, 1, tx, rx, 3, 1, 0, 10, 0, 0) == PS_OK);
        n.instant_dma = 1;
        n.r[PS_SR] = PS_TXE;
        CHECK(ps_step(&q, qi, 1, 0) == PS_BUSY && !q.tx_owned && !q.rx_owned &&
              q.cs_owned);
        n.r[PS_SR] = PS_TXE;
        CHECK(ps_step(&q, qi, 1, 1) == PS_OK);
    }
    puts("PS_PASS");
    return 0;
}
