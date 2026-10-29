#ifndef CMASTERY_OBJECT_RUNTIME_MARKING_H
#define CMASTERY_OBJECT_RUNTIME_MARKING_H
#include "object_runtime.h"

struct orc_handle
{
    uint8_t slot, generation;
};

enum orc_policy
{
    ORC_BLACK = 0,
    ORC_GRAY = 1,
    ORC_RESTART = 2
};

enum
{
    ORC_REV_LIMIT = 65535
};

struct orc_heap
{
    struct or_graph graph;
    uint8_t generation[16], roots[16], work[16], used, policy;
    uint32_t revision, scans, restarts, cycles;
};

struct orc_finish
{
    const struct orc_heap *owner;
    uint32_t revision;
    uint16_t white;
};

void orcm_init(struct orc_heap *, enum orc_policy);
int orcm_invariant(const struct orc_heap *);
enum or_status orcm_allocate(struct orc_heap *, struct orc_handle first,
                             struct orc_handle second, struct orc_handle *out);
enum or_status orcm_retain(struct orc_heap *, struct orc_handle);
enum or_status orcm_release(struct orc_heap *, struct orc_handle);
enum or_status orcm_store(struct orc_heap *, struct orc_handle owner,
                          unsigned field, struct orc_handle target);
enum or_status orcm_read(const struct orc_heap *, struct orc_handle owner,
                         struct orc_handle edges[2]);
enum or_status orcm_begin(struct orc_heap *);
enum or_status orcm_step(struct orc_heap *, unsigned budget, unsigned *scanned);
enum or_status orcm_prepare(const struct orc_heap *, struct orc_finish *out);
enum or_status orcm_commit(struct orc_heap *, struct orc_finish,
                           uint16_t *dead);
#endif
