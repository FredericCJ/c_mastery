#include "ledger.h"
void mf_emit(const char *, const uint32_t *, size_t);
int mf_fail(const char *, unsigned);
#define CHECK(x)                          \
    do                                    \
    {                                     \
        if (!(x))                         \
            return mf_fail(#x, __LINE__); \
    } while (0)

static int equal(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a, *y = b;
    for (size_t i = 0; i < n; ++i)
    {
        if (x[i] != y[i])
        {
            return 0;
        }
    }
    return 1;
}

static void poison(void *p, size_t n)
{
    unsigned char *q = p;
    for (size_t i = 0; i < n; ++i)
    {
        q[i] = 0xa5;
    }
}

static int reset(void)
{
    uint32_t table[256] = {0x20002000, 0x08000101};
    struct mf_boot c = {MF_FLASH,   5,          0x08000000, 0x20000000,
                        0x20030000, 0x08000080, 0x08000200};
    struct mf_reset r, old;
    CHECK(mf_reset(c, table, 256, &r) == MF_OK);
    CHECK(r.reset_fetch[0] == 0 && r.reset_fetch[1] == 4);
    CHECK(r.reset_backing[0] == 0x08000000 && r.initial_msp == 0x20002000 &&
          r.reset_pc == 0x08000100);
    CHECK(r.exception == 21 && r.irq_vector == 0x08000054);
    uint32_t packet[] = {c.boot,           c.vtor,
                         r.reset_fetch[0], r.reset_backing[0],
                         r.reset_pc,       r.irq_vector};
    mf_emit("reset", packet, 6);
    for (unsigned boot = 0; boot < 2; ++boot)
    {
        for (unsigned relocated = 0; relocated < 2; ++relocated)
        {
            uint32_t base = boot ? 0x20000000 : 0x08000000;
            c.boot = boot;
            c.vtor = relocated ? base : 0;
            c.code_low = base + 0x80;
            c.code_high = base + 0x200;
            table[1] = base + 0x101;
            CHECK(mf_reset(c, table, 256, &r) == MF_OK &&
                  r.reset_pc == base + 0x100 && r.reset_fetch[0] == 0);
            uint32_t p[] = {
                boot,       c.vtor,      r.reset_fetch[0], r.reset_backing[0],
                r.reset_pc, r.irq_vector};
            mf_emit("remap", p, 6);
        }
    }
    poison(&r, sizeof r);
    old = r;
    c.vtor = 0x08000000;
    CHECK(mf_reset(c, table, 256, &r) == MF_UNSUPPORTED &&
          equal(&r, &old, sizeof r));
    c.vtor = 0;
    for (size_t n = 0; n < 22; ++n)
    {
        CHECK(mf_reset(c, table, n, &r) == MF_SHORT &&
              equal(&r, &old, sizeof r));
    }
    table[1] &= ~1u;
    CHECK(mf_reset(c, table, 256, &r) == MF_STATE && equal(&r, &old, sizeof r));
    table[1] |= 1;
    table[0] = c.stack_low;
    CHECK(mf_reset(c, table, 256, &r) == MF_STATE);
    table[0] = 0x20002004;
    CHECK(mf_reset(c, table, 256, &r) == MF_STATE);
    table[0] = 0x20002000;
    table[1] = c.code_high | 1u;
    CHECK(mf_reset(c, table, 256, &r) == MF_STATE);
    c.irq = 240;
    CHECK(mf_reset(c, table, 256, &r) == MF_INVALID);
    return 0;
}

static int capture(void)
{
    uint32_t w[27] = {10, 20, 30, 40, 120, 0x08000111, 0x08000200, 0x01000000};
    struct xf_frame f, old;
    CHECK(mf_capture(w, 8, 0x20000fe0, 0xfffffffd, 0, 0, 0, 1, 0, &f) == MF_OK);
    CHECK(f.pc == 0x08000200 && f.stacked_lr == 0x08000111 &&
          f.stacked_lr != 0xfffffffd);
    CHECK(f.original_sp == 0x20001000 && f.hardware_words == 8 && f.thread &&
          f.psp);
    uint32_t p[] = {f.r[0],       f.r[1], f.r[2], f.r[3],       f.r12,
                    f.stacked_lr, f.pc,   f.xpsr, f.original_sp};
    mf_emit("fixture", p, 9);
    poison(&f, sizeof f);
    old = f;
    for (size_t n = 0; n < 8; ++n)
    {
        CHECK(mf_capture(w, n, 0x20000fe0, 0xfffffffd, 0, 0, 0, 1, 0, &f) ==
                  MF_SHORT &&
              equal(&f, &old, sizeof f));
    }
    w[7] |= 512;
    CHECK(mf_capture(w, 9, 0x20000fe0, 0xfffffffd, 0, 0, 0, 1, 0, &f) ==
              MF_UNSUPPORTED &&
          equal(&f, &old, sizeof f));
    CHECK(mf_capture(w, 9, 0x20000fe0, 0xfffffffd, 1, 0, 0, 1, 0, &f) ==
              MF_OK &&
          f.original_sp == 0x20001004);
    w[7] &= ~512u;
    CHECK(mf_capture(w, 27, 0x20000f98, 0xffffffed, 0, 1, 1, 1, 0, &f) ==
          MF_UNSUPPORTED);
    for (unsigned extended = 0; extended < 2; ++extended)
    {
        for (unsigned pad = 0; pad < 2; ++pad)
        {
            for (unsigned material = 0; material <= extended; ++material)
            {
                w[7] = 0x01000000u + pad * 512;
                for (unsigned i = 0; i < 16; ++i)
                {
                    w[8 + i] = 0x3f800000u + i;
                }
                w[24] = 0x01000000;
                unsigned size = extended ? 26 : 8;
                uint32_t sp = extended ? 0x20000f98 : 0x20000fe0,
                         er = extended ? 0xffffffed : 0xfffffffd;
                CHECK(mf_capture(w, size + pad, sp, er, 1, 1, material, 1, 0,
                                 &f) == MF_OK);
                CHECK(f.pc == w[6] && f.hardware_words == size &&
                      f.original_sp == sp + 4u * (size + pad) &&
                      f.fp_materialized == material);
                CHECK(f.fp[0] == (material ? w[8] : 0));
                uint32_t q[] = {extended, pad,           material,
                                f.pc,     f.original_sp, f.hardware_words,
                                f.fp[0]};
                mf_emit("extended", q, 7);
                poison(&f, sizeof f);
                old = f;
                CHECK(mf_capture(w, size + pad - 1, sp, er, 1, 1, material, 1,
                                 0, &f) == MF_SHORT &&
                      equal(&f, &old, sizeof f));
            }
        }
    }
    w[7] = 0;
    CHECK(mf_capture(w, 8, 0x20000fe0, 0xfffffffd, 1, 0, 0, 1, 0, &f) ==
              MF_STATE &&
          equal(&f, &old, sizeof f));
    CHECK(mf_capture(w, 0, 0, 0xfffffff5, 1, 0, 0, 1, 0, &f) ==
              MF_UNSUPPORTED &&
          equal(&f, &old, sizeof f));
    CHECK(mf_capture(w, 0, 0, 0xfffffffd, 1, 0, 0, 1, 1, &f) == MF_STATE &&
          equal(&f, &old, sizeof f));
    return 0;
}

static int ownership(void)
{
    struct mf_ownership r;
    for (unsigned r9 = 0; r9 < 2; ++r9)
    {
        CHECK(mf_ownership(r9, &r) == MF_OK);
        CHECK(r.hardware == 0x1d00f && !(r.hardware & (1u << 4)));
        CHECK(r.callee_preserved == (0x2df0u | (r9 ? 0x200u : 0u)) &&
              r.call_clobbered == 0x1500f && r.r9_platform == !r9);
        uint32_t p[] = {r9, r.hardware, r.callee_preserved, r.call_clobbered,
                        r.r9_platform};
        mf_emit("ownership", p, 5);
    }
    CHECK(mf_ownership(2, &r) == MF_INVALID);
    return 0;
}

static int nested(void)
{
    uint32_t a[8] = {10, 20, 30, 40, 120, 0x08000111, 0x08000200, 0x01000000};
    uint32_t b[8] = {101, 102,        103,        104,
                     112, 0xfffffffd, 0x08000300, 0x01000010};
    struct xf_frame outer, inner;
    struct mf_return_order r, old;
    struct xf_priority_config config = {4, 3};
    CHECK(mf_capture(a, 8, 0x20000fe0, 0xfffffffd, 1, 0, 0, 1, 0, &outer) ==
              MF_OK &&
          mf_capture(b, 8, 0x20002fe0, 0xfffffff1, 1, 0, 0, 1, 0, &inner) ==
              MF_OK);
    CHECK(mf_nested(&outer, 0x20000fe0, 16, &inner, 0x20002fe0, config, 0x60,
                    0x50, &r) == MF_OK);
    CHECK(r.first_stack == 0 && r.second_stack == 1 &&
          r.first_pc == 0x08000300 && r.second_pc == 0x08000200);
    CHECK(r.first_sp == 0x20003000 && r.second_sp == 0x20001000 &&
          inner.stacked_lr == 0xfffffffd && outer.stacked_lr == 0x08000111);
    uint32_t p[] = {r.first_stack,    r.first_pc,      r.first_sp,
                    r.second_stack,   r.second_pc,     r.second_sp,
                    inner.stacked_lr, outer.stacked_lr};
    mf_emit("nested", p, 8);
    poison(&r, sizeof r);
    old = r;
    config.grouping = 6;
    CHECK(mf_nested(&outer, 0x20000fe0, 16, &inner, 0x20002fe0, config, 0x60,
                    0x50, &r) == MF_STATE &&
          equal(&r, &old, sizeof r));
    config.grouping = 3;
    CHECK(mf_nested(&outer, 0x20000fe0, 17, &inner, 0x20002fe0, config, 0x60,
                    0x50, &r) == MF_STATE);
    inner.psp = 1;
    CHECK(mf_nested(&outer, 0x20000fe0, 16, &inner, 0x20002fe0, config, 0x60,
                    0x50, &r) == MF_STATE);
    return 0;
}

int mf_run(unsigned which)
{
    switch (which)
    {
        case 0:
            return reset();
        case 1:
            return capture();
        case 2:
            return ownership();
        case 3:
            return nested();
        default:
            return 1;
    }
}
