#include "active_import.h"

int pa_active_init(struct pa_task_active *a, struct pf_lock b)
{
    (void)a;
    (void)b;
    return PT_UNIMPLEMENTED;
}

int pa_active_post(struct pa_task_active *a, unsigned b, unsigned c)
{
    (void)a;
    (void)b;
    (void)c;
    return PT_UNIMPLEMENTED;
}

int pa_active_step(struct pa_task_active *a, unsigned b, unsigned char *c,
                   size_t d, size_t *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return PT_UNIMPLEMENTED;
}

int pa_active_discard(struct pa_task_active *a, unsigned b, unsigned *c)
{
    (void)a;
    (void)b;
    (void)c;
    return PT_UNIMPLEMENTED;
}
