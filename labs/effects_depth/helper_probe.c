/*
 * Compiled (not run) for Cortex-M0 and Cortex-M4 to see which runtime helper
 * calls the compiler emits for a 64-bit division.
 */
#include <stdint.h>

/* Returns value / divisor, or 0 when divisor is 0. */
uint64_t ef_divide(uint64_t value, uint64_t divisor)
{
    return divisor ? value / divisor : 0;
}
