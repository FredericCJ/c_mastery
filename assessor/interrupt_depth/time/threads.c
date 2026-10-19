#include "checkpoint_firmware.h"
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
static CPFClock clock_state;
static atomic_uint ready, done;
static void *writer(void *unused) {
  (void)unused;
  while (!atomic_load(&ready))
    sched_yield();
  for (uint32_t i = 1; i <= 5000; ++i)
    if (!cpf_clock_write(&clock_state, ((uint64_t)(~i) << 32) | i))
      return (void *)1;
  atomic_store(&done, 1);
  return 0;
}
int main(void) {
  pthread_t thread;
  void *result;
  if (!cpf_clock_init(&clock_state) ||
      !cpf_clock_write(&clock_state, UINT64_C(0xffffffff00000000)))
    return 1;
  if (pthread_create(&thread, 0, writer, 0))
    return 1;
  atomic_store(&ready, 1);
  unsigned accepted = 0, busy = 0;
  do {
    uint64_t value = 0;
    if (cpf_clock_read(&clock_state, 2, &value)) {
      if ((uint32_t)(value >> 32) != ~(uint32_t)value)
        return 2;
      ++accepted;
    } else
      ++busy;
  } while (!atomic_load(&done));
  if (pthread_join(thread, &result) || result || !accepted)
    return 3;
  uint64_t final;
  if (!cpf_clock_read(&clock_state, 1, &final) || (uint32_t) final != 5000)
    return 4;
  printf("XK_THREADS_PASS writes=5000 accepted=%u bounded_refusals=%u\n",
         accepted, busy);
  return 0;
}
