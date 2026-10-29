#include "os.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "OF_CHECK:%d:%s\n", __LINE__, #x);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int schedules(void) {
  os_job jobs[] = {{0, 8, 1}, {1, 2, 1}, {2, 1, 1}};
  for (unsigned policy = 0; policy < 4; policy++)
    for (unsigned overhead = 0; overhead <= 2; overhead++) {
      struct of_schedule r;
      CHECK(of_schedule(jobs, 3, (os_policy)policy, 2, overhead, &r) == OF_OK);
      unsigned service[3] = {0}, first[3] = {UINT_MAX, UINT_MAX, UINT_MAX},
               finish[3] = {0}, idle = 0, switching = 0, dispatches = 0;
      int previous = -1;
      for (unsigned t = 0; t < r.schedule.ticks; t++) {
        int id = r.schedule.timeline[t];
        if (id == -1) {
          idle++;
          previous = -1;
        } else if (id == -2) {
          switching++;
          previous = -1;
        } else {
          CHECK(id >= 0 && id < 3);
          if (previous != id)
            dispatches++;
          previous = id;
          service[id]++;
          if (first[id] == UINT_MAX)
            first[id] = t;
          finish[id] = t + 1;
        }
      }
      for (unsigned i = 0; i < 3; i++)
        CHECK(
            service[i] == jobs[i].service && first[i] == r.schedule.first[i] &&
            finish[i] == r.schedule.finish[i] &&
            r.schedule.waiting[i] + service[i] == finish[i] - jobs[i].arrival &&
            r.queued[i] + r.dispatch_charge[i] == r.schedule.waiting[i]);
      CHECK(idle + switching + 11 == r.schedule.ticks &&
            dispatches == r.schedule.dispatches &&
            switching == dispatches * overhead);
      if (policy == OS_SRTF && !overhead)
        CHECK(finish[0] == 11 && finish[1] == 3 && finish[2] == 4 &&
              first[0] == 0 && first[1] == 1 && first[2] == 3);
    }
  struct of_schedule out;
  memset(&out, 0x5a, sizeof out);
  unsigned char snapshot[sizeof out];
  memcpy(snapshot, &out, sizeof out);
  CHECK(of_schedule(jobs, 3, OS_RR, 0, 0, &out) == OF_INPUT &&
        !memcmp(&out, snapshot, sizeof out));
  os_job longjob = {255, 256, 1};
  CHECK(of_schedule(&longjob, 1, OS_FCFS, 1, 1, &out) == OF_LIMIT &&
        !memcmp(&out, snapshot, sizeof out));
  CHECK(of_schedule(NULL, 0, OS_FCFS, 1, 1, &out) == OF_OK &&
        out.schedule.ticks == 0);
  puts("OF:schedule:SRTF-B3-C4-A11:three-overheads:independent-metrics:refusal-"
       "preserves");
  return 0;
}
static int fairness(void) {
  unsigned weights[] = {1, 2, 4};
  struct of_fair a, b;
  CHECK(of_fair(1, 14, weights, 0, 28, NULL, 0, 0, &a) == OF_OK &&
        a.service[0] == 2 && a.service[1] == 4 && a.service[2] == 8);
  CHECK(of_fair(1, 14, weights, UINT64_MAX - 4, 28, NULL, 0, 0, &b) == OF_OK &&
        b.normalizations &&
        memcmp(a.timeline, b.timeline, 14 * sizeof(int)) == 0);
  struct of_event events[256];
  for (unsigned i = 0; i < 256; i++)
    events[i] = (struct of_event){i, 0, OF_YIELD, 0};
  CHECK(of_fair(0, 256, weights, 0, 1024, events, 256, 0, &a) == OF_OK &&
        a.boosts == 15);
  CHECK(of_fair(0, 256, weights, 0, 1024, events, 256, 1, &b) == OF_OK);
  CHECK(memcmp(a.timeline, b.timeline, sizeof a.timeline) != 0);
  for (unsigned i = 0; i < 3; i++)
    CHECK(a.service[i] > 0);
  struct of_event changes[] = {{20, 0, OF_SLEEP, 30}, {80, 0, OF_WEIGHT, 4}};
  CHECK(of_fair(1, 128, weights, 0, 1024, changes, 2, 0, &a) == OF_OK);
  for (unsigned t = 20; t < 50; t++)
    CHECK(a.timeline[t] != 0);
  printf("OF:fair:scale28=2,4,8:normalization:yield-mutant:boost15:sleep30:"
         "weight-change:service%u,%u,%u\n",
         a.service[0], a.service[1], a.service[2]);
  return 0;
}
static int pages(void) {
  uint8_t values[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
  struct of_reference refs[12];
  for (unsigned i = 0; i < 12; i++)
    refs[i] = (struct of_reference){values[i], (uint8_t)(i % 3 == 0)};
  for (unsigned cap = 3; cap <= 4; cap++)
    for (unsigned policy = 0; policy < 4; policy++) {
      struct of_pages r;
      CHECK(of_pages(refs, 12, cap, (enum of_page_policy)policy, &r) == OF_OK);
      if (policy == OF_FIFO)
        CHECK(r.faults == (cap == 3 ? 9u : 10u));
      if (policy == OF_CLOCK)
        CHECK(r.faults == os_clock_faults(values, 12, cap));
      for (unsigned i = 0; i < 12; i++) {
        struct of_page_step *step = &r.steps[i];
        CHECK(step->live_mask < (1u << cap) &&
              !(step->dirty_mask & ~step->live_mask));
        for (unsigned j = 0; j < cap; j++)
          for (unsigned k = j + 1; k < cap; k++)
            if ((step->live_mask & (1u << j)) && (step->live_mask & (1u << k)))
              CHECK(step->resident[j] != step->resident[k]);
      }
      printf("OF:pages:policy%u:frames%u:faults%u:writebacks%u\n", policy, cap,
             r.faults, r.writebacks);
    }
  struct of_pages out;
  memset(&out, 0x5a, sizeof out);
  unsigned char old[sizeof out];
  memcpy(old, &out, sizeof out);
  CHECK(of_pages(refs, 12, 0, OF_FIFO, &out) == OF_INPUT &&
        !memcmp(old, &out, sizeof out));
  CHECK(of_pages(NULL, 0, 1, OF_OPT, &out) == OF_OK && !out.faults);
  puts("OF:pages:FIFO9-10:CLOCK-provider:uniqueness:dirty-separate");
  return 0;
}
static int transfers(void) {
  struct of_transfer t = {7, 0, 0, 0};
  ptrdiff_t events[] = {-4, 2, -11, 2, -5};
  size_t expected[] = {0, 2, 2, 4, 4};
  for (unsigned i = 0; i < 5; i++) {
    enum of_status s = of_progress(&t, events[i], 0);
    CHECK(t.done == expected[i] && t.wanted - t.done == 7 - expected[i]);
    CHECK(s == (i == 4 ? OF_ERROR : i == 0 || i == 2 ? OF_WAIT : OF_OK));
  }
  CHECK(t.error == 5 && t.terminal);
  t = (struct of_transfer){7, 0, 0, 0};
  CHECK(of_progress(&t, 3, 1) == OF_ERROR && t.done == 3 && t.error == 71);
  t = (struct of_transfer){7, 0, 0, 0};
  CHECK(of_progress(&t, PTRDIFF_MIN, 0) == OF_INPUT && !t.done && !t.terminal);
  puts("OF:transfer:done0,2,2,4,4:EINTR-EAGAIN-terminal:record-partial-is-"
       "protocol-error");
  return 0;
}
int main(void) {
  if (schedules() || fairness() || pages() || transfers())
    return 1;
  puts("OF_PASS:core");
  return 0;
}
