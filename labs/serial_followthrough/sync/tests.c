#include "sync.h"
void su_note(const char *);
void su_values(const char *, unsigned, unsigned, unsigned, unsigned);
#define CHECK(x)                       \
    do                                 \
    {                                  \
        if (!(x))                      \
        {                              \
            su_note("SU_ASSERT: " #x); \
            return 41;                 \
        }                              \
    } while (0)

struct mc_model
{
    uint32_t r[16];
    unsigned dr_reads, writes, sr_read, sequence_error;
    unsigned commands[256], values[256], log_count;
};

static uint32_t read_io(void *p, unsigned k)
{
    struct mc_model *m = p;
    if (k == PU_SOURCE)
    {
        return 1;
    }
    if (k == PU_SR)
    {
        m->sr_read = 1;
    }
    if (k == PU_DR)
    {
        if (!m->sr_read)
        {
            m->sequence_error = 1;
        }
        m->sr_read = 0;
        m->dr_reads++;
        m->r[PU_SR] &= ~47u;
    }
    return m->r[k];
}

static void write_io(void *p, unsigned k, uintptr_t v)
{
    struct mc_model *m = p;
    m->r[k] = (uint32_t)v;
    if (m->log_count < 256)
    {
        m->commands[m->log_count] = k;
        m->values[m->log_count++] = (unsigned)v;
    }
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

static int opened(struct mc_su *s, struct mc_model *m)
{
    struct pu_clock c = {42000000, 1, 1, 1, 1, 0};
    return pu_open(&s->byte, io(m), &c, 115200, 0, 1);
}

static int configured(struct mc_su *s, struct mc_model *m)
{
    if (opened(s, m))
    {
        return 1;
    }
    m->r[PU_CR1] &= ~12u;
    if (su_configure(s, io(m), 1, 0, 1))
    {
        return 1;
    }
    return su_enable(s, io(m), 1, 1);
}

static int edges(void)
{
    struct sd_sync t;
    unsigned bad = 99, total = 0;
    for (unsigned p = 0; p < 2; p++)
    {
        for (unsigned h = 0; h < 2; h++)
        {
            for (unsigned l = 0; l < 2; l++)
            {
                for (unsigned v = 0; v < 256; v++)
                {
                    CHECK(sd_sync(p, h, l, 1, 1, 0, (uint8_t)v, &t) == SD_OK);
                    CHECK(su_trace(p, h, l, &t, &bad) == PU_OK && bad == 0);
                    CHECK(t.edges == (l ? 16 : 14));
                    unsigned captures = 0;
                    for (unsigned i = 0; i < t.edges; i++)
                    {
                        CHECK(t.level[i] == (p ^ ((i + 1) & 1)));
                        CHECK(t.sample[i] == ((i & 1) == h));
                        captures += t.sample[i];
                    }
                    for (unsigned i = 0; i < 8; i++)
                    {
                        CHECK(t.data[i] == ((v >> i) & 1));
                    }
                    CHECK(captures == (l ? 8 : 7));
                    total++;
                }
            }
        }
    }
    CHECK(sd_sync(1, 0, 0, 1, 1, 0, 0xa5, &t) == SD_OK);
    CHECK(su_trace(1, 0, 1, &t, &bad) == PU_IO && bad == 15);
    CHECK(sd_sync(1, 1, 1, 1, 1, 0, 0xa5, &t) == SD_OK);
    CHECK(su_trace(1, 0, 1, &t, &bad) == PU_IO && bad == 1);
    CHECK(sd_sync(0, 0, 1, 0, 1, 0, 0, &t) == SD_INVALID);
    /* Externally clocked SPI slave: sample existing input edges, no USART TX. */
    unsigned value = 0;
    for (unsigned edge = 1; edge <= 16; edge++)
    {
        if (edge & 1)
        {
            value |= ((0x96u >> ((edge - 1) / 2)) & 1u) << ((edge - 1) / 2);
        }
    }
    CHECK(value == 0x96);
    su_values("edges", total, 14, 16, value);
    return 0;
}

static int config(void)
{
    struct mc_su s = {0};
    struct mc_model m = {0};
    CHECK(opened(&s, &m) == PU_OK);
    unsigned n = m.log_count;
    CHECK(su_configure(&s, io(&m), 1, 0, 1) == PU_BUSY && m.log_count == n);
    m.r[PU_CR1] &= ~8u;
    CHECK(su_configure(&s, io(&m), 1, 0, 1) == PU_BUSY && m.log_count == n);
    m.r[PU_CR1] &= ~4u;
    CHECK(su_configure(&s, io(&m), 2, 0, 1) == PU_INVALID && m.log_count == n);
    for (unsigned p = 0; p < 2; p++)
    {
        for (unsigned h = 0; h < 2; h++)
        {
            for (unsigned l = 0; l < 2; l++)
            {
                CHECK(su_configure(&s, io(&m), p, h, l) == PU_OK);
                CHECK(m.r[PU_CR2] == (0x800u | p << 10 | h << 9 | l << 8) &&
                      m.r[PU_CR3] == 0);
            }
        }
    }
    n = m.log_count;
    CHECK(su_enable(&s, io(&m), 1, 1) == PU_OK);
    CHECK(m.log_count == n + 1 && (m.r[PU_CR1] & 12) == 12);
    su_values("config", m.r[PU_CR2], m.r[PU_CR1], 8, 1);
    return 0;
}

static int transfer(void)
{
    uint8_t tx[8] = {0, 1, 0x96, 0xa5, 0x35, 0xff, 0x80, 0x7f};
    for (unsigned count = 1; count <= 8; count++)
    {
        struct mc_su s = {0};
        struct mc_model m = {0};
        struct pu_byte rx[8] = {{0}};
        CHECK(configured(&s, &m) == PU_OK);
        m.r[PU_CR1] &= ~8u;
        CHECK(su_begin(&s, io(&m), 1, 1, tx, count, rx, count, 0, 100) ==
              SU_UNSUPPORTED);
        CHECK(!s.active && m.writes == 0);
        m.r[PU_CR1] |= 8u;
        CHECK(su_begin(&s, io(&m), 2, 1, tx, count, rx, count, 0, 100) ==
              PU_STALE);
        CHECK(su_begin(&s, io(&m), 1, 1, tx, count, rx, count, 0, 100) ==
              PU_OK);
        CHECK(su_step(&s, io(&m), 2, 0) == PU_STALE && s.received == 0);
        for (unsigned i = 0; i < count; i++)
        {
            unsigned now = i * 3;
            m.r[PU_SR] = PU_TXE | PU_TC;
            CHECK(su_step(&s, io(&m), 1, now) == PU_BUSY && s.active &&
                  s.byte.line_owned);
            CHECK(m.r[PU_DR] == tx[i] && m.writes == i + 1);
            m.r[PU_DR] = tx[i] ^ 0x5au;
            m.r[PU_SR] = PU_RXNE | PU_TXE;
            CHECK(su_step(&s, io(&m), 1, now + 1) == PU_BUSY &&
                  s.byte.line_owned);
            CHECK(rx[i].value == (uint8_t)(tx[i] ^ 0x5au) && rx[i].error == 0);
            m.r[PU_SR] = PU_TC | PU_TXE;
            CHECK(su_step(&s, io(&m), 1, now + 2) ==
                  (i + 1 == count ? PU_OK : PU_BUSY));
        }
        CHECK(!s.active && s.received == count && m.dr_reads == count &&
              !m.sequence_error);
        CHECK(su_step(&s, io(&m), 1, 30) == PU_STALE);
        su_values("transfer", count, m.writes, m.dr_reads, s.received);
    }
    return 0;
}

static int failures(void)
{
    uint8_t tx[2] = {0xa5, 0x96};
    struct pu_byte rx[2] = {{17, 0}, {19, 0}};
    for (unsigned error = 1; error <= 8; error *= 2)
    {
        struct mc_su s = {0};
        struct mc_model m = {0};
        CHECK(configured(&s, &m) == PU_OK);
        CHECK(su_begin(&s, io(&m), 1, 1, tx, 2, rx, 2, UINT32_MAX - 2, 9) ==
              PU_OK);
        m.r[PU_SR] = PU_TXE;
        CHECK(su_step(&s, io(&m), 1, UINT32_MAX - 2) == PU_BUSY);
        m.r[PU_SR] = PU_RXNE | error;
        m.r[PU_DR] = 0xa5;
        CHECK(su_step(&s, io(&m), 1, UINT32_MAX - 1) == PU_BUSY && s.active);
        CHECK(rx[0].value == 0xa5 && rx[0].error == error && s.received == 1);
        CHECK(su_configure(&s, io(&m), 0, 0, 1) == PU_BUSY);
        m.r[PU_SR] = PU_TC | PU_TXE;
        CHECK(su_step(&s, io(&m), 1, 0) == PU_IO && !s.active &&
              (m.r[PU_CR1] & 12) == 0);
        CHECK(m.writes == 1 && m.dr_reads == 1 && rx[1].value == 19);
    }
    struct mc_su s = {0};
    struct mc_model m = {0};
    CHECK(configured(&s, &m) == PU_OK);
    CHECK(su_begin(&s, io(&m), 1, 1, tx, 2, rx, 2, 0, 2) == PU_OK);
    m.r[PU_SR] = PU_TXE;
    CHECK(su_step(&s, io(&m), 1, 0) == PU_BUSY);
    m.r[PU_SR] = PU_TXE;
    CHECK(su_step(&s, io(&m), 1, 2) == PU_BUSY && s.active);
    CHECK(su_step(&s, io(&m), 1, 1) == PU_INVALID && s.active);
    m.r[PU_SR] = PU_TC;
    CHECK(su_step(&s, io(&m), 1, 3) == PU_TIMEOUT && !s.active &&
          s.received == 0);
    CHECK(m.writes == 1 && m.dr_reads == 0);
    CHECK(su_enable(&s, io(&m), 1, 1) == PU_OK);
    s.byte.token = UINT32_MAX - 1;
    CHECK(su_begin(&s, io(&m), 1, 2, tx, 2, rx, 2, 4, 10) == PU_STALE);
    su_values("failures", 4, PU_TIMEOUT, m.writes, m.dr_reads);
    return 0;
}

int su_cases(unsigned c)
{
    if (c == 0)
    {
        return edges();
    }
    if (c == 1)
    {
        return config();
    }
    if (c == 2)
    {
        return transfer();
    }
    if (c == 3)
    {
        return failures();
    }
    return 42;
}
