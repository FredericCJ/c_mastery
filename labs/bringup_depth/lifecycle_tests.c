#include "lifecycle.h"
#include <stdio.h>
#define CK(x)                                                                  \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("BU_LIFECYCLE_CHECK:%d\n", __LINE__);                             \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main(void) {
  struct bu_drain d;
  CK(bu_drain_begin(&d, 2, 3) == BU_OK);
  CK(d.stopped && !d.transferred);
  CK(bu_drain_step(&d, 1, 0) == BU_FAILED && !d.transferred);
  CK(bu_drain_step(&d, 0, 1) == BU_FAILED && !d.transferred);
  CK(bu_drain_step(&d, 0, 1) == BU_OK && d.transferred);
  CK(bu_drain_step(&d, 1, 0) == BU_INVALID);
  CK(bu_drain_begin(&d, 0, 1) == BU_OK);
  CK(bu_drain_step(&d, 1, 1) == BU_INVALID && d.remaining == 1);
  CK(bu_drain_step(&d, 1, 0) == BU_OK);
  CK(bu_drain_begin(&d, 1, 1) == BU_OK);
  CK(bu_drain_step(&d, 0, 1) == BU_TIMEOUT && !d.transferred);
  CK(bu_drain_step(&d, 1, 0) == BU_TIMEOUT && !d.transferred);
  struct bu_task t = {0};
  unsigned retained_machine_epoch = 19, startup_calls = 1;
  for (unsigned i = 1; i <= 3; ++i) {
    CK(bu_task_start(&t) == BU_OK && t.generation == i);
    for (unsigned j = 0; j < 8; ++j)
      CK(t.task_bytes[j] == 0);
    t.task_bytes[7] = 99;
    CK(bu_task_return(&t, i, -(int)i, 2) == BU_OK && t.phase == 2);
    CK(bu_task_start(&t) == BU_INVALID && t.generation == i);
    CK(bu_task_drained(&t, i - 1) == BU_INVALID && t.pending == 2);
    CK(bu_task_drained(&t, i) == BU_OK && t.phase == 2);
    CK(bu_task_drained(&t, i) == BU_OK && t.phase == 0);
    CK(bu_task_drained(&t, i) == BU_INVALID && t.last_result == -(int)i);
    CK(retained_machine_epoch == 19 && startup_calls == 1);
  }
  t.generation = UINT_MAX;
  CK(bu_task_start(&t) == BU_INVALID && t.phase == 0);
  puts("BU_LIFECYCLE_PASS drain acknowledgment callbacks restart generation");
  return 0;
}
