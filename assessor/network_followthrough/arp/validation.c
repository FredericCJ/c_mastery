#include "arp.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                     \
    do                               \
    {                                \
        if (!(x))                    \
        {                            \
            puts("NA_VALIDATE " #x); \
            return 1;                \
        }                            \
    } while (0)
static const struct na_config config = {{2, 0, 0, 0, 0, 2},
                                        {192, 0, 2, 2},
                                        {192, 0, 2, 1}};

static void packet(unsigned char p[60])
{
    const unsigned char a[42] = {2,   0, 0, 0, 0, 2, 2, 0, 0, 0, 0,   77, 8, 6,
                                 0,   1, 8, 0, 6, 4, 0, 2, 2, 0, 0,   0,  0, 77,
                                 192, 0, 2, 1, 2, 0, 0, 0, 0, 2, 192, 0,  2, 2};
    memset(p, 0, 60);
    memcpy(p, a, 42);
}

int main(void)
{
    struct na_arp s;
    unsigned char p[60], before[sizeof s];
    unsigned rejected = 0, accepted = 0;
    const unsigned fields[] = {0,  12, 13, 14, 15, 16, 17, 18, 19, 20,
                               21, 22, 27, 28, 31, 32, 37, 38, 41};
    for (unsigned f = 0; f < sizeof fields / sizeof fields[0]; ++f)
    {
        for (unsigned value = 0; value < 256; ++value)
        {
            packet(p);
            unsigned original = p[fields[f]];
            if (value == original)
            {
                continue;
            }
            p[fields[f]] = (unsigned char)value;
            CHECK(na_init(&s, config, 1) == NA_OK);
            memcpy(before, &s, sizeof s);
            /* opcode1 changes a reply into a valid request under this profile. */
            int valid = fields[f] == 21 && value == 1;
            int rc = na_receive(&s, p, 60, 0);
            if (valid)
            {
                CHECK(rc == NA_OK && s.reply_pending);
                ++accepted;
            }
            else
            {
                CHECK(rc == NA_INVALID && !memcmp(before, &s, sizeof s));
                ++rejected;
            }
        }
    }
    for (unsigned n = 0; n < 42; ++n)
    {
        packet(p);
        CHECK(na_init(&s, config, 1) == NA_OK);
        memcpy(before, &s, sizeof s);
        CHECK(na_receive(&s, p, n, 0) == NA_INVALID &&
              !memcmp(before, &s, sizeof s));
        ++rejected;
    }
    packet(p);
    p[12] = 0x81;
    p[13] = 0;
    CHECK(na_init(&s, config, 1) == NA_OK);
    memcpy(before, &s, sizeof s);
    CHECK(na_receive(&s, p, 60, 0) == NA_INVALID &&
          !memcmp(before, &s, sizeof s));
    ++rejected;
    /* Outer60-byte teaching frame is well bounded, but hlen20 declares a56-byte
     * inner record in46payload bytes. Fixed6/4 validation rejects before reads.
     */
    packet(p);
    p[18] = 20;
    CHECK(na_init(&s, config, 1) == NA_OK);
    CHECK(na_receive(&s, p, 60, 0) == NA_INVALID);
    ++rejected;
    packet(p);
    CHECK(na_receive(&s, p, 60, 0) == NA_OK && s.peer_mac[5] == 77);
    /* Fully consistent forged association is indistinguishable here. Harmless
     * local publication demonstrates lack of authentication, not host compromise.
     */
    p[11] = p[27] = 123;
    CHECK(na_receive(&s, p, 60, 1) == NA_OK && s.peer_mac[5] == 123);
    ++accepted;
#ifdef NA_TARGET
    CHECK(rejected == 4888 && accepted == 2);
    puts(
        "NA_VALIDATION_PASS rejected=4888 accepted_controls=2 unauthenticated=1");
#else
    printf(
        "NA_VALIDATION_PASS rejected=%u accepted_controls=%u unauthenticated=1\n",
        rejected, accepted);
#endif
    return 0;
}
