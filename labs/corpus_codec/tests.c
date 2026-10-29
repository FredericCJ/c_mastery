#include "codec.h"
#include <stdio.h>
#define CHECK(c)                                        \
    do                                                  \
    {                                                   \
        if (!(c))                                       \
        {                                               \
            fprintf(stderr, "CC_CHECK:%d\n", __LINE__); \
            return 1;                                   \
        }                                               \
    } while (0)

int main(void)
{
    uint8_t in[128], out[CC_FRAME_LIMIT], decoded[512], length[16] = {0};
    size_t used = 777, n = 0;
    uint16_t freq[16] = {0};
    struct cc_book book;
    CHECK(cc_lengths(freq, length) == CC_OK);
    for (unsigned i = 0; i < 16; i++)
    {
        CHECK(length[i] == 0);
    }
    freq[5] = 65535;
    CHECK(cc_lengths(freq, length) == CC_OK && length[5] == 1);
    for (unsigned i = 0; i < 16; i++)
    {
        freq[i] = 65535;
    }
    CHECK(cc_lengths(freq, length) == CC_OK);
    for (unsigned i = 0; i < 16; i++)
    {
        CHECK(length[i] == 4);
    }
    length[0] = length[1] = length[2] = 1;
    CHECK(cc_book(length, &book) == CC_INVALID);
    for (unsigned i = 0; i < 128; i++)
    {
        in[i] = (uint8_t)(i % 3);
    }
    CHECK(cc_compress(in, 128, out, sizeof out, &used) == CC_OK);
    CHECK(cc_expand(out, used, decoded, sizeof decoded, &n) == CC_OK &&
          n == 128);
    for (unsigned i = 0; i < 128; i++)
    {
        CHECK(decoded[i] == in[i]);
    }
    for (size_t cap = 0; cap < used; cap++)
    {
        uint8_t scratch[CC_FRAME_LIMIT];
        size_t sentinel = 777;
        for (size_t i = 0; i < sizeof scratch; i++)
        {
            scratch[i] = 0xa5;
        }
        CHECK(cc_compress(in, 128, scratch, cap, &sentinel) == CC_SPACE &&
              sentinel == 777);
        for (size_t i = 0; i < sizeof scratch; i++)
        {
            CHECK(scratch[i] == 0xa5);
        }
    }
    for (size_t cap = 0; cap < 128; cap++)
    {
        for (size_t i = 0; i < sizeof decoded; i++)
        {
            decoded[i] = 0xa5;
        }
        n = 777;
        CHECK(cc_expand(out, used, decoded, cap, &n) == CC_SPACE && n == 777);
        for (size_t i = 0; i < sizeof decoded; i++)
        {
            CHECK(decoded[i] == 0xa5);
        }
    }
    {
        uint8_t one = 3;
        CHECK(cc_hencode(&one, 1, out, sizeof out, &used) == CC_OK);
        out[used - 1] |= 1;
        n = 777;
        CHECK(cc_hdecode(out, used, decoded, sizeof decoded, &n) ==
                  CC_INVALID &&
              n == 777);
    }
    {
        uint8_t bad[] = {1, 3, 0};
        CHECK(cc_lzdecode(bad, sizeof bad, decoded, sizeof decoded, &n) ==
              CC_INVALID);
    }
    {
        uint8_t overlap[] = {0, 'x', 0, 'y', 1, 5, 2};
        CHECK(cc_lzdecode(overlap, sizeof overlap, decoded, sizeof decoded,
                          &n) == CC_OK &&
              n == 7);
        for (unsigned i = 0; i < 7; i++)
        {
            CHECK(decoded[i] == (i % 2 ? 'y' : 'x'));
        }
    }
    {
        uint8_t repeated[] = {'a', 'a', 'a', 'a'};
        CHECK(cc_lzoptimal(repeated, 4, out, sizeof out, &used) == CC_OK &&
              used == 5);
    }
    CHECK(cc_compress(NULL, 0, out, sizeof out, &used) == CC_OK && used == 18);
    CHECK(cc_expand(out, used, NULL, 0, &n) == CC_OK && n == 0);
    CHECK(cc_compress(NULL, 1, out, sizeof out, &used) == CC_INVALID);
    CHECK(cc_compress(in, 129, out, sizeof out, &used) == CC_INVALID);
    puts("CC_CHECK: boundaries, transactional failures and composition passed");
    return 0;
}
