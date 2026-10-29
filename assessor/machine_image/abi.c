#include "machine.h"

uint64_t mi_mix(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e,
                uint64_t f, uint64_t g, uint64_t h)
{
    return a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + 7 * g + 8 * h;
}
