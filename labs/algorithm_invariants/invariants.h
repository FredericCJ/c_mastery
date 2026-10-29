#ifndef AI_INVARIANTS_H
#define AI_INVARIANTS_H
#include <stddef.h>
#include <stdint.h>
#include "containers.h"

enum ai_status
{
    AI_OK,
    AI_DONE,
    AI_INVALID,
    AI_UNIMPLEMENTED
};

struct ai_state
{
    uint32_t a, b;
};
enum ai_status ai_step(struct ai_state, unsigned subtract, struct ai_state *);

struct ai_record
{
    uint32_t key, tag;
};
enum ai_status ai_permutation(const struct ai_record *,
                              const struct ai_record *, size_t, unsigned *);

struct ai_growth
{
    size_t copies, writes, worst, capacity, acquisitions, releases;
};
enum ai_status ai_growth(unsigned count, unsigned additive, struct ai_growth *);

struct ai_queues
{
    struct cd_ring ring;
    int backing[3];
    struct cd_queue queue;
};

void ai_queues_init(struct ai_queues *);
enum cd_status ai_queue_event(struct ai_queues *, unsigned put, int value,
                              int *removed);
#endif
