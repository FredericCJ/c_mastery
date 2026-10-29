#include "pin_host.h"

enum ow_status opa_port_init(struct opa_port *p, int32_t *b, size_t n)
{
    (void)p;
    (void)b;
    (void)n;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status opa_port_start(struct opa_port *p, int32_t v, opa_receiver_fn f,
                              void *c, uint32_t *t)
{
    (void)p;
    (void)v;
    (void)f;
    (void)c;
    (void)t;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status opa_port_cancel(struct opa_port *p, uint32_t t, int e)
{
    (void)p;
    (void)t;
    (void)e;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status opa_port_allow_backend(struct opa_port *p, uint32_t t)
{
    (void)p;
    (void)t;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status opa_port_observe(struct opa_port *p, struct opa_observation *o)
{
    (void)p;
    (void)o;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status opa_port_join(struct opa_port *p, uint32_t t, int w, int32_t *o)
{
    (void)p;
    (void)t;
    (void)w;
    (void)o;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status opa_port_rebind(struct opa_port *p, int32_t *b, size_t n)
{
    (void)p;
    (void)b;
    (void)n;
    return OW_NOT_IMPLEMENTED;
}

enum ow_status opa_port_destroy(struct opa_port *p)
{
    (void)p;
    return OW_NOT_IMPLEMENTED;
}
