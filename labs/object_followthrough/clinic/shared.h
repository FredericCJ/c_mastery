#ifndef MC_SHARED_H
#define MC_SHARED_H
#include "clinic.h"
#include <pthread.h>
typedef struct {pthread_mutex_t mutex;unsigned *value;unsigned references,releases;int initialized;McAllocator allocation;} McShared;
int mc_shared_init(McShared *,McAllocator);
int mc_shared_retain(McShared *,McShared **);
int mc_shared_add(McShared *,unsigned);
int mc_shared_read(McShared *,unsigned *);
int mc_shared_drop(McShared **);
int mc_shared_finish(McShared *);
#endif
