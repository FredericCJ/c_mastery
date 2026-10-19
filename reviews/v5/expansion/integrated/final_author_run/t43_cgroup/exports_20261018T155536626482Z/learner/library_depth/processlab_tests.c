#include "processlab.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "PROCESSLAB-CHECK:%s line%d\n", #x, __LINE__);           \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main(void) {
  long double out = 77;
  CHECK(lt_cpu_interval((clock_t)-1, 0, &out) == LT_UNAVAILABLE && out == 77);
  CHECK(lt_cpu_interval(3, 2, &out) == LT_RANGE && out == 77);
  CHECK(lt_cpu_interval(0, CLOCKS_PER_SEC, &out) == LT_OK && out == 1);
  CHECK(lt_cpu_interval(0, 0, &out) == LT_OK && out == 0);
  char first[64], second[64];
  memset(first, '!', sizeof first);
  CHECK(lt_environment("CM_LIBRARY_A", first) == LT_OK &&
        !strcmp(first, "alpha"));
  CHECK(lt_environment("CM_LIBRARY_B", second) == LT_OK &&
        !strcmp(second, "beta") && !strcmp(first, "alpha"));
  CHECK(lt_environment("CM_LIBRARY_MISSING", first) == LT_UNAVAILABLE &&
        !strcmp(first, "alpha"));
  CHECK(lt_environment("CM_LIBRARY_LONG", first) == LT_RANGE &&
        !strcmp(first, "alpha"));
  CHECK(lt_environment(NULL, first) == LT_ARGUMENT &&
        !strcmp(first, "alpha"));
  CHECK(lt_cpu_interval(0, 0, NULL) == LT_ARGUMENT);
  struct tm saved, other;
  CHECK(lt_calendar(2000, &saved) == LT_OK && saved.tm_year == 100 &&
        saved.tm_mon == 1 && saved.tm_mday == 29);
  CHECK(lt_calendar(2001, &other) == LT_OK && other.tm_year == 101 &&
        saved.tm_year == 100 && saved.tm_mday == 29);
  CHECK(lt_calendar(1969, &saved) == LT_RANGE && saved.tm_year == 100);
  printf("PROCESSLAB-PASS:clock_bytes=%zu time_bytes=%zu "
         "ticks_per_second=%.0Lf;copied environment/calendar\n",
         sizeof(clock_t), sizeof(time_t), (long double)CLOCKS_PER_SEC);
  return 0;
}
