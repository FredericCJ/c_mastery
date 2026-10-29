#include "durable.h"

int cpx_sink_accept(struct cpx_sink *s, struct cp_queue *q)
{
    (void)s;
    (void)q;
    return SDR_UNIMPLEMENTED;
}

int cpx_sink_commit(struct cpx_sink *s)
{
    (void)s;
    return SDR_UNIMPLEMENTED;
}

int cpx_sink_acknowledge(struct cpx_sink *s, int32_t *o)
{
    (void)s;
    (void)o;
    return SDR_UNIMPLEMENTED;
}
