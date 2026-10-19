#include <stdlib.h>
int eh_tests(unsigned);
int eh_stack(void);
int eh_reassembly(void);
int main(int argc, char **argv) {
  if (argc != 2)
    return 41;
  unsigned n = (unsigned)strtoul(argv[1], 0, 10);
#ifdef EH_CORE_ONLY
  return eh_tests(n);
#else
  return n == 4 ? eh_stack() : n == 5 ? eh_reassembly() : eh_tests(n);
#endif
}
