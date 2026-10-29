#include "mmio.h"

uint32_t pm_target_read(const struct pm_gpio *p)
{
    (void)p;
    return 0;
}

int pm_target_write(struct pm_gpio *p, unsigned s, unsigned c)
{
    (void)p;
    (void)s;
    (void)c;
    return -1;
}
