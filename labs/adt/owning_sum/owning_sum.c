#include "owning_sum.h"

void ado_init(struct ado_value *v)
{
    v->tag = ADO_EMPTY;
    v->data.number = 0;
}

void ado_drop(struct ado_value *v)
{
    (void)v;
}

enum ado_status ado_from_value(struct ado_value *o, struct ad_value v)
{
    (void)o;
    (void)v;
    return ADO_UNIMPLEMENTED;
}

enum ado_status ado_text(struct ado_value *o, const unsigned char *p, size_t n,
                         struct ado_ops ops)
{
    (void)o;
    (void)p;
    (void)n;
    (void)ops;
    return ADO_UNIMPLEMENTED;
}

enum ado_status ado_copy(struct ado_value *o, const struct ado_value *v)
{
    (void)o;
    (void)v;
    return ADO_UNIMPLEMENTED;
}

enum ado_status ado_move(struct ado_value *o, struct ado_value *v)
{
    (void)o;
    (void)v;
    return ADO_UNIMPLEMENTED;
}

enum ado_status ado_measure(const struct ado_value *v, uint64_t *o)
{
    (void)v;
    (void)o;
    return ADO_UNIMPLEMENTED;
}

enum ado_status ado_number(const struct ado_value *v, uint32_t *o)
{
    (void)v;
    (void)o;
    return ADO_UNIMPLEMENTED;
}
