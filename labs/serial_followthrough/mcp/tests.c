#include "mcp.h"
#include "spi_adapter.h"
#include <string.h>
#define CHECK(x)                      \
    do                                \
    {                                 \
        if (!(x))                     \
        {                             \
            mc_note("MC_ASSERT:" #x); \
            return 1;                 \
        }                             \
    } while (0)

struct mc_model
{
    uint8_t mem[4096];
    unsigned calls, fail, partial, corrupt_crc, all_ones, stuck, freeze, reset,
        illegal, ram_writes, tx_publications, rx_consumed, active, auto_loop;
    uint32_t ticks;
};
static struct mc_model m;

static uint32_t load(unsigned a)
{
    return (uint32_t)m.mem[a] | (uint32_t)m.mem[a + 1] << 8 |
           (uint32_t)m.mem[a + 2] << 16 | (uint32_t)m.mem[a + 3] << 24;
}

static void save(unsigned a, uint32_t v)
{
    for (unsigned i = 0; i < 4; i++)
    {
        m.mem[a + i] = (uint8_t)(v >> (8 * i));
    }
}

static uint16_t crc(const uint8_t *p, unsigned n)
{
    unsigned long c = 65535;
    for (unsigned i = 0; i < n * 8; i++)
    {
        unsigned b = (p[i / 8] >> (7 - i % 8)) & 1,
                 top = (unsigned)(c >> 15) & 1;
        c = (c << 1) & 65535;
        if (top ^ b)
        {
            c ^= 0x8005;
        }
    }
    return (uint16_t)c;
}

static void reset(void)
{
    memset(m.mem, 0, 0x400);
    memset(m.mem + 0xc00, 0, 0x400);
    save(0, 0x04800060);
    save(0xe00, 0x400);
    save(0xe14, 0);
    m.reset++;
}

static void start(void)
{
    memset(&m, 0, sizeof m);
    memset(m.mem + 0x400, 0xa5, 2048);
    reset();
    m.reset = 0;
}

static unsigned payload(void)
{
    static const unsigned n[] = {8, 12, 16, 20, 24, 32, 48, 64};
    return n[load(0x5c) >> 29];
}

static void complete(unsigned status)
{
    save(0x5c, load(0x5c) & ~0x200u);
    save(0x60, status | (status ? 0u : 5u));
}

static void inject(const struct mc_frame *f)
{
    unsigned stride = 8 + payload(), a = 0x400 + load(0x70);
    save(a,
         0xc0000000u | f->id); /* R0 top two bits are unimplemented/read x. */
    save(a + 4, (uint32_t)f->dlc | 0x80u | (unsigned)f->brs << 6 |
                    (unsigned)f->esi << 8);
    memcpy(m.mem + a + 8, f->data, payload());
    save(0x6c, 1 | 0x1f00);
    save(0x1c, load(0x1c) | 2);
    (void)stride;
}

static int transfer(void *ctx, const uint8_t *tx, uint8_t *rx, size_t n)
{
    (void)ctx;
    m.calls++;
    m.active = 1;
    memset(rx, 0, n);
    unsigned command = tx[0] >> 4, a = ((tx[0] & 15u) << 8) | tx[1];
    if (m.fail == m.calls && !m.partial)
    {
        m.active = 0;
        return -1;
    }
    if (n > 16 || n < 2)
    {
        m.illegal++;
        m.active = 0;
        return -1;
    }
    if (command == 0)
    {
        if (n != 2 || ((load(0) >> 21) & 7) != 4)
        {
            m.illegal++;
            m.active = 0;
            return -1;
        }
        reset();
    }
    else if (command == 11)
    {
        unsigned bytes = (a >= 0x400 && a < 0xc00) ? 4u * tx[2] : tx[2];
        if (n != bytes + 5 || a + bytes > 4096)
        {
            m.illegal++;
            m.active = 0;
            return -1;
        }
        uint8_t check[16];
        memcpy(check, tx, 3);
        memcpy(rx + 3, m.mem + a, bytes);
        if (m.all_ones)
        {
            memset(rx + 3, 255, bytes);
        }
        memcpy(check + 3, rx + 3, bytes);
        uint16_t c = crc(check, bytes + 3);
        rx[n - 2] = (uint8_t)(c >> 8);
        rx[n - 1] = (uint8_t)c;
        if (m.corrupt_crc)
        {
            rx[n - 1] ^= 1;
            m.corrupt_crc--;
        }
    }
    else if (command == 2)
    {
        unsigned bytes = (unsigned)n - 2;
        if (a + bytes > 4096 ||
            (a >= 0x400 && a < 0xc00 && ((a | bytes) & 3)) ||
            (a < 0xe08 && a + bytes > 0xe04 && bytes != 1))
        {
            m.illegal++;
            m.active = 0;
            return -1;
        }
        if (a == 0x5d && bytes == 1)
        {
            unsigned value = tx[2];
            if (value & 4)
            {
                if (load(0x5c) & 0x200)
                {
                    m.illegal++;
                }
                save(0x60, 5);
                save(0x64, 0);
                save(0x5c, load(0x5c) & ~0x700u);
            }
            else if (value & 2)
            {
                m.tx_publications++;
                if (!(value & 1))
                {
                    m.active = 0;
                    return 0;
                }
                save(0x5c, load(0x5c) | 0x200);
                save(0x60, 0);
                if (m.auto_loop)
                {
                    unsigned stride = 8 + payload();
                    memcpy(m.mem + 0x400 + stride, m.mem + 0x400, stride);
                    save(0x6c, 1);
                    complete(0);
                }
            } /* abort is request; completion injected separately */
        }
        else if (a == 0x69 && bytes == 1)
        {
            if (tx[2] & 1)
            {
                m.rx_consumed++;
                save(0x6c, 0);
                save(0x1c, load(0x1c) & ~2u);
            }
        }
        else if (a == 0x6c && bytes == 1)
        {
            m.mem[a] &= tx[2];
        }
        else
        {
            uint32_t oldcon = load(0);
            memcpy(m.mem + a, tx + 2, bytes);
            if (a == 0 || a == 3)
            {
                unsigned want = m.mem[3] & 7;
                uint32_t v = load(0);
                v = (v & ~(7u << 21)) | (oldcon & (7u << 21));
                if (!m.stuck)
                {
                    v = (v & ~(7u << 21)) | want << 21;
                }
                save(0, v);
                if (want != 4 && !m.stuck)
                {
                    unsigned stride = 8 + payload();
                    save(0x5c, load(0x5c) & ~0x400u);
                    save(0x68, load(0x68) & ~0x400u);
                    save(0x60, 5);
                    save(0x64, 0);
                    save(0x70, stride);
                }
            }
            if (a == 0xe00)
            {
                save(0xe00, load(0xe00) | 0x400);
            }
            if (a >= 0x400 && a < 0xc00)
            {
                m.ram_writes += bytes;
            }
        }
    }
    else
    {
        m.illegal++;
        m.active = 0;
        return -1;
    }
    m.active = 0;
    return m.fail == m.calls ? -1 : 0;
}

static uint32_t now(void *ctx)
{
    (void)ctx;
    if (!m.freeze)
    {
        m.ticks++;
    }
    return m.ticks;
}

static struct mc_bus bus(void)
{
    struct mc_bus b = {0, transfer, now};
    return b;
}

static struct mc_config config(unsigned code)
{
    struct mc_config c = {40000000, 1000000, code, 4, 1, 2000000, 0, 0x123};
    return c;
}

int mc_test_layout(void)
{
    CHECK(mc_crc((const uint8_t *)"123456789", 9) == 0xaee7);
    struct mc_layout l = {0};
    unsigned combos = 0;
    for (unsigned c = 0; c < 8; c++)
    {
        for (unsigned n = 1; n <= 16; n++)
        {
            CHECK(mc_layout(c, n, &l) == MC_OK);
            CHECK(l.total == l.tx_bytes + l.rx_bytes + 464 && l.total <= 2048);
            CHECK(l.rx_bytes == n * l.tx_bytes);
            combos++;
        }
    }
    l.total = 99;
    CHECK(mc_layout(8, 1, &l) == MC_INVALID && l.total == 99);
    CHECK(mc_layout(7, 0, &l) == MC_INVALID);
    CHECK(mc_layout(7, 17, &l) == MC_INVALID);
    mc_values("layout", combos, 1688, 2048, 0xaee7);
    return 0;
}

int mc_test_init(void)
{
    for (unsigned code = 0; code < 8; code++)
    {
        for (unsigned rate = 1; rate <= 2; rate++)
        {
            start();
            struct mc_driver d = {0};
            struct mc_config c = config(code);
            c.data_rate = rate * 1000000;
            CHECK(mc_init(&d, bus(), &c, 100) == MC_OK && d.state == MC_READY);
            CHECK(m.reset == 1 && m.ram_writes == 2048 && !m.illegal &&
                  !m.active);
            CHECK(load(4) == 0x003a1303 &&
                  load(8) == (rate == 2 ? 0x000d0403 : 0x001c0903));
            CHECK(load(0x1f4) == 0x400007ff && load(0x1d0) == 0x82);
            CHECK((load(0) & 0x380000) == 0);
            CHECK(load(0xe0c) == 7);
            CHECK(m.mem[0xe04] == 3 && m.mem[0xe07] == 0);
        }
    }
    unsigned calls = m.calls;
    start();
    struct mc_driver d = {0};
    struct mc_config c = config(7);
    m.corrupt_crc = 1;
    CHECK(mc_init(&d, bus(), &c, 100) == MC_OK);
    CHECK(m.calls == calls + 1);
    mc_values("init", calls, 16, 2048, 17000000);
    return 0;
}

int mc_test_failure(void)
{
    start();
    struct mc_driver d = {0};
    struct mc_config c = config(7);
    CHECK(mc_init(&d, bus(), &c, 100) == MC_OK);
    unsigned count = m.calls;
    for (unsigned phase = 1; phase <= count; phase++)
    {
        for (unsigned partial = 0; partial < 2; partial++)
        {
            start();
            memset(&d, 0, sizeof d);
            m.fail = phase;
            m.partial = partial;
            CHECK(mc_init(&d, bus(), &c, 100) != MC_OK && d.state != MC_READY &&
                  !m.active);
            CHECK(m.calls == phase);
        }
    }
    start();
    memset(&d, 0, sizeof d);
    m.all_ones = 1;
    CHECK(mc_init(&d, bus(), &c, 100) == MC_DEVICE && m.reset == 0);
    start();
    memset(&d, 0, sizeof d);
    m.corrupt_crc = 3;
    CHECK(mc_init(&d, bus(), &c, 100) == MC_CRC && m.calls == 3);
    start();
    memset(&d, 0, sizeof d);
    m.stuck = 1;
    save(0, 0);
    CHECK(mc_init(&d, bus(), &c, 4) == MC_TIMEOUT && m.reset == 0);
    start();
    memset(&d, 0, sizeof d);
    m.stuck = m.freeze = 1;
    save(0, 0);
    CHECK(mc_init(&d, bus(), &c, 4) == MC_TIMEOUT && m.calls == 1026);
    start();
    memset(&d, 0, sizeof d);
    c.spi_hz = 17000001;
    CHECK(mc_init(&d, bus(), &c, 100) == MC_INVALID && m.calls == 0);
    mc_values("failure", count * 2, 3, 1026, 0);
    return 0;
}

int mc_test_transfer(void)
{
    static const unsigned lengths[] = {0, 1,  2,  3,  4,  5,  6,  7,
                                       8, 12, 16, 20, 24, 32, 48, 64};
    unsigned admitted = 0;
    for (unsigned code = 0; code < 8; code++)
    {
        for (unsigned dlc = 0; dlc < 16; dlc++)
        {
            start();
            struct mc_driver d = {0};
            struct mc_config c = config(code);
            CHECK(mc_init(&d, bus(), &c, 100) == MC_OK);
            struct mc_frame f = {.id = 0x123, .dlc = (uint8_t)dlc, .brs = 1};
            memset(f.data, 0xa5, 64);
            unsigned calls = m.calls;
            int r = mc_send(&d, bus(), 1, &f);
            if (lengths[dlc] > d.payload)
            {
                CHECK(r == MC_INVALID && calls == m.calls);
                continue;
            }
            CHECK(r == MC_OK && d.pending && m.tx_publications == 1);
            CHECK(mc_poll(&d, bus()) == MC_BUSY);
            CHECK(mc_send(&d, bus(), 2, &f) == MC_BUSY);
            unsigned out = 99;
            CHECK(mc_take(&d, bus(), 1, 100, &out) == MC_BUSY && out == 99);
            CHECK(mc_cancel(&d, bus()) == MC_OK &&
                  mc_poll(&d, bus()) == MC_BUSY);
            complete(0);
            CHECK(mc_poll(&d, bus()) == MC_OK &&
                  mc_take(&d, bus(), 1, 100, &out) == MC_OK && out == MC_OK);
            CHECK(mc_send(&d, bus(), 1, &f) == MC_STALE);
            admitted++;
        }
    }
    start();
    struct mc_driver d = {0};
    struct mc_config c = config(7);
    CHECK(mc_init(&d, bus(), &c, 100) == MC_OK);
    struct mc_frame f = {.id = 0x123};
    CHECK(mc_send(&d, bus(), UINT32_MAX, &f) == MC_OK);
    complete(0x90);
    CHECK(mc_poll(&d, bus()) == MC_OK);
    unsigned out = 99;
    CHECK(mc_take(&d, bus(), UINT32_MAX, 100, &out) == MC_OK &&
          out == MC_FAILED);
    CHECK(mc_send(&d, bus(), 1, &f) == MC_STALE);
    mc_values("transfer", admitted, 128, 1, UINT32_MAX);
    return 0;
}

int mc_test_receive(void)
{
    start();
    struct mc_driver d = {0};
    struct mc_config c = config(7);
    CHECK(mc_init(&d, bus(), &c, 100) == MC_OK);
    struct mc_frame f = {.id = 0x123, .dlc = 15, .brs = 1}, out = {.id = 999};
    for (unsigned i = 0; i < 64; i++)
    {
        f.data[i] = (uint8_t)i;
    }
    CHECK(mc_receive(&d, bus(), &out) == MC_EMPTY && out.id == 999);
    inject(&f);
    CHECK(mc_receive(&d, bus(), &out) == MC_OK && out.id == f.id &&
          !memcmp(out.data, f.data, 64));
    CHECK(m.rx_consumed == 1 && !(load(0x1c) & 2));
    inject(&f);
    m.fail = m.calls + 12; /* failure count varied explicitly below instead */
    m.fail = 0;
    save(0x70, 73);
    out.id = 999;
    CHECK(mc_receive(&d, bus(), &out) == MC_DEVICE && out.id == 999 &&
          m.rx_consumed == 1);
    start();
    memset(&d, 0, sizeof d);
    CHECK(mc_init(&d, bus(), &c, 100) == MC_OK);
    save(0x1c, load(0x1c) | 0x800);
    save(0x6c, 8);
    uint32_t flags = 0;
    CHECK(mc_interrupt(&d, bus(), &flags) == MC_OK && flags == 0x800 &&
          !(load(0x6c) & 8));
    save(0xe10, 2);
    CHECK(mc_interrupt(&d, bus(), &flags) == MC_DEVICE &&
          d.state == MC_POISONED);
    mc_values("receive", 64, 1, 73, 2);
    return 0;
}

int mc_test_bench(void)
{
    unsigned bytes = 0;
    for (unsigned brs = 0; brs < 2; brs++)
    {
        for (unsigned i = 0; i < 6; i++)
        {
            struct mc_frame f;
            CHECK(mc_bench_frame(i, brs, &f) == MC_OK &&
                  mc_bench_match(i, brs, &f) == MC_OK);
            CHECK(mc_bench_match((i + 1) % 6, brs, &f) == MC_FAILED);
            CHECK(mc_bench_match(i, 1 - brs, &f) == MC_FAILED);
            unsigned len[] = {0, 8, 12, 16, 20, 64};
            bytes += len[i];
            if (i)
            {
                f.data[len[i] - 1] ^= 1;
                CHECK(mc_bench_match(i, brs, &f) == MC_FAILED);
            }
        }
    }
    CHECK(mc_bench_frame(6, 0, 0) == MC_INVALID);
    mc_values("bench", 12, bytes, 0x300, 0x305);
    return 0;
}

struct mc_wire
{
    unsigned cs, ready, index, n, reply, fail_wire, reset_stuck,
        fail_after_byte;
    uint8_t tx[16], rx[16];
};
static struct mc_wire w;

static uint32_t spi_read(void *ctx, unsigned reg)
{
    (void)ctx;
    if (reg == PS_SR)
    {
        return w.fail_wire ? (w.reset_stuck ? PS_BSY : 0)
                           : (PS_TXE | (w.ready ? PS_RXNE : 0));
    }
    if (reg == PS_DR)
    {
        if (w.fail_after_byte)
        {
            w.fail_wire = 1;
        }
        w.ready = 0;
        return w.reply;
    }
    return 0;
}

static void spi_write(void *ctx, unsigned reg, uintptr_t value)
{
    (void)ctx;
    if (reg == PS_CS)
    {
        if (!value)
        {
            w.index = w.n = 0;
            w.cs = 1;
        }
        else if (w.cs)
        {
            if (w.index >= 2 && w.tx[0] >> 4 != 11)
            {
                (void)transfer(0, w.tx, w.rx, w.index);
            }
            w.cs = 0;
        }
    }
    if (reg == PS_RESET && !w.reset_stuck)
    {
        w.fail_wire = 0;
        w.ready = 0;
    }
    if (reg == PS_DR)
    {
        unsigned at = w.index++;
        w.tx[at] = (uint8_t)value;
        w.reply = 0;
        if (at == 2 && (w.tx[0] >> 4) == 11)
        {
            unsigned addr = ((w.tx[0] & 15u) << 8) | w.tx[1];
            w.n = (addr >= 0x400 && addr < 0xc00 ? 4u * w.tx[2] : w.tx[2]) + 5;
            if (w.n <= 16)
            {
                (void)transfer(0, w.tx, w.rx, w.n);
            }
        }
        if (w.n && at >= 3 && at < w.n)
        {
            w.reply = w.rx[at];
        }
        w.ready = 1;
    }
}

int mc_test_spi(void)
{
    start();
    memset(&w, 0, sizeof w);
    struct mc_spi a = {0};
    a.io = (struct ps_io){0, spi_read, spi_write};
    a.ticks = now;
    a.budget = 1000;
    a.setup = 1;
    a.hold = 1;
    struct pu_clock clock = {16000000, 1, 1, 1, 1, 0};
    CHECK(ps_open(&a.spi, a.io, &clock, 0, 3) == PS_OK);
    struct mc_bus b = {&a, mc_spi_transfer, mc_spi_now};
    struct mc_driver d = {0};
    struct mc_config c = config(7);
    CHECK(mc_init(&d, b, &c, 1000) == MC_OK && !w.cs && !a.spi.cs_owned);
    m.auto_loop = 1;
    struct mc_frame f = {.id = 0x123, .dlc = 0}, out;
    CHECK(mc_send(&d, b, 1, &f) == MC_OK && mc_poll(&d, b) == MC_OK &&
          mc_receive(&d, b, &out) == MC_OK && out.dlc == 0);
    unsigned result = 99;
    CHECK(mc_take(&d, b, 1, 1000, &result) == MC_OK && result == MC_OK);
    /* Original stuck-peripheral control: passed buffers are no longer borrowed,
       but internal staging and CS remain quarantined, so subsequent use rejects.
     */
    w.fail_after_byte = w.reset_stuck = 1;
    a.budget = 4;
    uint8_t tx[2] = {0}, rx[2] = {77, 88};
    CHECK(mc_spi_transfer(&a, tx, rx, 2) != 0 && rx[0] == 77);
    CHECK(a.spi.phase && a.spi.cs_owned && w.cs);
    CHECK(mc_spi_transfer(&a, tx, rx, 2) != 0);
    mc_values("spi", 490, 16, 1, 1);
    return 0;
}

int mc_test_atomic(void)
{
    struct mc_config c = config(7);
    struct mc_frame f = {.id = 0x123, .dlc = 15};
    memset(f.data, 0x5a, 64);
    start();
    struct mc_driver d = {0};
    CHECK(mc_init(&d, bus(), &c, 100) == MC_OK);
    unsigned before = m.calls;
    CHECK(mc_send(&d, bus(), 1, &f) == MC_OK);
    unsigned send_calls = m.calls - before;
    for (unsigned i = 1; i <= send_calls; i++)
    {
        for (unsigned partial = 0; partial < 2; partial++)
        {
            start();
            memset(&d, 0, sizeof d);
            CHECK(mc_init(&d, bus(), &c, 100) == MC_OK);
            m.fail = m.calls + i;
            m.partial = partial;
            CHECK(mc_send(&d, bus(), 1, &f) != MC_OK && d.state == MC_POISONED);
            CHECK(m.calls == m.fail);
            if (i == send_calls)
            {
                CHECK(d.pending && d.token == 1);
            }
        }
    }
    start();
    memset(&d, 0, sizeof d);
    CHECK(mc_init(&d, bus(), &c, 100) == MC_OK);
    inject(&f);
    before = m.calls;
    struct mc_frame out;
    CHECK(mc_receive(&d, bus(), &out) == MC_OK);
    unsigned receive_calls = m.calls - before;
    for (unsigned i = 1; i <= receive_calls; i++)
    {
        for (unsigned partial = 0; partial < 2; partial++)
        {
            start();
            memset(&d, 0, sizeof d);
            CHECK(mc_init(&d, bus(), &c, 100) == MC_OK);
            inject(&f);
            out.id = 999;
            m.fail = m.calls + i;
            m.partial = partial;
            CHECK(mc_receive(&d, bus(), &out) != MC_OK && out.id == 999 &&
                  d.state == MC_POISONED);
            CHECK(m.rx_consumed == (i == receive_calls && partial ? 1u : 0u));
        }
    }
    mc_values("atomic", send_calls * 2, receive_calls * 2, 1, 999);
    return 0;
}
