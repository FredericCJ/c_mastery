#include "checkpoint_operation.h"

enum cpo_status cpo_init(struct cp_operation *o, unsigned n, void *c,
                         cpo_allocate_fn a, cpo_release_fn f, cpo_report_fn r)
{
    (void)o;
    (void)n;
    (void)c;
    (void)a;
    (void)f;
    (void)r;
    return CPO_ERROR;
}

enum cpo_status cpo_begin(struct cp_operation *o, const unsigned char *p,
                          size_t n, unsigned *g)
{
    (void)o;
    (void)p;
    (void)n;
    (void)g;
    return CPO_ERROR;
}

enum cpo_status cpo_cancel(struct cp_operation *o, unsigned g)
{
    (void)o;
    (void)g;
    return CPO_ERROR;
}

enum cpo_status cpo_complete(struct cp_operation *o, unsigned g, int q)
{
    (void)o;
    (void)g;
    (void)q;
    return CPO_ERROR;
}

enum cpo_status cpo_destroy(struct cp_operation *o)
{
    (void)o;
    return CPO_ERROR;
}
