#include "sleep.h"

unsigned cs_idle(unsigned e, volatile uint32_t *q, void (*h)(unsigned))
{
    (void)e;
    (void)q;
    (void)h;
    return 2;
}
