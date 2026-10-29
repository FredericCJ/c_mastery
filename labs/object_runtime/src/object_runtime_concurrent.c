#include "object_runtime_concurrent.h"

enum or_status orcm_concurrent_init(struct orc_concurrent *c, enum orc_policy p)
{
    (void)c;
    (void)p;
    return OR_UNIMPLEMENTED;
}

enum or_status orcm_concurrent_call(struct orc_concurrent *c,
                                    const struct orc_request *q,
                                    struct orc_response *r)
{
    (void)c;
    (void)q;
    (void)r;
    return OR_UNIMPLEMENTED;
}

enum or_status orcm_concurrent_close(struct orc_concurrent *c)
{
    (void)c;
    return OR_UNIMPLEMENTED;
}
