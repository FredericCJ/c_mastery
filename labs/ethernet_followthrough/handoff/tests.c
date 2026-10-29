#include "handoff.h"
#include <string.h>
int puts(const char *);
#define ECHECK(c, m)              \
    do                            \
    {                             \
        if (!(c))                 \
        {                         \
            puts("EH_ASSERT:" m); \
            return 41;            \
        }                         \
    } while (0)
static const unsigned char local[6] = {2, 0, 0, 0, 0, 2};

static void p32(unsigned char *p, uint32_t n)
{
    p[0] = (unsigned char)(n >> 24);
    p[1] = (unsigned char)(n >> 16);
    p[2] = (unsigned char)(n >> 8);
    p[3] = (unsigned char)n;
}

static void packet(unsigned char *p, uint32_t src, uint32_t dst)
{
    uint32_t s = 0;
    memset(p, 0, 40);
    p[0] = 0x45;
    p[3] = 40;
    p[8] = 64;
    p[9] = 6;
    p32(p + 12, src);
    p32(p + 16, dst);
    for (unsigned i = 0; i < 20; i += 2)
    {
        s += (unsigned)p[i] * 256 + p[i + 1];
    }
    while (s >> 16)
    {
        s = (s & 65535) + (s >> 16);
    }
    s ^= 65535;
    p[10] = (unsigned char)(s >> 8);
    p[11] = (unsigned char)s;
}

static int route(void)
{
    struct eh_route r = {0xc0000202, 0xffffff00, 0xc0000201};
    struct eh_neighbor n = {0xc0000201, {2, 0, 0, 0, 1, 1}};
    uint32_t next = 9;
    unsigned char p[40], out[100], old[100];
    size_t written = 999;
    ECHECK(eh_next(&r, 0xc0000209, &next) == EH_OK && next == 0xc0000209,
           "local_neighbor");
    ECHECK(eh_next(&r, 0xc6336409, &next) == EH_OK && next == r.gateway,
           "gateway_not_final");
    packet(p, r.local, 0xc6336409);
    ECHECK(eh_frame(&r, local, &n, p, 40, out, sizeof out, &written) == EH_OK &&
               written == 60,
           "route_frame");
    ECHECK(!memcmp(out, n.mac, 6) && !memcmp(out + 14, p, 40) &&
               out[30] == 198 && out[33] == 9,
           "final_ip_preserved");
    memcpy(old, out, sizeof out);
    n.address = 0xc6336409;
    written = 999;
    ECHECK(eh_frame(&r, local, &n, p, 40, out, sizeof out, &written) ==
                   EH_NEIGHBOR &&
               written == 999 && !memcmp(old, out, sizeof out),
           "wrong_neighbor");
    r.gateway = 0;
    next = 9;
    ECHECK(eh_next(&r, 0xc6336409, &next) == EH_NO_ROUTE && next == 9,
           "missing_route");
    r.gateway = 0xc0000201;
    r.mask = 0xff00ff00;
    ECHECK(eh_next(&r, 0xc6336409, &next) == EH_INVALID, "mask_shape");
    r.mask = 0xffffff00;
    ECHECK(eh_next(&r, 0xc00002ff, &next) == EH_INVALID &&
               eh_next(&r, 0xc0000200, &next) == EH_INVALID &&
               eh_next(&r, 0xe0000001, &next) == EH_INVALID,
           "unicast_bounds");
    puts("OBS handoff_route local=c0000209 gateway=c0000201 final=c6336409 "
         "frame=60 unchanged_on_refusal");
    return 0;
}

static int clocks(void)
{
    uint32_t t[3] = {0, 5, 20};
    uint64_t low[3], high[3];
    struct eh_clock c = {0, 100, 98, 102};
    ECHECK(ed_capture(t, 3, 100, 1, 0, low, high) == ED_OK && low[0] == 100 &&
               low[1] == 105 && low[2] == 120,
           "constant_offset");
    ECHECK(ed_capture(t, 3, 100, 1, 2, low, high) == ED_OK && low[0] == 98 &&
               high[0] == 102 && low[2] == 118 && high[2] == 122,
           "anchor_uncertainty");
    t[1] = 20;
    t[2] = 5;
    ECHECK(ed_capture(t, 3, 100, 1, 0, low, high) == ED_INVALID &&
               low[2] == 118,
           "regression");
    t[0] = 0;
    t[1] = 1;
    t[2] = 2000000;
    ECHECK(eh_correlate(&c, t, 3, low, high) == EH_OK && low[0] == 98 &&
               high[0] == 102 && low[1] == 98 && high[1] == 104 &&
               low[2] == 1999898 && high[2] == 2000302,
           "drift_interval");
    c.anchor = 1;
    low[0] = 999;
    ECHECK(eh_correlate(&c, t, 3, low, high) == EH_INVALID && low[0] == 999,
           "before_anchor");
    c.anchor = 0;
    c.upper = UINT64_MAX;
    c.lower = UINT64_MAX - 1;
    t[0] = 1;
    ECHECK(eh_correlate(&c, t, 1, low, high) == EH_INVALID && low[0] == 999,
           "clock_overflow");
    c = (struct eh_clock){0, 1000, 0, 0};
    t[0] = UINT32_MAX;
    ECHECK(eh_correlate(&c, t, 1, low, high) == EH_OK &&
               low[0] == 4290672327ULL && high[0] == 4299262263ULL,
           "drift_extreme");
    puts("OBS handoff_clock fixed=100,105,120 interval=1999898,2000302 "
         "rounded=98,104 max=4290672327,4299262263");
    return 0;
}

static int divergence(void)
{
    unsigned first = 99;
    unsigned char p[40], f[100];
    size_t n;
    struct ed_frame v;
    struct eh_route r = {0xc0000202, 0xffffff00, 0};
    uint32_t next = 9;
    packet(p, r.local, 0xc6336409);
    ECHECK(ed_first_failure(0, &first) == ED_OK && first == 0, "first_link");
    ECHECK(ed_frame_build(local, local, 0x800, p, 40, 1, f, sizeof f, &n) ==
               ED_OK,
           "fixture");
    f[20] ^= 1;
    ECHECK(ed_frame_parse(f, n, 1, &v) == ED_IO, "first_frame");
    ECHECK(ed_first_failure(3, &first) == ED_OK && first == 2,
           "frame_precedes_route");
    f[20] ^= 1;
    ECHECK(ed_frame_parse(f, n, 1, &v) == ED_OK &&
               eh_next(&r, 0xc6336409, &next) == EH_NO_ROUTE,
           "next_route");
    ECHECK(ed_first_failure(7, &first) == ED_OK && first == 3,
           "route_after_frame");
    puts(
        "OBS handoff_divergence link=0 bad_fcs=2 repaired_frame_missing_route=3");
    return 0;
}

static int budget(void)
{
    struct ed_budget b;
    ECHECK(ed_budget(32, 100, &b) == ED_OK && b.ip == 72 && b.frame == 90 &&
               b.slot == 110 && b.nanoseconds == 8800,
           "ordinary_budget");
    ECHECK(ed_budget(0, 10, &b) == ED_OK && b.ip == 40 && b.frame == 64 &&
               b.slot == 84 && b.nanoseconds == 67200,
           "padding_budget");
    puts("OBS handoff_budget 72 90 110 8800 40 64 84 67200");
    return 0;
}

int eh_tests(unsigned n)
{
    switch (n)
    {
        case 0:
            return route();
        case 1:
            return clocks();
        case 2:
            return divergence();
        case 3:
            return budget();
        default:
            return 41;
    }
}
