#define _POSIX_C_SOURCE 200809L
#include "index.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static uint64_t now(void) {
  struct timespec t;
  if (clock_gettime(CLOCK_MONOTONIC, &t))
    exit(2);
  return (uint64_t)t.tv_sec * 1000000000u + (uint64_t)t.tv_nsec;
}
static unsigned flat_count(const unsigned char *t, unsigned n,
                           const unsigned char *p, unsigned m) {
  unsigned count = 0;
  for (unsigned i = 0; i + m <= n; i++) {
    unsigned j = 0;
    while (j < m && t[i + j] == p[j])
      j++;
    count += j == m;
  }
  return count;
}
int main(void) {
  const unsigned sizes[] = {8, 32, 64};
  puts("size,distribution,round,order,build_ns,index_ns,scan_ns,queries,"
       "checksum,object_bytes");
  for (unsigned z = 0; z < 3; z++)
    for (unsigned distribution = 0; distribution < 3; distribution++)
      for (unsigned round = 0; round < 9; round++) {
        unsigned n = sizes[z];
        unsigned char bytes[CI_CAP], patterns[32][4];
        for (unsigned i = 0; i < n; i++)
          bytes[i] = distribution == 0 ? 'a'
                     : distribution == 1
                         ? (unsigned char)(i % 4)
                         : (unsigned char)((i * 53u + round * 17u) % 251u);
        for (unsigned k = 0; k < 32; k++)
          for (unsigned j = 0; j < 4; j++)
            patterns[k][j] =
                (k % 2) ? bytes[(k + j) % n] : (unsigned char)(252 + j);
        CiSuffix s;
        uint64_t t = now();
        for (unsigned k = 0; k < 1000; k++)
          if (ci_suffix_build(&s, bytes, n) != CI_OK)
            return 3;
        uint64_t build = now() - t;
        uint64_t durations[2], sums[2];
        for (unsigned order = 0; order < 2; order++) {
          unsigned mode = (order + round) % 2;
          uint64_t sum = 0;
          t = now();
          for (unsigned k = 0; k < 20000; k++) {
            unsigned lo, hi;
            if (!mode) {
              if (ci_suffix_range(&s, patterns[k % 32], 4, &lo, &hi) != CI_OK)
                return 4;
              sum += hi - lo;
            } else
              sum += flat_count(bytes, n, patterns[k % 32], 4);
          }
          durations[mode] = now() - t;
          sums[mode] = sum;
        }
        if (sums[0] != sums[1])
          return 5;
        printf("%u,%u,%u,%u,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",20000,%" PRIu64
               ",%zu\n",
               n, distribution, round, round % 2, build, durations[0],
               durations[1], sums[0], sizeof s);
      }
  return 0;
}
