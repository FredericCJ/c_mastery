#include <stdint.h>

/* Dependency-audit negative specimen, never accepted as a linked target. */
uint64_t vx_implicit(uint64_t numerator, uint64_t denominator)
{
    return numerator / denominator;
}
