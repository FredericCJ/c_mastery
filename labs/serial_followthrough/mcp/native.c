#include "mcp.h"
#include <stdio.h>
#include <stdlib.h>
void mc_note(const char *s) { puts(s); }
void mc_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d) {
  printf("OBS mcp_%s %u %u %u %u\n", s, a, b, c, d);
}
int main(int argc, char **argv) {
  int (*tests[])(void) = {mc_test_layout,   mc_test_init,    mc_test_failure,
                          mc_test_transfer, mc_test_receive, mc_test_bench,
                          mc_test_spi,      mc_test_atomic};
  if (argc == 2) {
    unsigned i = (unsigned)strtoul(argv[1], 0, 10);
    return i < 8 ? (tests[i]() ? 41 : 0) : 2;
  }
  for (unsigned i = 0; i < 8; i++)
    if (tests[i]())
      return 41;
  puts("MCP_NATIVE_PASS");
  return 0;
}
