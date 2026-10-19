# T52.09 assessor analysis

The retained helper's loop is input-order first fit and writes `cpu[i]` before
later tasks are validated. The homogeneous wrapper must stage both sorting and
output. Its normalized weight H/P×C is exact because divisibility is checked.
Stable ties use original index. The heterogeneous heuristic deliberately sorts
by worst eligible utilization; its failure is not an optimality result.

The independent oracle enumerates actual jobs contributing to each interval;
the C implementation uses the demand formula. Compare the two different
constructions, rather than reusing the C calculation as its own expected result.
With constrained D≤P and common multiple H, dbf(t+H)=dbf(t)+sum(H/P×C).
Thus service≤H plus all first-hyperperiod checks extends by induction. State
the independent-job, full-preemption, fixed-cost and no-interference premises.
Costs≤P and n≤8 bound every intermediate sum by2048. Invalid arbitrary horizons
must fail before calling retained arithmetic.

The affinity witness `{(0.6,{0,1}),(0.5,{0})}` demonstrates a feasible problem
that this first-fit heuristic cannot place. The exact search finds assignment
`[1,0]`; no conclusion about physical deadlines follows. Each accepted table
is also fed to actual `cmx_simulate` with selected costs. The producer's job
overwrite is harmless only after admission proves the constrained model meets
deadlines; do not import that behavior as an overloaded-queue policy.

The native experiment uses two singleton masks from the calling process's
allowed mask. It retains concrete CPU observations and raw wall/thread times.
Barrier coordination and measured elapsed time do not grant either thread
real-time priority or establish a worst-case latency. Repeated samples support
the actual affinity observation; the kernel mask contract supplies the broader
eligibility rule. Compare clocks only under their respective definitions.

The payload handoff reserves one borrowed work interval; preparation is legal
only after finish. A mutex and barrier make publication visible before target
acceptance. The new epoch invalidates old authority without freeing the object.
Both workers are joined before object destruction. At UINT64_MAX no new epoch
is available; existing work can finish with zero additional counted work, so
exhaustion does not force a permanent borrowed interval. A failed target can
be handled by aborting the unaccepted preparation; asynchronous thread death
is outside this adapter's contract.

For the changed migration condition, require a new per-CPU admission and
interference argument, including destination costs and transfer overhead. The
lease does not perform that proof. The controlled400-step payload transfer is
native ownership evidence, not a migrated hard-real-time schedule. For the
parent's heterogeneous M7/M4 variant, synthetic cost-table support is ready;
physical per-core cost and interference observations are still distinct.

Reject omitted-sort, partial-publication, utilization-only, affinity-ignoring,
active-borrow transfer and nonincrementing-epoch controls. A stub must fail
semantically after compiling. Guard changes must never turn a compiler error,
generic fault, failed affinity call or missing CPU into a successful control.
