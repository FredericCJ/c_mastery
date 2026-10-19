# Ordinary, deferred and concurrent reference-count lifetimes

This required X-T74-010 extension uses the actual `or_trial_delete` kernel with
an original finite owner, ordinary zero-count cascade, deferred decrement queue,
candidate buffering and a POSIX mutex adapter. It does not implement an atomic
reference count by itself and claim that reclamation follows automatically.

## Representation and accounting invariant

`ORRHeap` owns sixteen identities, two strong edge fields per object, up to 255
external roots per identity, eight pending decrement entries and a sixteen-bit
candidate set. Generation 1 through 3 admits an identity; 4 retires its slot.
At every public operation boundary, each live object's stored count equals its
external roots plus incoming live edges plus queued decrements naming it. A
queued decrement is a temporary retention obligation, not a logical root. Thus
the 16-bit count is bounded by 255+32+8=295 without relying on unsigned wrap.
All live counts are positive. Dead objects have no roots, counts or edge fields.

`new` creates one external root. `retain` adds a root to a currently live,
generation-matching identity; it can be used as a checked weak-handle acquisition
only while the owner remains live and access is serialized. Copying handle bits
alone adds no root. `release` requires an actual external root and consumes one;
it cannot release an incoming edge's count. Root exhaustion, finite slot exhaustion
and the 255-root limit return `OR_FULL` unchanged. A zero-root release is
`OR_STATE`; invalid/stale identities return `OR_INVALID` unchanged. Null edges
are `{0,0}`. All outputs and trial tokens occupy valid, distinct storage disjoint
from the owner and operation inputs. No arbitrary token forgery or direct state
mutation is authorized. Initialize fresh storage only; keep its address stable.

## Immediate and deferred transitions

`store` validates both identities and field 0 or 1, then admits any pending-queue
space before changing either count. For a changed target, increment its count,
publish the edge, then discharge or enqueue the old target's decrement. This
order handles aliases, self edges and replacement without prematurely retiring
the new target. An unchanged target does no count work but advances the revision.

Immediate decrement records nonzero results in the candidate bitmap. Zero count
retires the object and decrements each outgoing edge, including duplicate edges.
Use the bounded iterative cascade; no recursive destructor is called. A cascade
retires at most sixteen objects and processes at most 32 outgoing edges. This
simple implementation rescans sixteen slots to find each next zero, so its
generalized bound is O(V squared + E), not the linear bound of a queue algorithm.
It may retire the owner object of a self-edge update; the enclosing heap remains
live and the operation reports the exact retired mask.

Deferred release removes the logical root immediately but leaves its count charge
in an eight-entry FIFO queue. Deferred edge replacement similarly removes the edge
while retaining the old target's charge. Full queue means `OR_FULL`, unchanged
heap and output; silently dropping an entry leaks a charge. `flush(budget)` removes
at most that many FIFO entries, runs the same decrement/cascade for each, and
reports processed entries and the union of reclaimed identities. Zero budget is
legal. `OR_DONE` means the queue is empty; `OR_OK` means work remains. A budget
limits dequeued obligations, not the number of objects retired by their cascades.
Candidates coalesce in the bitmap and never need a growing buffer.

## Trial preparation, publication and concurrent use

Preparation refuses any outstanding deferred entries. It expands the candidate
set through strong edges and passes that entire closure to the original trial
kernel in scratch storage. True counts are not edited during this analysis.
Positive residual counts rescue their reachable descendants; residual zero alone
is insufficient. The token contains owner identity, revision, proposed count
state and reclaimed set. Keep tokens unchanged. Preparation is read-only and
does not grant permission to access dead payloads.

Every successful state-changing operation advances revision 1 through 65535.
Commit requires the same owner/revision, an empty deferred queue and an active
owner; stale tokens return `OR_STATE` with output and state unchanged. Commit
publishes scratch counts, retires dead generations, clears candidates and advances
revision. Repeated commit therefore fails. At revision65535, mutations return
`OR_FULL` without wrap. Explicit whole-owner close is still allowed after all
client handles, tokens and concurrent users are discarded. It destroys this
synthetic pool even if roots or pending counts remain; it is not a substitute for
closing external resources or stopping live threads. Close cannot run concurrently
with any other operation, and reinitialization starts a different owner lifetime.

`ORRConcurrent` serializes all owner operations with the actual POSIX mutex. Its
immutable request and output remain caller-owned for the call. An actual worker
retains cycle member A after preparation and before commit: commit must refuse,
then a fresh trial must retain A and B. After that root is released, a fresh trial
may reclaim both. The request's inspected state is copied while locked. Clients
must not read `heap` directly while another thread runs, destroy a busy mutex,
asynchronously cancel inside a call, or reenter the nonrecursive owner lock.

Mutex exclusion gives an internally consistent trial snapshot; revision checking
connects that snapshot to later publication. If each intervening mutation changes
revision without wrap, equality at commit means no relevant mutation intervened.
The mutex supplies memory visibility, not a fairness bound. Repeated mutator
activity can force repeated retry. An eventual successful commit requires an
interval with no intervening mutation and a caller that retries. This is a
synchronized alternative to concurrently traversing mutable raw object fields.

## Concrete deliverables, controls and transfer

D1 derives the accounting equation, transfer order, count bound, cascade bound,
candidate-closure argument and no-wrap snapshot proof. D2 implements every core
and concurrent-header operation and reuses the original trial kernel. D3 supplies
the public tests, six separately compiled semantic controls and an independent
tracing model. D4 supplies the concurrent late-retain/release history, a sustained
1000-retain/release worker versus 1000 trial attempts, resource measurements and
the shared thirteen-dimensional cost review. Inspect actual compiler code and
count work before timing.

The independent oracle covers 4096 three-object/two-edge graphs, eight root sets
and immediate/deferred release: 65,536 exact live sets and survivor counts. It
traces from roots, then independently sums incoming surviving edges; it does not
simulate candidate subtraction. Existing direct trial fixtures retain candidate
subset, inconsistent-count and rescue tests. Additional cases include duplicate
edges, root underflow, 255-root exhaustion, queue wrap/full rejection, generation
retirement, revision retirement, pending-trial refusal, foreign-owner token
refusal and unchanged failure outputs.

Reject controls that omit cascade decrements, allow root underflow, silently drop
a full-queue obligation, omit candidate insertion, publish a stale trial or admit
a trial with pending decrements. Each compiles and fails a semantic assertion.
No control relies on a signal, C undefined behavior or a sanitizer finding as its
success condition. The attacker can choose valid local operation sequences and
adverse thread timing in this original synthetic target. The violated property is
retention/count integrity or lifetime preservation. Demonstrate a wrong mask,
count, status or missing obligation; do not claim arbitrary code execution.
Repair the exact transition, rerun the control and retain the caller-lifetime,
finite-capacity, scheduling and no-forgery limits.

Hints: account for removed-but-not-yet-applied edges; then show why queue capacity
must be admitted before publishing a replacement; finally identify what can
change between a valid trial and its use. R1 assesses the derivation, R2 the
ordinary/cycle integration, R3 the independent oracle and controls, and R4 the
deferred/synchronized changed-contract cases. X-T74-010-DELAYED requires the
late-retain history under changed scheduling with a defended proof, not copying
the supplied worker test. Independent learner performance is not yet assessed.

## Source comparison and evidence limits

Bacon and Rajan's 2001 paper uses candidate buffering, copied cyclic counts and
epoch-based mutation accounting. Its safety tests combine external-count evidence
with evidence that concurrent increments did not invalidate a candidate; its
liveness result is conditional on adverse mutation not persisting indefinitely.
Our original owner instead freezes each analysis with a mutex and validates a
global revision before publication. It does not reproduce that concurrent
traversal algorithm or inherit its proof, throughput or pause measurements.

The actual author-upload PDF text, sections 2, 3.1, 4 and 5, was inspected through
[the primary paper](https://www.researchgate.net/profile/Vadakkedathu-Rajan/publication/2528969_Concurrent_Cycle_Collection_in_Reference_Counted_Systems/links/0c9605177f3187ac92000000/Concurrent-Cycle-Collection-in-Reference-Counted-Systems.pdf).
Some extracted mathematical glyphs and diagram layout remain unavailable; source
registration states that limit. ISO C supplies integer and object semantics,
runtime checks enforce bounded admission, POSIX supplies mutex synchronization,
and clients supply valid storage, ownership and quiescence. Atomic count updates
alone would not protect a pointer loaded before a competing final release.
Host tests and finite enumeration do not prove every interleaving, independent
mastery, a production collector or physical target behavior.
