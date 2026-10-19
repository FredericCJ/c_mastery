#include "rational_units.h"
#include <stdio.h>
#ifndef WRONG
#define WRONG 0
#endif
#ifndef BYPASS
#define BYPASS 0
#endif
int main(void) {
    AdqTicks ticks={5};AdqMicroseconds us={5};AdqResult result;
#if WRONG
    AdqStatus status=adq_to_ticks(ticks,(AdqPeriod){3,2},&result);
#elif BYPASS
    /* Explicitly dismantling a type defeats the dimensional boundary. */
    AdqStatus status=adq_to_ticks((AdqMicroseconds){ticks.value},(AdqPeriod){3,2},&result);
#else
    AdqStatus status=adq_to_ticks(us,(AdqPeriod){3,2},&result);
#endif
    (void)ticks;(void)us;
    if(status!=ADQ_OK||result.value!=3||result.inexact!=1)return 1;
    puts("ADQ_NOMINAL exact-result3 inexact1");return 0;
}
