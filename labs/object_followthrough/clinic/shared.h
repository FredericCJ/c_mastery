#ifndef MC_SHARED_H
#define MC_SHARED_H
#include "clinic.h"
#include <pthread.h>
struct mc_shared {pthread_mutex_t mutex;unsigned *value;unsigned references,releases;int initialized;struct mc_allocator allocation;};
int mc_shared_init(struct mc_shared *,struct mc_allocator);
int mc_shared_retain(struct mc_shared *,struct mc_shared **);
int mc_shared_add(struct mc_shared *,unsigned);
int mc_shared_read(struct mc_shared *,unsigned *);
int mc_shared_drop(struct mc_shared **);
int mc_shared_finish(struct mc_shared *);
#endif
