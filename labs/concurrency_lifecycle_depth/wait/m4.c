#include "irq.h"
#define REG(address) (*(volatile uint32_t *)(uintptr_t)(address))
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;
static cmx_kernel kernel;
static CWWorld world;
static volatile unsigned delivered, last_ipsr, inject;

static void semihost(uint32_t op, const void *arg) {
  register uint32_t r0 __asm__("r0") = op;
  register const void *r1 __asm__("r1") = arg;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}
__attribute__((noreturn)) static void stop(unsigned status) {
  uint32_t arguments[2] = {0x20026u, status};
  semihost(4, status == 0 ? "CW_M4_PASS\n"
                          : (status == 1 ? "CW_M4_CHECK\n" : "CW_M4_FAULT\n"));
  semihost(0x20, arguments);
  for (;;) {
  }
}
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      stop(1);                                                                 \
  } while (0)
static uint32_t mask(void) {
  uint32_t p;
  __asm__ volatile("mrs %0,primask" : "=r"(p));
  return p;
}
void cw_irq_enrollment_hook(void) {
  CHECK(mask() == 1);
  if (inject) {
    unsigned before = delivered;
    REG(0xe000e200) = 1; /* NVIC ISPR[0], original synthetic IRQ0 only. */
    __asm__ volatile("dsb\nisb" ::: "memory");
    CHECK(delivered == before);
  }
}
static void irq0(void) {
  uint32_t ipsr;
  __asm__ volatile("mrs %0,ipsr" : "=r"(ipsr));
  last_ipsr = ipsr;
  CHECK(cw_irq_post(&world, 0) == CMX_OK);
  ++delivered;
}
static void await_interrupt(unsigned expected) {
  for (unsigned spin = 0; spin < 100000 && delivered != expected; ++spin)
    __asm__ volatile("nop" ::: "memory");
  CHECK(delivered == expected && last_ipsr == 16);
}
static void exercise(void) {
  cmx_init(&kernel, CMX_FIXED);
  CHECK(cmx_task_add(&kernel, 3, 100, 0x20006000, 256) == 0);
  cw_init(&world, &kernel);
  CHECK(cw_object_init(&world, 0, CW_SEMAPHORE, 0, 2) == CMX_OK);
  REG(0xe000e100) = 1; /* NVIC ISER[0]. Other external IRQs remain disabled. */
  inject = 1;
  CWToken token = {0};
  CHECK(cw_irq_begin(&world, 0, 0, cmx_forever(), 0, &token) == CMX_BLOCKED);
  await_interrupt(1);
  CHECK(mask() == 0 && !world.nodes[0].linked && world.nodes[0].terminals == 1);
  CHECK(world.nodes[0].result == CMX_OK &&
        world.objects[0].semaphore.count == 0);
  uint32_t outer = cw_irq_save();
  CHECK(outer == 0 && mask() == 1);
  CHECK(cw_irq_begin(&world, 0, 0, cmx_forever(), 0, &token) == CMX_BLOCKED);
  CHECK(mask() == 1 && delivered == 1 && world.nodes[0].linked);
  cw_irq_restore(outer);
  await_interrupt(2);
  CHECK(!world.nodes[0].linked && world.nodes[0].terminals == 2);
  /* Pre-existing readiness must be consumed by the protected recheck. */
  inject = 0;
  CHECK(cw_irq_post(&world, 0) == CMX_OK);
  CHECK(cw_irq_begin(&world, 0, 0, cmx_at(kernel.now), 0, &token) == CMX_OK);
  CHECK(world.objects[0].semaphore.count == 0 && world.nodes[0].terminals == 3);
  CHECK(world.objects[0].posts == 3 && world.objects[0].acquired == 3);
  CHECK(cw_object_init(&world, 1, CW_MUTEX, 0, 0) == CMX_OK);
  world.objects[1].acquired = UINT64_MAX - 1;
  CHECK(cw_begin(&world, 1, 0, cmx_forever(), 0, &token) == CMX_OK);
  CHECK(world.objects[1].acquired == UINT64_MAX);
  CHECK(cw_unlock(&world, 1, 0) == CMX_OK && cw_close(&world, 1) == CMX_OK);
  CHECK(cmx_invariant(&kernel));
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
__attribute__((section(".vectors"), used)) const void *cw_vectors[] = {
    &cl_stack_top, cl_reset, fault, fault, fault, fault, fault, 0, 0, 0, 0,
    fault,         fault,    0,     fault, fault, irq0};
