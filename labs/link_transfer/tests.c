#include "transfer.h"
#include <string.h>
void lt_note(const char *);
#define C(x)                          \
    do                                \
    {                                 \
        if (!(x))                     \
        {                             \
            lt_note("LT_ASSERT:" #x); \
            return 41;                \
        }                             \
    } while (0)

static int serial(void)
{
    unsigned pairs = 0;
    for (unsigned m = 2; m <= 65; m++)
    {
        for (unsigned a = 0; a < m; a++)
        {
            for (unsigned b = 0; b < m; b++)
            {
                int v = 77;
                unsigned d = 0, pos = a;
                while (pos != b)
                {
                    if (++pos == m)
                    {
                        pos = 0;
                    }
                    d++;
                }
                int expected = d == 0 ? 0 : d <= m / 2 ? -1 : 1;
                int r = lt_order(m, a, b, &v);
                if (d && !(m & 1) && d == m / 2)
                {
                    C(r == LT_AMBIGUOUS && v == 77);
                }
                else
                {
                    C(r == LT_OK && v == expected);
                }
                pairs++;
            }
        }
    }
    unsigned base = 13, outstanding = 3;
    C(lt_ack(15, &base, &outstanding, 0) == LT_OK && base == 0 &&
      outstanding == 1);
    C(lt_ack(15, &base, &outstanding, 14) == LT_STALE && base == 0 &&
      outstanding == 1);
    C(lt_ack(15, &base, &outstanding, 1) == LT_OK && outstanding == 0);
    int o = 9;
    C(lt_order(12, 0, 6, &o) == LT_AMBIGUOUS && o == 9);
    C(lt_order(15, 14, 0, &o) == LT_OK && o == -1);
    C(((0u - 14u) & 14u) != 1);
    C(pairs == 93664);
    lt_note("LT_SERIAL_PASS 93664 15 0 1");
    return 0;
}

static int sizes(void)
{
    struct lt_layer p[2] = {{3, 8, 2}, {5, 7, 1}};
    uint32_t out[2] = {91, 92};
    C(lt_sizes(12, p, 2, out) == LT_OK && out[0] == 19 && out[1] == 31);
    p[1].header = UINT32_MAX;
    out[0] = 91;
    out[1] = 92;
    C(lt_sizes(12, p, 2, out) == LT_FULL && out[0] == 91 && out[1] == 92);
    p[1].header = 5;
    p[1].alignment = 0;
    C(lt_sizes(12, p, 2, out) == LT_INVALID && out[0] == 91);
    p[1].alignment = 7;
    C(lt_sizes(0, 0, 0, 0) == LT_OK);
    C(lt_sizes(UINT32_MAX, p, 1, out) == LT_FULL);
    lt_note("LT_SIZES_PASS 19 31 91 92");
    return 0;
}

static int borrow(void)
{
    unsigned char input[256], wire[258];
    for (unsigned i = 0; i < 256; i++)
    {
        input[i] = (unsigned char)i;
    }
    for (unsigned n = 0; n <= 256; n++)
    {
        for (unsigned limit = 1; limit <= 17; limit++)
        {
            struct lt_borrow s = {0};
            size_t used = 0;
            C(lt_begin(&s, input, n) == LT_OK);
            while (s.phase == 1)
            {
                const unsigned char *p = 0;
                size_t amount = 0;
                C(lt_offer(&s, &p, &amount) == LT_OK && amount > 0);
                size_t take = amount < limit ? amount : limit;
                memcpy(wire + used, p, take);
                used += take;
                int r = lt_accept(&s, take);
                C(r == (used == n + 2 ? LT_DONE : LT_OK));
            }
            C(used == n + 2 && wire[0] == (n >> 8) && wire[1] == (n & 255) &&
              memcmp(wire + 2, input, n) == 0 && !s.bytes);
        }
    }
    struct lt_borrow s = {0};
    const unsigned char *p;
    size_t n;
    C(lt_begin(&s, input, 3) == LT_OK && s.bytes == input);
    C(lt_offer(&s, &p, &n) == LT_OK && n == 2);
    C(lt_accept(&s, 3) == LT_INVALID && s.used == 0);
    C(lt_accept(&s, 1) == LT_OK && s.used == 1);
    C(lt_cancel(&s, 0) == LT_BUSY && s.bytes == input && s.phase == 3);
    C(lt_offer(&s, &p, &n) == LT_BUSY);
    C(lt_cancel(&s, 1) == LT_OK && !s.bytes && !s.phase);
    C(lt_begin(&s, 0, 0) == LT_OK && lt_offer(&s, &p, &n) == LT_OK && n == 2 &&
      lt_accept(&s, 2) == LT_DONE);
    lt_note("LT_BORROW_PASS 4369 258 1 0");
    return 0;
}

static int bridge(void)
{
    struct lt_bridge s = {0}, old;
    unsigned m = 99;
    const uint8_t a[6] = {2, 0, 0, 0, 0, 1}, b[6] = {2, 0, 0, 0, 0, 2};
    C(lt_bridge(&s, 10, a, b, 1, 0, &m) == LT_OK && m == 6);
    C(lt_bridge(&s, 20, a, b, 2, 1, &m) == LT_OK && m == 5);
    C(lt_bridge(&s, 10, b, a, 3, 2, &m) == LT_OK && m == 1);
    C(lt_bridge(&s, 20, b, a, 3, 3, &m) == LT_OK && m == 2);
    old = s;
    m = 99;
    C(lt_bridge(&s, 30, b, a, 3, 4, &m) == LT_INVALID && m == 99 &&
      !memcmp(&s, &old, sizeof s));
    C(lt_bridge(&s, 10, b, a, 3, 2, &m) == LT_INVALID);
    C(lt_bridge(&s, 10, b, a, 3, 10, &m) == LT_OK &&
      m == 3); /* VID10 A expired; VID20 independent. */
    lt_note("LT_BRIDGE_PASS 6 5 1 2 3");
    return 0;
}

struct lt_bus
{
    uint16_t reg[32];
    uint32_t control, tick;
    unsigned reads, writes, bsr, fail, stuck;
};

static int read_bus(void *ctx, unsigned csr, uint32_t *out)
{
    struct lt_bus *s = ctx;
    if (++s->reads == s->fail)
    {
        return 1;
    }
    if (!csr)
    {
        *out = (s->stuck && s->writes) ? 1 : 0;
        return 0;
    }
    if (csr != 1)
    {
        return 1;
    }
    unsigned reg = (s->control >> 6) & 31;
    *out = (reg == 1 && s->bsr++ == 0) ? 0 : s->reg[reg];
    return 0;
}

static int write_bus(void *ctx, unsigned csr, uint32_t v)
{
    struct lt_bus *s = ctx;
    if (csr)
    {
        return 1;
    }
    s->control = v;
    s->writes++;
    return 0;
}

static uint32_t tick(void *ctx)
{
    return ((struct lt_bus *)ctx)->tick++;
}

static struct lt_bus fresh(void)
{
    struct lt_bus s = {0};
    s.reg[2] = 0x2000;
    s.reg[3] = 0x5c90;
    s.reg[1] = 0x24;
    s.reg[16] = 0x15;
    return s;
}

static int phy(void)
{
    struct lt_bus s = fresh();
    struct ed_bus b = {&s, read_bus, write_bus, tick};
    struct ed_phy p = {0}, old;
    C(lt_dp83848(&b, 168000000, 1, 4, 100, &p) == ED_OK && p.historical == 0 &&
      p.current == 1 && p.speed == 100 && p.full == 1 && s.bsr == 2 &&
      s.writes == 10);
    s = fresh();
    s.reg[16] = 0x17;
    C(lt_dp83848(&b, 168000000, 1, 4, 100, &p) == ED_OK && p.speed == 10);
    old = p;
    s = fresh();
    s.reg[3] = 0x5c91;
    C(lt_dp83848(&b, 168000000, 1, 4, 100, &p) == ED_WRONG_PHY &&
      !memcmp(&p, &old, sizeof p));
    s = fresh();
    s.reg[16] = 0x11;
    C(lt_dp83848(&b, 168000000, 1, 4, 100, &p) == ED_UNSUPPORTED &&
      !memcmp(&p, &old, sizeof p));
    s = fresh();
    s.reg[16] = 0x1d;
    C(lt_dp83848(&b, 168000000, 1, 4, 100, &p) == ED_UNSUPPORTED);
    for (unsigned fail = 1; fail <= 15; fail++)
    {
        s = fresh();
        s.fail = fail;
        C(lt_dp83848(&b, 168000000, 1, 4, 100, &p) == ED_IO &&
          !memcmp(&p, &old, sizeof p));
    }
    s = fresh();
    s.stuck = 1;
    C(lt_dp83848(&b, 168000000, 1, 4, 100, &p) == ED_TIMEOUT && s.writes == 2);
    s = fresh();
    C(ed_phy_query(&b, 168000000, 1, 4, 100, &p) == ED_WRONG_PHY);
    s = fresh();
    s.reg[2] = 7;
    s.reg[3] = 0xc132;
    C(lt_dp83848(&b, 168000000, 1, 4, 100, &p) == ED_WRONG_PHY);
    lt_note("LT_PHY_PASS 8192 23696 100 10 15");
    return 0;
}

static int partial(void)
{
    unsigned char bytes[25] = {0};
    struct e7_sender sender = {0};
    struct lt_partial s = {0};
    const unsigned char *p;
    size_t n;
    C(e7_send_begin(&sender, 1, 7, 8, bytes, 25, 0) == E7_OK &&
      lt_partial_begin(&s, &sender, 0) == LT_OK);
    C(lt_offer(&s.lower, &p, &n) == LT_OK && n == 2 &&
      lt_partial_progress(&s, 1, 1) == LT_OK && s.lower.used == 1 &&
      !sender.waiting && sender.due == 0 && sender.offered);
    C(lt_offer(&s.lower, &p, &n) == LT_OK &&
      lt_partial_progress(&s, n, 2) == LT_OK && !sender.waiting);
    C(lt_offer(&s.lower, &p, &n) == LT_OK && n == 32);
    struct edu_fragment f;
    C(edu_fragment_decode(p, n, &f) == 0 && f.id == 1 && f.length == 24);
    C(lt_partial_progress(&s, n, 10) == LT_DONE && sender.waiting &&
      sender.due == 110 && !sender.offered);
    struct e7_sender next = {0};
    s = (struct lt_partial){0};
    C(e7_send_begin(&next, 2, 7, 8, bytes, 25, 0) == E7_OK &&
      lt_partial_begin(&s, &next, 0) == LT_OK);
    C(lt_offer(&s.lower, &p, &n) == LT_OK &&
      lt_partial_progress(&s, 1, 49) == LT_OK &&
      lt_partial_progress(&s, 0, 50) == LT_TIMEOUT && s.active &&
      next.offered && !next.waiting);
    C(lt_partial_abort(&s, 0, 51) == LT_BUSY && s.active && next.offered &&
      lt_partial_abort(&s, 1, 52) == LT_OK && !s.active && !next.offered &&
      !next.waiting);
    C(lt_partial_begin(&s, &next, 53) == LT_OK &&
      lt_partial_abort(&s, 0, 53) == LT_OK);
    lt_note("LT_PARTIAL_PASS 0 110 50 52");
    return 0;
}

int lt_tests(unsigned which)
{
    switch (which)
    {
        case 0:
            return serial();
        case 1:
            return sizes();
        case 2:
            return borrow();
        case 3:
            return bridge();
        case 4:
            return phy();
        case 5:
            return partial();
        default:
            return 41;
    }
}
