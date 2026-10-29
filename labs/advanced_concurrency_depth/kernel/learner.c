#include "kernel.h"

enum ak_result ak_copy(struct ak_packet *p, size_t n, ak_copy_fn f, void *c)
{
    (void)p;
    (void)n;
    (void)f;
    (void)c;
    return AK_UNIMPLEMENTED;
}

enum ak_result ak_init(struct ak_service *s, struct ak_backend b)
{
    (void)s;
    (void)b;
    return AK_UNIMPLEMENTED;
}

enum ak_result ak_enqueue(struct ak_service *s, struct as_value v)
{
    (void)s;
    (void)v;
    return AK_UNIMPLEMENTED;
}

enum ak_result ak_start(struct ak_service *s)
{
    (void)s;
    return AK_UNIMPLEMENTED;
}

enum ak_result ak_complete(struct ak_service *s, struct as_value *v)
{
    (void)s;
    (void)v;
    return AK_UNIMPLEMENTED;
}
