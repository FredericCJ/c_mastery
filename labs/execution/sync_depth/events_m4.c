#include "events.h"
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;
static cmx_kernel kernel;
static CDEvents events;
static void semihost(uint32_t op, const void *arg) {
  register uint32_t r0 __asm__("r0") = op;
  register const void *r1 __asm__("r1") = arg;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}
__attribute__((noreturn)) static void stop(unsigned status) {
  uint32_t arguments[2] = {0x20026u, status};
  semihost(4, status == 0 ? "CDE_M4_PASS\n" : status == 1 ? "CDE_M4_CHECK\n" : "CDE_M4_FAULT\n");
  semihost(0x20, arguments);
  for (;;) {}
}
#define CHECK(c) do { if (!(c)) stop(1); } while (0)
static void exercise(void) {
  cmx_init(&kernel, CMX_FIXED);
  for (unsigned t = 0; t < 3; t++)
    CHECK(cmx_task_add(&kernel, 9 - t, 0, 0x20006000 + 0x200 * t, 256) == (int)t);
  CHECK(cde_init(&events, &kernel) == CMX_OK);
  uint32_t out = 999;
  CHECK(cde_set(&events, 5) == CMX_OK);
  CHECK(cde_wait(&events, 0, 3, 1, 1, cmx_at(0)) == CMX_TIMEOUT);
  CHECK(cde_result(&events, 0, &out) == CMX_TIMEOUT && out == 999);
  CHECK(cde_wait(&events, 0, 1, 0, 1, cmx_forever()) == CMX_OK);
  CHECK(cde_result(&events, 0, &out) == CMX_OK && out == 1 && events.event.bits == 4);
  CHECK(cde_wait(&events, 0, 4, 0, 0, cmx_forever()) == CMX_OK);
  CHECK(cde_result(&events, 0, &out) == CMX_OK && out == 4 && events.event.bits == 4);
  CHECK(cde_clear(&events, 4) == CMX_OK);
  CHECK(cde_wait(&events, 0, 3, 1, 1, cmx_forever()) == CMX_BLOCKED);
  CHECK(cde_wait(&events, 1, 1, 0, 1, cmx_forever()) == CMX_BLOCKED);
  CHECK(cde_set(&events, 3) == CMX_OK);
  CHECK(cde_result(&events, 0, &out) == CMX_OK && out == 3);
  CHECK(cde_result(&events, 1, &out) == CMX_BLOCKED);
  CHECK(cmx_cancel(&kernel, 1) == CMX_OK && cde_result(&events, 1, &out) == CMX_CLOSED);
  CHECK(cde_set(&events, UINT32_C(1) << 31) == CMX_OK);
  CHECK(cde_wait(&events, 0, UINT32_C(1) << 31, 0, 1, cmx_forever()) == CMX_OK);
  CHECK(cde_result(&events, 0, &out) == CMX_OK && out == (UINT32_C(1) << 31));
  CHECK(cde_wait(&events, 0, 1, 0, 0, cmx_forever()) == CMX_BLOCKED);
  CHECK(cde_close(&events) == CMX_OK && cde_result(&events, 0, &out) == CMX_CLOSED);
  CDESequence sequence; CDECursor cursor;
  uint64_t delta, missed;
  cde_sequence_init(&sequence);
  sequence.current = (CDECursor){7, UINT32_MAX - 1}; cursor = sequence.current;
  CHECK(cde_sequence_publish(&sequence) == CMX_OK && cde_sequence_publish(&sequence) == CMX_OK);
  CHECK(sequence.current.epoch == 8 && sequence.current.serial == 0);
  CHECK(cde_sequence_read(&sequence, &cursor, &delta, &missed) == CMX_OK && delta == 2 && missed == 1);
  CHECK(cmx_invariant(&kernel));
}
void cl_reset(void) {
  uintptr_t source = (uintptr_t)&cl_data_load;
  for (uintptr_t p = (uintptr_t)&cl_data_start; p < (uintptr_t)&cl_data_end; p += 4, source += 4)
    *(uint32_t *)p = *(const uint32_t *)source;
  for (uintptr_t p = (uintptr_t)&cl_bss_start; p < (uintptr_t)&cl_bss_end; p += 4)
    *(uint32_t *)p = 0;
  exercise(); stop(0);
}
static void fault(void) { stop(2); }
__attribute__((section(".vectors"), used)) const void *cde_vectors[] = {
  &cl_stack_top, cl_reset, fault, fault, fault, fault, fault, 0, 0, 0, 0,
  fault, fault, 0, fault, fault};
