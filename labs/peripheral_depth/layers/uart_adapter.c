#include "uart_adapter.h"

int pl_uart_open(struct pl_uart *a, struct pu_io b, const struct pu_clock *c,
                 uint32_t d, unsigned e, unsigned f)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    return PL_UNIMPLEMENTED;
}

struct pl_transport pl_uart_transport(struct pl_uart *u)
{
    (void)u;
    struct pl_transport t = {0};
    return t;
}
