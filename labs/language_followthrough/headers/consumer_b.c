#include <limits.h>
#include <headers.h>

hh_fn hh_b_address(void)
{
    return hh_mix;
}

unsigned *hh_b_state(void)
{
    return hh_state();
}

unsigned hh_b_call(unsigned x)
{
    return hh_mix(x);
}
