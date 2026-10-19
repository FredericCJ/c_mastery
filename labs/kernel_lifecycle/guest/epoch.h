#ifndef B06_EPOCH_H
#define B06_EPOCH_H
#include <stdint.h>
typedef struct { uint32_t current,retired,reader[8]; unsigned closed; } KEpoch;
/* Live disjoint arguments; serialized model, not an ISO C RCU primitive. */
int ke_enter(KEpoch *,unsigned id);
int ke_leave(KEpoch *,unsigned id);
int ke_replace(KEpoch *);
int ke_reclaim(KEpoch *);
int ke_close(KEpoch *);
int ke_requeue(const KEpoch *,uint32_t captured_generation);
#endif
