#include <stdio.h>
#include <stdlib.h>
void lt_note(const char *s) { puts(s); }
int lt_tests(unsigned);
int main(int argc, char **argv) {
  return argc == 2 ? lt_tests((unsigned)strtoul(argv[1], 0, 10)) : 41;
}
