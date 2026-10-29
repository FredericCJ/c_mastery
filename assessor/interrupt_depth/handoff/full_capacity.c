#include "cmx.h"
#include <limits.h>
#include <stdio.h>
#define REQUIRE(x)                   \
    do                               \
    {                                \
        if (!(x))                    \
        {                            \
            puts("XI_CMX_FAIL " #x); \
            return 1;                \
        }                            \
    } while (0)

int main(void)
{
    cmx_spsc q;
    uint32_t value = 999;
    unsigned accepted = 0;
    REQUIRE(!cmx_spsc_init(&q, 0) && !cmx_spsc_init(&q, 3));
    for (unsigned cap = 1; cap <= 16; cap *= 2)
    {
        REQUIRE(cmx_spsc_init(&q, cap));
        /* Quiescent initialization near modular wrap, before publishing owners. */
        atomic_store(&q.head, UINT_MAX - 3);
        atomic_store(&q.tail, UINT_MAX - 3);
        for (unsigned round = 0; round < 64; ++round)
        {
            for (unsigned i = 0; i < cap; ++i)
            {
                REQUIRE(cmx_spsc_push(&q, round * 16 + i) == CMX_OK);
                ++accepted;
            }
            REQUIRE(cmx_spsc_push(&q, 77) == CMX_FULL);
            for (unsigned i = 0; i < cap; ++i)
            {
                REQUIRE(cmx_spsc_pop(&q, &value) == CMX_OK &&
                        value == round * 16 + i);
            }
            value = 999;
            REQUIRE(cmx_spsc_pop(&q, &value) == CMX_RETRY && value == 999);
        }
    }
    REQUIRE(accepted == 1984);
    puts("XI_CMX_PASS full_capacity=1,2,4,8,16 records=1984 wrap=unsigned");
    return 0;
}
