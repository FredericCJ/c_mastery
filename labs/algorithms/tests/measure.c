#define _POSIX_C_SOURCE 200809L
#include "algo.h"
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
static uint64_t ns(struct timespec t) {
  return (uint64_t)t.tv_sec * UINT64_C(1000000000) + (uint64_t)t.tv_nsec;
}
int main(void) {
  uint32_t data[1024];
  for (uint32_t i = 0; i < 1024; ++i)
    data[i] = i * 2;
  puts("sample,queries,key_comparisons,wall_ns,cpu_ns,checksum,array_bytes,"
       "scope");
  for (unsigned sample = 0; sample < 9; ++sample) {
    struct timespec wall0, wall1, cpu0, cpu1;
    algo_work work = {0};
    uint64_t checksum = 0;
    if (clock_gettime(CLOCK_MONOTONIC, &wall0) ||
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu0))
      return 77;
    for (uint32_t q = 0; q < 32768; ++q) {
      uint32_t result = 0, key = (q * 17) % 2049;
      if (algo_lower(data, 1024, key, &result, &work) != ALGO_OK ||
          result != (key + 1) / 2)
        return 1;
      checksum += result;
    }
    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &cpu1) ||
        clock_gettime(CLOCK_MONOTONIC, &wall1))
      return 77;
    printf("%u,32768,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64
           ",%zu,checked_C_call_including_validation_and_counter_overhead\n",
           sample, work.comparisons, ns(wall1) - ns(wall0), ns(cpu1) - ns(cpu0),
           checksum, sizeof data);
  }
  return 0;
}
