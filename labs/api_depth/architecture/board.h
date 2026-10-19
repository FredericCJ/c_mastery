#ifndef CMASTERY_ARCH_BOARD_H
#define CMASTERY_ARCH_BOARD_H
#include "arch.h"
typedef struct {
  void *context;
  uint32_t (*read)(void *, uintptr_t);
  void (*write)(void *, uintptr_t, uint32_t);
  uintptr_t gpio;
  unsigned pin, active_low;
} ArBoard;
int ar_board_init(ArBoard *);
int ar_board_set(void *, unsigned);
int ar_clear_pending(ArBoard *, uintptr_t, uint32_t);
#endif
