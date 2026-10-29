#ifndef SYSTEM_FOLLOW_H
#define SYSTEM_FOLLOW_H
#include "protocol_follow.h"
#include "cmx.h"
#include "adt.h"
#include "algo.h"

enum
{
    PF_COMPLETE = 1,
    PF_RESET = 2,
    PF_QUIET = 3
};

struct pf_message
{
    struct ad_brand brand;
    struct pf_recovery recovery;
    uint32_t epoch;
    unsigned kind, idle;
};

struct pf_system
{
    struct pf_driver driver;
    cmx_kernel kernel;
    cmx_queue queue;
    struct ad_pool pool;
    struct pf_message messages[4];
    algo_ring history;
    unsigned lost_history;
    uint64_t deadline, last_time;
    unsigned timer_armed;
    struct pf_recovery timer_recovery;
    _Alignas(16) unsigned char stacks[3][512];
};

int pf_system_init(struct pf_system *, struct fw_io, uint32_t, pf_free_fn,
                   void *);
int pf_system_post(struct pf_system *, unsigned producer, unsigned kind,
                   struct pf_recovery, uint32_t, unsigned, struct ad_brand *);
int pf_system_step(struct pf_system *);
int pf_system_arm_deadline(struct pf_system *, struct pf_recovery, uint64_t,
                           unsigned);
int pf_system_tick(struct pf_system *, uint64_t);
#endif
