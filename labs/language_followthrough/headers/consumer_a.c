/*
 * Independent consumer a: shares nothing with the other consumer except the
 * header. The include order differs between the consumers on purpose
 * (header first, then <limits.h>) to show the header works after either.
 */
#include <headers.h>
#include <limits.h>

/* Address of hh_mix as seen from this translation unit. */
hh_fn hh_a_address(void)
{
    return hh_mix;
}

/* Address of this unit's header-local counter. */
unsigned *hh_a_state(void)
{
    return hh_state();
}

unsigned hh_a_call(unsigned value)
{
    return hh_mix(value);
}
