#include "frame.h"
#if defined(XF_STANDALONE)
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
            printf("XF_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                 \
        }                                             \
    } while (0)

int main(void)
{
    uint32_t words[27] = {1, 2, 3, 4, 12, 0x08000201u, 0x08000120u, 1u << 24};
    struct xf_frame f = {0};
    CHECK(xf_frame(words, 8, 0x20000000u, 0xfffffffdu, 1, 0, 1, 0, &f) ==
          XF_OK);
    CHECK(f.pc == 0x08000120u && f.stacked_lr == 0x08000201u && f.psp &&
          f.thread && !f.extended && f.original_sp == 0x20000020u);
    for (unsigned n = 0; n < 8; ++n)
    {
        CHECK(xf_frame(words, n, 0x20000000u, 0xfffffffdu, 1, 0, 1, 0, &f) ==
              XF_SHORT);
    }
    CHECK(f.pc == 0x08000120u && f.original_sp == 0x20000020u);
    for (unsigned low = 0; low < 32; ++low)
    {
        uint32_t er = 0xffffffe0u + low;
        unsigned valid = low == 1 || low == 9 || low == 13 || low == 17 ||
                         low == 25 || low == 29;
        words[7] = (1u << 24) | ((low & 8) ? 0u : 16u);
        int rc = xf_frame(words, 27, 0x20000000u, er, 1, 0, 1, 0, &f);
        CHECK(valid ? rc == XF_OK : rc == XF_UNSUPPORTED);
    }
    words[7] = (1u << 24) | (1u << 9);
    CHECK(xf_frame(words, 8, 0x20000000u, 0xfffffffdu, 1, 0, 1, 0, &f) ==
          XF_SHORT);
    CHECK(xf_frame(words, 9, 0x20000000u, 0xfffffffdu, 1, 0, 1, 0, &f) ==
              XF_OK &&
          f.alignment_word && f.original_sp == 0x20000024u);
    for (unsigned i = 0; i < 16; ++i)
    {
        words[8 + i] = 0x3f800000u + i;
    }
    words[24] = 0x01000000u;
    words[25] = 0xdeadbeefu;
    CHECK(xf_frame(words, 26, 0x20000000u, 0xffffffedu, 1, 1, 1, 0, &f) ==
          XF_SHORT);
    CHECK(xf_frame(words, 27, 0x20000000u, 0xffffffedu, 1, 1, 1, 0, &f) ==
              XF_OK &&
          f.pc == 0x08000120u && f.fp[15] == 0x3f80000fu &&
          f.fpscr == 0x01000000u && f.original_sp == 0x2000006cu);
    CHECK(xf_frame(words, 27, 0x20000000u, 0xffffffedu, 1, 0, 1, 0, &f) ==
              XF_OK &&
          !f.fp_materialized && !f.fp[0]);
    CHECK(xf_frame(words, 27, 0x20000000u, 0xffffffedu, 0, 0, 1, 0, &f) ==
          XF_UNSUPPORTED);
    CHECK(xf_frame(words, 0, 0x20000000u, 0xfffffffdu, 1, 0, 1, 1, &f) ==
          XF_FRAME);
    CHECK(xf_frame(words, 27, 0xfffffff8u, 0xfffffffdu, 1, 0, 1, 0, &f) ==
          XF_FRAME);
    words[7] = 0;
    CHECK(xf_frame(words, 27, 0x20000000u, 0xfffffffdu, 1, 0, 1, 0, &f) ==
          XF_FRAME);
    words[7] = 1u << 24;
    words[6] |= 1;
    CHECK(xf_frame(words, 27, 0x20000000u, 0xfffffffdu, 1, 0, 1, 0, &f) ==
          XF_FRAME);
    words[6] &= ~1u;
    struct xf_priority_config cfg = {4, 5};
    struct xf_priority q;
    unsigned chosen = 99, allowed = 99;
    CHECK(xf_priority(cfg, 0x7f, &q) == XF_OK && q.encoded == 0x70 &&
          q.logical == 7 && q.group == 1 && q.sub == 3);
    struct xf_irq irq[3] = {
        {16, 0x60, 1, 0, 0, 0}, {17, 0x20, 0, 1, 0, 0}, {18, 0x50, 1, 1, 0, 0}};
    CHECK(xf_pick(cfg, irq, 3, 0x60, 0, 0, &chosen) == XF_NONE &&
          chosen == 99); /* subpriority cannot preempt */
    irq[1].enabled = 1;
    CHECK(xf_pick(cfg, irq, 3, 0x60, 0, 0, &chosen) == XF_OK && chosen == 17);
    CHECK(xf_pick(cfg, irq, 3, -1, 1, 0, &chosen) == XF_NONE);
    CHECK(xf_pick(cfg, irq, 3, -1, 0, 0x20, &chosen) == XF_NONE);
    irq[1].active = 1;
    CHECK(xf_pick(cfg, irq, 3, -1, 0, 0, &chosen) == XF_NONE);
    irq[1].active = 0;
    CHECK(xf_source(&irq[0], 1) == XF_OK && irq[0].pending);
    CHECK(xf_source(&irq[0], 0) == XF_OK && irq[0].pending);
    CHECK(xf_pend(&irq[0], 0) == XF_OK && !irq[0].pending);
    cfg.grouping = 3;
    CHECK(xf_pick(cfg, irq, 3, 0x60, 0, 0x40, &chosen) == XF_OK &&
          chosen == 17);
    CHECK(xf_api_allowed(cfg, 0, 0x50, &allowed) == XF_OK && !allowed);
    CHECK(xf_api_allowed(cfg, 0x60, 0x50, &allowed) == XF_OK && allowed);
    CHECK(xf_api_allowed(cfg, 0x60, 0, &allowed) == XF_INVALID);
    CHECK(xf_api_allowed(cfg, 0x60, 0x51, &allowed) == XF_INVALID);
    cfg.grouping = 4;
    CHECK(xf_api_allowed(cfg, 0x60, 0x40, &allowed) == XF_INVALID);
    cfg = (struct xf_priority_config){8, 0};
    CHECK(xf_api_allowed(cfg, 4, 5, &allowed) == XF_INVALID);
    CHECK(xf_api_allowed(cfg, 4, 4, &allowed) == XF_OK && allowed);
    for (unsigned bits = 3; bits <= 8; ++bits)
    {
        for (unsigned grouping = 0; grouping <= 7; ++grouping)
        {
            for (unsigned raw = 0; raw < 256; ++raw)
            {
                cfg = (struct xf_priority_config){bits, grouping};
                CHECK(xf_priority(cfg, raw, &q) == XF_OK);
                unsigned unit = 1u << (8 - bits), clean = raw - raw % unit,
                         block = 1u << (grouping + 1);
                CHECK(q.encoded == clean &&
                      q.group == clean / (block > unit ? block : unit) &&
                      q.sub == (clean % block) / unit);
                unsigned logical = raw / unit;
                uint8_t normal, opposite;
                CHECK(xf_urgency(cfg, logical, 0, &normal) == XF_OK &&
                      xf_urgency(cfg, logical, 1, &opposite) == XF_OK &&
                      (unsigned)normal + opposite == 256 - unit);
            }
        }
    }
    puts("XF_PASS priority_cells=12288");
    return 0;
}
