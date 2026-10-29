#include "cmx_advanced.h"
#include "jobs.h"
#include "response.h"
#ifdef SJ_TARGET
#include <stddef.h>
extern void *memset(void *, int, size_t);
extern void *memcpy(void *, const void *, size_t);
static int sj_target_compare(const void *a, const void *b, size_t count) {
  const unsigned char *x = a, *y = b;
  for (size_t i = 0; i < count; i++)
    if (x[i] != y[i])
      return 1;
  return 0;
}
#define memcmp sj_target_compare
#define puts sj_target_puts
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      sj_target_stop(1);                                                       \
  } while (0)
#else
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "SJ_CHECK:%d:%s\n", __LINE__, #x);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)
#endif
static int order_case(void) {
  struct sj_task t[] = {{2, 10, 4, 0}, {3, 6, 6, 0}, {1, 20, 3, 0}, {0, 6, 6, 0}};
  unsigned o[4] = {99, 99, 99, 99};
  CHECK(sj_order(t, 4, 0, o) == SJ_OK && o[0] == 1 && o[1] == 3 && o[2] == 0 &&
        o[3] == 2);
  CHECK(sj_order(t, 4, 1, o) == SJ_OK && o[0] == 2 && o[1] == 0 && o[2] == 1 &&
        o[3] == 3);
  struct rta_task r[] = {{1, 20, 3, 0, 0, 0}, {2, 10, 4, 0, 0, 0}, {3, 6, 6, 0, 0, 0}};
  struct rta_result z;
  CHECK(rta_response(r, 3, 0, 128, &z) == RTA_CONVERGED && z.estimate == 1);
  CHECK(rta_response(r, 3, 1, 128, &z) == RTA_CONVERGED && z.estimate == 3);
  CHECK(rta_response(r, 3, 2, 128, &z) == RTA_CONVERGED && z.estimate == 6);
  r[2].cost = 4;
  CHECK(rta_response(r, 3, 2, 128, &z) == RTA_EXCEEDED && z.estimate == 7);
  uint32_t stated_phase[3] = {0}, stated_jitter[6] = {0};
  struct sj_job stated_jobs[6];
  unsigned stated_count;
  struct sj_result stated_result;
  CHECK(sj_generate(t, 3, stated_phase, stated_jitter, 2, stated_jobs, 6,
                    &stated_count) == SJ_OK);
  CHECK(sj_simulate(t, 3, stated_jobs, stated_count, SJ_DM, SJ_REJECT_NEW, 6,
                    24, 0, &stated_result) == SJ_OK &&
        stated_result.misses == 0 && stated_result.completed == 6);
  CHECK(sj_simulate(t, 3, stated_jobs, stated_count, SJ_RM, SJ_REJECT_NEW, 6,
                    24, 0, &stated_result) == SJ_OK &&
        stated_result.jobs[0].missed && stated_result.jobs[2].missed);
  stated_jobs[1].cost = 4;
  CHECK(sj_simulate(t, 3, stated_jobs, stated_count, SJ_DM, SJ_REJECT_NEW, 6,
                    24, 0, &stated_result) == SJ_OK &&
        stated_result.jobs[1].missed);
  struct rta_task jittered[] = {{1, 4, 4, 1, 0, 0}, {2, 10, 10, 0, 1, 0}};
  CHECK(rta_response(jittered, 2, 1, 128, &z) == RTA_CONVERGED &&
        z.estimate == 5);
  struct sj_task harmonic[] = {{2, 4, 4, 0}, {4, 8, 8, 0}};
  unsigned sufficient = 99;
  CHECK(sj_rm_bound(harmonic, 2, &sufficient) == SJ_OK && !sufficient);
  struct rta_task hr[] = {{2, 4, 4, 0, 0, 0}, {4, 8, 8, 0, 0, 0}};
  CHECK(rta_response(hr, 2, 1, 128, &z) == RTA_CONVERGED && z.estimate == 8);
  uint32_t phase[] = {0, 0}, jit[8] = {0};
  struct sj_job jobs[8];
  unsigned count;
  struct sj_result result;
  CHECK(sj_generate(harmonic, 2, phase, jit, 4, jobs, 8, &count) == SJ_OK &&
        count == 8);
  CHECK(sj_simulate(harmonic, 2, jobs, count, SJ_RM, SJ_REJECT_NEW, 8, 32, 0,
                    &result) == SJ_OK &&
        !result.misses && result.completed == 8);
  puts("SJ_ORDER:PASS");
  return 0;
}
static int edf_case(void) {
  struct sj_task t[] = {{3, 10, 6, 0}, {1, 10, 2, 0}, {1, 10, 3, 0}};
  struct sj_job j[] = {{0, 0, 0, 3, 0, 0}, {1, 0, 1, 1, 0, 0}, {2, 0, 2, 1, 0, 0}};
  struct sj_result r;
  unsigned expected[] = {0, 1, 2, 0, 0, SJ_NONE};
  CHECK(sj_simulate(t, 3, j, 3, SJ_EDF, SJ_REJECT_NEW, 3, 6, 0, &r) == SJ_OK &&
        r.misses == 0 && r.completed == 3 && r.preemptions == 1);
  for (unsigned i = 0; i < 6; i++)
    CHECK(r.ticks[i].job == expected[i]);
  cmx_job finite[] = {{2, 5, 2, 0, 0}, {2, 5, 3, 0, 0}};
  CHECK(cmx_edf_admit(finite, 2, 2) == CMX_OK &&
        cmx_edf_admit(finite, 2, 3) == CMX_TIMEOUT);
  struct sj_task d[] = {{2, 5, 2, 0}, {2, 5, 3, 0}};
  struct sj_job q[] = {{0, 0, 0, 2, 0, 0}, {1, 0, 0, 2, 0, 0}};
  CHECK(sj_simulate(d, 2, q, 2, SJ_EDF, SJ_REJECT_NEW, 2, 3, 0, &r) == SJ_OK &&
        r.misses == 1 && r.jobs[1].missed && r.jobs[1].remaining == 1);
  q[0].cost = 0;
  q[1].release = 3;
  CHECK(sj_simulate(d, 2, q, 2, SJ_EDF, SJ_REJECT_NEW, 1, 3, 0, &r) == SJ_OK &&
        r.completed == 1 && r.jobs[1].state == SJ_FUTURE && r.idle == 3);
  puts("SJ_EDF:PASS");
  return 0;
}
static int backlog_case(void) {
  struct sj_task t[] = {{4, 2, 6, 0}};
  struct sj_job j[] = {{0, 0, 0, 4, 0, 0}, {0, 1, 2, 4, 0, 0}, {0, 2, 4, 4, 0, 0}};
  struct sj_result r;
  CHECK(sj_simulate(t, 1, j, 3, SJ_EDF, SJ_REJECT_NEW, 3, 10, 0, &r) == SJ_OK &&
        r.completed == 2 && r.misses == 1 && r.jobs[2].remaining == 2 &&
        r.jobs[0].executed == 4);
  CHECK(sj_simulate(t, 1, j, 3, SJ_EDF, SJ_REJECT_NEW, 1, 10, 0, &r) == SJ_OK &&
        r.rejected == 1 && r.jobs[1].state == SJ_REJECTED && r.completed == 2 &&
        r.peak_active == 1);
  CHECK(sj_simulate(t, 1, j, 3, SJ_EDF, SJ_DROP_OLDEST, 1, 10, 0, &r) ==
            SJ_OK &&
        r.dropped == 2 && r.jobs[0].executed == 2 && r.jobs[1].executed == 2 &&
        r.completed == 1);
  j[0].nonpreempt_length = 4;
  CHECK(sj_simulate(t, 1, j, 3, SJ_EDF, SJ_DROP_OLDEST, 1, 10, 0, &r) ==
            SJ_OK &&
        r.rejected == 1 && r.dropped == 0 && r.completed == 2);
  puts("SJ_BACKLOG:PASS");
  return 0;
}
static int assumptions_case(void) {
  struct sj_task t[] = {{3, 10, 8, 0}, {1, 10, 1, 0}};
  struct sj_job j[] = {{0, 0, 0, 3, 0, 0}, {1, 0, 1, 1, 0, 0}};
  struct sj_result r;
  CHECK(sj_simulate(t, 2, j, 2, SJ_EDF, SJ_REJECT_NEW, 2, 5, 0, &r) == SJ_OK &&
        !r.misses);
  j[0].nonpreempt_length = 3;
  CHECK(sj_simulate(t, 2, j, 2, SJ_EDF, SJ_REJECT_NEW, 2, 5, 0, &r) == SJ_OK &&
        r.misses == 1 && r.jobs[1].finish == 4);
  struct rta_task blocked[] = {{1, 10, 1, 0, 2, 0}};
  struct rta_result bound;
  CHECK(rta_response(blocked, 1, 0, 128, &bound) == RTA_EXCEEDED &&
        bound.estimate == 3);
  j[0].nonpreempt_length = 0;
  CHECK(sj_simulate(t, 2, j, 2, SJ_EDF, SJ_REJECT_NEW, 2, 5, 1, &r) == SJ_OK &&
        r.misses == 1 && r.overhead == 1 && r.work == 4 &&
        r.jobs[1].finish == 3);
  struct sj_task jt[] = {{1, 2, 5, 4}};
  uint32_t phase[] = {0}, jitter[] = {4, 0};
  struct sj_job generated[2];
  unsigned count;
  CHECK(sj_generate(jt, 1, phase, jitter, 2, generated, 2, &count) == SJ_OK &&
        count == 2 && generated[0].release == 4 && generated[1].release == 2);
  CHECK(sj_simulate(jt, 1, generated, 2, SJ_EDF, SJ_REJECT_NEW, 2, 6, 0, &r) ==
            SJ_OK &&
        r.jobs[1].start == 2 && r.jobs[0].start == 4);
  struct sj_task equal[] = {{1, 5, 5, 0}, {1, 5, 5, 0}};
  struct sj_job ties[] = {{0, 0, 0, 1, 0, 0}, {1, 0, 0, 1, 0, 0}};
  CHECK(sj_simulate(equal, 2, ties, 2, SJ_EDF, SJ_REJECT_NEW, 2, 2, 0, &r) ==
            SJ_OK &&
        r.jobs[0].finish == 1 && r.jobs[1].finish == 2);
  puts("SJ_ASSUMPTIONS:PASS");
  return 0;
}
static int refusal_case(void) {
  struct sj_task t[] = {{1, 5, 5, 0}, {1, 0, 5, 0}};
  unsigned o[] = {91, 92};
  CHECK(sj_order(t, 2, 0, o) == SJ_INVALID && o[0] == 91 && o[1] == 92);
  t[1].period = 5;
  struct sj_job j[] = {{0, 0, 0, 1, 0, 0}, {0, 0, 0, 1, 0, 0}};
  struct sj_result r, before;
  memset(&r, 0x55, sizeof r);
  before = r;
  CHECK(sj_simulate(t, 2, j, 2, SJ_EDF, SJ_REJECT_NEW, 2, 2, 0, &r) ==
            SJ_INVALID &&
        memcmp(&r, &before, sizeof r) == 0);
  uint32_t p[] = {UINT32_MAX, 0}, v[] = {0, 0};
  struct sj_job out[2], copy[2];
  unsigned count = 93;
  memset(out, 0x55, sizeof out);
  memcpy(copy, out, sizeof out);
  CHECK(sj_generate(t, 2, p, v, 1, out, 2, &count) == SJ_RESOURCE &&
        count == 93 && memcmp(out, copy, sizeof out) == 0);
  unsigned sufficient = 94;
  struct sj_task large[] = {{1, 65521, 65521, 0}, {1, 65519, 65519, 0}};
  CHECK(sj_rm_bound(large, 2, &sufficient) == SJ_RESOURCE && sufficient == 94);
  puts("SJ_REFUSAL:PASS");
  return 0;
}
#ifndef SJ_TARGET
static int stream(void) {
  unsigned n, k, p, v, c, h, x;
  while (scanf("%u%u%u%u%u%u%u", &n, &k, &p, &v, &c, &h, &x) == 7) {
    struct sj_task t[SJ_TASKS];
    struct sj_job j[SJ_JOBS];
    struct sj_result r;
    if (n > SJ_TASKS || k > SJ_JOBS)
      return 2;
    for (unsigned i = 0; i < n; i++)
      if (scanf("%u%u%u%u", &t[i].cost, &t[i].period, &t[i].deadline,
                &t[i].jitter) != 4)
        return 2;
    for (unsigned i = 0; i < k; i++)
      if (scanf("%u%u%u%u%u%u", &j[i].task, &j[i].sequence, &j[i].release,
                &j[i].cost, &j[i].nonpreempt_start,
                &j[i].nonpreempt_length) != 6)
        return 2;
    enum sj_status status = sj_simulate(t, n, j, k, (enum sj_policy)p,
                                        (enum sj_overload)v, c, h, x, &r);
    printf("%u", status);
    if (status == SJ_OK) {
      printf(" %u %u %u %u %u %u %u %u %u %u", r.elapsed, r.work, r.overhead,
             r.idle, r.preemptions, r.misses, r.rejected, r.dropped,
             r.completed, r.peak_active);
      for (unsigned i = 0; i < k; i++)
        printf(" %u %u %u %u %u %u", r.jobs[i].state, r.jobs[i].remaining,
               r.jobs[i].executed, r.jobs[i].start, r.jobs[i].finish,
               r.jobs[i].missed);
      for (unsigned i = 0; i < h; i++)
        printf(" %u %u", r.ticks[i].job, r.ticks[i].kind);
    }
    putchar('\n');
  }
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "order"))
    return order_case();
  if (!strcmp(argv[1], "edf"))
    return edf_case();
  if (!strcmp(argv[1], "backlog"))
    return backlog_case();
  if (!strcmp(argv[1], "assumptions"))
    return assumptions_case();
  if (!strcmp(argv[1], "refusal"))
    return refusal_case();
  if (!strcmp(argv[1], "stream"))
    return stream();
  return 2;
}
#endif
