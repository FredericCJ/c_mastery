#include "frontier.h"

enum ocf_status ocf_construct(struct ocf_owner *o, struct v4_allocator a,
                              struct ocf_callbacks c, const struct ocf_plan *p)
{
    (void)o;
    (void)a;
    (void)c;
    (void)p;
    return OCF_NOT_IMPLEMENTED;
}

enum ocf_status ocf_destroy(struct ocf_owner *o)
{
    (void)o;
    return OCF_NOT_IMPLEMENTED;
}
