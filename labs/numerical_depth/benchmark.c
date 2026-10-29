#define _POSIX_C_SOURCE 200809L
#include "numeric.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>
static struct nd_record aos[4096];
static double xs[4096], ys[4096];
static char labels[4096][24];
static volatile double sink;
static uint64_t now(void) {
  struct timespec t;
  if (clock_gettime(CLOCK_MONOTONIC, &t))
    return 0;
  return (uint64_t)t.tv_sec * 1000000000u + (uint64_t)t.tv_nsec;
}
__attribute__((noinline)) static double run(size_t n, int soa, int all) {
  double s = 0;
  for (size_t i = 0; i < n; i++) {
    s += soa ? xs[i] : aos[i].x;
    if (all) {
      s += soa ? ys[i] : aos[i].y;
      for (size_t k = 0; k < 24; k++)
        s += soa ? labels[i][k] : aos[i].label[k];
    }
  }
  return s;
}
int main(void) {
  for (size_t i = 0; i < 4096; i++) {
    aos[i].x = xs[i] = (double)(i % 16);
    aos[i].y = ys[i] = (double)(i % 8);
    for (size_t k = 0; k < 24; k++)
      aos[i].label[k] = labels[i][k] = (char)(k % 4);
  }
  for (size_t n = 16; n <= 4096; n *= 16)
    for (int all = 0; all < 2; all++)
      for (unsigned rep = 0; rep < 9; rep++)
        for (int soa = 0; soa < 2; soa++) {
          double reference = run(n, 0, all), value = 0;
          uint64_t start = now();
          for (unsigned i = 0; i < 256; i++) {
            __asm__ __volatile__("" : : "g"(xs), "g"(aos) : "memory");
            value = run(n, soa, all);
            sink = value;
          }
          uint64_t elapsed = now() - start;
          if (value != reference)
            return 1;
          printf("{\"n\":%zu,\"all_fields\":%d,\"soa\":%d,\"repeat\":%u,\"ns\":"
                 "%llu,\"result\":\"%a\"}\n",
                 n, all, soa, rep, (unsigned long long)elapsed, value);
        }
  return 0;
}
