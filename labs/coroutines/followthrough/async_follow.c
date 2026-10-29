#include "async_follow.h"

int ca_create(struct co_allocator a, unsigned b, uint32_t c,
              struct ca_frame **d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return CO_UNIMPLEMENTED;
}

int ca_reserve(struct ca_frame *a, struct ca_ticket *b)
{
    (void)a;
    (void)b;
    return CO_UNIMPLEMENTED;
}

int ca_notify(struct ca_ticket *a, const unsigned char *b, size_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return CO_UNIMPLEMENTED;
}

int ca_cancel(struct ca_frame *a)
{
    (void)a;
    return CO_UNIMPLEMENTED;
}

int ca_ack(struct ca_frame *a)
{
    (void)a;
    return CO_UNIMPLEMENTED;
}

int ca_poll(struct ca_frame *a, unsigned char *b, size_t *c)
{
    (void)a;
    (void)b;
    (void)c;
    return CO_UNIMPLEMENTED;
}

int ca_destroy(struct ca_frame **a)
{
    (void)a;
    return CO_UNIMPLEMENTED;
}
