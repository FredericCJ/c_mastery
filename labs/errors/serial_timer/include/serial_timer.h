#ifndef CMASTERY_SERIAL_TIMER_H
#define CMASTERY_SERIAL_TIMER_H
#include "errors.h"
typedef struct {uint32_t id,deadline,age;} EatWindowEntry;
typedef struct {unsigned bits;uint32_t now;size_t count;EatWindowEntry entry[4];} EatWindow;
typedef struct {uint32_t id;uint64_t deadline;} EatEpochEntry;
typedef struct {unsigned bits;uint64_t now;size_t count;EatEpochEntry entry[4];} EatEpoch;
/* Fresh output for init; all other controls valid, live, disjoint and serialized.
   Every refusal preserves control and output. IDs need only be unique while queued. */
er_code eat_serial(unsigned bits,uint32_t a,uint32_t b,int *order);
er_code eat_window_init(EatWindow *,unsigned bits,uint32_t now);
er_code eat_window_arm(EatWindow *,uint32_t id,uint32_t delay);
er_code eat_window_advance(EatWindow *,uint32_t certified_elapsed);
er_code eat_window_pop(EatWindow *,uint32_t *id);
er_code eat_epoch_init(EatEpoch *,unsigned display_bits,uint64_t now);
er_code eat_epoch_arm(EatEpoch *,uint32_t id,uint64_t delay);
er_code eat_epoch_advance(EatEpoch *,uint64_t elapsed);
er_code eat_epoch_pop(EatEpoch *,uint32_t *id);
#endif
