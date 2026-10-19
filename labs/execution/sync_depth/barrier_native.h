#ifndef CMASTERY_BARRIER_NATIVE_H
#define CMASTERY_BARRIER_NATIVE_H
#include "barrier.h"
#include <pthread.h>
typedef struct {
  cmx_kernel kernel;
  CDBarrier barrier;
  CDBMember member[4];
  pthread_mutex_t exclusion;
  pthread_cond_t changed;
  unsigned parties,live;
  uint64_t arrivals;
} CDBNative;
typedef struct { CDBNative leaf[2],root; unsigned live; } CDBTree;
int cdbn_init(CDBNative *,unsigned parties);
int cdbn_wait(CDBNative *,unsigned participant);
int cdbn_destroy(CDBNative *);
int cdbt_init(CDBTree *);
int cdbt_wait(CDBTree *,unsigned participant);
int cdbt_destroy(CDBTree *);
#endif
