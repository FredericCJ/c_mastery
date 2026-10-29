#ifndef CMASTERY_EVENTS_DEPTH_H
#define CMASTERY_EVENTS_DEPTH_H
#include "cmx.h"

enum
{
    CDE_IDLE,
    CDE_PENDING,
    CDE_DONE
};

struct cde_operation
{
    unsigned phase;
    int result;
    uint32_t observed;
};

struct cd_events
{
    cmx_kernel *kernel;
    cmx_event event;
    struct cde_operation operation[CMX_TASKS];
};

struct cde_cursor
{
    uint32_t epoch, serial;
};

struct cde_sequence
{
    struct cde_cursor current;
};

int cde_init(struct cd_events *, cmx_kernel *);
int cde_wait(struct cd_events *, unsigned task, uint32_t mask, unsigned all,
             unsigned consume, cmx_deadline);
int cde_result(struct cd_events *, unsigned task, uint32_t *observed);
int cde_set(struct cd_events *, uint32_t bits);
int cde_clear(struct cd_events *, uint32_t bits);
int cde_close(struct cd_events *);
int cde_destroy(struct cd_events *);
void cde_sequence_init(struct cde_sequence *);
int cde_sequence_publish(struct cde_sequence *);
int cde_sequence_read(const struct cde_sequence *, struct cde_cursor *,
                      uint64_t *delta, uint64_t *missed);
#endif
