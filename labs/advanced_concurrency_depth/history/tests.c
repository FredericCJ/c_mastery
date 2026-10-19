#include "cmx.h"
#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "AH_ASSERT:%s:%d:%s\n", __FILE__, __LINE__, #x);         \
      exit(41);                                                                \
    }                                                                          \
  } while (0)
static void history(void) {
  AHOperation h[] = {{0, 3, 0, AH_INSERT, 11, AH_VALUE},
                     {1, 2, 1, AH_INSERT, 22, AH_VALUE},
                     {4, 5, 0, AH_REMOVE, 11, AH_VALUE}};
  AHWitness w = {0};
  CHECK(ah_history(h, 3, 2, AH_FIFO, AH_COMPLETE_ONLY, &w) == AH_VALID);
  CHECK(w.length == 3 && w.order[0] < 3 && w.order[1] < 3 && w.order[2] < 3);
  h[0].respond = 1;
  h[1].invoke = 2;
  h[1].respond = 3;
  h[2].value = 22;
  CHECK(ah_history(h, 3, 2, AH_FIFO, AH_COMPLETE_ONLY, &w) == AH_NONLINEAR);
  h[2].value = 11;
  CHECK(ah_history(h, 3, 2, AH_FIFO, AH_COMPLETE_ONLY, &w) == AH_VALID);
  AHOperation p[] = {{0, AH_PENDING, 0, AH_INSERT, 11, AH_VALUE},
                     {1, 2, 1, AH_REMOVE, 11, AH_VALUE}};
  CHECK(ah_history(p, 2, 2, AH_FIFO, AH_COMPLETE_ONLY, &w) == AH_INCOMPLETE);
  CHECK(ah_history(p, 2, 2, AH_FIFO, AH_EXTEND_PENDING, &w) == AH_VALID &&
        w.completed_pending == 1);
  p[0].value = 22;
  CHECK(ah_history(p, 2, 2, AH_FIFO, AH_EXTEND_PENDING, &w) == AH_NONLINEAR);
  h[2].value = 22;
  CHECK(ah_history(h, 3, 2, AH_SET, AH_COMPLETE_ONLY, &w) == AH_VALID);
  h[1].value = 11;
  h[1].result = AH_EXISTS;
  h[2].value = 11;
  CHECK(ah_history(h, 3, 2, AH_SET, AH_COMPLETE_ONLY, &w) == AH_VALID);
  h[2].value = 22;
  h[2].result = AH_ABSENT;
  CHECK(ah_history(h, 3, 2, AH_SET, AH_COMPLETE_ONLY, &w) == AH_VALID);
  AHWitness before = w;
  CHECK(ah_history(h, 3, 0, AH_SET, AH_COMPLETE_ONLY, &w) == AH_INVALID &&
        !memcmp(&w, &before, sizeof w));
  h[1].invoke = h[0].invoke;
  CHECK(ah_history(h, 3, 2, AH_SET, AH_COMPLETE_ONLY, &w) == AH_INVALID);
  puts("OBS history ordinary pending set invalid");
}
static void separation(void) {
  atomic_uint data[2], length;
  atomic_init(&length, 0);
  atomic_init(&data[0], 0);
  atomic_init(&data[1], 0);
  atomic_store(&data[0], 11);
  atomic_store(&length, 1);
  atomic_store(&data[1], 22);
  atomic_store(&length, 2);
  unsigned at = atomic_load(&length) - 1, result = atomic_load(&data[at]);
  atomic_store(&length, at);
  AHOperation h[] = {{0, 1, 0, AH_INSERT, 11, AH_VALUE},
                     {2, 3, 0, AH_INSERT, 22, AH_VALUE},
                     {4, 5, 0, AH_REMOVE, result, AH_VALUE}};
  AHWitness w;
  CHECK(result == 22);
  CHECK(ah_history(h, 3, 2, AH_FIFO, AH_COMPLETE_ONLY, &w) == AH_NONLINEAR);
  CHECK(ah_history(h, 3, 2, AH_SET, AH_COMPLETE_ONLY, &w) == AH_VALID);
  puts("OBS separation actual-SC LIFO fails-FIFO succeeds-set");
}
static void points(void) {
  cmx_hp_queue q;
  uint32_t value = 99;
  cmx_hp_init(&q);
  CHECK(cmx_hp_pop(&q, 0, &value) == CMX_RETRY && value == 99);
  /* Reachable SC prefix: publisher initialized node1 and linked it, then was
   * suspended before its optional Tail advance. No invalid pointer is used. */
  q.node[1].value = 11;
  atomic_store(&q.node[1].state, 1);
  atomic_store(&q.node[0].next, 1);
  CHECK(atomic_load(&q.head) == 0 && atomic_load(&q.tail) == 0);
  CHECK(cmx_hp_pop(&q, 1, &value) == CMX_OK && value == 11);
  CHECK(atomic_load(&q.head) == 1 && atomic_load(&q.tail) == 1 &&
        (atomic_load(&q.node[0].state) & 3u) == 2);
  CHECK(cmx_hp_pop(&q, 1, &value) == CMX_RETRY);
  CHECK(cmx_hp_collect(&q) == 1);
  puts("OBS points linked-before-tail helped-head retired-dummy");
}
static void pressure(void) {
  cmx_hp_queue q;
  uint32_t value = 99;
  cmx_hp_init(&q);
  for (unsigned i = 0; i < 31; i++) {
    CHECK(cmx_hp_push(&q, 0, i) == CMX_OK);
    CHECK(cmx_hp_pop(&q, 1, &value) == CMX_OK && value == i);
  }
  CHECK(cmx_hp_pop(&q, 1, &value) == CMX_RETRY);
  CHECK(cmx_hp_push(&q, 0, 77) == CMX_FULL);
  unsigned retired = 0;
  for (unsigned i = 0; i < 32; i++)
    retired += (atomic_load(&q.node[i].state) & 3u) == 2;
  CHECK(retired == 31);
  CHECK(cmx_hp_collect(&q) == 31);
  CHECK(cmx_hp_push(&q, 0, 77) == CMX_OK);
  CHECK(cmx_hp_pop(&q, 1, &value) == CMX_OK && value == 77);
  printf("OBS pressure logical0 retired31 collect31 recovery lockfree=%d\n",
         atomic_is_lock_free(&q.head));
}
static void retry(void) {
  for (unsigned budget = 1; budget <= 8; budget++)
    for (unsigned n = 0; n <= budget; n++) {
      CHECK(ah_retry(n, budget, 0) ==
            (n == budget ? AH_INCOMPLETE : AH_NONLINEAR));
      CHECK(ah_retry(n, budget, 1) == AH_VALID);
    }
  CHECK(ah_retry(9, 8, 0) == AH_INVALID && ah_retry(0, 0, 0) == AH_INVALID);
  puts("OBS retry bounded-return is-not successful-service");
}
static int driver(void) {
  unsigned n, cap, spec, policy;
  while (scanf("%u%u%u%u", &n, &cap, &spec, &policy) == 4) {
    if (n > 7)
      return 2;
    AHOperation op[7];
    AHWitness w;
    for (unsigned i = 0; i < n; i++)
      if (scanf("%u%u%u%u%u%u", &op[i].invoke, &op[i].respond,
                &op[i].participant, &op[i].operation, &op[i].value,
                &op[i].result) != 6)
        return 3;
    int result = ah_history(op, n, cap, spec, policy, &w);
    printf("%d", result);
    if (result == AH_VALID) {
      printf(" %u %u", w.length, w.completed_pending);
      for (unsigned i = 0; i < w.length; i++) {
        unsigned at = w.order[i];
        if (at >= n)
          return 4;
        printf(" %u %u %u", at, w.result[at], w.value[at]);
      }
    }
    putchar('\n');
  }
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "history"))
    history();
  else if (!strcmp(argv[1], "separation"))
    separation();
  else if (!strcmp(argv[1], "points"))
    points();
  else if (!strcmp(argv[1], "pressure"))
    pressure();
  else if (!strcmp(argv[1], "retry"))
    retry();
  else if (!strcmp(argv[1], "driver"))
    return driver();
  else
    return 2;
  return 0;
}
