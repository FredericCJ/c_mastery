#include "rational_units.h"
#include <stdio.h>
#ifndef WRONG
#define WRONG 0
#endif
#ifndef BYPASS
#define BYPASS 0
#endif
int main(void)
{
    struct adq_ticks ticks = {5};
    struct adq_microseconds us = {5};
    struct adq_result result;
#if WRONG
    enum adq_status status =
        adq_to_ticks(ticks, (struct adq_period){3, 2}, &result);
#elif BYPASS
    /* Explicitly dismantling a type defeats the dimensional boundary. */
    enum adq_status status =
        adq_to_ticks((struct adq_microseconds){ticks.value},
                     (struct adq_period){3, 2}, &result);
#else
    enum adq_status status =
        adq_to_ticks(us, (struct adq_period){3, 2}, &result);
#endif
    (void)ticks;
    (void)us;
    if (status != ADQ_OK || result.value != 3 || result.inexact != 1)
    {
        return 1;
    }
    puts("ADQ_NOMINAL exact-result3 inexact1");
    return 0;
}
