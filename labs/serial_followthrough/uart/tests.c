#include "uart_depth.h"
void ua_note(const char *);
void ua_values(const char *, unsigned, unsigned, unsigned, unsigned);
#define CHECK(x)                       \
    do                                 \
    {                                  \
        if (!(x))                      \
        {                              \
            ua_note("UA_ASSERT: " #x); \
            return 41;                 \
        }                              \
    } while (0)

struct mc_model
{
    uint32_t r[16];
    unsigned reads, writes, previous, ordered;
};

static uint32_t read_io(void *p, unsigned k)
{
    struct mc_model *m = p;
    if (k == PU_SOURCE)
        return 1;
    if (k == PU_DR)
    {
        m->reads++;
        if (m->previous != PU_SR)
            m->ordered = 1;
        m->r[PU_SR] &= ~47u;
    }
    m->previous = k;
    return m->r[k];
}

static void write_io(void *p, unsigned k, uintptr_t v)
{
    struct mc_model *m = p;
    m->r[k] = (uint32_t)v;
    if (k == PU_DR)
    {
        m->writes++;
        m->r[PU_SR] &= ~(PU_TXE | PU_TC);
    }
}

static struct pu_io io(struct mc_model *m)
{
    struct pu_io r = {m, read_io, write_io};
    return r;
}

static int open_uart(struct ua_tx *s, struct mc_model *m)
{
    struct pu_clock c = {42000000, 1, 1, 1, 1, 0};
    return pu_open(&s->byte, io(m), &c, 115200, 0, 1);
}

static int frames(void)
{
    struct ua_format f;
    unsigned total = 0;
    uint8_t value = 99;
    for (unsigned width = 7; width <= 8; width++)
        for (unsigned parity = 0; parity < 3; parity++)
            for (unsigned stop = 1; stop <= 2; stop++)
                for (unsigned v = 0; v < (1u << width); v++)
                {
                    if (width == 7 && !parity)
                    {
                        CHECK(ua_format(42000000, 115200, 0, width, parity,
                                        stop, v, &f) == PU_INVALID);
                        continue;
                    }
                    CHECK(ua_format(42000000, 115200, 0, width, parity, stop, v,
                                    &f) == PU_OK);
                    CHECK(((f.cr1 >> 12) & 1u) == (width == 8 && parity != 0));
                    CHECK(((f.cr1 >> 10) & 1u) == (parity != 0) &&
                          ((f.cr1 >> 9) & 1u) == (parity == 2));
                    CHECK(f.payload_mask == ((1u << width) - 1) &&
                          f.cr2 == (stop == 2 ? 0x2000u : 0));
                    CHECK(sd_uart_decode(f.wire.bits, f.wire.length, width,
                                         parity, stop, &value) == SD_OK &&
                          value == v);
                    unsigned at = width + (parity != 0) + 1;
                    f.wire.bits[at] ^= 1;
                    CHECK(sd_uart_decode(f.wire.bits, f.wire.length, width,
                                         parity, stop, &value) == SD_IO &&
                          value == v);
                    total++;
                }
    CHECK(ua_format(42000000, 115200, 0, 8, 1, 1, 1, &f) == PU_OK);
    f.wire.bits[9] ^= 1;
    value = 19;
    CHECK(sd_uart_decode(f.wire.bits, f.wire.length, 8, 1, 1, &value) ==
              SD_IO &&
          value == 19);
    CHECK(ua_format(42000000, 115200, 0, 8, 1, 1, 1, &f) == PU_OK);
    struct ua_tx s = {0};
    struct mc_model m = {0};
    CHECK(open_uart(&s, &m) == PU_OK);
    CHECK(ua_apply(&s.byte, io(&m), &f) == PU_BUSY);
    m.r[PU_CR1] &= ~12u;
    CHECK(ua_apply(&s.byte, io(&m), &f) == PU_OK && m.r[PU_CR1] == 0x340c &&
          m.r[PU_BRR] == 365);
    ua_values("frames", total, m.r[PU_CR1], 365, 19);
    return 0;
}

static int baud(void)
{
    const uint32_t clocks[] = {42000000, 21000000, 16000000};
    for (unsigned i = 0; i < 3; i++)
        for (unsigned over = 0; over < 2; over++)
        {
            struct ua_format f;
            CHECK(ua_format(clocks[i], 115200, over, 8, 0, 1, 0xa5, &f) ==
                  PU_OK);
            ua_values("baud", clocks[i], over, f.wire.divisor, f.wire.brr);
        }
    struct ua_format f;
    CHECK(ua_format(42000000, 3000000, 0, 8, 0, 1, 0, &f) == PU_INVALID);
    CHECK(ua_format(42000000, 3000000, 1, 8, 0, 1, 0, &f) == PU_OK &&
          f.wire.divisor == 14 && f.wire.brr == 22);
    struct ua_tx s = {0};
    struct mc_model m = {0};
    uint8_t b = 0;
    CHECK(open_uart(&s, &m) == PU_OK);
    CHECK(ua_begin(&s, io(&m), 2, 1, &b, 1, 0, 100) == PU_STALE &&
          m.writes == 0);
    ua_values("baud_limit", 14, 22, PU_STALE, m.writes);
    return 0;
}

static int transmission(void)
{
    uint8_t bytes[16];
    for (unsigned i = 0; i < 16; i++)
        bytes[i] = (uint8_t)(i ^ 0xa5);
    for (unsigned n = 1; n <= 16; n++)
    {
        struct ua_tx s = {0};
        struct mc_model m = {0};
        CHECK(open_uart(&s, &m) == PU_OK);
        CHECK(ua_begin(&s, io(&m), 1, 1, bytes, n, 0, 100) == PU_OK);
        unsigned now = 0;
        while (s.active)
        {
            if (s.byte.position < s.byte.count)
            {
                m.r[PU_SR] = PU_TXE | PU_TC;
                unsigned index = s.offset + s.byte.position;
                CHECK(ua_step(&s, io(&m), 1, now++) == PU_BUSY && s.active &&
                      s.byte.line_owned);
                CHECK(m.r[PU_DR] == bytes[index]);
            }
            else
            {
                m.r[PU_SR] = PU_TXE;
                CHECK(ua_step(&s, io(&m), 1, now++) == PU_BUSY && s.active);
                m.r[PU_SR] = PU_TXE | PU_TC;
                int r = ua_step(&s, io(&m), 1, now++);
                CHECK(r == (s.active ? PU_BUSY : PU_OK));
            }
        }
        CHECK(m.writes == n && s.offset == n && s.byte.token == (n + 7) / 8);
        ua_values("tx", n, m.writes, s.offset, s.byte.token);
    }
    return 0;
}

static int receive(void)
{
    struct mc_model m = {0};
    struct pu_byte slots[32] = {{0}}, b = {99, 77};
    struct ua_rx s = {0};
    s.slots = slots;
    s.capacity = 32;
    CHECK(ua_rx(&s, io(&m)) == PU_TIMEOUT && s.count == 0 && m.reads == 0);
    for (unsigned i = 0; i < 32; i++)
    {
        m.r[PU_SR] = PU_RXNE | (i & 15u);
        m.r[PU_DR] = i;
        CHECK(ua_rx(&s, io(&m)) == PU_OK && s.count == i + 1);
    }
    m.r[PU_SR] = PU_RXNE | PU_ORE;
    m.r[PU_DR] = 0xff;
    CHECK(ua_rx(&s, io(&m)) == PU_FULL && m.reads == 32 &&
          (m.r[PU_SR] & PU_RXNE));
    for (unsigned i = 0; i < 32; i++)
    {
        CHECK(ua_take(&s, &b) == PU_OK && b.value == i && b.error == (i & 15u));
    }
    CHECK(ua_take(&s, &b) == PU_BUSY && b.value == 31);
    CHECK(ua_rx(&s, io(&m)) == PU_OK && ua_take(&s, &b) == PU_OK &&
          b.value == 0xff && b.error == PU_ORE);
    CHECK(m.reads == 33 && !m.ordered);
    s.bridge.errors = UINT32_MAX;
    m.r[PU_SR] = PU_RXNE;
    CHECK(ua_rx(&s, io(&m)) == PU_FULL && m.reads == 33);
    ua_values("rx", m.reads, s.count, b.value, b.error);
    return 0;
}

static int deadline(void)
{
    uint8_t b[16] = {0};
    struct ua_tx s = {0};
    struct mc_model m = {0};
    CHECK(open_uart(&s, &m) == PU_OK);
    CHECK(ua_begin(&s, io(&m), 1, 1, b, 16, 0, 8) == PU_OK);
    for (unsigned i = 0; i < 8; i++)
    {
        m.r[PU_SR] = PU_TXE;
        CHECK(ua_step(&s, io(&m), 1, i) == PU_BUSY);
    }
    m.r[PU_SR] = PU_TC;
    CHECK(ua_step(&s, io(&m), 1, 8) == PU_TIMEOUT && !s.active &&
          s.offset == 8 && m.writes == 8);
    CHECK(ua_begin(&s, io(&m), 1, 2, b, 16, UINT32_MAX - 1, 9) == PU_OK);
    m.r[PU_SR] = PU_TXE;
    CHECK(ua_step(&s, io(&m), 2, UINT32_MAX - 1) == PU_BUSY);
    CHECK(ua_cancel(&s, io(&m), 2) == PU_OK && s.active);
    m.r[PU_SR] = PU_TXE;
    CHECK(ua_step(&s, io(&m), 2, 0) == PU_BUSY && s.active);
    m.r[PU_SR] = PU_TC;
    CHECK(ua_step(&s, io(&m), 2, 1) == PU_CANCELLED && !s.active);
    CHECK(m.writes == 9);
    ua_values("deadline", 8, 8, m.writes, PU_CANCELLED);
    return 0;
}

int ua_cases(unsigned c)
{
    if (c == 0)
        return frames();
    if (c == 1)
        return baud();
    if (c == 2)
        return transmission();
    if (c == 3)
        return receive();
    if (c == 4)
        return deadline();
    return 42;
}
