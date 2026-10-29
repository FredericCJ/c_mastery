#include "object_runtime_vm.h"

void orv_init(struct or_vm *o, struct cvm_cvm *v)
{
    (void)o;
    (void)v;
}

enum or_status orv_pause(struct or_vm *o, int p)
{
    (void)o;
    (void)p;
    return OR_UNIMPLEMENTED;
}

enum or_status orv_register(struct or_vm *o, struct orv_derived *r, uint64_t *t)
{
    (void)o;
    (void)r;
    (void)t;
    return OR_UNIMPLEMENTED;
}

enum or_status orv_unregister(struct or_vm *o, uint64_t t)
{
    (void)o;
    (void)t;
    return OR_UNIMPLEMENTED;
}

enum or_status orv_collect(struct or_vm *o, unsigned b, unsigned f,
                           struct orv_stats *s)
{
    (void)o;
    (void)b;
    (void)f;
    (void)s;
    return OR_UNIMPLEMENTED;
}

enum or_status orv_pair(struct or_vm *o, struct orv_derived *a,
                        struct orv_derived *b, int c, unsigned f,
                        struct cvm_value *v)
{
    (void)o;
    (void)a;
    (void)b;
    (void)c;
    (void)f;
    (void)v;
    return OR_UNIMPLEMENTED;
}

enum or_status orv_call(struct or_vm *o, struct cvm_value a, struct cvm_value b,
                        orv_foreign_fn f, void *c, struct cvm_value *v)
{
    (void)o;
    (void)a;
    (void)b;
    (void)f;
    (void)c;
    (void)v;
    return OR_UNIMPLEMENTED;
}
