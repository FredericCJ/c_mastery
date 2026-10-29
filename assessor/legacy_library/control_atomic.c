#include "control.h"

void lg_init(struct lg_publication *p)
{
    atomic_init(&p->ready, 0);
    p->payload = 0;
}

void lg_publish(struct lg_publication *p, int value)
{
#ifdef LG_MUTANT_PAYLOAD
    value = 0;
#endif
    p->payload = value;
    atomic_store_explicit(&p->ready, 1, memory_order_release);
}

enum lg_status lg_receive(struct lg_publication *p, int *out)
{
    if (!p || !out)
    {
        return LG_ARGUMENT;
    }
    if (!atomic_load_explicit(&p->ready, memory_order_acquire))
    {
        return LG_PENDING;
    }
    *out = p->payload;
    return LG_OK;
}
