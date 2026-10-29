#ifndef B06_EPOCH_H
#define B06_EPOCH_H
#include <stdint.h>

struct k_epoch
{
    uint32_t current, retired, reader[8];
    unsigned closed;
};

/* Live disjoint arguments; serialized model, not an ISO C RCU primitive. */
int ke_enter(struct k_epoch *, unsigned id);
int ke_leave(struct k_epoch *, unsigned id);
int ke_replace(struct k_epoch *);
int ke_reclaim(struct k_epoch *);
int ke_close(struct k_epoch *);
int ke_requeue(const struct k_epoch *, uint32_t captured_generation);
#endif
