#include "support.h"

int lc_host_backend(struct lc_backend *p, int value)
{
    ++p->calls;
    return value + p->bias;
}

int lc_backend_run(struct lc_backend_ops ops, int value)
{
    return ops.apply(ops.context, value);
}
