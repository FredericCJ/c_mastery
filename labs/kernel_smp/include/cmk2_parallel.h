#ifndef CMK2_PARALLEL_H
#define CMK2_PARALLEL_H
#include <stdint.h>

struct cmk2_parallel_result
{
    uint64_t sum, cycles;
    unsigned jobs, mask, errors, dag_value;
};

/* Original bounded user workpool: 1..4 workers, two reusable dispatch rounds,
 * integer reduction and a 16-node dependency graph. Returns 0 on success.
 * TSC interval is an emulator observation, never a hardware WCET claim. */
/* mode1: reduction; mode2: reused pool; mode3: pool plus DAG. */
int cmk2_parallel_run(struct cmk2_parallel_result *, unsigned mode);
#endif
