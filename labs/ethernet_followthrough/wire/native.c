#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
void ew_note(const char *s) { puts(s); }
void ew_observation(const char *s, const uint32_t *v, size_t n) {
  printf("OBS wire_%s", s);
  for (size_t i = 0; i < n; ++i)
    printf(" %u", (unsigned)v[i]);
  puts("");
}
int ew_tests(unsigned);
int main(int argc, char **argv) {
  return argc == 2 ? ew_tests((unsigned)strtoul(argv[1], 0, 10)) : 41;
}
