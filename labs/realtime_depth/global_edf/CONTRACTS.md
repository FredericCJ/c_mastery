# T52.10 global EDF, explicit jobs and ready strands

## 1. Identity and families

This continuation implements `AX-PR-T52-10-T52.10.a`, `.b`, `.c` and their
original `-DELAYED` variants. Families A/B/C/D/F/G/H/I/J cover the Dhall trace,
sequential versus DAG ownership, complete job identities, defective dispatch,
independent finite histories, overlap rejection, explicit overhead, affinity
restrictions and changed parallelism assumptions.

## 2. Objectives and inspected sources

`research/v5/expansion/legacy_realtime_global_edf/resources.json` records the
actually inspected Linux scheduling documentation: task definitions, partitioned
versus global EDF, Dhall's counterexample and a sufficient multiprocessor bound.
The original project and its exact fixture remain the starting inputs. The
explicit job/DAG/serialized-overhead model below is original course analysis;
it is not an implementation of the Linux scheduler or an uninspected paper.

## 3. Prerequisites and artifacts

Bring `PR-T52-09` assignment/affinity evidence, `PR-T52-08` checked time
reasoning, a DAG topological-order argument, and the actual completed retained
`cmx_simulate`. Inspect its chosen-job bitmask and its unfinished-job overwrite
at another periodic release. Use it only for the restricted comparison stated
below; preserve all job generations in the new model. Bring a written prediction
of the original C10/D10 plus two C1/D9 timeline.

## 4. Environment and dependency profile

The core uses C17 with live, disjoint objects and external serialization.
It is a bounded scheduling model, not an SMP kernel. Integer CPU identities,
affinity masks and predecessor sets are model state; ISO C does not supply
CPU ownership, a deadline scheduler or runtime DAG checks automatically.
The strict Cortex-M4 target links the explicit model, retained `advanced.c`,
audited course byte helpers and original test image. Python/stdio/sanitizers
are host verification tools. No target library implements the scheduler.

## 5. Inputs, operations and output

Implement the public unsolved `global_edf.c`. Supply1–16 explicit jobs and one
or two CPUs. A job has task0–7, a nonzero generation unique within that task,
nominal/actual release, a declared maximum release jitter, an absolute deadline,
an allowed CPU mask and1–4 strands. Each strand has positive cost≤256 and a
predecessor bitset. Reject cycles, self edges, unknown predecessors, invalid
affinities, release outside0..256, actual release before nominal, jitter excess,
deadline≤nominal and deadlines beyond256. Model horizons are1..256. No implicit
periodic release is manufactured; the complete explicit job list is the input.

`ge_simulate` validates everything before publishing output. Invalid input
leaves the prior `GEResult` unchanged. At each tick before the horizon, admit
releases in input order. If an earlier generation of that task is still active,
reject the new release and retain both identities and the earlier work. A new
release exactly when prior work has completed is admissible. Releases at the
horizon are outside execution and remain future. Check misses after admission
and at the horizon, including a release whose anchored deadline already passed.

The absolute deadline is supplied separately from actual release. For jitter
experiments it remains anchored to nominal release; do not silently grant a
later deadline just because release was delayed. Completed-at-deadline is on
time. A missed job continues unless the specified overlap policy rejects a new
generation; lateness does not erase either the missed deadline or its service.

At each tick, snapshot ready strands whose predecessors completed before that
tick. A sequential job has one strand and can occupy at most one CPU. Distinct
ready strands of a DAG may run simultaneously, but no strand has two owners.
Choose smallest absolute deadline, then original job index, then strand index;
visit CPUs in increasing index. Respect each affinity and, in partitioned mode,
the configured task home. Those deterministic choices are model policy, not
an optimality claim for affinities or constrained parallel DAGs.

Dispatch overhead is explicit. When a CPU changes strand, acquiring the single
modeled ready-queue lock consumes `queue_cost` ticks,0–32; at most one CPU pays
that serialized queue work in any tick. A competing CPU records queue-wait
ticks and does no job service. Lock release becomes visible on the next tick.
After acquisition, a previously serviced strand changing CPU pays the configured
`migration_cost`,0–32, on the receiving CPU. First dispatch is not a migration.
Both charges reserve that strand, preventing duplicate dispatch. An already
paid dispatch executes one service unit before the next EDF choice; a newly
earlier job arriving during bounded overhead does not restart the charge.
This explicit nonpreemptible dispatch interval changes timing assumptions.

Record every CPU/tick as idle, service, queue work, queue wait or migration work.
Retain job/generation/strand and remaining service where a strand is reserved.
Record each job's remaining/executed strands, first service, finish, complete
mask, missed flag and disposition. Totals include misses, rejected generations,
completed jobs, actual service-owner migrations and each overhead class.

`ge_sufficient` evaluates the documented sufficient global-EDF bound for up to
eight independent implicit-deadline sequential tasks using exact service units
over a divisible horizon. Check zero jitter/blocking and all arithmetic premises.
The caller must separately justify unrestricted identical CPUs, full preemption
and zero overhead. Failure of the bound is inconclusive; it is not rejection
by an exact schedulability decision procedure.

## 6. Mechanisms and counterexamples

For C10/D10 and two C1/D9 jobs at0, both short jobs run0–1. The sequential
long job starts1 and finishes11, missing deadline10 despite utilization1.2<2.
Scaling its cost/period to N and keeping two positive unit jobs gives utilization
1+2/N and finish N+1. Execute N=10,20,50,100 and derive the limit; never replace
a positive small cost with zero and claim the same delay.

A diamond DAG with costs1,2,2,1 and edges A→B,A→C,B→D,C→D can finish in4
service intervals on two CPUs, with total service6. Running D while B or C is
unfinished is not legal parallelism. Running one sequential strand twice in a
tick is likewise not a speedup. Two cost3/deadline4 jobs pinned to the same CPU
cannot both meet deadline; allowing separate CPUs changes the feasible set.

Two tasks C9/P10 and C3/P10 are feasible on separate processors although the
stated sufficient global bound fails. This distinguishes bound failure, observed
finite success and a universal schedulability proof. The retained producer is
compared only for independent zero-overhead jobs released at0 with no subsequent
release within the horizon, so its overwrite policy cannot hide an old job.

## 7. Reasoning and resource obligations

Prove one owner per strand, predecessor visibility at tick boundaries, service
conservation and complete identity retention. For each admitted strand,
executed+remaining equals original cost; rejected/future work has no service.
Every CPU interval belongs to exactly one of the five trace kinds. A queue-lock
tick on one CPU may coexist with service on another, but never with queue-lock
work on another. Migration overhead does not decrement job service.

Explain global selection across all eligible CPUs versus fixed partitions and
selection within a declared cluster. An affinity mask restricts legal owners;
it does not make the unrestricted global-EDF theorem applicable. Global greedy
selection may move a strand back to a lower-numbered CPU and incur more cost.
Do not erase that behavior to improve a plot.

Bound storage by the job/strand/CPU/horizon constants. Ready selection scans
at most64 strands for each of two CPUs; the acyclicity check is bounded. Report
actual stack usage, code size and helper closure. The complete staged result
is intentionally finite and consumes substantial stack; disclose that target
cost rather than claiming a constant-size production scheduler. Model overhead
values are supplied costs, not measurements of physical queue locking or caches.

## 8. Deliverables

Submit the model, bounded DAG validation, sufficient-bound argument, all raw
job/CPU traces and independent expected histories. Include the original missed
deadline, scaled small-job examples, DAG parallelism, overlapping-generation
rejection, jitter, serialized lock charges, migration charges and affinity
restrictions. Preserve actual retained-producer identity and target audits.

## 9. Verification and pass/fail

Run dhall, dag, overlap, costs, affinity and comparison. The independent oracle
uses explicit node sets and reserved event lists rather than the C bitmask/
stage representation. It checks466 finite histories and25,604 rows, retaining
7,318 service units,2,048 queue-work ticks,86 migration-work ticks,769 missed
deadlines and245 rejected overlapping releases. These outcomes are intentional
observations of the specified inputs, not all successful schedules.

Controls permit duplicate strand dispatch, omit migration cost, ignore DAG
predecessors, overwrite old work on a new generation, ignore affinity or omit
queue-lock cost. The sequential duplicate produces impossible service and must
fail conservation. Public model and retained-producer stubs must compile and
then fail semantically. Compile errors, generic faults, sanitizer findings and
harness timeouts are not successful semantic controls.

## 10. Objective-specific rubric

For `.a`, level0 scales the single-core utilization bound by CPU count; level1
quotes the miss; level2 derives every ownership/service event; level3 derives
the positive-cost limiting family and explains the zero-cost discontinuity.
For `.b`, level0 double-dispatches or overwrites; level1 handles ordinary jobs;
level2 retains generations and explicit rejections with conservation; level3
replaces sequential ownership with validated ready-strand dependencies.
For `.c`, level0 equates finite tardiness with deadlines met; level1 reports
utilization only; level2 compares equal job contracts and all overhead/jitter
classes; level3 rederives affinity restrictions and separates throughput
observations from feasibility and sufficient bounds.

## 11. Hints and private analysis

Hint1: record remaining service before and after both CPU choices. Hint2: ready
strands need a tick-start dependency snapshot. Hint3: an overhead reservation
still owns a strand. Hint4: a missed deadline does not mean the job disappeared.
Hint5: the inequality's hypotheses are part of its result. Private solutions
are `assessor/legacy_realtime/global_edf.c` and `global_edf.md`.

## 12. Delayed variants

Preserve each original seven-day delay. `.a-DELAYED` scales the small positive
jobs; `.b-DELAYED` implements the diamond and other bounded DAGs; `.c-DELAYED`
adds pinned tasks, records lost parallel capacity and only then compares finite
throughput. These executable reference opportunities are author preparation,
not independent learner attempts.

## 13. Laboratory scope

Only original teaching jobs and locally owned processes/emulator images are
used. The model does not change host scheduling or security settings, interact
with third-party targets, flash a board or process real data. A deadline miss
or impossible service trace is a synthetic property violation, not demonstrated
privilege escalation or arbitrary code execution.

## 14. Evidence status and limitations

Pilot and current full campaign status come from `tools/run_global_edf_depth.py`
and its source-bound checker. Model, actual retained-producer execution, strict
emulator execution, independent review, learner attempts and physical timing
remain separate. This model does not implement a native SMP ready queue or
prove arbitrary DAG schedulability, physical cache costs or a universal tardiness
bound. No finite passing campaign proves absolute mastery.
