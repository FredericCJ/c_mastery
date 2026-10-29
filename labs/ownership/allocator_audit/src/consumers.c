#include "allocator_audit.h"

struct oaa_upstream oaa_ow_upstream(struct ow_allocator *a)
{
    (void)a;
    return (struct oaa_upstream){0};
}

er_result oaa_parse_call(const unsigned char *a, size_t b, uint32_t *c,
                         oaa_format_fn d, void *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    er_result r = {0};
    r.tag = ER_NOT_IMPLEMENTED;
    return r;
}

enum ow_status oaa_format_result(struct oaa_formatting *a, er_result b)
{
    (void)a;
    (void)b;
    return OW_NOT_IMPLEMENTED;
}

void oaa_allocating_formatter(void *a, er_result b)
{
    (void)a;
    (void)b;
}
