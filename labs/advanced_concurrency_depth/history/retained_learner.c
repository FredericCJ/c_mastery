#include "cmx.h"

void cmx_hp_init(cmx_hp_queue *q)
{
    (void)q;
}

enum cmx_result cmx_hp_push(cmx_hp_queue *q, unsigned p, uint32_t value)
{
    (void)q;
    (void)p;
    (void)value;
    return CMX_INVALID;
}

enum cmx_result cmx_hp_pop(cmx_hp_queue *q, unsigned p, uint32_t *value)
{
    (void)q;
    (void)p;
    (void)value;
    return CMX_INVALID;
}

unsigned cmx_hp_collect(cmx_hp_queue *q)
{
    (void)q;
    return 0;
}
