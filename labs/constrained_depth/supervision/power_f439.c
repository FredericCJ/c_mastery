#include "power.h"

struct cs_bus cs_f439_bus(void)
{
    return (struct cs_bus){0};
}

enum cs_status cs_f439_stop(struct cs_stop *s, unsigned p)
{
    (void)s;
    (void)p;
    return CS_UNIMPLEMENTED;
}
