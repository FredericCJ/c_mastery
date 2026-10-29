#include "object_runtime_rc_concurrent.h"

enum or_status orrc_init(struct orr_concurrent *h)
{
    (void)h;
    return OR_UNIMPLEMENTED;
}

enum or_status orrc_call(struct orr_concurrent *h, const struct orr_request *q,
                         struct orr_response *r)
{
    (void)h;
    (void)q;
    (void)r;
    return OR_UNIMPLEMENTED;
}

enum or_status orrc_close(struct orr_concurrent *h)
{
    (void)h;
    return OR_UNIMPLEMENTED;
}
