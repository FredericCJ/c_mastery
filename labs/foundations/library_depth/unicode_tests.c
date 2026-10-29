#include "unicode.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "UNICODE-CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                               \
        }                                                           \
    } while (0)

static int same(struct lu_state a, struct lu_state b)
{
    return a.used == b.used && !memcmp(a.bytes, b.bytes, 4);
}

int main(void)
{
    const unsigned char euro[] = {0xe2, 0x82, 0xac},
                        face[] = {0xf0, 0x9f, 0x98, 0x80};
    struct lu_state s = {0};
    struct lu_result r = {99, 99};
    uint32_t out[8] = {99, 99, 99, 99, 99, 99, 99, 99};
    CHECK(lu_decode(&s, euro, 1, out, 8, LU_STRICT, &r) == LU_INCOMPLETE &&
          r.consumed == 1 && r.produced == 0 && s.used == 1 && out[0] == 99);
    CHECK(lu_decode(&s, euro + 1, 2, out, 8, LU_STRICT, &r) == LU_OK &&
          r.consumed == 2 && r.produced == 1 && out[0] == 0x20ac &&
          s.used == 0);
    for (size_t cut = 1; cut < 4; ++cut)
    {
        struct lu_state a = {0}, b = {0};
        CHECK(lu_decode(&a, face, cut, out, 8, LU_STRICT, &r) == LU_INCOMPLETE);
        CHECK(lu_decode(&b, euro, 1, out, 8, LU_STRICT, &r) == LU_INCOMPLETE);
        CHECK(lu_decode(&a, face + cut, 4 - cut, out, 8, LU_STRICT, &r) ==
                  LU_OK &&
              out[0] == 0x1f600 && r.consumed == 4 - cut);
        CHECK(lu_decode(&b, euro + 1, 2, out, 8, LU_STRICT, &r) == LU_OK &&
              out[0] == 0x20ac);
    }
    const unsigned char bad[][4] = {
        {0xc0, 0xaf},       {0xed, 0xa0, 0x80},       {0xf4, 0x90, 0x80, 0x80},
        {0xe0, 0x80, 0x80}, {0xf0, 0x80, 0x80, 0x80}, {0xff}};
    const size_t sizes[] = {2, 3, 4, 3, 4, 1};
    for (unsigned i = 0; i < 6; ++i)
    {
        s = (struct lu_state){0};
        struct lu_state before = s;
        r = (struct lu_result){99, 99};
        out[0] = 99;
        CHECK(lu_decode(&s, bad[i], sizes[i], out, 8, LU_STRICT, &r) ==
                  LU_INVALID &&
              same(s, before) && r.consumed == 99 && out[0] == 99);
        CHECK(lu_decode(&s, bad[i], sizes[i], out, 8, LU_REPLACE, &r) ==
                  LU_OK &&
              r.produced == sizes[i]);
        for (size_t j = 0; j < r.produced; ++j)
        {
            CHECK(out[j] == 0xfffd);
        }
    }
    const unsigned char mixed[] = {0x41, 0xe2, 0x42};
    s = (struct lu_state){0};
    r = (struct lu_result){99, 99};
    out[0] = 99;
    CHECK(lu_decode(&s, mixed, 3, out, 8, LU_STRICT, &r) == LU_INVALID &&
          s.used == 0 && out[0] == 99 && r.produced == 99);
    CHECK(lu_decode(&s, mixed, 3, NULL, 0, LU_STRICT, &r) == LU_INVALID &&
          s.used == 0 && r.produced == 99);
    CHECK(lu_decode(&s, mixed, 3, out, 8, LU_REPLACE, &r) == LU_OK &&
          r.produced == 3 && out[0] == 0x41 && out[1] == 0xfffd &&
          out[2] == 0x42);
    const unsigned char impossible[] = {0xed, 0xa0};
    s = (struct lu_state){0};
    CHECK(lu_decode(&s, impossible, 2, out, 8, LU_STRICT, &r) == LU_INVALID &&
          s.used == 0);
    s = (struct lu_state){0};
    CHECK(lu_decode(&s, face, 2, out, 8, LU_STRICT, &r) == LU_INCOMPLETE);
    struct lu_state before = s;
    r = (struct lu_result){99, 99};
    out[0] = 99;
    CHECK(lu_decode(&s, face + 2, 2, NULL, 0, LU_STRICT, &r) == LU_FULL &&
          same(s, before) && out[0] == 99 && r.consumed == 99);
    CHECK(lu_finish(&s, out, 8, LU_STRICT, &r) == LU_INVALID &&
          same(s, before));
    CHECK(lu_finish(&s, NULL, 0, LU_REPLACE, &r) == LU_FULL && same(s, before));
    CHECK(lu_finish(&s, out, 8, LU_REPLACE, &r) == LU_OK && out[0] == 0xfffd &&
          r.consumed == 0 && r.produced == 1 && s.used == 0);
    CHECK(lu_decode(&s, NULL, 0, NULL, 0, LU_STRICT, &r) == LU_OK &&
          r.consumed == 0 && r.produced == 0);
    CHECK(lu_decode(&s, NULL, 1, out, 8, LU_STRICT, &r) == LU_ARGUMENT);
    const uint32_t points[] = {0, 0x41, 0x20ac, 0x1f600, 0x10ffff};
    const unsigned char wire[][4] = {{0},
                                     {0x41},
                                     {0xe2, 0x82, 0xac},
                                     {0xf0, 0x9f, 0x98, 0x80},
                                     {0xf4, 0x8f, 0xbf, 0xbf}};
    const size_t lens[] = {1, 1, 3, 4, 4};
    for (unsigned i = 0; i < 5; ++i)
    {
        for (size_t cap = 0; cap <= 5; ++cap)
        {
            unsigned char bytes[6] = {99, 99, 99, 99, 99, 99};
            size_t n = 99;
            enum lu_status st =
                lu_encode(points[i], cap ? bytes : NULL, cap, &n);
            if (cap < lens[i])
            {
                CHECK(st == LU_FULL && n == 99);
                for (unsigned j = 0; j < 6; ++j)
                {
                    CHECK(bytes[j] == 99);
                }
            }
            else
            {
                CHECK(st == LU_OK && n == lens[i] &&
                      !memcmp(bytes, wire[i], n) && bytes[n] == 99);
            }
        }
    }
    uint16_t units[3] = {99, 99, 99};
    size_t n = 99;
    CHECK(lu_utf16(0x1f600, units, 1, &n) == LU_FULL && units[0] == 99 &&
          n == 99);
    CHECK(lu_utf16(0x1f600, units, 2, &n) == LU_OK && n == 2 &&
          units[0] == 0xd83d && units[1] == 0xde00 && units[2] == 99);
    CHECK(lu_utf16(0x20ac, units, 2, &n) == LU_OK && n == 1 &&
          units[0] == 0x20ac);
    for (uint32_t u = 0xd800; u <= 0xdfff; ++u)
    {
        unsigned char bytes[4] = {99};
        n = 99;
        CHECK(lu_encode(u, bytes, 4, &n) == LU_INVALID && n == 99 &&
              bytes[0] == 99);
        CHECK(lu_utf16(u, units, 2, &n) == LU_INVALID && n == 99);
    }
    CHECK(lu_utf16(0x110000, units, 2, &n) == LU_INVALID);
    puts("UNICODE-PASS:chunks:rollback:replacement:encoding:utf16");
    return 0;
}
