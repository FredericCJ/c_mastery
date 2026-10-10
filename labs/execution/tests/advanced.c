#include "cmx_advanced.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x);                          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int priorities(void) {
  cmx_job j[] = {{1, 5, 2, 0, 0}, {1, 3, 3, 0, 0}, {1, 8, 1, 0, 0}};
  unsigned o[3];
  CHECK(cmx_priority_order(j, 3, 0, o) == CMX_OK && o[0] == 1 && o[1] == 0 &&
        o[2] == 2);
  CHECK(cmx_priority_order(j, 3, 1, o) == CMX_OK && o[0] == 2 && o[1] == 0 &&
        o[2] == 1);
  return 0;
}
static int partition(void) {
  cmx_job j[] = {{1, 2, 2, 0, 0}, {1, 2, 2, 0, 0}, {3, 3, 3, 0, 0}};
  unsigned cpu[3];
  cmx_sim_result a, b;
  CHECK(cmx_partition(j, 3, 2, 6, cpu) == CMX_OK && cpu[0] == 0 &&
        cpu[1] == 0 && cpu[2] == 1);
  CHECK(cmx_simulate(j, 3, 2, cpu, 6, &a) == CMX_OK && a.misses == 0 &&
        a.work == 12);
  CHECK(cmx_simulate(j, 3, 2, NULL, 6, &b) == CMX_TIMEOUT && b.misses > 0);
  return 0;
}
static int server(void) {
  cmx_server s;
  CHECK(cmx_server_init(&s, 2, 5) == CMX_OK);
  CHECK(cmx_server_charge(&s, 0, 1) == CMX_OK);
  CHECK(cmx_server_charge(&s, 1, 1) == CMX_OK && s.remaining == 0 &&
        s.count == 2);
  CHECK(cmx_server_charge(&s, 2, 1) == CMX_FULL);
  CHECK(cmx_server_update(&s, 4) == CMX_OK && s.remaining == 0);
  CHECK(cmx_server_update(&s, 5) == CMX_OK && s.remaining == 1);
  CHECK(cmx_server_update(&s, 6) == CMX_OK && s.remaining == 2);
  CHECK(cmx_server_update(&s, 5) == CMX_INVALID);
  return 0;
}
static int identity(void) {
  cmx_identity_pool p;
  cmx_identity_init(&p);
  uint32_t a = cmx_identity_take(&p);
  CHECK(a && cmx_identity_valid(&p, a));
  CHECK(cmx_identity_put(&p, a) == CMX_OK);
  uint32_t b = cmx_identity_take(&p);
  CHECK((a & 255) == (b & 255) && a != b && !cmx_identity_valid(&p, a));
  CHECK(cmx_identity_put(&p, a) == CMX_INVALID);
  CHECK(cmx_identity_put(&p, b) == CMX_OK);
  p.generation[0] = 65535;
  a = cmx_identity_take(&p);
  CHECK(cmx_identity_put(&p, a) == CMX_OK);
  b = cmx_identity_take(&p);
  CHECK((b & 255) == 2);
  return 0;
}
static int progress(void) {
  cmx_reservation q;
  cmx_reservation_init(&q);
  uint32_t a, b, v;
  CHECK(cmx_reserve(&q, &a) == CMX_OK);
  CHECK(cmx_reserve(&q, &b) == CMX_OK);
  CHECK(cmx_publish(&q, b, 2) == CMX_OK);
  CHECK(cmx_reserved_pop(&q, &v) == CMX_RETRY);
  CHECK(cmx_publish(&q, a, 1) == CMX_OK);
  CHECK(cmx_reserved_pop(&q, &v) == CMX_OK && v == 1);
  CHECK(cmx_reserved_pop(&q, &v) == CMX_OK && v == 2);
  return 0;
}
static int history(void) {
  unsigned witness[8];
  cmx_history_op good[] = {{0, 0, 1, 4, 7, CMX_OK}, {1, 1, 2, 3, 7, CMX_OK}};
  CHECK(cmx_history_check(good, 2, 2, witness) == CMX_OK && witness[0] == 0);
  cmx_history_op bad[] = {{0, 0, 1, 2, 7, CMX_OK},
                          {0, 0, 3, 4, 8, CMX_OK},
                          {1, 1, 5, 6, 8, CMX_OK}};
  CHECK(cmx_history_check(bad, 3, 2, witness) == CMX_BROKEN);
  good[1].invoke = 0;
  good[1].respond = 1;
  CHECK(cmx_history_check(good, 2, 2, witness) == CMX_OK);
  good[1].respond = 0;
  CHECK(cmx_history_check(good, 2, 2, witness) == CMX_INVALID);
  return 0;
}
static int rcu(void) {
  cmx_rcu r;
  unsigned root;
  cmx_rcu_init(&r, 1);
  CHECK(cmx_rcu_read(&r, 0, &root) == CMX_OK && root == 1);
  CHECK(cmx_rcu_publish(&r, 2) == 1);
  CHECK(!cmx_rcu_quiescent(&r));
  CHECK(cmx_rcu_read(&r, 1, &root) == CMX_OK && root == 2);
  CHECK(cmx_rcu_leave(&r, 0) == CMX_OK);
  CHECK(!cmx_rcu_quiescent(&r));
  CHECK(cmx_rcu_leave(&r, 1) == CMX_OK && cmx_rcu_quiescent(&r));
  CHECK(cmx_rcu_leave(&r, 1) == CMX_INVALID);
  return 0;
}
static int comparison(void) {
  cmx_kernel k;
  cmx_queue q;
  cmx_init(&k, CMX_RR);
  CHECK(cmx_task_add(&k, 4, 0, 0x1000, 256) == 0);
  CHECK(cmx_task_add(&k, 4, 0, 0x2000, 256) == 1);
  cmx_queue_init(&q, 8);
  uint32_t produced = 0, count = 0, sum = 0;
  while (count < 256) {
    while (produced < 256 && q.count < 8) {
      CHECK(cmx_queue_send(&k, &q, 0, produced, cmx_forever()) == CMX_OK);
      produced++;
    }
    CHECK(cmx_queue_recv(&k, &q, 1, cmx_forever()) == CMX_OK);
    CHECK(k.task[1].value == count);
    sum += k.task[1].value;
    count++;
  }
  CHECK(cmx_queue_close(&k, &q) == CMX_OK);
  CHECK(cmx_queue_recv(&k, &q, 1, cmx_forever()) == CMX_CLOSED);
  CHECK(sum == 32640 && count == 256);
  puts("{\"count\":256,\"checksum\":32640,\"capacity\":8,\"close_drained\":"
       "true,\"clock_kind\":\"logical\"}");
  return 0;
}
int main(int argc, char **argv) {
  struct entry {
    const char *name;
    int (*test)(void);
  } t[] = {{"priorities", priorities},
           {"partition", partition},
           {"server", server},
           {"identity", identity},
           {"progress", progress},
           {"history", history},
           {"rcu", rcu},
           {"comparison", comparison}};
  if (argc != 2)
    return 2;
  for (unsigned i = 0; i < sizeof t / sizeof t[0]; i++)
    if (!strcmp(argv[1], t[i].name)) {
      int r = t[i].test();
      if (!r)
        printf("PASS execution.%s\n", argv[1]);
      return r;
    }
  return 2;
}
