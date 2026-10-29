#include <stdint.h>

/* Changed operation: nonzero divisor is a caller precondition. Inspect the
 * object import before deciding whether an original helper is admitted. */
uint64_t af_changed_division(uint64_t value, uint64_t divisor)
{
    return value / divisor;
}
