#ifndef CMASTERY_ACTIVE_IMPORT_H
#define CMASTERY_ACTIVE_IMPORT_H
#include "event_follow.h"
#include "cmx.h"

struct pa_task_active
{
    cmx_kernel kernel;
    cmx_queue queue;
    struct pt_machine machine;
    struct pf_lock lock;
    unsigned held, event, discarded;
    _Alignas(16) unsigned char stacks[4][256];
};

int pa_active_init(struct pa_task_active *, struct pf_lock);
int pa_active_post(struct pa_task_active *, unsigned, unsigned);
int pa_active_step(struct pa_task_active *, unsigned, unsigned char *, size_t,
                   size_t *);
int pa_active_discard(struct pa_task_active *, unsigned, unsigned *);
#endif
