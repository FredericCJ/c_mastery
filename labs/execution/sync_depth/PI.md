# T51.04: transitive donation under changed base priorities

This required successor implements the three original `PR-T51-04` criteria and
their existing `-DELAYED` variants. Retain their identifiers, seven-day delay,
original brief, prerequisite artifacts and assessment evidence. Historical
execution descriptions remain historical. This package adds an explicit CMX
import, an unsolved extension, public experiments and a separate author solution.

## Inputs, prerequisites and environment

Supply completed `PR-T51-01`, `PR-T51-03` and `PR-T50-06` CMX implementations and
the original lock/wait state traces. Compile `pi.c`, `pi.h`, `pi_tests.c` and
`pi_model.c` against the unchanged `cmx.h` contract. The public `run_pi.py`
requires `--execution-source`, `--execution-include` and `--extension-source`;
it never locates an assessor solution automatically. No priority-inheritance
library may implement this exercise. ISO C17 governs the extension. The hosted
measurement harness additionally uses POSIX `CLOCK_MONOTONIC`; the clock is a
host-side observation dependency, not part of the target core.

CMX task slots and registered mutexes must remain live at stable addresses.
All API operations, graph construction and reprioritization occur under the
same external exclusion domain. There are at most16 tasks and16 mutexes;
smaller priority numbers are more urgent. A blocked call completes through
the original task result. Calling the lock operation again is not completion.
This experiment does not add native task context switching or a physical port.

## Representation, operation and failure contract

Implement `cdpi_reprioritize(kernel, task, new_base, stats)`. Accept a live
nonexited task and a base in0..255. Derive every effective priority from current
base priorities, immediate-ceiling contributions and the current mutex-wait
graph, then publish all priorities and the requested base together under the
caller’s exclusion. Do not turn a donated priority into a new base priority.

A directed edge goes from a blocked mutex waiter to its mutex owner. Derive
the least fixed point under the numeric minimum operation. A caller may hold
multiple mutexes; removing one edge must preserve donations through the others.
Reject a missing registered wait object, an invalid owner, a closed registered
mutex, an invalid new base, or a base that violates an owned/waited immediate
ceiling with `CMX_INVALID`. Reject a dependency cycle with `CMX_DEADLOCK`.
On refusal, preserve the kernel and output statistics. Pointer validity is a
precondition: no C check can validate an arbitrary expired pointer.

The output records live vertices, direct edges, closure probes and reachable
donor/owner pairs. State the exact counting convention. Use no allocation,
recursion, library graph routine or unbounded traversal. Explain stack costs,
the bounded execution cost and how both change if the task limit changes.

## Construction and semantic experiments

For `T51.04.a`, construct L9 owning A, M7 owning B and waiting A, and H3 waiting
B. Explain and check every effective priority. Add a separate H5 waiter on A.
Expire H3, then cancel M7, then cancel H5. State which surviving graph edges
justify each remaining donation. Translate the original fixture’s larger-is-
more-urgent convention into this smaller-is-more-urgent contract explicitly;
mixing the conventions must produce a concrete wrong scheduler choice.

For `T51.04.b`, let one owner hold A and B with H5 waiting A and H3 waiting B.
Release B first. Change the owner’s base while donated, then change a donor’s
base and remove it. Preserve the newly assigned base after disinheritance.
Build the closing edge of a two-owner cycle through actual CMX calls, then
inject a cyclic graph into a private copy to test the extension’s refusal.
Explain why cycle rejection is not a proof of starvation freedom. Exercise
the maximum task/priority bounds, immediate-ceiling rejection and output
preservation. Analyze the selected fixed-point algorithm independently of the
retained CMX implementation’s repeated-relaxation algorithm.

For `T51.04.c`, run identical L/M/H release states through actual `cmx_schedule`.
L holds two nested mutexes and needs five declared work units; M has six
unrelated work units; H is blocked on A and needs one unit after acquisition.
Compare normal donation with an explicitly named no-inheritance experiment
that projects each effective priority back to its base before scheduling.
Do not modify the retained scheduler source. Record the selected task on every
step and account separately for owner work, unrelated M work, H work and
unmodeled scheduling/interrupt costs. The two traces must differ for the
mechanism-specific reason, not because their workloads differ.

## Verification and observations

Run `chain`, `nested`, `limits`, `costs` and `model`. The independent finite oracle walks
supplied acyclic owner paths rather than using the extension’s reachability
matrix. It checks120 upper-triangular five-task owner graphs for243 base
assignments, before and after one base change:29160 combinations and291600
priority comparisons. Explain exactly which cyclic graphs, larger task sets,
ceiling states and future transitions this enumeration does not cover.

Require all reference checks to pass; the unsolved extension and an unsolved
CMX import must compile and fail the intended assertion. Stale-donation,
missing-transitive-edge, base-overwrite and incorrect comparison-policy
controls must fail an appropriate semantic check. A compiler error, timeout,
arbitrary crash or sanitizer finding does not satisfy the negative control.

Measure31 batches of1000 reprioritizations and31 batches of1000 actual nested
CMX critical sections. Record raw minima/maxima and toolchain/profile, and
derive per-call observations only with their batching limitations. These
observed wall times include host scheduling and instrumentation effects;
neither the minimum nor the largest sample proves a worst-case execution time.
The original abstract five-unit blocking trace is a separate stated model.

## Required artifacts, rubric and changed conditions

Deliver the extension source, graph/ownership derivation, every expected trace,
refusal snapshots, independent oracle explanation, mutation results and a cost
worksheet. Every criterion is assessed separately:

* `T51.04.a`: level1 reproduces a priority list; level2 derives every surviving
  donor edge and the H5 restoration; level3 transfers the argument to a new
  chain and diagnoses a numeric-convention error without a supplied solution.
* `T51.04.b`: level1 handles a single owner; level2 preserves bases, other-lock
  donations and failure atomicity; level3 independently derives the new
  base-change/cycle contract and falsifies overstated progress claims.
* `T51.04.c`: level1 reports faster execution; level2 explains equivalent
  workload traces and measurement limits; level3 supplies the delayed nested
  blocking analysis with explicit scheduling assumptions and counterexamples.

The existing delayed variants remain required: an H5 donor directly on A,
a changed base during donation, and nested critical-section costs passed into
T52 response-time analysis. For the last variant, derive a recurrence only
under explicitly stated uniprocessor, fixed-priority, independent periodic
higher-priority jobs, zero jitter, no self-suspension and a justified single
blocking term. Use the abstract blocking fixture and compare any empirical
cost substitution separately; measured costs alone cannot certify deadlines.
Explain how nested locking, multiple blocking intervals or interrupts invalidate
an unjustified single-blocking assumption. Execute `blocking.py` on the four
supplied `blocking.json` workloads: the independent tick simulation must agree
with the recurrence; omitting blocking or using floor division must diverge.
The inspected historical fixture maps priorities by the order-reversing
translation p -> 6-p, so A1/B2/C5 become A5/B4/C1 and all effective values become1.
Actual unaided delayed assessment
is still separate from running the author’s experiment now.

## Graduated hints and evidence limits

1. A donor contributes its current base/ceiling seed along every owner path.
2. Keep derived priorities in temporary storage until all refusals are known.
3. Reachability followed by a minimum over donor seeds avoids depending on
   the order in which the original CMX relaxation scans task slots.

Assessor analysis and completed code are distributed separately. All inputs
are synthetic and local; no external target or security configuration change
is needed. Authored/executed evidence is reported by the current scoped campaign;
independent review, learner assessment and physical observations are not implied.
