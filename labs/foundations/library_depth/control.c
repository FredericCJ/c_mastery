#include "control.h"

enum lg_status lg_jump(struct lg_owner *o, unsigned m, int *r)
{
    (void)o;
    (void)m;
    (void)r;
    return LG_NOT_IMPLEMENTED;
}

enum lg_status lg_raise(void)
{
    return LG_NOT_IMPLEMENTED;
}

void lg_init(struct lg_publication *p)
{
    atomic_init(&p->ready, 0);
    p->payload = 0;
}

void lg_publish(struct lg_publication *p, int v)
{
    (void)p;
    (void)v;
}

enum lg_status lg_receive(struct lg_publication *p, int *v)
{
    (void)p;
    (void)v;
    return LG_NOT_IMPLEMENTED;
}
