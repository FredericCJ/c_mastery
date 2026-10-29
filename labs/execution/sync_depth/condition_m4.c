#include "condition.h"
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;
static cmx_kernel kernel;
static struct cd_queue queue;
static cmx_mutex other;
static void semihost(uint32_t op, const void *arg) {
  register uint32_t r0 __asm__("r0") = op;
  register const void *r1 __asm__("r1") = arg;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}
__attribute__((noreturn)) static void stop(unsigned status) {
  uint32_t arguments[2] = {0x20026u, status};
  semihost(4, status == 0   ? "CDQ_M4_PASS\n"
              : status == 1 ? "CDQ_M4_CHECK\n"
                            : "CDQ_M4_FAULT\n");
  semihost(0x20, arguments);
  for (;;) {
  }
}
#define CHECK(c)                                                               \
  do {                                                                         \
    if (!(c))                                                                  \
      stop(1);                                                                 \
  } while (0)
static void setup(void) {
  cmx_init(&kernel, CMX_FIXED);
  for (unsigned t = 0; t < 4; t++)
    CHECK(cmx_task_add(&kernel, 9 - t, 0, 0x20006000 + 0x200 * t, 256) ==
          (int)t);
  CHECK(cdq_init(&queue, &kernel, 1, 1) == CMX_OK);
}
static void exercise(void) {
  uint32_t value;
  int result;
  setup();
  CHECK(cdq_begin(&queue, 0, CDQ_GET, 0, cmx_forever()) == CMX_BLOCKED);
  CHECK(cmx_mutex_lock(&kernel, &queue.mutex, 3, cmx_forever()) == CMX_OK);
  CHECK(cdq_put_locked(&queue, 3, 42) == CMX_OK &&
        cdq_get_locked(&queue, 3, &value) == CMX_OK);
  CHECK(cmx_mutex_unlock(&kernel, &queue.mutex, 3) == CMX_OK);
  CHECK(cdq_resume(&queue, 0) == CMX_BLOCKED && queue.count == 0);
  CHECK(cmx_mutex_lock(&kernel, &queue.mutex, 3, cmx_forever()) == CMX_OK);
  CHECK(cdq_shutdown(&queue, 3) == CMX_OK);
  CHECK(cmx_mutex_unlock(&kernel, &queue.mutex, 3) == CMX_OK);
  CHECK(cdq_resume(&queue, 0) == CMX_CLOSED);
  CHECK(cdq_result(&queue, 0, &result, &value) == CMX_OK &&
        result == CMX_CLOSED);
  CHECK(cdq_destroy(&queue) == CMX_OK);
  setup();
  CHECK(cmx_mutex_init(&kernel, &other, 0, 0) == CMX_OK);
  CHECK(cmx_mutex_lock(&kernel, &other, 1, cmx_forever()) == CMX_OK);
  CHECK(cdq_pulse(&queue, &other, 1, 1) == CMX_INVALID);
  CHECK(cmx_mutex_unlock(&kernel, &other, 1) == CMX_OK);
  CHECK(cdq_begin(&queue, 0, CDQ_GET, 0, cmx_at(5)) == CMX_BLOCKED);
  CHECK(cmx_mutex_lock(&kernel, &queue.mutex, 3, cmx_forever()) == CMX_OK);
  cmx_tick(&kernel, 5);
  CHECK(cdq_resume(&queue, 0) == CMX_BLOCKED);
  cmx_tick(&kernel, 99);
  CHECK(cdq_resume(&queue, 0) == CMX_BLOCKED);
  CHECK(cmx_mutex_unlock(&kernel, &queue.mutex, 3) == CMX_OK);
  CHECK(cdq_resume(&queue, 0) == CMX_TIMEOUT);
  CHECK(cdq_result(&queue, 0, &result, &value) == CMX_OK &&
        result == CMX_TIMEOUT);
  CHECK(cdq_destroy(&queue) == CMX_OK);
  setup();
  for (unsigned t = 0; t < 3; t++)
    CHECK(cdq_begin(&queue, t, CDQ_GET, 0, cmx_forever()) == CMX_BLOCKED);
  CHECK(cmx_mutex_lock(&kernel, &queue.mutex, 3, cmx_forever()) == CMX_OK);
  CHECK(cdq_put_locked(&queue, 3, 44) == CMX_OK);
  for (unsigned t = 0; t < 3; t++)
    CHECK(kernel.task[t].wait == CMX_W_MUTEX);
  CHECK(cmx_mutex_unlock(&kernel, &queue.mutex, 3) == CMX_OK);
  CHECK(cdq_resume(&queue, 0) == CMX_OK &&
        cdq_resume(&queue, 1) == CMX_BLOCKED &&
        cdq_resume(&queue, 2) == CMX_BLOCKED);
  CHECK(cdq_result(&queue, 0, &result, &value) == CMX_OK && result == CMX_OK &&
        value == 44);
  CHECK(cmx_mutex_lock(&kernel, &queue.mutex, 3, cmx_forever()) == CMX_OK &&
        cdq_shutdown(&queue, 3) == CMX_OK);
  CHECK(cmx_mutex_unlock(&kernel, &queue.mutex, 3) == CMX_OK);
  for (unsigned t = 1; t < 3; t++) {
    CHECK(cdq_resume(&queue, t) == CMX_CLOSED);
    CHECK(cdq_result(&queue, t, &result, &value) == CMX_OK &&
          result == CMX_CLOSED);
  }
  CHECK(cdq_destroy(&queue) == CMX_OK && cmx_invariant(&kernel));
}
void cl_reset(void) {
  uintptr_t source = (uintptr_t)&cl_data_load;
  for (uintptr_t p = (uintptr_t)&cl_data_start; p < (uintptr_t)&cl_data_end;
       p += 4, source += 4)
    *(uint32_t *)p = *(const uint32_t *)source;
  for (uintptr_t p = (uintptr_t)&cl_bss_start; p < (uintptr_t)&cl_bss_end;
       p += 4)
    *(uint32_t *)p = 0;
  exercise();
  stop(0);
}
static void fault(void) { stop(2); }
__attribute__((section(".vectors"), used)) const void *cdq_vectors[] = {
    &cl_stack_top, cl_reset, fault, fault, fault, fault, fault, 0, 0, 0, 0,
    fault,         fault,    0,     fault, fault};
