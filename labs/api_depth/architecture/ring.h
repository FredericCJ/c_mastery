#ifndef CMASTERY_ARCH_RING_H
#define CMASTERY_ARCH_RING_H
#include "arch.h"
#include "containers.h"

struct ar_ring_policy
{
    struct cd_ring ring;
    int indices[AR_CAP];
    struct ar_item values[AR_CAP];
    unsigned live[AR_CAP];
};

int ar_ring_init(struct ar_ring_policy *, size_t);
int ar_ring_push(struct ar_ring_policy *, struct ar_item, unsigned,
                 struct ar_eviction *);
int ar_ring_pop(struct ar_ring_policy *, struct ar_item *);
#endif
