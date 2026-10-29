# Declared users, immediate ceilings and the single-CPU proof boundary

## 1. Identity and families

This required successor implements the exact original criteria
`AX-PR-T52-04-T52.04.a/b/c` (three separate IDs) and their three `-DELAYED`
variants. Families: semantic reasoning, construction, diagnosis, boundary
analysis, falsification, resource/performance analysis and independent transfer.
Original unit identities, descriptive hours and historical evidence remain.

## 2. Objectives and inspected references

T52.04.a distinguishes immediate ceiling, inheritance and original PCP;
T52.04.b assesses declared-user admission, restoration and refusal;
T52.04.c compares identical workloads and derives the scope of a blocking bound.
`R-LRC-BOOK` and `R-LRC-COURSE` in
`research/v5/expansion/legacy_realtime_ceiling/resources.json` register exact
read passages: Buttazzo third edition physical226–233,244–245, the original
project/criteria, and actual CMX priority, mutex, cancellation and scheduler
functions. The book uses larger numbers for higher priorities; this C API uses
smaller numbers. Translate every inequality explicitly. The printed proof
sentence on physical231 uses P2 where its preemption argument requires the
preempting task's nominal priority; supply the causal argument independently.

## 3. Prerequisite artifacts

Complete PR-T51-03 CMX mutex/PI and PR-T52-03 response analysis. Import actual
completed `cmx.c` and its header; do not replace its priority recomputation with
a test double. Know wait enrollment, owner handoff, cancellation, deadline
half-range, task versus interrupt context and the single-CPU scheduler model.
The original `cmx_schedule` rotates equal effective priorities. The new
experiment separately requires `sc_choose` to continue the current task on
equal effective priority. Do not silently identify those policies.

## 4. Execution and dependencies

C17, externally serialized original teaching state machine; up to8 tasks and4
resources. No allocation. Hosted stdio belongs only to the test transport.
The Cortex-M4 executable is the same logical model, with explicit course byte
helpers and no libc, libgcc, compiler-rt, libatomic or preexisting lock library.
Audit generated code, link map, undefined symbols and stack use. The numeric
stack extents supplied to CMX task registration are model identities, not
executable PSP stacks. This is not an actual RTOS port or an SMP lock.

## 5. Inputs, operations and concrete experiment

Implement the unsolved `ceiling.c` under `ceiling.h`. `struct sc_config` gives unique
base priorities0..255, a nonempty bitset of declared users per resource, the
exact ceiling (minimum numeric priority among those users), and whether
releases must be strictly nested. Reject absent users, out-of-range masks,
incorrect ceilings, duplicate task priorities and invalid sizes before
publishing any initialized state. Init takes fresh, stable, disjoint storage;
no old reference survives a later destruction/reinitialization. The complete
configuration is frozen. `sc_set_base` accepts an identical value but rejects
an actual change; rebuilding a new quiescent configuration is a separate step.

`struct sc_world` owns the CMX kernel/mutex records and declared-user bookkeeping.
Its address remains fixed; copying it after initialization is forbidden.
`sc_init` stages initialization and rebases the core's registered mutex
pointers when publishing. Validate source imports: an unsolved CMX must fail
semantic acceptance, not be silently substituted with assessor code.

`sc_lock` checks task/resource/context, declared membership and increasing
resource-index acquisition order. Reentry or reverse order is refused.
Immediate mode uses actual CMX immediate boosting; inheritance mode uses its
actual donation mechanism without immediate ceilings. Admission is based on
the frozen user/base table, not a temporary inherited priority. The supplied
context enum is part of this model, not hardware-authenticated ISR detection.
Interrupt-context calls are refused without beginning a blocking operation.

A blocked acquire returns CMX_BLOCKED and records one pending result slot.
After a wake, `sc_poll` records a successful handoff in the held set, or returns
the selected timeout/cancel result. Until poll consumes that result, another
task operation is refused as BLOCKED. The raw owner may already have changed
before polling: that owned-but-uncollected interval must satisfy the invariant.
Cancel uses actual CMX cancellation; tick uses actual timeout processing and
rejects a backwards/half-range-ambiguous clock advance. Use deadlines within
the inherited CMX half-range contract. Sequence exhaustion refuses enrollment
without preventing existing owners from releasing their locks.

`sc_unlock` removes the chosen held resource and lets CMX recompute all
remaining ceilings and donations. With strict nesting it must release the
highest held resource index first; with general ordered locking it may release
any owned resource. The latter illustrates restoration after only B remains
but is not admitted to the well-nested blocking theorem. `sc_choose` selects
the most urgent runnable task in the supplied released/noncompleted mask,
continuing current on a tie. The mask is a trusted finite workload input, not
an authorization boundary for an operational kernel.

Use priorities2,5,8 plus an undeclared priority1 client. A is used by2/8 and
has ceiling2; B by5/8 and has ceiling5. Owner8 holding both has priority2;
after A alone is released, only B remains and priority is5. In the strict
case acquire A then B, cancel an A waiter, release B then A. Compare immediate
ceilings with active PI donations in every intermediate state. Refuse an
undeclared priority5 user even though the raw core's ceiling inequality alone
would allow it. Rebuild a separate fresh table that includes priority1 and
recompute A's ceiling to1 before admitting that client.

## 6. Mechanisms and misconceptions

A ceiling number is not a declared-user table. Restoring a saved/base priority
can erase another ceiling or a still-active donation. A task can spend time
ready behind an elevated owner without having attempted a lock: this is not
the same measurement as direct mutex blocking. Immediate boosting does not
implement original PCP's system-ceiling admission rule. A fixed-priority
schedule with equal-priority round robin does not silently satisfy a proof
that assumes an elevated owner cannot be displaced by an equal-priority
arrival. None of these mechanisms alone supplies an SMP blocking theorem.

## 7. Invariants, ownership, failure and cost reasoning

Every owner is declared; each collected held bit names its actual core owner.
An owned handoff not yet collected has exactly one matching pending result.
Every waiting task has one pending acquisition and owns no new held bit for
it. Effective priority is the minimum of frozen base, all held immediate
ceilings and the fixed point of current transitive donations. A cancelled or
timed-out waiter ceases donating, while unrelated donations/ceilings persist.
Invalid membership, order, context, configuration and base changes preserve
ownership and queues. The representation is caller-owned, requires external
serialization and survives all pending results; no arbitrary stale-pointer
safety or forced-owner-death recovery is claimed.

Prove bounded scans over declared tasks/resources, inherited CMX wait capacity,
and the cost of priority fixed-point recomputation. Explain the temporary
initialization copy and its rebased interior pointers. Measure complete image
size/stack bounds from generated artifacts; those include harness/imports and
do not constitute isolated abstraction overhead or WCET. Compare protocols
with the same release times, service demands and uncharged transition costs.

## 8. Required deliverables

Supply the adapter, invariant, static user/ceiling table, refused-state snapshots,
and three exact workload traces. Use `run.py` with explicit completed CMX;
exercise table, nested, admission and workloads. Feed `oracle.py input` to
the stream case and compare complete state rows. Retain normal/boundary and
mutant outputs, compiler configuration, target maps/symbols/assembly/stack
use and the distinction between logical ticks and measured physical latency.
The assessor implementation and analysis are separate, never learner exports.

## 9. Verification and expected observations

The identical workload has low priority8 release0/service4 holding A, high
priority2 release1/service1 using A, and medium priority5 release2/service3
using no resource. Lock/unlock transitions consume zero model service.
Without a protocol the tick trace is L,L,M,M,M,L,L,H; H completes at8,
directly blocked for6 ticks. PI produces L,L,L,L,H,M,M,M, H completes5,
direct blocking3 and boosted low execution3. Immediate ceiling produces the
same service trace but direct blocking0 and boosted execution4: the high task
does not get to attempt its lock until the elevated low owner releases.
Medium starts2 without a protocol and5 with either protection. Record these
different metrics rather than calling every delay a mutex wait.

The independent Python set/queue oracle checks512 finite traces (6,144
transitions) across both protocols and both release policies. It checks all
effective priorities, owner IDs, held sets, pending results and waiting flags.
The C implementation must also reject wrong ceilings, undeclared users,
illegal nesting, restoration to base, stale donations, dynamic-base mutation
and equal-priority preemption controls after successful compilation. No crash,
sanitizer finding or compile failure counts as the expected rejection.

`oracle.py` also executes two original analysis witnesses. Its PCP predicate
denies a free requested resource when another owner's system ceiling equals
the caller's nominal priority; that is a predicate comparison, not a complete
PCP implementation. Its two-CPU event model permits low on CPU0 to release A
and acquire B before high on CPU1 drains A and requests B. High waits on A
from1..3 and B from3..6: total5 exceeds the single-CPU discrete3−1 bound2.
This demonstrates the failed nonpreemption premise, not a working SMP protocol.

## 10. Objective-specific rubric

Score each original criterion separately:0 missing/wrong;1 reproduces an ordinary
number without the mechanism;2 supplies its full implementation, invariant and
falsification evidence;3 independently handles its delayed changed condition.

| Objective | Level2 evidence | Required delayed transfer |
|---|---|---|
| T52.04.a | Exact table, both-held/only-B priorities, PI versus immediate versus PCP predicate | Reject undeclared priority1; fresh table recomputes A to1 before admission |
| T52.04.b | Actual CMX import, nested release/cancel restoration, atomic refusals, pending handoff | Frozen base policy rejects dynamic change; explain what safe reconfiguration would require |
| T52.04.c | Same workload, direct/ready delay and boosted counts, conditional blocking argument | Two-CPU witness identifies the broken preemption premise; proposed replacement must state a new protocol/proof obligation |

The original minimum7-day delay and unaided learner evidence are still required.
Author campaigns do not award independent mastery or human review.

## 11. Hints and assessor separation

Hint1: derive effective priority from the current ownership/dependency graph.
Hint2: separate a pending acquisition from an owned result not yet collected.
Hint3: test release of one resource while another still raises priority.
Private derivation/implementation: `assessor/legacy_realtime/ceiling.md` and
`ceiling.c`. Do not export those files or count copied solutions as learner work.

## 12. Changed assumptions

Execute all three original DELAYED variants. Also contrast general ordered
release with strict well-nested release; only the latter may use the quoted
HLP bound under its remaining assumptions. Re-enable equal-priority rotation
and explain the changed scheduling premise. Add self-suspension or migration
only after defining a replacement analysis; do not reuse the old bound.

## 13. Security scope

All jobs, counters and faults are synthetic and confined to the original
teaching model or owned QEMU image. No host real-time policy, physical device,
production system or external target is changed. A violated priority or
ownership invariant is not demonstrated arbitrary code execution.

## 14. Validation status and limits

The source-bound successor report records actual hosted and emulated outcomes.
Learner stubs remain unsolved; independent review and delayed learner attempts
are separate. No physical latency/WCET, arbitrary priority protocol proof,
SMP implementation, scheduler-port completion or whole-T52 completion follows
from these finite experiments. Other T52 obligations remain required.
