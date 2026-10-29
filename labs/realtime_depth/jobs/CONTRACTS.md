# Multiple outstanding jobs: fixed priorities, EDF and changed assumptions

## 1. Identity and exercise families

This additive contract supplies executable work for the existing six criteria
`AX-PR-T52-01-T52.01.a` through `.c`, `AX-PR-T52-02-T52.02.a` through `.c`,
and their exact `-DELAYED` variants. It does not replace those identities,
historical units, hours or acceptance records. Families: semantic reasoning,
construction, boundary analysis, diagnosis, falsification, resource analysis,
changed assumptions and independent transfer.

## 2. Objectives and inspected references

T52.01.a assesses RM/DM ordering and sufficient versus necessary bounds;
T52.01.b stable assignment, release identities and transactional refusal;
T52.01.c equivalent-cost schedule comparison and blocking. T52.02.a assesses
absolute-deadline selection; T52.02.b bounded backlog and deadline detection;
T52.02.c independent event/demand oracles and the limits of finite horizons.
Exact inspected passages are registered as `R-LRJ-BOOK` and `R-LRJ-COURSE` in
`research/v5/expansion/legacy_realtime_jobs/resources.json`: Buttazzo third
edition, selected physical PDF pages97,103,117,120–128; complete original
PR-T52-01/02 briefs; actual retained priority/simulator/demand functions. The
full book, arbitrary deadline optimality proofs and physical timing were not
inspected or established by this batch. The release-jitter recurrence is the
explicit arrival-bound premise of the completed response-analysis contract.

## 3. Prerequisites and completed artifacts

Complete PR-T52-01 priority construction, PR-T52-02 finite demand analysis and
PR-T52-03 response analysis. Import your completed `cmx_priority_order` from
`advanced.c`, `cmx_edf_admit` from `cmx.c`, their public headers, and the completed
`rta_response` implementation/header. The runners take every source path
explicitly; they never find or copy assessor implementations. Complete C17
array/lifetime/checked-integer reasoning and distinguish task, job, release,
relative deadline, absolute deadline, priority and processor capacity.

## 4. Environment and dependency profile

The mechanism is a deterministic one-CPU integer-time model. Hosted C17 uses
stdio only in the test transport. The freestanding Cortex-M4 image uses no
hosted runtime, allocator, scheduler library, libgcc or implicit division
helper. Its only permitted linked helpers are the explicitly imported course
byte-copy/fill/compare functions; inspect symbol, assembly, map and stack-use
reports. QEMU MPS2-AN386 executes the model, not an actual preemptive task port.
ISO C provides storage and arithmetic rules; scheduling policy and the
integer clock are this laboratory's contracts. No board timing is implied.

## 5. Problem, inputs, outputs and supplied artifacts

Implement `jobs.c` against `jobs.h`. The public scaffold remains unsolved.
`struct sj_task` has cost, period, relative deadline and maximum release jitter.
`struct sj_job` has task index, sequence identity, actual release, actual cost and one
nonpreemptible service interval. Costs range0..1024; periods/deadlines are
positive uint32 values. A finite experiment has1..8 tasks,0..32 jobs,
capacity1..32, horizon1..128 and preemption cost0..128. Infeasible cost greater
than deadline is valid experimental input, not malformed input. All input and
output storage is live and disjoint. Duplicate `(task,sequence)` identities,
bad task indices, invalid sections, and unrepresentable absolute deadlines
are invalid. No wraparound time interpretation is provided.

`sj_order` calls the actual completed CMX priority producer through a staged
array. RM uses period, DM relative deadline, and equal keys retain input order.
It preserves every caller output on rejection, even though the retained raw
producer can publish a prefix before discovering a later invalid period.

`sj_generate` creates `jobs_per_task` releases for each task, job-major in its
output: release = phase + sequence*period + supplied jitter. Jitter entries
must not exceed the task bound. Every addition/product and the total capacity
is checked before publishing output/count. Jitter can exceed a period: output
identity order therefore need not equal actual release order. Generated
sections are empty; explicitly assign sections in the changed-condition work.

`sj_simulate` publishes a complete `struct sj_result`: every input job retains state,
remaining/executed service, first dispatch/finish, and whether a miss was
observed. Every tick records job identity and service/overhead/idle. Counters
include preemptions, misses, completed/rejected/dropped jobs and peak backlog.
Never overwrite unfinished work when another job of the same task arrives.
Unused result entries are zero-initialized; `SJ_NONE` marks no dispatch/finish.

`sj_rm_bound` compares exact rational utilization with
`n*(2**(1/n)-1)` without floating rounding: compare
`(n*H+sum(C_i*H/T_i))**n <= 2*(n*H)**n`. The LCM H is limited to65535,
and powers use checked uint64 storage. Implicit deadlines and zero jitter are
required. `RESOURCE` means the selected arithmetic representation cannot
answer; it does not mean the task set is unschedulable. A false sufficient
test is not a necessity result. All `INVALID`/`RESOURCE` results preserve
published outputs; successful model results may contain deadline misses.

## 6. Why these experiments are necessary

The retained simulator's one remaining-service slot per task cannot describe
overlapping jobs and can replace unfinished work. A finite demand check that
passes horizon2 says nothing about horizon3. Confusing a sufficient RM bound
with an exact feasibility test rejects valid harmonic schedules. Confusing
RM period with DM deadline changes actual priorities. Adding jitter, blocking
or preemption overhead changes the model assumptions even when utilization
is unchanged. No finite successful trace establishes universal feasibility.

## 7. State order, invariants, ownership and costs

At integer t, first mark each READY unfinished job whose absolute deadline is
t, once only. Completion at t from service over `[t-1,t)` was already recorded
and is on time. At the horizon, perform this miss check then stop: releases
exactly at the horizon are outside the experiment. At other ticks admit jobs
with release t in input order; zero-cost jobs complete instantly without
consuming backlog capacity. A miss records a property violation and does not
silently discard the job.

At full capacity, REJECT_NEW records the incoming job as REJECTED.
DROP_OLDEST selects the smallest `(release,input-index)` among ready jobs,
except a currently executing job inside its nonpreemptible interval. If none
is eligible, reject the new job. A dropped job retains executed/remaining
history, is terminal, and cannot later complete. Capacity counts READY jobs,
not all historical records. Drops are explicit policy transitions, not normal
preemptions. Completion/rejection/drop counters cannot be substituted for
deadline-miss counts; a rejected job has not received a feasibility guarantee.

Select ready work by fixed task rank for RM/DM, or by absolute job deadline
for EDF, then release and input index. A running job whose executed service
lies in `[nonpreempt_start,start+length)` continues. Switching away from an
unfinished running job counts one preemption and charges the specified
overhead ticks; initial dispatch, completion and dispatch from idle cost zero
in this model. Releases/miss checks continue during overhead. When overhead
ends, recompute selection without charging the same switch twice. This
explicit policy is not an assertion about a real operating system's costs.

Prove `work+overhead+idle=horizon`, sum of job executed service equals work,
executed+remaining equals admitted job cost even after drop, no job receives
two services at one tick, and active backlog never exceeds capacity. Inputs
remain borrowed only for the synchronous call; the result owns its values.
No dynamic allocation occurs. Analyze O(n²) priority ordering,
O(horizon*jobs+jobs²) simulation and bounded result/temporary stack storage.
Report generated stack/code sizes and equivalent correctness contracts before
comparing alternatives; neither code size nor emulator time measures WCET.

## 8. Implementations and experimental deliverables

Submit all four APIs, a state-transition description, a separate hand event
table, a theorem-assumption table, cost analysis, raw outputs and compilation
commands. Run `run.py` for order, edf, backlog, assumptions and refusal with
explicit completed imports. Feed `oracle.py input` to the stream case and
compare using `oracle.py`; Python is a host verifier, never a target dependency.
Build `target_run.py` using explicit sources, linker and byte helpers; retain
map, assembly, symbols and stack usage. Explain every compiler-generated
helper. Keep attempted failures and fixes distinct in the evidence log.

## 9. Verification and independent expectations

For A(T10,D4), B(T6,D6), C(T20,D3), predict RM B,A,C and DM C,A,B. With costs
2,3,1, the DM response bounds are1,3,6 in priority order; changing B to4
exceeds D6 at7. Generate at least two releases of every task, compare RM/DM
event tables and observe the first misses. The harmonic C2/T4 and C4/T8 set
has utilization1 above the two-task sufficient bound but exact responses2,8.

EDF jobs A(r0,C3,d6), B(r1,C1,d3), C(r2,C1,d5) produce A,B,C,A,A then idle.
The two synchronous C2,D2,T5 and C2,D3,T5 tasks pass a demand check at2 but
need4 units by3. Check deadlines at the horizon; separately test zero-cost
completion, stable ties and release exactly at the horizon. For C4,T2,D6
jobs at0,2,4, retain all three: by10 the first two finish, the third has2
units remaining and one miss. Capacity1 must explicitly reject or drop,
never silently replace a slot. Test that a nonpreemptible current job cannot
be selected as a drop victim.

The independent Python ledger checks all job results and ticks for3000 seeded
cases plus432 exhaustive two-job release/deadline/policy/overhead combinations.
Its separate release enumeration checks finite synchronous demand counts
against the floor formula. These are bounded falsification observations.
Acceptance requires all declared cases, all original imports and all named
semantic controls to compile, then reference success and intended assertion
rejections. A compiler failure, crash or sanitizer report is not a successful
semantic control. Inspect11 distinct mutants: wrong EDF key, omitted miss,
silent replacement, ignored nonpreemption, free preemption, swapped priority
field, premature output, ignored capacity, omitted jitter, unstable tie, and
treating a sufficient bound as always passed.

## 10. Per-objective assessor rubric

Each original criterion is scored independently0..3. Level0 has a wrong
contract or missing artifact;1 reproduces a supplied trace without the
invariant/counterexample;2 implements and explains the full relevant contract
with its independent checks;3 completes its delayed changed condition without
copying a worked solution, explains limits and transfers the argument.

| Criterion | Required level2 evidence | Level3 changed condition |
|---|---|---|
| T52.01.a | Separate RM/DM keys, stable tie, exact harmonic counterexample, assumption table | Explicit jitter releases and changed arrival-count premise; no reuse of zero-jitter theorem |
| T52.01.b | Actual priority import, atomic wrapper, two releases/task, invalid-input preservation | D>T with simultaneous identities, retained backlog, bounded rejection/drop |
| T52.01.c | Feasible/infeasible chosen costs, RM/DM event comparison, equivalent overhead accounting | Nonpreemptible section, derive blocking and re-evaluate the missed deadline |
| T52.02.a | Every dispatch and remaining service, absolute deadline and stable tie | Second A arrives before first completion; explicit retention/drop decision |
| T52.02.b | Bounded ready structure, miss-before-release, horizon2/3 counterexample | Arbitrary deadlines, simultaneous jobs and overload ledger |
| T52.02.c | Hand trace, independent demand/event oracle, key/miss mutations, endpoints | Per-preemption charge exposes a previously feasible job missing its deadline |

No author campaign awards independent learner level3. Record the real study
and delayed attempt dates under the original minimum7-day policies.

## 11. Graduated hints and separate analysis

Hint1: write one row per job, not per task. Hint2: enumerate the state changes
at a time boundary before selecting service. Hint3: stage the complete result
and only publish after validation; separate an arrival key from a scheduling
key. The assessor's derivation and completed implementation are withheld in
`assessor/legacy_realtime/jobs.md` and `jobs.c`; exporting the laboratory must
not copy them.

## 12. Changed-condition variants

Execute every original DELAYED variant in the rubric. In addition, compare a
jitter sequence4,0 for T2: sequence1 actually releases before sequence0.
For low-priority service3 begun at0 and a high-priority C1,D1 job at1,
preemption permits on-time completion; a three-unit nonpreemptible section
leaves two units of blocking and misses. Without that section, charging one
overhead tick per preemption also misses, with unchanged useful utilization.
Do not apply an implicit-deadline utilization theorem to these variants.

## 13. Security and containment

This is a bounded original teaching model with synthetic jobs. Malformed
input/overload tests execute locally; no host scheduler policy, real-time
priority, external service, physical board or deployed equipment is changed.
The observable property violations are wrong job accounting and missed model
deadlines, not demonstrated privilege escalation or arbitrary code execution.

## 14. Validation status and limitations

Canonical successor support and source-bound campaign reports distinguish
authored, compiled and actually executed work. Learner scaffolds remain
unsolved. Physical cost measurements, independent review and unaided delayed
attempts remain separate observations. Finite schedules do not prove global
feasibility, arbitrary-release optimality, no starvation or absolute mastery.
T52 server, overload-policy, tick, multicore, FreeRTOS and Linux experiments
remain separate required work; this contract does not close those domains.
