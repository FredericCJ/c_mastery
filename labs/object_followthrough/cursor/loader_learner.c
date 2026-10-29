#include "loader.h"

int oc_open(struct oc_module *m, const char *p)
{
    (void)m;
    (void)p;
    return OC_LOAD_BAD;
}

int oc_borrow(struct oc_module *m, const struct oc_plugin **p)
{
    (void)m;
    (void)p;
    return OC_LOAD_BAD;
}

int oc_return(struct oc_module *m, const struct oc_plugin **p)
{
    (void)m;
    (void)p;
    return OC_LOAD_BAD;
}

int oc_close(struct oc_module *m)
{
    (void)m;
    return OC_LOAD_BAD;
}

int oc_function_lookup(struct oc_module *p, int *v)
{
    (void)p;
    (void)v;
    return OC_LOAD_BAD;
}
