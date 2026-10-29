#include "mutex.h"
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;
static cmx_kernel kernel;
static struct cd_recursive recursive;
static cmx_mutex a, b;
static struct cd_record record;
static void semihost(uint32_t op, const void *arg) {
  register uint32_t r0 __asm__("r0") = op;
  register const void *r1 __asm__("r1") = arg;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}
__attribute__((noreturn)) static void stop(unsigned status) {
  uint32_t arguments[2] = {0x20026u, status};
  semihost(4, status == 0   ? "CDM_M4_PASS\n"
              : status == 1 ? "CDM_M4_CHECK\n"
                            : "CDM_M4_FAULT\n");
  semihost(0x20, arguments);
  for (;;) {
  }
}
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      stop(1);                                                                 \
  } while (0)
static void observe(void *context) {
  (void)context;
  CHECK(cmx_mutex_lock(&kernel, &a, 1, cmx_at(kernel.now)) == CMX_TIMEOUT);
}
static void exercise(void) {
  cmx_init(&kernel, CMX_FIXED);
  CHECK(cmx_task_add(&kernel, 9, 0, 0x20006000, 256) == 0);
  CHECK(cmx_task_add(&kernel, 3, 0, 0x20006200, 256) == 1);
  CHECK(cdm_recursive_init(&recursive, &kernel, 2) == CMX_OK);
  CHECK(cdm_recursive_lock(&recursive, 0, cmx_forever()) == CMX_OK);
  CHECK(cdm_recursive_lock(&recursive, 0, cmx_forever()) == CMX_OK);
  CHECK(cdm_recursive_lock(&recursive, 0, cmx_forever()) == CMX_FULL);
  CHECK(cdm_recursive_lock(&recursive, 1, cmx_forever()) == CMX_BLOCKED);
  CHECK(cdm_recursive_unlock(&recursive, 0) == CMX_OK &&
        recursive.mutex.owner == 0);
  CHECK(cdm_recursive_unlock(&recursive, 0) == CMX_OK &&
        recursive.mutex.owner == 1 && recursive.depth == 1);
  CHECK(cdm_recursive_unlock(&recursive, 1) == CMX_OK &&
        cdm_recursive_close(&recursive) == CMX_OK);
  CHECK(cmx_mutex_init(&kernel, &a, 0, 0) == CMX_OK &&
        cmx_mutex_init(&kernel, &b, 0, 0) == CMX_OK);
  cmx_mutex *locks[] = {&a, &b};
  const unsigned ranks[] = {10, 20};
  CHECK(cdm_ordered_lock(&kernel, locks, ranks, 2, 1, 0, cmx_forever()) ==
        CMX_OK);
  CHECK(cdm_ordered_lock(&kernel, locks, ranks, 2, 0, 0, cmx_forever()) ==
        CMX_DEADLOCK);
  CHECK(cmx_mutex_unlock(&kernel, &b, 0) == CMX_OK);
  CHECK(cdm_ordered_lock(&kernel, locks, ranks, 2, 0, 0, cmx_forever()) ==
        CMX_OK);
  cdm_record_init(&record, 0);
  CHECK(cdm_record_apply(&kernel, &a, 0, &record, 1, observe, 0) == CMX_OK &&
        a.owner == 0);
  cdm_record_init(&record, INT32_MAX);
  uint32_t checksum = record.checksum;
  CHECK(cdm_record_apply(&kernel, &a, 0, &record, 1, 0, 0) == CMX_FULL &&
        record.balance == INT32_MAX && record.checksum == checksum);
  CHECK(cmx_mutex_unlock(&kernel, &a, 0) == CMX_OK && cmx_invariant(&kernel));
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
__attribute__((section(".vectors"), used)) const void *cdm_vectors[] = {
    &cl_stack_top, cl_reset, fault, fault, fault, fault, fault, 0, 0, 0, 0,
    fault,         fault,    0,     fault, fault};
