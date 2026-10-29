#include "variant.h"
#include "http_depth.h"
#include "systems.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "NV_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int checksum(void)
{
    unsigned char bytes[33];
    for (unsigned i = 0; i < 33; i++)
    {
        bytes[i] = (unsigned char)(i * 83 + 1);
    }
    unsigned cases = 0;
    for (size_t n = 0; n <= 33; n++)
    {
        for (size_t a = 0; a <= n; a++)
        {
            for (size_t b = a; b <= n; b++)
            {
                struct nv_sum x, y, z, ab, bc, left, right, whole;
                CHECK(!nv_sum(bytes, a, &x) && !nv_sum(bytes + a, b - a, &y) &&
                      !nv_sum(bytes + b, n - b, &z));
                CHECK(!nv_combine(x, y, &ab) && !nv_combine(ab, z, &left));
                CHECK(!nv_combine(y, z, &bc) && !nv_combine(x, bc, &right));
                CHECK(!nv_sum(bytes, n, &whole));
                CHECK(left.sum == whole.sum && right.sum == whole.sum &&
                      left.odd == whole.odd && right.odd == whole.odd);
                cases++;
            }
        }
    }
    const unsigned char allones[] = {255, 255, 255, 255}, zero[] = {0, 0};
    struct nv_sum s;
    CHECK(!nv_sum(allones, 4, &s) && s.sum == 65535);
    CHECK(!nv_sum(zero, 2, &s) && s.sum == 0);
    s = (struct nv_sum){19, 1};
    CHECK(nv_combine((struct nv_sum){0, 2}, s, &s) == NV_INVALID &&
          s.sum == 19 && s.odd == 1);
    CHECK(!nv_sum(0, 0, &s) && s.sum == 0 && s.odd == 0);
    struct s_checksum streaming = {0, 0, 0};
    uint16_t first, last;
    const unsigned char pair[] = {1, 2};
    CHECK(!s_checksum_add(&streaming, pair, 1));
    struct s_checksum saved = streaming;
    CHECK(!s_checksum_finish(&streaming, &first) && first == 0xfeff);
    CHECK(streaming.sum == saved.sum && streaming.pending == saved.pending &&
          streaming.octet == saved.octet);
    CHECK(!s_checksum_finish(&streaming, &last) && last == first);
    CHECK(!s_checksum_add(&streaming, pair + 1, 1) &&
          !s_checksum_finish(&streaming, &last) && last == 0xfefd);
    /* IPv4 TCP pseudoheader + synthetic 20-byte TCP header + odd payload. */
    unsigned char packet[37] = {192, 0,  2,    1,    198,  51,   100,  2, 0, 6,
                                0,   25, 0x12, 0x34, 0,    80,   0,    0, 0, 1,
                                0,   0,  0,    0,    0x50, 0x02, 0x10, 0, 0, 0,
                                0,   0,  1,    2,    3,    4,    5};
    struct nv_sum a, b, c, all;
    CHECK(!nv_sum(packet, 12, &a) && !nv_sum(packet + 12, 20, &b) &&
          !nv_sum(packet + 32, 5, &c));
    CHECK(!nv_combine(a, b, &a) && !nv_combine(a, c, &a) &&
          !nv_sum(packet, 37, &all) && a.sum == all.sum);
    printf("NV_OK:checksum:%u:pseudoheader=%u\n", cases,
           (unsigned)(uint16_t)~a.sum);
    return 0;
}

static int length(void)
{
    uint32_t value = 77;
    CHECK(nv_length((const unsigned char *)"9", 1, 0, 1024, &value) == 400 &&
          value == 77);
    CHECK(!nv_length((const unsigned char *)"000", 3, 0, 0, &value) &&
          value == 0);
    value = 77;
    CHECK(nv_length((const unsigned char *)"1025x", 5, UINT32_MAX, 1024,
                    &value) == 400 &&
          value == 77);
    CHECK(nv_length((const unsigned char *)"4294967296", 10, UINT32_MAX, 1024,
                    &value) == 400);
    CHECK(nv_length((const unsigned char *)"4294967295", 10, UINT32_MAX, 1024,
                    &value) == 413);
    CHECK(!nv_length((const unsigned char *)"7", 1, 7, 8, &value) &&
          value == 7);
    CHECK(nv_length((const unsigned char *)"8", 1, 7, 8, &value) == 400 &&
          value == 7);
    puts("NV_OK:length");
    return 0;
}

static int date(void)
{
    char a[30], b[30];
    size_t n = 77, m = 0;
    unsigned cases = 0;
    for (uint32_t d = 10957; d < 10957 + 146097; d++)
    {
        CHECK(!nv_date(1, d, 86399, a, sizeof a, &n) &&
              !hd_date(1, d, 86399, b, sizeof b, &m));
        CHECK(n == 29 && m == 29 && !memcmp(a, b, 30));
        cases++;
    }
    CHECK(!nv_date(1, 0, 0, a, 30, &n) &&
          !strcmp(a, "Thu, 01 Jan 1970 00:00:00 GMT"));
    CHECK(!nv_date(1, 2932896, 86399, a, 30, &n) &&
          !strcmp(a, "Fri, 31 Dec 9999 23:59:59 GMT"));
    a[0] = '!';
    n = 77;
    CHECK(nv_date(1, 2932897, 0, a, 30, &n) == NV_INVALID && a[0] == '!' &&
          n == 77);
    CHECK(!nv_date(0, UINT32_MAX, UINT32_MAX, 0, 0, &n) && n == 0);
    struct nv_stamp s = {1, 1, NV_POSITIVE_LEAP, 17166, 86400};
    CHECK(!nv_stamp(s, 17166, a, 30, &n) &&
          !strcmp(a, "Sat, 31 Dec 2016 23:59:60 GMT"));
    a[0] = '!';
    n = 77;
    CHECK(nv_stamp(s, UINT32_MAX, a, 30, &n) == NV_INVALID && a[0] == '!' &&
          n == 77);
    s.scale = NV_ORDINARY_UTC;
    CHECK(nv_stamp(s, 17166, a, 30, &n) == NV_INVALID);
    s.scale = NV_TAI;
    s.seconds = 0;
    CHECK(nv_stamp(s, 17166, a, 30, &n) == NV_INVALID);
    s.scale = NV_SMEARED;
    CHECK(nv_stamp(s, 17166, a, 30, &n) == NV_INVALID);
    s.trusted = 0;
    CHECK(!nv_stamp(s, UINT32_MAX, a, 0, &n) && n == 0 && a[0] == '!');
    printf("NV_OK:date:%u:complete-era-and-leap-authority\n", cases);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "checksum"))
    {
        return checksum();
    }
    if (!strcmp(argv[1], "length"))
    {
        return length();
    }
    if (!strcmp(argv[1], "date"))
    {
        return date();
    }
    return 2;
}
