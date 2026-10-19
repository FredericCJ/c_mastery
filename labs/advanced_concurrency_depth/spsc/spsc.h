#ifndef ADVANCED_SPSC_H
#define ADVANCED_SPSC_H
#include "cmx.h"
typedef struct {
  uint32_t sequence, inverse, salt, triple;
} ASValue;
typedef void (*ASHook)(void *);
typedef struct {
  _Alignas(64) atomic_uint head;
  _Alignas(64) atomic_uint tail;
  unsigned capacity, producer_active, consumer_active;
  ASValue data[16];
} ASQueue;
enum cmx_result as_init(ASQueue *, unsigned);
enum cmx_result as_push(ASQueue *, ASValue, ASHook, void *);
enum cmx_result as_pop(ASQueue *, ASValue *, ASHook, void *);
typedef struct {
  atomic_uint head, tail;
  unsigned storage;
  ASValue data[16];
} ASCircular;
enum cmx_result as_circular_init(ASCircular *, unsigned);
enum cmx_result as_circular_push(ASCircular *, ASValue);
enum cmx_result as_circular_pop(ASCircular *, ASValue *);
#endif
