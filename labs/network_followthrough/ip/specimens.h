#ifndef NI_SPECIMENS_H
#define NI_SPECIMENS_H
#include "ip.h"
#ifdef NI_TARGET
int puts(const char *);
#else
#include <stdio.h>
#endif
#define CHECK(x)                  \
    do                            \
    {                             \
        if (!(x))                 \
        {                         \
            puts("NI_CHECK " #x); \
            return 1;             \
        }                         \
    } while (0)

static inline void ni_test_put16(unsigned char *p, unsigned n)
{
    p[0] = (unsigned char)(n >> 8);
    p[1] = (unsigned char)n;
}

/* Independent contiguous test builder: accumulate without per-word folding.
 * n<=1500 bounds the unfurled sum to750*65535, within uint32_t. */
static inline unsigned ni_test_sum(const unsigned char *p, size_t n)
{
    uint32_t sum = 0;
    for (size_t i = 0; i < n; i++)
    {
        sum += (uint32_t)p[i] << (i % 2 ? 0 : 8);
    }
    while (sum >> 16)
    {
        sum = (sum & 65535) + (sum >> 16);
    }
    return (unsigned)(sum ^ 65535);
}

static inline void ni_test_header(unsigned char *p, unsigned payload,
                                  unsigned id, unsigned offset, unsigned more,
                                  unsigned protocol)
{
    for (unsigned i = 0; i < 20; i++)
    {
        p[i] = 0;
    }
    p[0] = 0x45;
    ni_test_put16(p + 2, payload + 20);
    ni_test_put16(p + 4, id);
    ni_test_put16(p + 6, offset / 8 + (more ? 8192 : 0));
    p[8] = 64;
    p[9] = (unsigned char)protocol;
    p[12] = 192;
    p[14] = 2;
    p[15] = 1;
    p[16] = 192;
    p[18] = 2;
    p[19] = 2;
    ni_test_put16(p + 10, ni_test_sum(p, 20));
}

static inline void ni_test_recheck(unsigned char *p)
{
    p[10] = p[11] = 0;
    ni_test_put16(p + 10, ni_test_sum(p, 20));
}

static inline void ni_test_echo(unsigned char *p, unsigned size)
{
    ni_test_header(p, size + 8, 0, 0, 0, 1);
    for (unsigned i = 20; i < 28; i++)
    {
        p[i] = 0;
    }
    p[20] = 8;
    p[24] = 0x12;
    p[25] = 0x34;
    p[27] = 1;
    for (unsigned i = 0; i < size; i++)
    {
        p[28 + i] = (unsigned char)(0x61 + i);
    }
    ni_test_put16(p + 22, ni_test_sum(p + 20, size + 8));
}
#endif
