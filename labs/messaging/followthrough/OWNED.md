# Owned channel buffers and actor mailboxes

This successor implements X-T73-002-R4 and X-T73-004-R2/R4 with the actual
`struct msg_hub` and `struct msg_actor`, introducing the shared `struct mfo_value` owner representation in
`owned_value.h`. X-T73-003 then consumes this representation and drop operation;
the channel exercise does not depend on the later operation-state exercise.
It is required work, not a replacement for the core channel/select and actor
invariants. Implement `src/owned_follow.c`; headers and tests are public, assessor
code is separate. Linux C17/POSIX host assumptions apply. No service or third-party
target is contacted. Payloads are original synthetic heap records with counted
destructors; incorrect owners can be demonstrated without using real data.

## Shared value owner

`struct mfo_value` is a unique descriptor containing `data` and `drop`.
A nonnull `data` requires a nonnull `drop`; a null `data` may still carry a drop
callback representing another owned resource. An all-zero descriptor owns nothing.
`mfo_drop` first copies the descriptor and clears the caller's descriptor, then
invokes the saved callback exactly once when it is nonnull. The callback receives
the saved data; it returns normally and must not access the cleared
descriptor through a retained alias. Clearing before callback permits the callback
to release its enclosing owner without a later access by `mfo_drop`. Copying the
descriptor does not create another ownership token. ISO C does not enforce any of
these uniqueness or lifetime rules; the API contract and targeted tests enforce
observable portions, and callers remain responsible for valid aliases.

## Channel representation and transfer boundary

Two original unsigned channels carry indices into eight stable records. Each record
holds a unique value plus `used`, `send_done` and `delivered`. A separate record
mutex protects the records and active-call count. The underlying hub's **common
mutex** continues to own readiness, selection and wait registration. Never hold the
record mutex while calling a potentially blocking hub operation; otherwise the
receiver may need the same mutex before it can complete the sender's handoff.

Initialize unused storage with capacities 0..4. Invalid parameters leave it
unchanged; runtime initialization failure leaves no usable hub and takes no payload.
Successful initialization owns two mutexes and a condition variable. All controls
stay at one address. Required outputs and input descriptors must be disjoint from
control storage and from each other. A select output must own no prior payload;
uninitialized descriptor storage is allowed. Each descriptor has one API caller at
a time and a payload ownership token must not be copied or concurrently accessed.

On send, an eight-record reservation establishes an escrowed descriptor while the
underlying send executes. The caller may not access the payload during this call.
Failure leaves ownership with the caller; OK clears its descriptor and transfers
ownership at the underlying accepted-send boundary. A receiver may already have
consumed and destroyed the payload before that sender returns. Therefore the
sender's return path must not dereference the payload. Nor may a receiver recycle
the record while that return path still holds its index: release metadata only
after both sender bookkeeping and receiver delivery finish. This second condition
prevents an old sender from clearing a newly reused slot. An eight-record limit can
return FULL even if a channel has room; `wait` waits on channel readiness, not on
record capacity. No unbounded allocation hides this admission limit.

The optional after-submit hook is a bounded scheduler-injection point, called on
the sending thread after the underlying send selects its result but before record
bookkeeping. It receives no payload. It may synchronize with a test coordinator,
returns normally and does not reenter channel APIs or retain the control. The
hook's lifetime is included in the active send. It is not a completion callback.

Select copies the accepted owner to its output and marks delivery. Failure leaves
outputs unchanged. Close rejects new sends and permits existing buffered owners to
drain. Cancellation is a request that wakes users and makes subsequent receive
return CANCELLED; accepted queued owners remain owned by the hub until destruction.
An accepted rendezvous remains OK even if cancellation arrives before sender
return. An unaccepted cancelled rendezvous preserves caller ownership. Requesting
cancellation never silently destroys a payload still in use by a receiver.

Destruction requires an external admission gate closed to all new users, and all
close/cancel callers quiescent. It checks in-flight send/select calls under the
record mutex and returns BUSY without destruction if any remain. After users join,
it drops each accepted undelivered owner once, outside locks, then destroys the
underlying synchronization objects. Drop callbacks may not reenter or retain the
hub. No operation, including repeated destroy, is valid after successful destroy
until new initialization. A BUSY check alone does not stop future callers.

## Why keep the common readiness lock

The delayed condition allows an explicit decision to retain the common lock.
Seal that decision here: keep it because both channels belong to one bounded hub;
the cost is serialized readiness and a two-channel capacity bound. Separate locks
would need a shared registration object, registration-before-final-recheck under
a documented lock order, generation-checked unlink and quiescence before reclaim.
A check-empty / release-channel-lock / register-wait split loses notification if
the producer publishes in the gap. The original local test schedules precisely
that gap, records a ready predicate before entering timed wait and permits the
POSIX spurious-wakeup result. It does not credit a timeout as a repair. The abstract
counterexample is the ready-but-unregistered state; the imported hub's one mutex
and atomic wait-unlock exclude that gap. Ordinary scheduling fairness is still a
premise; a finite test does not prove starvation freedom.

Required evidence: full/empty/closed unchanged owners, drain and cancellation
destructors, both rendezvous outcomes, accepted receiver-before-sender-return with
an unrecycled metadata record, BUSY destruction, spurious broadcasts and two real
producer threads sending 1,000 distinct owned payloads across a rendezvous and
buffered channel. Independent Python deque/owner accounting covers 9,334 finite
histories. Reject the consumed-failed-send and premature-record-reuse controls.
Use separate TSan plus ASan/UBSan campaigns. Hints: identify payload and metadata
lifetimes separately, then draw both possible sender/receiver completion orders.

## Actor representation and restart

A serialized `struct mfa_actor` owns the actual four-message actor and five record slots
(four pending plus one current allowance). Messages carry record indices. The
handler borrows the current value for its call and may enqueue another unique
owner, including to itself, but cannot recursively dispatch, destroy or restart
the actor while stepping. The wrapper releases the current payload after handler
return. Handler failure drops all abandoned queued owners, including self-enqueued
ones, before the base actor clears its queue. Drop callbacks do not reenter the
actor, and neither handler nor drop may free the actor control. Normal returns
only; there is no asynchronous cancellation-safe unwind guarantee.

On a matching live incarnation, successful send consumes the input; FULL/CLOSED
preserve it. A mismatched incarnation deliberately has a different policy: send
clears the input and transfers it to the required dead-letter callback, then
returns STALE. The callback receives the original generation and owns that value;
it must drop or transfer it. It may not reenter/destroy the actor or retain borrowed
controls. This is an explicit dead-letter sink, not delivery to the restarted
actor. Generations are finite freshness tokens, not authentication. Restart uses
the actual checked increment; UINT64_MAX refuses with OVERFLOW rather than reuse.

Actor destruction refuses while stepping, otherwise drops pending owners and marks
the actor dead. It does not stop external threads; all actor access is serialized.
Tests force current plus queued plus self-enqueued destruction, no recursive
dispatch, restart, stale reply routing, mailbox FULL ownership and generation
exhaustion. Controls omit abandoned-owner cleanup and ignore incarnation mismatch.
Explain why C cannot enforce uniqueness/borrow duration and why a callback-count
test alone cannot establish ownership. Record fixed control size, payload heap
cost, destructor work and lock contention separately from unsigned-copy throughput.
