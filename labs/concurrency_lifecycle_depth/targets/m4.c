#include "cl.h"
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;
static void semihost(uint32_t op, const void *arg) {
  register uint32_t r0 __asm__("r0") = op;
  register const void *r1 __asm__("r1") = arg;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}
__attribute__((noreturn)) static void stop(unsigned status) {
  uint32_t args[2] = {0x20026u, status};
  semihost(4, status == 0   ? "CL_M4_PASS\n"
              : status == 1 ? "CL_M4_CHECK\n"
                            : "CL_M4_FAULT\n");
  semihost(0x20, args);
  for (;;) {
  }
}
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      stop(1);                                                                 \
  } while (0)
static void exercise(void) {
  CLMessage slots[2];
  CLQueue q;
  unsigned char b[8] = {3, 0, 9, 7, 1, 2, 3, 4}, out[8];
  size_t n = 99;
  uint32_t p = 99, s = 99;
  cl_queue_init(&q, slots);
  CHECK(cl_send(&q, 1, 1, b, 4) == CL_OK);
  b[0] = 88;
  CHECK(cl_send(&q, 2, 1, 0, 0) == CL_OK && cl_send(&q, 3, 1, b, 1) == CL_FULL);
  CHECK(cl_recv(&q, out, 3, &n, &p, &s) == CL_RANGE && n == 99);
  CHECK(cl_recv(&q, out, 4, &n, &p, &s) == CL_OK && q.count == 1 &&
        out[0] == 3 && out[1] == 0 && s == 1);
  CHECK(cl_abort(&q) == 1 && q.accepted == q.delivered + q.discarded);
  CLPool pool;
  CLOwnedQueue owned;
  CLBuffer h[4], large;
  cl_pool_init(&pool);
  cl_owned_init(&owned);
  for (unsigned i = 0; i < 4; i++)
    CHECK(cl_alloc(&pool, 4, &h[i]) == CL_OK);
  CHECK(cl_release(&pool, h[0]) == CL_OK && cl_release(&pool, h[2]) == CL_OK);
  CHECK(cl_alloc(&pool, 6, &large) == CL_FRAGMENTED &&
        cl_release(&pool, h[1]) == CL_OK &&
        cl_alloc(&pool, 6, &large) == CL_OK);
  CHECK(cl_owned_send(&pool, &owned, large) == CL_OK &&
        cl_release(&pool, large) == CL_BUSY);
  CHECK(cl_owned_abort(&pool, &owned) == 1 &&
        cl_release(&pool, h[3]) == CL_OK && pool.allocations == pool.releases);
  CLLife life;
  CLLease lease;
  cl_life_init(&life);
  CHECK(cl_acquire(&life, &lease) == CL_OK);
  CHECK(cl_transition(&life, lease, CL_ACTIVE, CL_COND_RELOCK) == CL_OK);
  cl_life_close(&life);
  CHECK(life.leases[lease.slot].state == CL_COND_RELOCK &&
        cl_life_drained(&life) == CL_OK && cl_destroy(&life) == CL_BUSY);
  CHECK(cl_transition(&life, lease, CL_COND_RELOCK, CL_ACTIVE) == CL_OK &&
        cl_drop(&life, lease) == CL_OK);
  CHECK(cl_destroy(&life) == CL_OK && cl_restart(&life) == CL_OK &&
        cl_drop(&life, lease) == CL_STALE);
  CLRecord r;
  uint32_t value = 99;
  cl_record_init(&r, 7);
  CHECK(cl_record_begin(&r, 1) == CL_OK &&
        cl_record_stage(&r, 1, 13) == CL_OK &&
        cl_record_read(&r, &value) == CL_POISONED);
  CHECK(cl_record_dead(&r, 1) == CL_OK && cl_repair_begin(&r, 2) == CL_OK &&
        cl_repair_finish(&r, 2, 1) == CL_OK);
  CHECK(cl_record_read(&r, &value) == CL_OK && value == 7);
  CLAck ack;
  cl_ack_init(&ack, 5);
  CHECK(cl_request(&ack, 5, 1, 7, &value) == CL_PENDING &&
        cl_apply(&ack, 1) == CL_OK);
  CHECK(cl_request(&ack, 5, 1, 7, &value) == CL_OK && value == 7 &&
        ack.value == 7);
  CHECK(cl_request(&ack, 5, 1, 8, &value) == CL_RANGE);
}
void cl_reset(void) {
  /* Numeric linker ranges are a Cortex-M linker/ABI contract, not C arrays. */
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
__attribute__((section(".vectors"), used)) const void *cl_vectors[] = {
    &cl_stack_top, cl_reset, fault, fault, fault, fault, fault, 0, 0, 0, 0,
    fault,         fault,    0,     fault, fault};
