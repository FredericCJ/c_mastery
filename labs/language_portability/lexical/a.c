#include "config.h"

unsigned lp_a(void)
{
    TYPE local = 7;
    (void)local;
    return sizeof local;
}
