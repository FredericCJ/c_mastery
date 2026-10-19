#include <stdio.h>
#include <stdlib.h>
void dr_note(const char *s) { fputs(s, stdout); }
int dr_test(unsigned);
int main(int n, char **a) {
  return n == 2 ? dr_test((unsigned)strtoul(a[1], 0, 10)) : 41;
}
