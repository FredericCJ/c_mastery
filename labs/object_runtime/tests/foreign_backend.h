#ifndef ORX_FOREIGN_BACKEND_H
#define ORX_FOREIGN_BACKEND_H
#include <pthread.h>
#include <stddef.h>
typedef struct {
  pthread_mutex_t lock;
  pthread_cond_t changed;
  const unsigned char *bytes;
  size_t length;
  unsigned ready, go, sum;
} ORXBackend;
void *orx_backend_worker(void *context);
#endif
