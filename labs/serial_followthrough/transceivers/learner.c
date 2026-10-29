#include "transceivers.h"

int tr_sample(const struct tr_sample *s, int *d)
{
    (void)s;
    (void)d;
    return TR_UNIMPLEMENTED;
}

int tr_connections(const struct tr_connector *a, const struct tr_connector *b,
                   const struct tr_wire *w, size_t n)
{
    (void)a;
    (void)b;
    (void)w;
    (void)n;
    return TR_UNIMPLEMENTED;
}

int tr_topology(unsigned a, unsigned b, unsigned c, unsigned d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return TR_UNIMPLEMENTED;
}

int tr_232_circuit(const struct tr232_circuit *c)
{
    (void)c;
    return TR_UNIMPLEMENTED;
}

int tr_capture(unsigned d, const struct tr_point *p, size_t n, unsigned s,
               struct tr_result *r)
{
    (void)d;
    (void)p;
    (void)n;
    (void)s;
    (void)r;
    return TR_UNIMPLEMENTED;
}

int tr_edge(const struct tr_point *p, size_t n, unsigned e, unsigned s,
            struct tr_edge *r)
{
    (void)p;
    (void)n;
    (void)e;
    (void)s;
    (void)r;
    return TR_UNIMPLEMENTED;
}

int tr_grant(struct tr_bus *b, unsigned p, uint32_t s, uint64_t n)
{
    (void)b;
    (void)p;
    (void)s;
    (void)n;
    return TR_UNIMPLEMENTED;
}

int tr_release(struct tr_bus *b, unsigned p, uint32_t s, uint64_t n, unsigned t,
               uint64_t d, uint64_t g)
{
    (void)b;
    (void)p;
    (void)s;
    (void)n;
    (void)t;
    (void)d;
    (void)g;
    return TR_UNIMPLEMENTED;
}

int tr_ready(struct tr_bus *b, uint64_t n)
{
    (void)b;
    (void)n;
    return TR_UNIMPLEMENTED;
}
