#include "cmx.h"

/* Atomic implementation intentionally absent. */
int cmx_spsc_init(cmx_spsc *a0, unsigned a1)
{
    return 0;
}

enum cmx_result cmx_spsc_push(cmx_spsc *a0, uint32_t a1)
{
    return CMX_UNIMPLEMENTED;
}

enum cmx_result cmx_spsc_pop(cmx_spsc *a0, uint32_t *a1)
{
    return CMX_UNIMPLEMENTED;
}

void cmx_hp_init(cmx_hp_queue *a0)
{
    *a0 = (cmx_hp_queue){0};
}

enum cmx_result cmx_hp_push(cmx_hp_queue *a0, unsigned a1, uint32_t a2)
{
    return CMX_UNIMPLEMENTED;
}

enum cmx_result cmx_hp_pop(cmx_hp_queue *a0, unsigned a1, uint32_t *a2)
{
    return CMX_UNIMPLEMENTED;
}

unsigned cmx_hp_collect(cmx_hp_queue *a0)
{
    return 0;
}

void cmx_epoch_init(cmx_epoch *a0)
{
    *a0 = (cmx_epoch){0};
}

enum cmx_result cmx_epoch_enter(cmx_epoch *a0, unsigned a1)
{
    return CMX_UNIMPLEMENTED;
}

void cmx_epoch_leave(cmx_epoch *a0, unsigned a1)
{
}

int cmx_epoch_advance(cmx_epoch *a0)
{
    return 0;
}
