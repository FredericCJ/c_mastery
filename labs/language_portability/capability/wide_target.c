#include <stdatomic.h>
static _Atomic(unsigned long long) counter;

unsigned long long lp_wide(void)
{
    return atomic_fetch_add(&counter, 1);
}
