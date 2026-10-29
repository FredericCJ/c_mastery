#include <stdint.h>

/* Same finite call tree compiled under both ABIs. No hardware frame implied. */
__attribute__((noinline)) uint32_t sp_abi_probe(unsigned depth, uint32_t seed)
{
    volatile uint32_t frame[7];
    for (unsigned i = 0; i < 7; ++i)
    {
        frame[i] = seed + i;
    }
    uint32_t value =
        depth && depth <= 4 ? sp_abi_probe(depth - 1, seed + 17) : seed;
    for (unsigned i = 0; i < 7; ++i)
    {
        value ^= frame[i];
    }
    return value;
}
