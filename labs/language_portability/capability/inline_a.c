#include "inline.h"
int (*lp_pointer_a)(int) = lp_inline;

int lp_a(void)
{
    return lp_pointer_a(4);
}
