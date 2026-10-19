#ifndef THREAD_METRICS_H
#define THREAD_METRICS_H
#include "progress.h"
#include <pthread.h>
#include <stdatomic.h>
typedef struct {
  uint64_t sum, checksum, items, shared_updates;
} TPWork;
int tp_separated(uintptr_t, size_t, size_t, size_t, size_t);
uint32_t tp_mix(uint32_t, unsigned);
int tp_accumulate(const unsigned *, size_t, unsigned, unsigned, unsigned,
                  unsigned, unsigned, _Atomic(uint64_t) *, TPWork *);
typedef struct {
  pthread_mutex_t lock;
  uint64_t value, items;
} TPTotal;
int tp_total_init(TPTotal *);
int tp_total_add(TPTotal *, uint64_t);
int tp_total_read(TPTotal *, uint64_t *, uint64_t *);
int tp_total_destroy(TPTotal *);
#endif
