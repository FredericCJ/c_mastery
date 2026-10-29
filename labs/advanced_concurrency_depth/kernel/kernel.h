#ifndef ADVANCED_KERNEL_BOUNDARY_H
#define ADVANCED_KERNEL_BOUNDARY_H
#include "spsc.h"
#include <stddef.h>
enum ak_result {
  AK_OK,
  AK_INVALID,
  AK_FAULT,
  AK_BUSY,
  AK_IO,
  AK_EMPTY,
  AK_UNIMPLEMENTED
};
typedef size_t (*ak_copy_fn)(void *, unsigned char *, size_t);
struct ak_packet {
  unsigned char bytes[32];
  size_t length;
};
enum ak_result ak_copy(struct ak_packet *, size_t, ak_copy_fn, void *);
struct ak_backend {
  int (*map)(void *, const struct as_value *, unsigned *);
  void (*sync_device)(void *, unsigned);
  int (*submit)(void *, unsigned);
  int (*quiesce)(void *, unsigned);
  void (*sync_cpu)(void *, unsigned);
  void (*unmap)(void *, unsigned);
  void *context;
};
struct ak_service {
  struct as_queue queue;
  struct ak_backend backend;
  struct as_value pending;
  unsigned state, cookie;
};
enum ak_result ak_init(struct ak_service *, struct ak_backend);
enum ak_result ak_enqueue(struct ak_service *, struct as_value);
enum ak_result ak_start(struct ak_service *);
enum ak_result ak_complete(struct ak_service *, struct as_value *);
#endif
