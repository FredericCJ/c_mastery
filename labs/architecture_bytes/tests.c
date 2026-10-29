#include "bytes.h"
#include <limits.h>
#define CHECK(x)                      \
    do                                \
    {                                 \
        if (!(x))                     \
        {                             \
            ab_failure(#x, __LINE__); \
            return 41;                \
        }                             \
    } while (0)

static int equal(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a, *y = b;
    for (size_t i = 0; i < n; ++i)
        if (x[i] != y[i])
            return 0;
    return 1;
}

static int name(const char *a, const char *b)
{
    while (*a && *a == *b)
    {
        ++a;
        ++b;
    }
    return *a == *b;
}

static int registers(void)
{
    uint64_t old = UINT64_C(0x1122334455667788), v = 0, actual[3] = {0};
    unsigned widths[] = {1, 2, 4};
    uint32_t input[] = {0xaa, 0xbbcc, 0x89abcdef};
    uint64_t want[] = {UINT64_C(0x11223344556677aa),
                       UINT64_C(0x112233445566bbcc), UINT64_C(0x89abcdef)};
    for (unsigned i = 0; i < 3; ++i)
    {
        CHECK(ab_write(old, widths[i], input[i], &v) == AB_OK && v == want[i]);
        old = v;
    }
#ifndef AB_ARM
    ab_registers(actual);
    CHECK(equal(actual, want, sizeof(want)));
    for (unsigned kind = 0; kind < 2; ++kind)
        for (unsigned count = 0; count < 8; ++count)
            for (unsigned add = 0; add < 256; ++add)
            {
                uint64_t got = ab_dependency(kind, UINT64_C(0x1122334455667788),
                                             count, add);
                CHECK(got == (count ? ((17 + add) & 255)
                                    : UINT64_C(0x1122334455667788)));
            }
#else
    CHECK(ab_thumb_register() == 0xaa);
    for (unsigned i = 0; i < 3; ++i)
        actual[i] = want[i];
#endif
    ab_emit("registers", actual, 3);
    v = 99;
    CHECK(ab_write(0, 3, 1, &v) == AB_INVALID && v == 99);
    CHECK(ab_write(0, 1, 256, &v) == AB_INVALID && v == 99);
    CHECK(ab_write(0, 2, 65536, &v) == AB_INVALID && v == 99);
    CHECK(ab_write(0, 4, 0, 0) == AB_INVALID);
    return 0;
}

static int bytes(void)
{
    const unsigned char p[] = {0x78, 0x56, 0x34, 0x12};
    uint32_t word = 99;
    CHECK(ab_word(p, 4, 0, &word) == AB_OK && word == 0x12345678);
    CHECK(ab_word(p, 4, 1, &word) == AB_OK && word == 0x78563412);
    for (unsigned n = 0; n < 4; ++n)
    {
        word = 99;
        CHECK(ab_word(p, n, 0, &word) == AB_TRUNCATED && word == 99);
    }
    CHECK(ab_word(0, 4, 0, &word) == AB_INVALID);
    CHECK(ab_word(p, 4, 2, &word) == AB_INVALID);
    uint64_t start = 99, end = 88;
    CHECK(ab_address(0x1000, 3, 4, 8, 4, 32, &start, &end) == AB_OK &&
          start == 0x1014 && end == 0x1018);
    CHECK(ab_address(0x1000, 3, 4, 8, 4, 23, &start, &end) == AB_RANGE &&
          start == 0x1014 && end == 0x1018);
    CHECK(ab_address(0, UINT64_MAX, 2, 0, 1, UINT64_MAX, &start, &end) ==
          AB_RANGE);
    CHECK(ab_address(0, 0, 1, INT64_MIN, 1, 32, &start, &end) == AB_RANGE);
    CHECK(ab_address(UINT64_MAX - 1, 0, 1, 0, 2, 2, &start, &end) == AB_RANGE);
    CHECK(ab_address(0, 0, 1, 0, 1, 1, &start, &start) == AB_INVALID);
    for (unsigned scale = 1; scale <= 8; scale *= 2)
        for (unsigned index = 0; index < 9; ++index)
            for (int disp = -9; disp <= 9; ++disp)
                for (unsigned width = 1; width <= 8; ++width)
                {
                    int pos = (int)(scale * index) + disp;
                    unsigned admit =
                        pos >= 0 && pos <= 32 && width <= 32 - (unsigned)pos;
                    start = 99;
                    end = 88;
                    enum ab_status s = ab_address(0x1000, index, scale, disp,
                                                  width, 32, &start, &end);
                    CHECK(s == (admit ? AB_OK : AB_RANGE));
                    CHECK(admit ? (start == 0x1000u + (unsigned)pos &&
                                   end == start + width)
                                : (start == 99 && end == 88));
                    uint64_t row[] = {scale, index, (uint64_t)(int64_t)disp,
                                      width, s,     start,
                                      end};
                    ab_emit("address", row, 7);
                }
    return 0;
}

static int storage(void)
{
    int32_t values[] = {INT32_MIN, -1, 0, 1, INT32_MAX};
    struct aw_record pool[2] = {{0, 0}, {0, 0}};
    unsigned char wire[8];
    for (unsigned a = 0; a < 5; ++a)
        for (unsigned b = 0; b < 5; ++b)
            for (unsigned big = 0; big < 2; ++big)
            {
                pool[0] = (struct aw_record){values[a], values[b]};
                CHECK(ab_record_encode(pool, big, wire, 8) == AB_OK);
                pool[1] = (struct aw_record){77, 88};
                CHECK(ab_record_decode(wire, 8, big, pool + 1) == AB_OK &&
                      pool[1].id == pool[0].id && pool[1].rank == pool[0].rank);
                CHECK(ab_record_copy(pool + 1, pool) == AB_OK);
                CHECK(ab_record_copy(pool, pool) == AB_OK);
                uint64_t row[] = {a,       b,       big,     wire[0],
                                  wire[1], wire[2], wire[3], wire[4],
                                  wire[5], wire[6], wire[7]};
                ab_emit("storage", row, 11);
            }
    pool[1] = (struct aw_record){77, 88};
    for (unsigned n = 0; n < 8; ++n)
        CHECK(ab_record_decode(wire, n, 0, pool + 1) == AB_TRUNCATED &&
              pool[1].id == 77 && pool[1].rank == 88);
    CHECK(ab_record_decode(wire, 8, 2, pool + 1) == AB_INVALID &&
          pool[1].id == 77);
    for (unsigned i = 0; i < 8; ++i)
        wire[i] = 0xa5;
    CHECK(ab_record_encode(pool, 0, wire, 7) == AB_TRUNCATED);
    for (unsigned i = 0; i < 8; ++i)
        CHECK(wire[i] == 0xa5);
    CHECK(ab_record_copy(0, pool) == AB_INVALID);
    return 0;
}

static int decode(void)
{
    struct ab_instruction out = {0}, old = {0};
    old.target = 123;
    old.length = 77;
    const unsigned char cmp[] = {0x48, 0x3d, 255, 255, 255, 255};
    CHECK(ab_x86(cmp, 6, 0, 0, &out) == AB_OK && out.length == 6 &&
          out.operand_bits == 64 && out.signed_immediate == -1 &&
          out.immediate == UINT32_MAX);
    for (unsigned n = 0; n < 6; ++n)
    {
        out = old;
        CHECK(ab_x86(cmp, n, 0, 0, &out) == AB_TRUNCATED &&
              equal(&out, &old, sizeof(out)));
    }
    const unsigned char unsupported[] = {0x66, 0x3d, 0, 0},
                        ext[] = {0x3c, 0x7f};
    out = old;
    CHECK(ab_x86(unsupported, 4, 0, 1, &out) == AB_UNSUPPORTED &&
          equal(&out, &old, sizeof(out)));
    CHECK(ab_x86(ext, 2, 0, 0, &out) == AB_UNSUPPORTED &&
          equal(&out, &old, sizeof(out)));
    CHECK(ab_x86(ext, 2, 0, 1, &out) == AB_OK && out.operand_bits == 8 &&
          out.immediate == 127);
    out = old;
    CHECK(ab_x86(ext, 1, 0, 1, &out) == AB_TRUNCATED &&
          equal(&out, &old, sizeof(out)));
    unsigned char p[4] = {0};
    unsigned counts[3] = {0};
    for (unsigned h = 0; h < 65536; ++h)
    {
        unsigned len = (h >> 11) >= 29 ? 4 : 2;
        CHECK(ab_thumb_length((uint16_t)h) == len);
        p[0] = (unsigned char)h;
        p[1] = (unsigned char)(h >> 8);
        out = old;
        enum ab_status status = ab_thumb(p, 2, 0x2000, 0, &out);
        if (len == 4)
        {
            CHECK(status == AB_TRUNCATED && equal(&out, &old, sizeof(out)));
            ++counts[0];
            CHECK(ab_thumb(p, 4, 0x2000, 0, &out) == AB_UNSUPPORTED &&
                  equal(&out, &old, sizeof(out)));
        }
        else if ((h & 0xf800) == 0x2800)
        {
            CHECK(status == AB_OK && out.kind == AB_CMP && out.length == 2 &&
                  out.operand_bits == 32 && out.reg == ((h >> 8) & 7) &&
                  out.immediate == (h & 255));
            ++counts[1];
            uint64_t row[] = {h, out.reg, out.immediate};
            ab_emit("thumbcmp", row, 3);
        }
        else
        {
            CHECK(status == AB_UNSUPPORTED && equal(&out, &old, sizeof(out)));
            ++counts[2];
        }
    }
    uint64_t row[] = {counts[0], counts[1], counts[2]};
    ab_emit("widths", row, 3);
    CHECK(counts[0] == 6144 && counts[1] == 2048 && counts[2] == 57344);
    out = old;
    CHECK(ab_thumb(p, 1, 0, 0, &out) == AB_TRUNCATED &&
          equal(&out, &old, sizeof(out)));
    CHECK(ab_thumb(p, 4, 1, 0, &out) == AB_INVALID &&
          equal(&out, &old, sizeof(out)));
    return 0;
}

static int branches(void)
{
    unsigned char p[] = {0x77, 0};
    struct ab_instruction out = {0}, old = {0};
    old.target = 123;
    for (unsigned base = 0; base < 3; ++base)
        for (unsigned imm = 0; imm < 256; ++imm)
        {
            uint64_t pc = 0x1006 + base * 0x1000;
            p[1] = (unsigned char)imm;
            CHECK(ab_x86(p, 2, pc, 0, &out) == AB_OK);
            int delta = imm < 128 ? (int)imm : (int)imm - 256;
            CHECK(out.target == pc + 2 + delta);
            uint64_t row[] = {pc, imm, out.target};
            ab_emit("xbranch", row, 3);
        }
    out = old;
    p[1] = 128;
    CHECK(ab_x86(p, 2, 0, 0, &out) == AB_RANGE &&
          equal(&out, &old, sizeof(out)));
    CHECK(ab_x86(p, 2, UINT64_MAX, 0, &out) == AB_RANGE &&
          equal(&out, &old, sizeof(out)));
    for (unsigned base = 0; base < 2; ++base)
        for (unsigned cond = 0; cond < 14; ++cond)
            for (unsigned imm = 0; imm < 256; ++imm)
            {
                uint32_t pc = 0x2000 + base * 0x1000;
                p[0] = (unsigned char)imm;
                p[1] = (unsigned char)(0xd0 + cond);
                CHECK(ab_thumb(p, 2, pc, 1, &out) == AB_OK);
                int delta = imm < 128 ? (int)imm : (int)imm - 256;
                CHECK(out.target == pc + 4 + delta * 2 &&
                      out.condition == cond);
                uint64_t row[] = {pc, cond, imm, out.target};
                ab_emit("tbranch", row, 4);
            }
    p[0] = 0;
    p[1] = 0xde;
    out = old;
    CHECK(ab_thumb(p, 2, 0, 1, &out) == AB_UNSUPPORTED &&
          equal(&out, &old, sizeof(out)));
    p[1] = 0xdf;
    CHECK(ab_thumb(p, 2, 0, 1, &out) == AB_UNSUPPORTED &&
          equal(&out, &old, sizeof(out)));
    p[1] = 0xd1;
    CHECK(ab_thumb(p, 2, 0, 0, &out) == AB_UNSUPPORTED &&
          equal(&out, &old, sizeof(out)));
    return 0;
}

int ab_test(const char *s)
{
    if (name(s, "registers"))
        return registers();
    if (name(s, "bytes"))
        return bytes();
    if (name(s, "storage"))
        return storage();
    if (name(s, "decode"))
        return decode();
    if (name(s, "branches"))
        return branches();
    return 42;
}
