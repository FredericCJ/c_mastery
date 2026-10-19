# T18 assessor: callbacks and API evolution

Withhold this file and all reference bodies from learner exports. Apply the
exact original AX criteria and seven-day variants; do not award one callback
test as credit for the entire topic. The public packet is the complete contract.

## T18.04.a — descendants change the quiescence set

The existing registry takes a handle snapshot before dispatch. Parent starts
with payload refcount1, adds child with refcount2, marks parent removing, then
returns. Deferred parent drop leaves count1; child did not belong to the snapshot.
The next dispatch reads 73 and removes the child, finally dropping to zero.
Destroy-between-dispatches drops the child without calling it. Destroy-inside-
parent marks closing and waits for parent return, then drops both registrations.
The test must observe two drops in all three paths; requiring the child to run
during shutdown would contradict this registry contract. The control block stays
live throughout. A valid alternative uses a scope-owned arena plus a pending
child count; it still needs explicit accounting and a final acknowledgment.

The early-remove mutant returns OK/drops during callback; reject before reading
released payload. The close-leak mutant skips slot1 and is rejected by drop/ref
counts. A sanitizer crash is unnecessary and is not the success criterion.

## T18.08.a — transitive effects

XOR transform uses eight bytes of automatic output and no allocator. Its logging
callback nevertheless calls malloc/free once. A wrapper's source-level lack of
malloc says nothing about reachable callbacks. The bounded recorder changes
the callback contract and supports a final M4 image with no undefined symbols.
Retain object relocations because a final successful link alone could have
silently supplied helpers. Stack reports are per function/compiler; sum along
reachable paths with ABI/interrupt context separately. Neither stack bytes nor
empty closure proves reentrancy, interrupt masking, bounded allocator latency,
or physically observed WCET. A learner claiming these equivalences fails.

## T18.08.b/c — terminal ownership

Construction order is validate → allocate → copy → f_submit → install box and
callback/context → publish ticket → optional inline finish. There is no remaining
fallible action between provider submit and publication under the serialized
premises. Check both generation and releases headroom before allocation; the
provider checks release saturation only in f_complete, too late to avoid stranded
accepted work if an adapter admits that state. No counter wraps.

Finish saves box/callback/context in locals, completes the provider, clears
operation ownership and sets reporting before calling out. Successful transfer
allows the recipient to free the box immediately. For cancel acknowledgment,
drop the box before callback and pass null. The operation reporting flag is reset
afterward, requiring live operation control through return even though receiver
context need not be accessed again. Reject nested operations during reporting.
Alternative designs can permit nested submissions, but must prove their
generation/state updates cannot be overwritten by outer completion; they do
not satisfy this deliberately nonreentrant contract without a version change.

Oracle starts running/uncommitted. C/T preserve active ownership and change to
cancel-requested. K commits once, including when cancellation is requested but
unacknowledged. A without request rejects. S completes successfully from either
active state; A completes cancelled if no commit, indeterminate if committed.
All events reject after terminal. Terminal implies one callback, one provider
release and one box free after immediate recipient consumption. This is a
sequential history specification, not an actual cross-thread linearizability
proof. The two event names C/T intentionally share a transition but distinguish
caller cause and the mistaken deadline guarantee.

The reference explores 1+5+25+125+625+3125 histories using an independent Python
state model and records each C result line. Mutants must compile and reach a
contract assertion or an oracle mismatch. Request-is-ack fails before terminal
arrival. Rollback-after-commit fails histories such as KCA. Missing initialization
before inline completion is caught by the receiver's observed state/box/reporting
checks, not merely by a final call count. A future change must rerun all profiles
and retain the exact bound rather than reporting only a final PASS string.

## T18.10.a/b/c — compatibility and migration

| Dimension | Retained v1 | Silently deferred same signature | Explicit v2 |
|---|---|---|---|
| Source/ABI | Original header/object | May compile and link | New types/symbol |
| Output | Borrow within callback | Lifetime changes | Owned box |
| Timing | Before return | After return possible | Inline or later |
| Core allocation | None | Depends on adapter | One allocator call |
| Failure | No callback; written unchanged | Often accidental mutation | Ticket/operation unchanged on rejection |
| Cancellation | No retained work | Unspecified old protocol | Request plus terminal acknowledgment |

The old implementation retained inside the v2 archive is an intentional
compatibility adapter, not a supposed allocation-free wrapper around allocating
v2. The external consumer cannot see private headers. Relocation must actually
remove the old prefix and use the existing consumer object, preventing accidental
source/build include dependencies. Changing failure initialization alone is a
semantic break and fails the written=71 check even though timing/ABI remain.

Migration failure has no box and no callback obligation; accepted cancellation
holds context until finish; success transfers one box and its allocator-lifetime
dependency. Retained input/output snapshots remain stable across input mutation.
A copied pointer is not an owned copy. The original borrowed view can only be
copied while its owner/lifetime predicate holds. SQLite's inspected column
contract provides a concrete counterexample to universal borrow persistence:
conversion, step, reset or finalize can invalidate a result. Its finalize contract
also shows why consume-on-error must be specified independently of status.

Assess 0–3 independently for each of the seven leaves and changed tasks. Ask the
learner to derive a case outside the provided history bound, explain why stale
control storage is outside the API, and identify what thread synchronization
would be added. Model/native/sanitizer agreement remains bounded author evidence;
it does not establish independent performance or mastery.
