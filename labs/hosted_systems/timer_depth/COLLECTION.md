# From a deadline requirement to a measured representation

An event loop needs to know which registration is due next. Begin with the
existing `TdTimer` lifecycle and `td_earliest` scan. Its O(N) query is a useful
baseline: it has no additional index to keep consistent, and cancellation
already distinguishes an armed registration from a borrowed callback.

This sequence extends PR-T24-05. Read CONTRACTS.md and complete its core and
native notification exercises first. Supply your accepted timer implementation;
the author campaign explicitly links the separate assessor provider. Passing
that campaign does not prove your imported component has been assessed.

## 1. Establish the common contract

Implement `collection.c` against `collection.h`. The caller owns fixed-capacity,
address-stable arrays and serializes every operation. Initialization validates
arguments before changing any storage. Nonoverlapping arrays of the documented
lengths are caller preconditions. There is no allocation, locking or callback
inside the collection. Reject invalid arguments with -1. Inapplicable operations
return 0 and preserve outputs; successful registration, selection and finish
return 1. Cancellation retains the timer contract's result 2 for an outstanding
callback borrow.

A token is `(slot, generation)` within its original collection and storage
lifetime. It is not a globally unique identity: callers must never submit tokens
from another collection or reinitialize storage while old tokens can exist.
Rearming an armed slot replaces its registration
with a fresh generation. A stale token cannot cancel its replacement. Generation
exhaustion preserves an existing armed registration; an exhausted idle slot
retires. Selection compares the pair `(absolute deadline, slot)`, so equal
deadlines always select the lower slot. A successful `tc_begin` removes the
registration from the collection and borrows it until `tc_finish`; periodic
finish reinserts only the timer's phase-preserving next deadline. Arithmetic
exhaustion and catch-up versus skip retain their original meanings.

Before coding, derive a state diagram showing collection membership, timer
state and callback ownership. These are related properties, not synonyms.
Show why removing a cancelled timer from an index is insufficient permission
to destroy storage while a callback owns a borrow.

## 2. Derive an index from the workload

Implement the unsorted scan first. Count admission, cancellation and next-deadline
operations separately. For A admissions, C cancellations and Q minimum queries,
derive the terms that depend on capacity and the terms that depend on occupancy.
Explain why benchmarking only removal of all timers chooses a workload for the
learner instead of answering the service's actual question.

Next implement an indexed binary heap. State both invariants: parent ordering
under the complete comparison pair, and the inverse relation between a slot's
position and the heap entry at that position. Repair a moved last entry upward
or downward as needed after arbitrary cancellation. A heap that sorts correctly
when drained can still corrupt its inverse index during cancellation.

Prove termination using distance to the root or remaining subtree height. Bound
child-index arithmetic before evaluating it. Distinguish these integer/bounds
obligations from the mathematical heap-order proof. Test replacement at the root,
middle and last positions, equal keys and a stale token after replacement.

## 3. Change distribution and precision

The wheel variant groups an absolute deadline by
`(deadline / quantum) % bucket_count`. It retains exact deadlines and a sorted
collision chain per bucket, including deadlines many revolutions apart. Inspect
bucket heads to find the exact global minimum. This bounded exact wheel is not
an assertion of constant-time timer dispatch: next selection visits B heads,
insertion may traverse a collision chain, and cancellation uses stored links.
State the worst case when all deadlines alias one bucket.

Vary quantum, bucket count, clustering and cancellation rate. A coarse bucket
does not authorize early dispatch or quantize this API's deadline. Separately
design a changed contract allowing bounded lateness: define rounding direction,
overflow checks and the admitted precision, then compare it only against variants
implementing that same changed contract. Reuse the small RZWheel model to reason
about revolutions, while explaining why its four-slot contract is not this API.

## 4. Connect finite checking, scale and Linux

The supplied tests run an independent one-shot deadline-array oracle through
36,000 histories and check representation membership after every transition.
The timer's own tests cover periodic arithmetic and borrowed cancellation.
These bounded histories support regression evidence; they are not exhaustive
proof of arbitrary capacity or every concurrent history.

Run the same public harness against your submitted sources. The author runner
`tools/run_deadline_pilot.py` explicitly uses assessor implementations and verifies
that the supplied learner scaffold is still rejected. Its native adapter arms
a real timerfd from the collection minimum, consumes the full notification
counter, dispatches due registrations and closes its descriptor. Observed
lateness is recorded, never interpreted as a worst-case bound.

For measurement, first predict whether scans, heap repairs or bucket collisions
dominate each operation mix. Validate equivalent traces before timing, warm up,
rotate representation order and retain every trial. Measure the uninstrumented
optimized binary separately from builds with operation counters or sanitizers.
Record storage, build identity and the actual operation mix alongside elapsed
time. The integrated PMU service experiment consumes these deadline collections
with segmented parsing and a bounded queue, so a representation must survive a
real consumer rather than only a standalone drain benchmark.

## Evidence and assessment

Submit invariants, operation-cost derivations, the ownership diagram, boundary
predictions, implementation, independent oracle traces and raw paired results.
Explain at least one workload where each alternative is plausible and one that
falsifies its attractive average-case story. Explain whether preparation cost,
callback work and storage count inside each reported measurement boundary.

The six existing rubric dimensions remain noncompensable. Visible tests establish
only exercised behavior. Independent selection and delayed transfer use the
separate assessor package; no author run establishes independent mastery.

The next consumer is the telemetry service's deadline/stream/queue integration.
Your final transfer brief supplies resource ceilings, cancellation rates and
precision requirements without naming a data structure. Revise the design when
those assumptions change and justify rejecting at least one plausible alternative.
