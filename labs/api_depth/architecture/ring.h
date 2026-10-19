#ifndef CMASTERY_ARCH_RING_H
#define CMASTERY_ARCH_RING_H
#include "arch.h"
#include "containers.h"
typedef struct {
  CDRing ring;
  int indices[AR_CAP];
  ArItem values[AR_CAP];
  unsigned live[AR_CAP];
} ArRingPolicy;
int ar_ring_init(ArRingPolicy *, size_t);
int ar_ring_push(ArRingPolicy *, ArItem, unsigned, ArEviction *);
int ar_ring_pop(ArRingPolicy *, ArItem *);
#endif
