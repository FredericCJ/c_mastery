#include "pipeline.h"

int ew_init(struct ew_node *a, unsigned b, unsigned c, uint16_t d,
            const struct r5_io *e, const struct pu_clock *f, uint32_t g)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    (void)f;
    (void)g;
    return EW_STUB;
}

int ew_queue(struct ew_node *a, unsigned b, const struct edu_frame *c)
{
    (void)a;
    (void)b;
    (void)c;
    return EW_STUB;
}

int ew_tick(struct ew_node *a, uint32_t b)
{
    (void)a;
    (void)b;
    return EW_STUB;
}

int ew_take(struct ew_node *a, struct es_event *b)
{
    (void)a;
    (void)b;
    return EW_STUB;
}

int ew_pause(struct ew_node *a)
{
    (void)a;
    return EW_STUB;
}

int ew_drained(const struct ew_node *a)
{
    (void)a;
    return 0;
}

int ew_rebind(struct ew_node *a, uint16_t b)
{
    (void)a;
    (void)b;
    return EW_STUB;
}

int ew_run(struct ew_node *a)
{
    (void)a;
    return EW_STUB;
}

int ew_physical_verdict(unsigned a, unsigned b)
{
    (void)a;
    (void)b;
    return EW_STUB;
}
