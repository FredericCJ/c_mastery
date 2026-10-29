# T52.05 continuation: cohort age, debt and disputed equivalence

## 1. Identity and exercise families

Required obligation `LRS-CLASSICAL-LONG-ACTIVE` continues the original
`AX-PR-T52-05-T52.05.a/b/c` and their delayed variants. Families A/B/C/D/E/F/G/H/I/J
apply. Preserve their original criteria and the earlier per-dispatch successor.

## 2. Objectives and inspected sources

The gate in `research/v5/expansion/legacy_realtime_cohort/research_gate.json`
reuses exact `R-LRS-BOOK`, `R-LRS-SPRUNT`, `R-LRS-DEFECTS` records. Compare the
historical equivalence claim with the later primary counterexamples, instead
of treating a theorem heading as conclusive. This original finite implementation
is derived from separate unit rights. It is not a transcription of the RTAS
pseudocode, and the revised2011 pseudocode remains an explicit access gap.
No assertion about a current POSIX implementation follows from a historical
paper or this original teaching target.

## 3. Prerequisites and learner artifacts

Complete the T52.05 per-dispatch `server.h` adapter and retained `advanced.c`.
Import those actual completed learner sources explicitly. Compare rights and
activation time, not merely function names. Understand preemption, fixed
priorities, ownership of a finite state object and checked nonwrapping time.

## 4. Environment and dependencies

C17, a serialized one-CPU integer-time model, hosted GCC/Clang and a strict
freestanding Cortex-M4 emulator. `run.py` accepts the cohort source, server
source, advanced source and both include roots. Target compilation additionally
requires explicit byte-helper and linker inputs; retain assembly, map, stack
usage and undefined-symbol checks. Python is a host oracle, never a target
dependency. No ready-made scheduler library implements the mechanism. This
model does not reproduce a whole operating system or certify physical timing.

## 5. Concrete problem and supplied inputs

Implement the unsolved `cohort.c` against `cohort.h`. `struct su_state` owns Q distinct
unit rights, `1<=Q<=32`, period `P>=Q`, a declared maximum overrun `0..Q`, a
nonwrapping32-bit clock,64-bit service/debt counters and active-priority state.
Each unit has its earliest eligible time and, when activated, its own anchor.
Objects remain live and externally serialized; callers do not alter fields.

`su_active(now,active)` advances time and reports whether the processor's
priority interval is active for this server. Higher-priority preemption keeps
that interval active. When an inactive interval becomes active, eligible units
receive the current anchor. A future unit becoming eligible during an already
active interval receives its own due-time anchor, never the older cohort's.
When the interval becomes idle, unused eligible units are deactivated and their
eligibility is conservatively advanced to now; inactive anchor fields normalize
to zero. Already pending future rights retain their separate times.

`su_run(now,n)` records n consecutive execution ticks. The priority interval
must be active, at least one unit eligible at admission, and
`n<=initial_available+declared_overrun`. This conservative admission limit does
not speculate about future credits arriving during the observed chunk. At each
tick choose the eligible unit with oldest anchor, then lowest index. Its next
eligibility is `max(anchor+P,tick+1)`. The maximum handles a late active interval
without moving an eligibility time earlier. Newly due units retain their own
activation anchors. If service exceeds eligibility, charge the earliest future
unit by postponing it another P, and increment the borrowed-service count.
The model accounts for an overrun after it occurs; it cannot undo its first
interference with another task. A real adapter must enforce the declared bound.

Every invalid, exhausted or overflowing operation preserves the entire state.
Backward time/invalid arguments give INVALID, resource/time/counter exhaustion
gives FULL, malformed state gives BROKEN. Reaching UINT32_MAX does not wrap into
fresh service. `su_available` observes currently eligible units; it does not
grant permission to run while the priority interval is inactive.

The intentionally flawed `specimens.c` is a supplied original teaching target,
not a learner solution to repair silently. It groups all newly arriving credits
under an old active anchor and forgives overrun debt. Its bounded driver never
exhausts its16-record array. Keep it separate from the corrected implementation.

## 6. Necessary counterexamples

Run the exact timeline with a server Q20/P50, requests `(0,18),(40,20),(90,20)`,
a higher-priority10-tick job released41 and a lower-priority49-tick job released0.
The old two-unit cohort activates40; the returned18-unit cohort activates50.
Combining those ages gives the teaching defect. Record every tick's chosen job,
available capacity and remaining lower-priority work, not only the final time.

Separately split Q4/P20 by service2@0, then record service3@10 with allowed
overrun1. The flawed target creates five future/available units from four.
The corrected state still owns four rights: one due20, two due30 and one due40.
At20 it admits at most its one available unit plus the single declared overrun;
an attempted three-unit chunk is refused unchanged.

## 7. Invariants, failure and resource reasoning

There are always exactly Q rights. No path clones a right, forgives a borrowed
unit or changes another cohort's anchor. Every ordinary use advances that
unit's activation eligibility by at least P. Charging an ineligible future
right advances its future reservation again; this is explicit debt repayment,
not an enlarged future amount. Prove checked addition before staging updates
and why borrowed<=served bounds the accounting counter.

Do not claim Q execution ticks in every sliding P-window for this cohort model:
activation and actual execution differ under preemption. Do not infer arbitrary
task-set schedulability from the99-tick witness. The inspected sources motivate
the distinction; the particular finite transition system and its boundaries
must be derived and falsified on their own terms.

Time is O(Q*n) for a bounded observed run (`n<=2Q`), O(Q) for an interval update,
and O(Q) fixed storage. Transactional staging consumes a full state-sized stack
copy. Discuss the tradeoff against aggregated cohorts and the per-dispatch
ledger, including timer events, proof complexity, capacity limits and delayed
eligibility. Whole-image size includes the driver and imported producers.

## 8. Deliverables

D1: predict both service schedules and identify each right's anchor and debt.
D2: implement unit-state transitions and explicit unchanged-refusal semantics.
D3: record ordinary/late/preempted/overflow cases, the full216-row defect/repair
timeline, independent histories and the intended semantic-control results.
D4: change arrival phase and enforcement overrun, then explain which arguments
survive; compose the real per-dispatch producer and identify differing rights.

## 9. Verification and evidence-based acceptance

Run `run.py` cases `cohort`, `overrun`, `guard`, `comparison` with explicit
completed imports. Feed `oracle.py` into case `stream`; validate its captured
output via `oracle.py --output PATH`. The immutable host model checks1250
exhaustive short histories plus1000 seeded longer histories, totaling23000
transitions. The separately specified witness checks all216 trace rows:
corrected completion99, flawed completion117.

Boundaries include newly due credits during a long active interval, partial
service before blocking, later reactivation, exhaustion, backward time, near
UINT32_MAX refusal and malformed state. Five compiled semantic controls merge
cohort ages, forgive debt, retain an old activation across idle, publish a time
advance on refusal, or replenish immediately. Public cohort and imported
per-dispatch stubs must also compile and fail the intended semantic assertions.
Crashes, sanitizer errors, missing dependencies or compile failures do not count.

## 10. Assessor rubric

Level0 violates ownership, arithmetic or containment, or asserts unsupported
equivalence. Level1 reproduces final99/117 without explaining the age/debt
transition. Level2 requires exact trace/history agreement, a causal explanation
for both defects, transactional failures, explicit producer composition and
generated-code closure. Level3 independently changes preemption/arrival/overrun
assumptions and derives the resulting schedule after the required delay.
For original`.a`, assess active interval and differing cohort ages; for`.b`,
assess rights conservation and finite refusal; for`.c`, assess actual victim
interference and a repair that does not merely cap a displayed total.

## 11. Hints and private analysis

First assign a permanent identity to each budget unit. Then separate when it
became eligible, when its cohort activated and when it actually executed.
Finally account for service that occurred without an eligible unit instead of
rounding it away. Assessor source and technical analysis remain separate.

## 12. Changed conditions and independent transfer

The earlier exact delayed variants remain required. Add a long active interval
spanning multiple intended replenishment times, then contrast this model's
explicit late-time rule with blindly scheduling a timer in the past. Split the
same total demand differently and vary overrun0/1/Q. Re-derive admission,
observed service and repayment; do not reuse a claimed sliding-window theorem.
Unaided delayed learner work is not established by these author references.

## 13. Local security scope

Only the original flawed in-process specimen and owned emulator are targets.
Attacker-controlled inputs are synthetic arrival phases and bounded overrun
observations; the violated property is temporal resource isolation. The harmless
success condition is excess ledger capacity or the victim's tick100 deadline
miss. Exploitability requires these exact modeled rules and workload; it is not
an attack on any current kernel. Retain repaired traces and semantic controls;
actual timer enforcement, caller discipline and finite arithmetic remain limits.

## 14. Validation status and remaining work

Consult the source-bound campaign/support for actual executed profiles. This
original model neither implements current SCHED_SPORADIC nor certifies the
historical corrected pseudocode. The revised report access gap, actual RTOS
server adapter, physical overhead, independent human assessment and delayed
learner evidence remain distinct. Finite successful checks do not prove mastery.
