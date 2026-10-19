#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int teaching_render(const char *, int *, char *, size_t);
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "SC_FORMAT_ASSERT:%s\n", #x);                            \
      exit(41);                                                                \
    }                                                                          \
  } while (0)
int main(void) {
  unsigned cap;
  for (cap = 1; cap <= 3; ++cap) {
    char out[4] = {'X', 'X', 'X', 'X'};
    int marker = 77;
    int n = teaching_render("%n", &marker, out, cap);
#ifdef SC_REPAIRED_FORMAT
    CHECK(marker == 77 && n == 2);
    CHECK(out[cap - 1] == 0);
    CHECK((n >= (int)cap) == (cap < 3));
    if (cap == 3)
      CHECK(!strcmp(out, "%n"));
#else
    CHECK(marker == 0 && n == 0 && out[0] == 0);
#endif
    CHECK(out[cap] == 'X');
  }
  puts("OBS format synthetic_marker_only=1 exact_type=int_pointer "
       "capacities=1,2,3 checked_truncation=1");
  return 0;
}
