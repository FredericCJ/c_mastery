#include "security_extension.h"

int mde_deliver(struct mde_object *o, struct mde_event e)
{
    (void)o;
    (void)e;
    return 0;
}

int mde_retire(struct mde_object *o)
{
    (void)o;
    return 0;
}

int mde_recreate(struct mde_object *o, unsigned a, unsigned b)
{
    (void)o;
    (void)a;
    (void)b;
    return 0;
}

int mde_log(const unsigned char *p, size_t n, char *o, size_t c)
{
    (void)p;
    (void)n;
    (void)o;
    (void)c;
    return 0;
}

int mde_destroy(struct mde_owner *o)
{
    (void)o;
    return 0;
}
