#include "tickless.h"

enum tq_status tq_relation(uint32_t a, uint32_t b, unsigned c, unsigned d,
                           enum tq_relation *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tq_ticks(uint64_t a, uint32_t b, uint32_t c, uint64_t *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tq_not_before(uint64_t a, uint64_t b, uint32_t c, uint32_t d,
                             uint64_t *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tq_init(struct tq_timer *a, uint32_t b)
{
    (void)a;
    (void)b;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tq_wait(struct tq_timer *a, unsigned b, uint32_t c, unsigned d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tq_cancel(struct tq_timer *a, unsigned b)
{
    (void)a;
    (void)b;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tq_advance(struct tq_timer *a, uint32_t b)
{
    (void)a;
    (void)b;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tq_collect(struct tq_timer *a, unsigned b, uint32_t *c,
                          enum cmx_result *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tq_plan(struct tq_timer *a, uint32_t b, struct tq_plan *c)
{
    (void)a;
    (void)b;
    (void)c;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tq_commit(struct tq_timer *a, struct tq_plan b, uint32_t c,
                         unsigned d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tq_abort(struct tq_timer *a)
{
    (void)a;
    return TQ_UNIMPLEMENTED;
}

enum tq_status tq_resume(struct tq_timer *a, struct tq_resume *b, uint32_t c)
{
    (void)a;
    (void)b;
    (void)c;
    return TQ_UNIMPLEMENTED;
}
