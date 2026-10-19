#ifndef AI_INVARIANTS_H
#define AI_INVARIANTS_H
#include <stddef.h>
#include <stdint.h>
#include "containers.h"
enum AIStatus { AI_OK,AI_DONE,AI_INVALID,AI_UNIMPLEMENTED };
typedef struct { uint32_t a,b; } AIState;
enum AIStatus ai_step(AIState,unsigned subtract,AIState *);
typedef struct { uint32_t key,tag; } AIRecord;
enum AIStatus ai_permutation(const AIRecord *,const AIRecord *,size_t,unsigned *);
typedef struct { size_t copies,writes,worst,capacity,acquisitions,releases; } AIGrowth;
enum AIStatus ai_growth(unsigned count,unsigned additive,AIGrowth *);
typedef struct { CDRing ring;int backing[3];CDQueue queue; } AIQueues;
void ai_queues_init(AIQueues *);
enum CDStatus ai_queue_event(AIQueues *,unsigned put,int value,int *removed);
#endif
