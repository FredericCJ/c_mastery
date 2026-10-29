#include "move.h"

enum ow_status omm_registry_create(struct omm_memory m, struct v4_allocator a,
                                   uint32_t g, struct omm_registry **o)
{
    (void)m;
    (void)a;
    (void)g;
    (void)o;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status omm_registry_destroy(struct omm_registry **r)
{
    (void)r;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status omm_token_open(struct omm_registry *r, const int32_t *p,
                              size_t n, struct omm_token *o)
{
    (void)r;
    (void)p;
    (void)n;
    (void)o;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status omm_token_move(struct omm_registry *r, struct omm_token *d,
                              struct omm_token *s)
{
    (void)r;
    (void)d;
    (void)s;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status omm_token_read(struct omm_registry *r, struct omm_token t,
                              size_t i, int32_t *o)
{
    (void)r;
    (void)t;
    (void)i;
    (void)o;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status omm_token_consume(struct omm_registry *r, struct omm_token *t)
{
    (void)r;
    (void)t;
    return OW_NOT_IMPLEMENTED;
}
