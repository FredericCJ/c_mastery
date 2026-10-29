# Withheld packet: cancellable service appointments

## Prerequisite check

Require the learner's accepted absolute-time arithmetic, timer-state lifecycle,
borrow/cancellation and collection artifacts. Ask for a proof of one generation
exhaustion transition and the meaning of cancellation while a callback runs.
Reject an imported author reference as evidence of learner construction.

## Initial task to release

A local service admits at most 4096 pending appointments. Each has a stable slot
identity, a monotonically increasing generation and an absolute uint64 deadline.
The workload starts with 4000 appointments spread across a minute, cancels half,
then admits replacements while a small ready set is dispatched repeatedly.
Cancellation of a running callback reports retained ownership until completion.
Equal deadlines dispatch in stable slot order. No appointment may run early or
more than once. Reject arithmetic/generation exhaustion without destroying an
already armed registration.

Choose and implement the collection and its event-loop adapter. Derive costs for
admission, earliest query, cancellation and expiration under the specified mix.
Give two alternatives, a concrete rejected design and a reversal condition.
Measure only after checking equal delivered identities and effects. Report storage,
code/operation costs and bounded timings separately; no implementation must win.

## Assessor trace, not released initially

Use slots 0, 1, 2, 3 with deadlines 100, 100, 90, 120. Cancel slot 2, rearm it at 110 with
a new generation and retain its old notification. At now 100, deliver 0 then 1;
the old slot 2 notification cannot dispatch. At now 110, deliver 2 exactly once.
Begin slot 3 at 120, request cancellation twice while it is borrowed, then finish;
its storage cannot be reclaimed before finish. Repeat at reduced generation
limit 3 so exhaustion is observable without long runs.

Compare against an independently coded stable scan of an immutable event log,
including periodic catch-up and integer-edge rejection. Run a tiny exhaustive
history first, then bounded larger traces. A source that merely labels failures
without invoking cancellation does not meet the test.

## Changed condition to withhold

After the delay, the service admits only 256 appointments, accepts wakeup lateness
up to 2 milliseconds, and receives bursts where almost every deadline is identical.
Cancellation rises to 95 percent, but occasional deadlines are an hour away.
The earlier ranking may reverse. Require an adapted design and matching semantic
comparison; do not name a heap, wheel, list or tree. A design that rounds times
must declare that revised precision contract before measuring it.

## Scoring discriminators

Mechanism 2 states and preserves every ordering/index invariant; 3 derives the
workload-dependent cost comparison. Correctness 2 matches the independent trace;
3 falsifies a stale-handle or tie-order mutant. Ownership 2 distinguishes request,
callback borrow and reclamation; 3 adapts that distinction through shutdown.
Resources 2 bounds live entries and stale state; 3 addresses worst-case cleanup
bursts rather than amortized means alone. Diagnosis 2 separates timer semantics
from measurement noise; 3 proposes an experiment that can reverse the design.
Transfer 2 selects independently under the new contract; 3 defends a choice that
may retain the original representation when evidence warrants it.
