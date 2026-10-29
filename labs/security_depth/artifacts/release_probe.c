/* Original reproducible but deliberately unauthorized synthetic effect. */
#include <stdio.h>

static unsigned effect(unsigned allowed)
{
#ifdef SA_RELEASE_FLAW
    (void)allowed;
    return 1;
#else
    return allowed == 1;
#endif
}

int main(void)
{
    printf("synthetic_effect=%u\n", effect(0));
    return 0;
}
