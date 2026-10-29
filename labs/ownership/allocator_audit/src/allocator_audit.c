#include "allocator_audit.h"

enum ow_status oaa_init(struct oaa_audit *a, struct oaa_upstream b)
{
    (void)a;
    (void)b;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status oaa_set_report(struct oaa_audit *a, oaa_reporter_fn b, void *c)
{
    (void)a;
    (void)b;
    (void)c;
    return OW_NOT_IMPLEMENTED;
}

struct v4_allocator oaa_allocator(struct oaa_audit *a)
{
    (void)a;
    return (struct v4_allocator){0};
}

enum ow_status oaa_resize(struct oaa_audit *a, int32_t *b, size_t c,
                          int32_t **d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return OW_NOT_IMPLEMENTED;
}
