#ifndef CMASTERY_BARRIER_DEPTH_H
#define CMASTERY_BARRIER_DEPTH_H
#include "cmx.h"

enum
{
    CDB_IDLE,
    CDB_PENDING,
    CDB_DONE
};

enum
{
    CDB_STALE = CMX_UNIMPLEMENTED + 1,
    CDB_RETIRED
};

struct cdb_slot
{
    unsigned state, round, task_generation;
    int result;
};

struct cd_barrier
{
    cmx_kernel *kernel;
    cmx_barrier raw;
    struct cdb_slot slot[CMX_TASKS];
    uint32_t members, registered, next_members, epoch;
    unsigned generation_limit, changing, live;
};

struct cdb_member
{
    struct cd_barrier *owner;
    unsigned task;
    uint32_t epoch;
};

struct cdb_ticket
{
    struct cdb_member member;
    unsigned round;
};

int cdb_init(struct cd_barrier *, cmx_kernel *, uint32_t members,
             unsigned generation_limit);
int cdb_register(struct cd_barrier *, unsigned task, struct cdb_member *);
int cdb_arrive(struct cdb_member, cmx_deadline, struct cdb_ticket *);
int cdb_result(struct cdb_ticket, unsigned *completed_round);
int cdb_cancel(struct cdb_ticket);
int cdb_stage(struct cd_barrier *, uint32_t next_members);
int cdb_release(struct cdb_member *);
int cdb_apply(struct cd_barrier *);
int cdb_close(struct cd_barrier *);
int cdb_destroy(struct cd_barrier *);
#endif
