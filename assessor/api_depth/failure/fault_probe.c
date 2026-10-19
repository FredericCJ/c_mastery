#include "observability.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int broken(void *c, const unsigned char *p, size_t n) {
  (void)c;
  (void)p;
  (void)n;
  return -1;
}
int main(int argc, char **argv) {
  if (argc != 3)
    return 2;
  unsigned capacity = (unsigned)strtoul(argv[2], 0, 10), work = 0;
  if (capacity > 3) {
    printf("FAULT-PASS:external=INVALID;work=0;\n");
    return 0;
  }
  if (!strcmp(argv[1], "sideeffect")) {
#ifdef AE_ASSERT_FLAW
    assert(++work == 1);
#else
    ++work;
    assert(work == 1);
#endif
    printf("SIDE-EFFECT:work=%u;\n", work);
    return 0;
  }
  unsigned internal = 4; /* original deliberate invariant defect */
  if (!strcmp(argv[1], "assert")) {
    assert(internal <= capacity);
    printf("ASSERT-DISABLED:required-work-separate;\n");
    return 0;
  }
  if (internal > capacity) {
    AeReporter r = {.fallback_available = 1};
    AeRecord event = {3, 1, 8, 1, 0, 0, {0}};
    (void)ae_report(&r, &event, broken, 0);
    if (!strcmp(argv[1], "panic")) {
      fprintf(stderr, "FAULT-PANIC:cleanup=0;fallback=%u;process-only;\n",
              r.stored);
      exit(73);
    }
    if (!strcmp(argv[1], "safe")) {
      unsigned outputs_enabled = 0;
      printf("FAULT-SAFE:outputs=%u;cleanup=0;fallback=%u;SIMULATED-NOT-"
             "HARDWARE;\n",
             outputs_enabled, r.stored);
      return 0;
    }
    if (!strcmp(argv[1], "return")) {
      printf("FAULT-RETURN:CONTRACT;cleanup=1;fallback=%u;\n", r.stored);
      return 0;
    }
  }
  return 2;
}
