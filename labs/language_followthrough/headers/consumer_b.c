/*
 * Independent consumer b: shares nothing with the other consumer except the
 * header. The include order differs between the consumers on purpose
 * (<limits.h> first, then the header) to show the header works after either.
 */
#include <limits.h>
#include <headers.h>

/* Address of hh_mix as seen from this translation unit. */
hh_fn hh_b_address(void)
{
    return hh_mix;
}

/* Address of this unit's header-local counter. */
unsigned *hh_b_state(void)
{
    return hh_state();
}

unsigned hh_b_call(unsigned value)
{
    return hh_mix(value);
}
