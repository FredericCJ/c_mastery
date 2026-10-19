#ifndef DISPATCH_SPIKE_ABI_H
#define DISPATCH_SPIKE_ABI_H
#include <stddef.h>
typedef struct {
  unsigned abi;
  size_t size;
  int (*apply)(int);
} SpikeAPI;
#endif
