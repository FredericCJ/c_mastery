#include "scope.h"

enum ow_status ocs_run(const struct ocs_plan *p, unsigned *e,
                       struct ocs_report *r)
{
    (void)p;
    (void)e;
    (void)r;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ocs_begin(struct ocs_recovery *r, struct ocs_ticket *t)
{
    (void)r;
    (void)t;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ocs_push(struct ocs_ticket t, ocs_closer_fn c, void *p,
                        unsigned i)
{
    (void)t;
    (void)c;
    (void)p;
    (void)i;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status ocs_recover(struct ocs_ticket t, struct ocs_recovered *r)
{
    (void)t;
    (void)r;
    return OW_NOT_IMPLEMENTED;
}
