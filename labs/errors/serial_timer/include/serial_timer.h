#ifndef CMASTERY_SERIAL_TIMER_H
#define CMASTERY_SERIAL_TIMER_H
#include "errors.h"

struct eat_window_entry
{
    uint32_t id, deadline, age;
};

struct eat_window
{
    unsigned bits;
    uint32_t now;
    size_t count;
    struct eat_window_entry entry[4];
};

struct eat_epoch_entry
{
    uint32_t id;
    uint64_t deadline;
};

struct eat_epoch
{
    unsigned bits;
    uint64_t now;
    size_t count;
    struct eat_epoch_entry entry[4];
};

/* Fresh output for init; all other controls valid, live, disjoint and serialized.
   Every refusal preserves control and output. IDs need only be unique while queued. */
er_code eat_serial(unsigned bits, uint32_t a, uint32_t b, int *order);
er_code eat_window_init(struct eat_window *, unsigned bits, uint32_t now);
er_code eat_window_arm(struct eat_window *, uint32_t id, uint32_t delay);
er_code eat_window_advance(struct eat_window *, uint32_t certified_elapsed);
er_code eat_window_pop(struct eat_window *, uint32_t *id);
er_code eat_epoch_init(struct eat_epoch *, unsigned display_bits, uint64_t now);
er_code eat_epoch_arm(struct eat_epoch *, uint32_t id, uint64_t delay);
er_code eat_epoch_advance(struct eat_epoch *, uint64_t elapsed);
er_code eat_epoch_pop(struct eat_epoch *, uint32_t *id);
#endif
