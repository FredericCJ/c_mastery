# T52.09 placement, demand and controlled ownership transfer

## 1. Identity and families

This required continuation supports `AX-PR-T52-09-T52.09.a`, `.b`, `.c` and
their original `-DELAYED` variants. Families A/B/C/D/F/G/H/I/J assess packing
counterexamples, construction, checked publication, defective retained-helper
assumptions, finite exhaustive assignments, rejection recovery, work/storage
costs, unequal CPU costs and an explicit ownership transfer.

## 2. Objectives and inspected references

Use `research/v5/expansion/legacy_realtime_partition/resources.json`. The
inspected scheduling documentation distinguishes per-CPU implicit-deadline
utilization from constrained-deadline demand and global scheduling. Buttazzo's
physical pages127–128 define jobs contributing to interval demand. The bounded
hyperperiod extension below includes its own argument rather than attributing
an unseen theorem to a bibliography entry. Linux affinity documentation defines
which CPUs a thread may use, its per-thread nature and the possibility of
kernel-imposed restrictions. None of these passages establishes physical WCET.

## 3. Prerequisites and learner artifacts

Bring `PR-T52-08` checked time arithmetic; `PR-T52-02` EDF model assumptions;
`PR-T51-01` serialized ownership; and the actual completed `execution` family's
`cmx_partition` and `cmx_simulate` implementations. Inspect their bodies before
wrapping them. The supplied retained helper processes input order and may write
a prefix before failing. A successful wrapper must not call that behavior
transactional or already sorted. Bring the original three0.6-task fixture and
predict its placement before consulting the oracle.

## 4. Environment and dependency profile

The core uses C17 and live, disjoint caller-owned objects. Calls are externally
serialized; ISO C does not enforce an owner CPU or prevent copying a lease.
The strict Cortex-M4 image links only the explicit placement implementation,
actual retained `advanced.c`, course byte helpers and original test image.
Audit generated instructions, symbols, stack files and link map for hidden
runtime helpers. Python integers and hosted stdio are verification tools.

The separate Linux experiment uses pthreads, barriers, a mutex and affinity
calls for its own threads only. It needs two CPUs in its allowed mask and a
representable `cpu_set_t`; report environment unavailability instead of passing
when those premises fail. It changes neither host scheduling policy nor CPU
isolation/security settings. Ordinary Linux scheduling and a virtualized clock
do not implement or validate a hard-real-time scheduler.

## 5. Concrete inputs and operations

Implement the unsolved public `partition.c`. `struct rp_task` supplies period, relative
deadline, zero jitter/blocking, a nonempty CPU-affinity mask and an execution
cost for each of up to four CPUs. `job.cost` is producer metadata; the selected
`cost[c]` is authoritative for the wrapper. Support1–8 tasks,1–4 CPUs and an
integer horizon1–256 divisible by every period. Each period and deadline is
positive, deadline≤period, and each cost≤period. Zero-cost jobs are permitted;
the retained simulator counts only jobs completed through a service decrement.
No exact count of instantaneous zero-cost completions is claimed.

`rp_fit` publishes a complete `struct rp_plan` or leaves output unchanged. With demand
mode0 all deadlines must equal periods. Mode1 checks constrained deadlines.
Sort by decreasing worst eligible-CPU utilization, retaining input-index order
for ties; restore original task indices in the published assignment. The common
homogeneous implicit-deadline/full-affinity case must actually invoke retained
`cmx_partition` on the sorted temporary array. A stubbed producer must fail.
For unequal costs, affinity or demand restrictions, test CPUs in ascending
order against the complete current per-CPU admission predicate. `RP_FULL`
means the heuristic failed, not that every assignment is impossible.

`rp_exact` enumerates assignments in original task-index and ascending CPU
order, pruning inadmissible prefixes. It returns the first complete witness;
`RP_IMPOSSIBLE` means no assignment satisfies this exact finite model. Invalid
inputs and failed searches preserve the prior output. `struct rp_plan.used[c]` is exact
integer service demand over the supplied hyperperiod, not a rounded percentage.
`rp_verify` rechecks every assignment, affinity, load and selected demand test;
it does not trust a plan supplied by a caller merely because its type matches.

`rp_simulate` verifies demand admission, maps the selected per-CPU costs into
temporary producer jobs and invokes actual retained `cmx_simulate`. Report its
work, completed nonzero jobs, misses and migrations. The admitted constrained
model prevents a missed job being overwritten at the next release. Do not use
the retained overwrite behavior to hide backlog in an inadmissible task set.

`struct rp_lease` supplies an independent explicit transfer protocol. Initialize one
owner and epoch1. `rp_begin` reserves one outstanding work interval. `rp_finish`
adds checked work and releases that interval; at a full work counter a zero-work
finish still releases ownership. `rp_prepare` requires the old owner/epoch,
no outstanding work, a different target and capacity for a new epoch. It freezes
new work. `rp_accept` at the target publishes the new owner and increments the
epoch. Old credentials become stale. `rp_abort` lets the old owner abandon an
unaccepted preparation. No address or payload storage is freed by these calls.
The caller retains the object and payload until both workers are joined.

## 6. Why these mechanisms are necessary

Three0.6 tasks cannot occupy two unit-capacity bins although total utilization
is1.8. Input-order first fit fails on0.4,0.4,0.6,0.6; stable decreasing order
places original tasks0/2 on CPU0 and1/3 on CPU1. A late failure must not publish
the retained helper's partial writes.

Two `(C=2,D=2,P=5)` tasks have utilization0.8 on one CPU but demand4 by time2;
they fail. `(5,5,10)` plus `(1,10,10)` have density1.1 yet satisfy exact demand.
For affinities, a0.6 task allowed on either CPU followed by a0.5 task restricted
to CPU0 defeats the stated heuristic; placing the first task on CPU1 succeeds.
Unequal cost tables can change both order and feasible placement. They are
declared input bounds, not fabricated measurements from an M7/M4 board.

## 7. Required reasoning, invariants and costs

For CPU c and integer interval t, sum the costs of jobs whose release and
deadline lie in that interval. In the synchronous constrained periodic model,
the demand beginning at0 is the sum of `max(0,1+floor((t-D)/P))*C`.
Explain the model assumptions: independent fully preemptible jobs, no overhead,
blocking, jitter or shared-memory interference and fixed costs on each CPU.

Because each period divides H and D≤P, demand at t+H equals demand at t plus
the exact service in one hyperperiod. If that service≤H and every integer
point1..H passes, all later intervals pass by decomposition. Demand changes
only at integer deadlines, so checking those integers also covers intervening
real-valued intervals. This extension is valid under the stated periodic/
sporadic demand premises, not an arbitrary asynchronous device workload.

An individual service term is≤H; total demand is≤8H and therefore fits32bits.
Sorting costs O(n²). A complete verification costs O(cpus×H×n); exhaustive
search has at most4^8 leaves and finite recursion depth8. Record actual stack
and code size. A search certificate is not a measurement of cache traffic,
interrupt interference or physical execution time.

Migration invalidates static-partition arguments. Before preparation, stop
admission of the moving task, drain its borrowed work, recheck destination
affinity/cost/admission and account for transfer/interference overhead under a
new model. The lease enforces only its finite ownership state, externally
serialized by a mutex. It does not enforce that admission analysis or bind
an integer owner to an OS affinity mask. The native experiment demonstrates
that binding explicitly for two selected allowed CPUs.

## 8. Deliverables

Submit the wrapper, exhaustive checker and transfer implementation; original
counterexamples; stable original-ID assignments; exact constrained-demand
derivation; unchanged-state failure evidence; actual producer identities;
independent assignment comparisons and per-core simulation results. Include
native CPU masks/samples, bounded worker traces and a drained transfer transcript,
plus strict target audits. Preserve public stubs and private assessor separation.

## 9. Verification and acceptance

Run packing, transaction, demand, affinity and handoff. The independent Python
oracle enumerates the specified two-task matrix and deterministic larger sets,
comparing all1,038 rows, every chosen witness and actual producer simulation
summary. It contains35 heuristic failures with an exact witness and497 accepted
placements. These counts describe this input corpus, not absolute completeness.

Controls omit sorting, expose a partial publication, replace constrained demand
with utilization, ignore affinity, prepare during borrowed work, or omit the
epoch increment. Core and retained-producer stubs must compile and reject.

The native harness starts two workers on distinct allowed CPUs and verifies
the masks and128 samples per worker. It replays64 copies of the accepted
four-task service set, records640 service units per worker, elapsed monotonic
and thread-CPU time, and compares a deterministic result. Its barriers are
experimental coordination overhead, not deadline enforcement. After one worker
finishes200 payload steps, the other accepts the prepared lease and performs200
more; old credentials are rejected and the400-step result is exact. A wrong
CPU observation and stale epoch must fail. Missing tools/CPUs, compile failures,
crashes, sanitizer diagnostics and timeouts are not semantic negative passes.

## 10. Objective-specific rubric

For `.a`, level0 substitutes aggregate utilization; level1 repeats one packing
example; level2 explains stable order and per-core assumptions; level3 derives
and implements constrained demand and rejects the utilization-only control.
For `.b`, level0 wraps the helper without publication control; level1 sorts but
loses IDs or failure atomicity; level2 preserves IDs and all checked failures;
level3 distinguishes affinity-induced heuristic failure from exhaustive proof.
For `.c`, level0 calls a packing pass real-time evidence; level1 supplies a
simulation only; level2 retains exact certificates, actual producer results
and native CPU traces with their limits; level3 implements a drained transfer
and rederives admission/ownership/interference assumptions.

## 11. Hints and separate analysis

Hint1: normalize service over a common divisible horizon before comparing
utilizations. Hint2: sort temporary original indices. Hint3: count jobs by their
deadlines, not merely releases. Hint4: a failed heuristic needs a separate
search before impossibility can be claimed. Hint5: a new owner must not begin
while old borrowed work remains. Private solutions are
`assessor/legacy_realtime/partition.c` and `partition.md`.

## 12. Changed-condition and delayed assessment

Preserve the original minimum seven-day delay. `.a-DELAYED` adds constrained
deadlines and exact per-core demand; `.b-DELAYED` introduces affinities and
requires an impossibility certificate or a counterexample to the heuristic;
`.c-DELAYED` permits one controlled migration and demands a new ownership and
interference argument. These implementations prepare those attempts; they are
not independent learner attempts. The parent's M7/M4 transfer requires actual
measured per-core costs on the selected target before asserting target-specific
placement or performance; supplied unequal tables do not close that observation.

## 13. Containment

Only original code, synthetic jobs and owned local processes/emulator images
are used. Affinity changes affect the harness's own threads. No scheduler
privilege, host-policy change, isolation setting, external target, board flashing
or real data is required. Logical deadline violations demonstrate the stated
timing property only; no privilege escalation claim follows.

## 14. Evidence status and limits

Pilot, full campaign, external review, learner attempts and physical observation
are separate. `tools/run_partition_depth.py` records current source identities,
commands and raw results. Actual native execution is ordinary Linux process
evidence; strict M4 execution validates code/dependency closure in an emulator.
Neither establishes physical WCET, simultaneous M7/M4 execution, cache-locality
benefit or all possible release patterns. Retain those target observations as
required when making their corresponding claims.
