#include "timer.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "TIMER-CHECK:%d:%s\n", __LINE__, #x);                    \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
static void history(void) {
  TdTimer t;
  TdDispatch d = {0};
  unsigned g, old;
  CHECK(td_init(&t, 3) == 0);
  CHECK(td_arm(&t, 10, 3, 0, &g) == 1 && g == 1);
  old = g;
  CHECK(td_begin(&t, g, 9, &d) == 0 && d.generation == 0);
  CHECK(td_begin(&t, g, 20, &d) == 1 && d.scheduled == 10 &&
        d.expirations == 4 && d.effects == 4 && d.next == 22);
  CHECK(td_begin(&t, g, 20, &d) == 0);
  CHECK(td_cancel(&t) == 2 && td_cancel(&t) == 2);
  CHECK(td_arm(&t, 30, 0, 0, &g) == 0 && t.state == TD_CANCELLING);
  CHECK(td_finish(&t, old) == 1 && td_finish(&t, old) == 0);
  CHECK(td_cancel(&t) == 0);
  CHECK(td_arm(&t, 10, 3, 1, &g) == 1 && g == 2);
  CHECK(td_begin(&t, old, 20, &d) == 0);
  CHECK(td_begin(&t, g, 20, &d) == 1 && d.expirations == 4 && d.effects == 1 &&
        d.next == 22);
  CHECK(td_finish(&t, g) == 1 && t.deadline == 22);
  CHECK(td_cancel(&t) == 1 && td_cancel(&t) == 0);
  CHECK(td_arm(&t, 1, 0, 0, &g) == 1 && g == 3);
  CHECK(td_arm(&t, 9, 0, 0, &g) == 0 && t.state == TD_ARMED && t.deadline == 1);
  CHECK(td_begin(&t, g, UINT64_MAX, &d) == 1 && d.expirations == 1 && !d.next);
  CHECK(td_finish(&t, g) == 1 && t.state == TD_IDLE);
  CHECK(td_arm(&t, 1, 0, 0, &g) == 0 && t.state == TD_RETIRED);
  CHECK(td_cancel(&t) == 0 && td_arm(&t, 1, 0, 0, &g) == 0);
}
static void edges(void) {
  TdTimer t;
  TdDispatch d;
  unsigned g;
  CHECK(td_init(&t, 65535) == 0 && td_init(&t, 0) == -1);
  CHECK(td_arm(&t, 0, 1, 0, &g) == -1 && t.generation == 0);
  CHECK(td_arm(&t, 1, 1, 0, &g) == 1);
  CHECK(td_begin(&t, g, UINT64_MAX, &d) == 1 && d.expirations == UINT64_MAX &&
        d.effects == UINT64_MAX && d.exhausted && !d.next);
  CHECK(td_finish(&t, g) == 1 && t.state == TD_IDLE);
  CHECK(td_arm(&t, UINT64_MAX, 1, 1, &g) == 1);
  CHECK(td_begin(&t, g, UINT64_MAX, &d) == 1 && d.expirations == 1 &&
        d.exhausted);
  CHECK(td_finish(&t, g) == 1);
  CHECK(td_arm(&t, 1, UINT64_MAX, 1, &g) == 1);
  CHECK(td_begin(&t, g, 1, &d) == 1 && d.exhausted && d.effects == 1);
}
static void earliest(void) {
  TdTimer t[4];
  unsigned g;
  size_t at = 99;
  uint64_t d = 99;
  for (size_t i = 0; i < 4; ++i)
    CHECK(td_init(&t[i], 4) == 0);
  CHECK(td_earliest(t, 4, &at, &d) == 0 && at == 99 && d == 99);
  CHECK(td_arm(&t[2], 30, 0, 0, &g) == 1);
  CHECK(td_arm(&t[1], 20, 0, 0, &g) == 1);
  CHECK(td_arm(&t[0], 20, 0, 0, &g) == 1);
  CHECK(td_earliest(t, 4, &at, &d) == 1 && at == 0 && d == 20);
  CHECK(td_cancel(&t[0]) == 1 && td_earliest(t, 4, &at, &d) == 1 && at == 1);
  CHECK(td_cancel(&t[1]) == 1 && td_earliest(t, 4, &at, &d) == 1 && at == 2 &&
        d == 30);
  CHECK(td_cancel(&t[2]) == 1 && td_earliest(t, 4, &at, &d) == 0);
}
int main(void) {
  history();
  edges();
  earliest();
  puts("TIMER-PASS:history=3;overflow_edges=3;earliest=4;");
}
