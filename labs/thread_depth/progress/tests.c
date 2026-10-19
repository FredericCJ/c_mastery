#include "progress.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "TP_ASSERT:%s:%d:%s\n", __FILE__, __LINE__, #x);         \
      exit(41);                                                                \
    }                                                                          \
  } while (0)
static void donation(void) {
  unsigned base[4] = {1, 2, 3, 7}, out[4] = {99, 99, 99, 99};
  int owners[4] = {-1, 0, 1, 2};
  CHECK(tp_donate(base, owners, 4, out) == TP_OK);
  CHECK(out[0] == 7 && out[1] == 7 && out[2] == 7 && out[3] == 7);
  owners[3] = -1;
  CHECK(tp_donate(base, owners, 4, out) == TP_OK);
  CHECK(out[0] == 3 && out[1] == 3 && out[2] == 3 && out[3] == 7);
  owners[2] = -1;
  CHECK(tp_donate(base, owners, 4, out) == TP_OK && out[0] == 2);
  /* Independent reachability: every waiting chain carries the base priority. */
  for (unsigned b = 0; b < 256; b++)
    for (unsigned g = 0; g < 625; g++) {
      unsigned code = b, graph = g, expected[4];
      for (unsigned i = 0; i < 4; i++) {
        base[i] = expected[i] = code % 4;
        code /= 4;
        owners[i] = (int)(graph % 5) - 1;
        graph /= 5;
      }
      for (unsigned i = 0; i < 4; i++) {
        int at = (int)i;
        for (unsigned depth = 0; depth < 4 && at >= 0; depth++) {
          if (expected[(unsigned)at] < base[i])
            expected[(unsigned)at] = base[i];
          at = owners[(unsigned)at];
        }
      }
      CHECK(tp_donate(base, owners, 4, out) == TP_OK);
      for (unsigned i = 0; i < 4; i++)
        CHECK(out[i] == expected[i]);
    }
  owners[0] = 4;
  out[0] = 77;
  CHECK(tp_donate(base, owners, 4, out) == TP_INVALID && out[0] == 77);
  puts("OBS donation160000 graph/priority combinations, recursive chain and "
       "withdrawal recomputed from base");
}
static void bound(void) {
  uint64_t cost[5] = {3, 3, 2, 4, 0}, out = 77;
  CHECK(tp_bound(cost, 5, 1, 12, &out) == TP_OK && out == 12);
  out = 77;
  CHECK(tp_bound(cost, 5, 1, 11, &out) == TP_UNSUPPORTED && out == 77);
  CHECK(tp_bound(cost, 5, 0, UINT64_MAX, &out) == TP_UNSUPPORTED && out == 77);
  cost[0] = UINT64_MAX;
  cost[1] = 1;
  CHECK(tp_bound(cost, 2, 1, UINT64_MAX, &out) == TP_OVERFLOW && out == 77);
  CHECK(tp_bound(cost, 0, 1, 0, &out) == TP_INVALID && out == 77);
  puts("OBS bound conditional FIFO wait6+critical2+interference4=12; "
       "unsupported unknown delay; checked overflow");
}
static void effects(void) {
  CHECK(tp_nonsleepable(0) == TP_OK);
  for (unsigned i = 0; i < 6; i++)
    CHECK(tp_nonsleepable(1u << i) == TP_UNSUPPORTED);
  CHECK(tp_nonsleepable(64) == TP_INVALID);
  uint64_t path[5] = {50, 200, 300, 400, 500}, out = 77;
  CHECK(tp_bound(path, 5, 0, 1000, &out) == TP_UNSUPPORTED && out == 77);
  CHECK(tp_nonsleepable(TP_ALLOC | TP_PAGEFAULT | TP_LOG | TP_BLOCK |
                        TP_PREEMPT | TP_UNAUDITED) == TP_UNSUPPORTED);
  puts("OBS effects short-lock50 does not bound "
       "allocation/fault/logging/preemption; six nonsleepable dependencies "
       "rejected");
}
static void history(void) {
  TPWitness out = {0};
  TPOp valid[4] = {{1, 2, 0, TP_ENQUEUE, 1, TP_VALUE},
                   {3, 4, 1, TP_ENQUEUE, 2, TP_VALUE},
                   {5, 6, 2, TP_DEQUEUE, 1, TP_VALUE},
                   {7, 8, 3, TP_DEQUEUE, 2, TP_VALUE}};
  CHECK(tp_history(valid, 4, TP_REQUIRE_COMPLETE, &out) == TP_OK &&
        out.length == 4);
  valid[2].value = 2;
  valid[3].value = 1;
  CHECK(tp_history(valid, 4, TP_REQUIRE_COMPLETE, &out) == TP_NONLINEAR);
  valid[2].value = 1;
  valid[3].value = 1;
  CHECK(tp_history(valid, 4, TP_REQUIRE_COMPLETE, &out) == TP_NONLINEAR);
  TPOp pending[2] = {{1, 0, 0, TP_ENQUEUE, 7, 0},
                     {2, 3, 1, TP_DEQUEUE, 7, TP_VALUE}};
  out.length = 77;
  CHECK(tp_history(pending, 2, TP_REQUIRE_COMPLETE, &out) == TP_INCOMPLETE &&
        out.length == 77);
  CHECK(tp_history(pending, 2, TP_ALLOW_EXTENSION, &out) == TP_OK &&
        out.completed_pending_mask == 1 && out.operation[0] == 0 &&
        out.result[0] == TP_VALUE && out.value[1] == 7);
  pending[0].invocation = 4;
  out.length = 77;
  CHECK(tp_history(pending, 2, TP_ALLOW_EXTENSION, &out) == TP_NONLINEAR &&
        out.length == 77);
  pending[0].invocation = 1;
  pending[0].kind = TP_DEQUEUE;
  pending[1].value = 0;
  pending[1].result = TP_EMPTY;
  CHECK(tp_history(pending, 2, TP_ALLOW_EXTENSION, &out) == TP_OK);
  TPOp full[4] = {{1, 2, 0, 0, 1, 0},
                  {3, 4, 0, 0, 2, 0},
                  {5, 6, 0, 0, 3, 0},
                  {7, 8, 0, 0, 4, TP_FULL}};
  CHECK(tp_history(full, 4, 0, &out) == TP_OK);
  full[3].result = TP_VALUE;
  CHECK(tp_history(full, 4, 0, &out) == TP_NONLINEAR);
  CHECK(tp_history(NULL, 0, 0, &out) == TP_OK && out.length == 0);
  valid[0].response = 1;
  out.length = 77;
  CHECK(tp_history(valid, 4, 0, &out) == TP_INVALID && out.length == 77);
  puts("OBS history FIFO counterexample, duplicate output, incomplete policy, "
       "synthesized pending completion, capacity3 and output preservation");
}
int main(int argc, char **argv) {
  CHECK(argc == 2);
  if (!strcmp(argv[1], "donation"))
    donation();
  else if (!strcmp(argv[1], "bound"))
    bound();
  else if (!strcmp(argv[1], "effects"))
    effects();
  else if (!strcmp(argv[1], "history"))
    history();
  else
    return 2;
  return 0;
}
