# Constant-bandwidth reservation: exact T52.06 continuation

## 1. Identity and families

This required continuation covers `AX-PR-T52-06-T52.06.a`, `.b`, `.c` and
their original `-DELAYED` variants. Families A/B/C/D/F/G/H/I/J apply to policy
distinction, construction, arithmetic boundaries, mutant diagnosis, histories,
refusal, resource accounting, target changes and composition. Preserve the
original project and historical fixture; completion22 is a throttled-service
observation, not the expected work-conserving soft-CBS answer.

## 2. Objectives and inspected references

Map each result to budget, scheduling deadline, application deadline, backlog
and actual charged owner. The source records are `R-LCBS-BOOK`,
`R-LCBS-1998` and `R-LCBS-LINUX` in
`research/v5/expansion/legacy_realtime_cbs/resources.json`. The 2011 book's
physical206–209 and the 1998 paper's physical2–5 were actually read; the
paper's formulas/figures were visually checked. Linux documentation §2.1 and
§§3.2/3.4 supplies the separately labeled constrained-deadline comparison.
The original paper delegates its main isolation proof to RETIS TR98-01;
that full report was not accessible. Keep this reference gap, distinguish
the inspected theorem statement from its proof, and do not claim proof inspection.

## 3. Prerequisites and learner artifacts

Import the completed `PR-T50-01`/`PR-T51-01` CMX task and mutex artifacts and
the retained `cmx_cbs_*` producer for `PR-T52-06`. Bring the `PR-T52-02` EDF
job analysis, `PR-T52-03` checked arithmetic discipline and `PR-T52-05`
server-policy comparison. The actual CMX source is an explicit build input;
the public producer stub must not pass integration. No scheduler, reservation
or synchronization library may implement the studied mechanism.

## 4. Environment and dependency profile

Use C17 with GCC/Clang hosted harnesses and strict freestanding Cortex-M4
QEMU images. Host `stdio`, Python unbounded integers and sanitizers belong to
verification, not target dependencies. Target imports are the explicit CMX
source and course byte helpers only; audit ELF symbols, link map, assembly,
stack-usage files and whole-image size. No libc/libgcc/libatomic, hidden
division helper, allocation or operating-system scheduler is permitted on
the target. This experiment does not set Linux SCHED_DEADLINE, change host
security/scheduler policy or flash physical hardware. Compiler/ABI and emulator
behavior remain distinct from ISO C semantics and mathematical EDF assumptions.

## 5. Problem, inputs, outputs and operations

Implement `cbs.h` using unsolved `cbs.c`. Storage and pointer arguments are
live, disjoint and externally serialized. `CBBudget` contains the actual
retained `cmx_cbs`, separate queued/throttled state, last observation and
relative deadline. Caller mutation and reinitialization of a live object are
forbidden. Choose one explicit policy:

- `CB_SOFT_RETAINED`: preserve the producer's strict `>` idle density test.
- `CB_SOFT_BOOK`: use the inspected non-strict `>=` boundary; immediate refill
  and deadline postponement still use the actual `cmx_cbs_charge` producer.
- `CB_HARD_DOCUMENT`: original finite comparison of documented Q,D,P wake and
  throttle rules, with Q≤D≤P; it is not execution of the Linux kernel.

Require `0<Q≤D≤P<2^31`; soft modes additionally require D=P. `cb_arrive`
means a request arrives, not every scheduling decision. While already queued,
retain budget/deadline. `cb_idle` clears pending-work state but preserves the
producer's initialized reservation; clearing its active bit would incorrectly
force a fresh reservation on every return. Time observations advance modulo
2^32 by less than half the range. Charge positive amounts no larger than the
remaining budget; never subtract a larger dispatch and repair it afterward.
Split work at depletion and reselect EDF before the next service unit.

Soft depletion immediately refills Q and adds P to the scheduling deadline.
Hard depletion leaves zero budget, throttles, and restores Q at the deadline
while adding P. Hard wake initializes deadline to now+D; the density comparison
still uses P. The finite hard model refuses with FULL if a late replenishment
is already a whole period behind the observation. Its output remains unchanged.
This explicitly exposed limit avoids inventing Linux catch-up behavior from
the summary documentation. Soft postponement also refuses an ambiguous
half-range future-deadline order. Other malformed/backward inputs are refused.

`cb_admit` returns an exact, sufficient two-client density decision using
64-bit cross-products. It does not claim necessity or prove arbitrary locking
schedulability. `cb_workload` accepts two reservations, up to24 concrete
requests, per-client capacity1..8, horizon1..128 and cost0..1024. Each request
has client, release, cost and separate application deadline. Use FIFO by
release then original input index within each client; EDF ties choose client0.
Zero work completes at release without consuming capacity. Full queues reject
the new request. Retain per-request admission/rejection/progress/finish/miss and
every tick's selected client, budgets, scheduling deadlines and backlog.
Invalid or unrepresentable plans publish no partial output.

`cb_mutex(critical_ticks)` composes actual CMX EDF task and mutex operations
for a finite model. Owner0 has Q1/P10 and a critical section2..12 units;
waiter1 arrives at1 with Q2/P5 and application deadline4. Charge the actual
running owner, not the blocked waiter's reservation. Priority donation in a
mutex is not bandwidth inheritance. This is a logical CMX experiment, not
a native-thread or physical port claim.

## 6. Mechanisms and misconceptions

For Q3/P10 and eight service units on an otherwise idle CPU, derive remaining
budget/deadline at cumulative service0,3,6,8: (3,10),(3,20),(3,30),(1,30).
The soft task finishes8; throttling finishes22. Postponing a scheduling deadline
does not extend an application's deadline. With a competing Q2/P5 reservation
releasing two units every5, soft client0 finishes14 and loses priority after
depletion at time5. Keep exact traces instead of comparing average rates.

At Q4/P8, consume2, become idle, then return at4: the book equality resets to
remaining4/deadline12 while the retained strict rule keeps2/deadline8. Neither
formula may silently replace the other. With Q2/D4/P10 the hard comparison
starts at deadline4 and replenishes with deadline14; substituting P for D is
a different policy. Finally, an actual shared mutex can delay the admitted
waiter until8 despite an independent-reservation density sum below1. The
independence/charging/blocking premises, not the utilization arithmetic, changed.

## 7. Required reasoning

State pre/postconditions for every operation and prove unchanged state/output
on refusal. Explain the bounded 64-bit products (each operand below2^31), the
half-range ordering restriction and the difference between modular arithmetic
and an unbounded scheduling timeline. Prove exact accounting across a split
dispatch and preserve queue ownership/identity under zero work and rejection.
Account for the task that actually runs through a lock, even when a more urgent
waiter has donated priority. Derive why the independent-reservation theorem
cannot simply absorb unbounded critical sections or uncharged interrupts.

Budget operations are constant bounded work; the supplied finite workload
model scans at most24 requests and two reservations per tick. State its
O(horizon×requests) bound and bounded staged trace storage; no heap is used.
Keep model overhead, target stack/code size, scheduling quanta and physical
worst-case execution time separate. Explain which conclusions change under
optimizing compilers, another integer width or a different timer/accounting port.

## 8. Deliverables

Submit implementation, representation/ownership diagram, exact equality and
wrap traces, split-dispatch proof, full workload output, independent oracle
comparison, and CMX mutex owner/charging trace. Include the three policy
contracts, actual producer identities, five semantic mutant diagnoses,
profile-specific sanitizer results and strict target dependency/stack audit.
Keep unsolved public and intentional negative specimens intact in the course.

## 9. Verification and pass/fail

The named tests are soft, equality, constrained, guards, workload and mutex.
`oracle.py` independently constructs600 finite plans and exact integer traces,
including overload, zero work, queue rejection, deadline misses and out-of-input-
order releases. The initial reference yields595 represented plans and five
explicit late-hard-model refusals; refusal must preserve the whole output.
The oracle result is checked from actual C output, not a duplicated pass count.

Reject all five semantic controls: throttle a soft server, omit consumption,
use P for D, erase the equality distinction, and charge a blocked waiter.
Separate own-source and CMX producer stubs must compile and fail specific
`CB_CHECK:`/`CB_M4_CHECK` assertions. Build failure, sanitizer findings, arbitrary
crashes or a generic target fault do not count as semantic rejection. Positive
M4 results require actual execution, explicit helper closure and stack files.
Finite tests support their stated observations; they do not prove absolute
mastery or supply the unavailable full isolation proof.

## 10. Objective-specific assessor rubric

For `.a`, level0 calls completion22 soft CBS; level1 prints a final value;
level2 derives all four state pairs and competing EDF priority changes;
level3 transfers to independent application deadlines and explains slack use.
For `.b`, level0 repairs unsigned underflow afterward; level1 covers only
ordinary parameters; level2 preserves refusal transactions and demonstrates
strict/non-strict equality, wrap and constrained D; level3 derives why the
hard comparison is a distinct sourced policy and identifies its catch-up limit.
For `.c`, level0 uses a checksum as isolation proof; level1 reports average
utilization; level2 retains per-tick/request evidence and discriminating mutants;
level3 explains exact CMX blocking/owner charging and constructs the changed
premise counterexample without claiming bandwidth inheritance or physical WCET.

## 11. Hints and separate analysis

Hint1: queue activity and the retained producer's initialized-state bit have
different roles. Hint2: cross-multiply before comparing density, but prove
the product/sum bounds rather than assuming 64 bits suffice. Hint3: after a
depletion, a different client may have the earliest deadline. Hint4: a waiting
client's deadline does not make it the CPU owner. Separate reference and
technical analysis are in `assessor/legacy_realtime/cbs.c` and `cbs.md`.

## 12. Changed-condition and delayed assessment

Keep the original seven-day delayed policy. `.a-DELAYED` uses the competing
Q2/P5 stream and derives the exact lost-priority boundary. `.b-DELAYED` uses
Q2/D4/P10 from the inspected documented policy and rejects field substitution.
`.c-DELAYED` changes the owner's critical section from2 to7: actual waiter
completion changes3 to8 and its deadline4 is missed while its budget remains
untouched during blocking. These are implemented preparation variants; they
are not evidence of an unaided learner completing a delayed attempt.

## 13. Security and containment

Only original local teaching programs and owned emulator images with synthetic
jobs are used. An adversarial client may exceed estimated service or hold its
original teaching lock longer; harmless observations are delayed completion,
missed synthetic deadlines or accounting violation in a named mutant. No host
real-time policy changes, deployed equipment, external service or real data is
in scope. Repair the exact charging, admission or blocking premise and retain
the counterexample; do not equate a timing failure with privilege escalation.

## 14. Validation status and evidence limits

Authored reference/public support is distinct from executed campaigns. Exact
checks and outcomes are produced by `tools/run_cbs_depth.py` and checked by
`tools/check_cbs_depth.py`; do not infer a completed campaign from this prose.
The main proof-source access gap remains in the research register. Independent
human review, delayed learner evidence and physical observations remain
unperformed dimensions. The finite constrained-deadline comparison is not a
claim that every current Linux kernel path follows its simplified model.
