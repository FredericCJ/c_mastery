#ifndef CMASTERY_REALTIME_JOBS_H
#define CMASTERY_REALTIME_JOBS_H
#include <stdint.h>
#define SJ_TASKS 8u
#define SJ_JOBS 32u
#define SJ_HORIZON 128u
#define SJ_NONE UINT32_MAX
enum sj_status { SJ_OK, SJ_INVALID, SJ_RESOURCE, SJ_UNIMPLEMENTED };
enum sj_policy { SJ_RM, SJ_DM, SJ_EDF };
enum sj_disposition {
  SJ_FUTURE,
  SJ_READY,
  SJ_COMPLETED,
  SJ_REJECTED,
  SJ_DROPPED
};
enum sj_tick_kind { SJ_IDLE, SJ_SERVICE, SJ_OVERHEAD };
enum sj_overload { SJ_REJECT_NEW, SJ_DROP_OLDEST };
struct sj_task {
  uint32_t cost, period, deadline, jitter;
};
struct sj_job {
  unsigned task;
  uint32_t sequence, release, cost, nonpreempt_start, nonpreempt_length;
};
struct sj_job_result {
  enum sj_disposition state;
  uint32_t remaining, executed, start, finish;
  unsigned missed;
};
struct sj_tick {
  uint32_t job;
  enum sj_tick_kind kind;
};
struct sj_result {
  struct sj_job_result jobs[SJ_JOBS];
  struct sj_tick ticks[SJ_HORIZON];
  unsigned elapsed, work, overhead, idle, preemptions, misses, rejected,
      dropped, completed, peak_active;
};
/* All arrays/control storage are live, disjoint. INVALID/RESOURCE preserve
 * published output. Order uses the actual completed cmx_priority_order. */
enum sj_status sj_order(const struct sj_task *, unsigned count, unsigned dm,
                        unsigned *order);
/* Release-jitter entries are job-major: jitter[k*count+task]. Sequence=k.
 * Periodic generation does not assume actual release order when jitter>=T. */
enum sj_status sj_generate(const struct sj_task *, unsigned count,
                           const uint32_t *phases, const uint32_t *jitter,
                           unsigned jobs_per_task, struct sj_job *out,
                           unsigned capacity, unsigned *out_count);
/* A finite one-CPU explicit job simulation. Every input identity is retained;
 * no unfinished job is overwritten on another release. Deadline checks occur
 * at horizon too, but releases at horizon are outside the simulated interval.
 */
enum sj_status sj_simulate(const struct sj_task *, unsigned count, const struct sj_job *,
                           unsigned jobs, enum sj_policy, enum sj_overload,
                           unsigned capacity, unsigned horizon,
                           unsigned preemption_cost, struct sj_result *);
/* Exact rational comparison U <= n(2^(1/n)-1) using integer powers. Applicable
 * only to implicit deadlines/no jitter; bounded intermediate storage may
 * return RESOURCE. PASS is sufficient; FAIL is not a necessity result. */
enum sj_status sj_rm_bound(const struct sj_task *, unsigned count,
                           unsigned *sufficient);
#endif
