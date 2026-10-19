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
typedef size_t (*AKCopy)(void *, unsigned char *, size_t);
typedef struct {
  unsigned char bytes[32];
  size_t length;
} AKPacket;
enum ak_result ak_copy(AKPacket *, size_t, AKCopy, void *);
typedef struct {
  int (*map)(void *, const ASValue *, unsigned *);
  void (*sync_device)(void *, unsigned);
  int (*submit)(void *, unsigned);
  int (*quiesce)(void *, unsigned);
  void (*sync_cpu)(void *, unsigned);
  void (*unmap)(void *, unsigned);
  void *context;
} AKBackend;
typedef struct {
  ASQueue queue;
  AKBackend backend;
  ASValue pending;
  unsigned state, cookie;
} AKService;
enum ak_result ak_init(AKService *, AKBackend);
enum ak_result ak_enqueue(AKService *, ASValue);
enum ak_result ak_start(AKService *);
enum ak_result ak_complete(AKService *, ASValue *);
#endif
