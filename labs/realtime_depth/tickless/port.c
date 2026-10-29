#include "port.h"

enum tq_status tp_init(struct tp_clock *a, struct tq_timer *b, uint32_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return TQ_UNIMPLEMENTED;
}

uint32_t tp_raw(void)
{
    return 0;
}

uint32_t tp_now(const struct tp_clock *a)
{
    (void)a;
    return 0;
}

enum tq_status tp_wait(struct tp_clock *a, unsigned b, uint32_t c, unsigned d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tp_at(struct tp_clock *a, unsigned b, uint32_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tp_arm(struct tp_clock *a, uint32_t b)
{
    (void)a;
    (void)b;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tp_interrupt(struct tp_clock *a)
{
    (void)a;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tp_stop(struct tp_clock *a)
{
    (void)a;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tp_resume(struct tp_clock *a)
{
    (void)a;
    return TQ_UNIMPLEMENTED;
}

void tp_shutdown(struct tp_clock *a)
{
    (void)a;
}
