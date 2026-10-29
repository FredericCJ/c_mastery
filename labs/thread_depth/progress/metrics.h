#ifndef THREAD_METRICS_H
#define THREAD_METRICS_H
#include "progress.h"
#include <pthread.h>
#include <stdatomic.h>
struct tp_work {
  uint64_t sum, checksum, items, shared_updates;
};
int tp_separated(uintptr_t, size_t, size_t, size_t, size_t);
uint32_t tp_mix(uint32_t, unsigned);
int tp_accumulate(const unsigned *, size_t, unsigned, unsigned, unsigned,
                  unsigned, unsigned, _Atomic(uint64_t) *, struct tp_work *);
struct tp_total {
  pthread_mutex_t lock;
  uint64_t value, items;
};
int tp_total_init(struct tp_total *);
int tp_total_add(struct tp_total *, uint64_t);
int tp_total_read(struct tp_total *, uint64_t *, uint64_t *);
int tp_total_destroy(struct tp_total *);
#endif
