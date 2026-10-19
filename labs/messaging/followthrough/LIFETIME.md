# Owned pointer payloads, growth and retained snapshots

This required successor implements X-T73-006-DELAYED and X-T73-009-DELAYED using
`lifetime.h` and learner `src/lifetime_follow.c`. It consumes the X-T73-002
`MfoValue`/drop contract. X-T73-006 produces the common counted-owner mechanism;
the later pointer snapshot consumes it. Keep the original atomic scalar and weak
ordering investigations: this changed representation has different progress and
resource costs. Linux C17 and POSIX mutexes are explicit dependencies. A mutex is
used to acquire a lifetime reference safely; reference counting alone cannot
resurrect an object loaded through an unprotected raw pointer.

## Counted immutable owner

`MflShared` is an allocated control containing an atomic unsigned reference count,
one unique `MfoValue` and a copied `MflAllocator`. `mfl_make` allocates control
storage; on success it clears the input descriptor and publishes one owned
reference. INVALID or allocation failure preserves both input and output. Each
successful `mfl_retain` creates exactly one further ownership token; it requires
an already retained object or a lock protecting an existing owner. Loading a
stale pointer and then trying to retain is an invalid caller action. Refuse at
UINT_MAX without changing the count. The test build also sets the same bound to7
to execute its exhaustion transition without billions of allocations.

Each token is represented by one pointer variable. Copying that pointer without a
successful retain does not create another token. `mfl_drop` takes a live pointer
to such a variable, accepts a null token, clears the variable first, then releases
one reference. Only the transition1→0 drops the payload and frees the control.
`mfl_borrow` exposes a const descriptor while the caller retains its token. The
payload is immutable during shared ownership; descriptor constness alone cannot
enforce that deep property in ISO C. Callback state and the allocator context
outlive the last retained token, including tokens surviving their former queue
or snapshot control. Allocator results have sufficient alignment for any ordinary
C object used here; deallocate matches allocate. Allocation/deallocation callbacks
must be thread-safe when called by concurrent users and must return normally.
Callbacks may not reenter, mutate or free an active queue/pointer control or retain
borrowed descriptor addresses. They can release their own payload storage.

All required control/output pointers are valid and disjoint. Output token storage
owns no previous token; uninitialized pointer storage is allowed. Operational
failure leaves it unchanged. Nonnull pointer validity is a caller precondition,
not something a null check establishes. Atomic reference counts serialize releases
but cannot make concurrent mutation of one token variable valid.

## Growable pointer deque

`MfgDeque` has a stable mutex, monotonic top/bottom indices, current allocated
array, capacity and maximum, plus active speculative-thief count. Start at
capacity2; maximum is a power of two from2 through64. Each array has a reference
count protected by the queue mutex. The current queue owns one array reference.
The queue owns one counted payload reference per live slot. Growth doubles
capacity and copies the live pointer slots without duplicating those queue
ownership tokens. It removes the queue's old-array reference only after publishing
the new array. Old arrays are freed when their last speculative user leaves.

Push consumes a unique `MfoValue` only after all needed array/control allocation
succeeds. Inject failure at both growth-array and shared-control allocation:
capacity, indices, existing payloads, input owner and output state must remain
unchanged. FULL at maximum and OVERFLOW at SIZE_MAX-1 likewise preserve input.
Pop transfers the queue's payload reference to the output. Empty leaves output
unchanged. Taking the final item advances both logical indices, preventing the
owner's last-pop/new-push cycle from making a stalled thief accept the old item.

Steal locks the queue, snapshots top and its array, and retains both the array
and the payload before releasing the lock. The optional test hook pauses here,
outside the mutex. It receives only its context, returns normally, and may
synchronize with the coordinator but may not operate on the queue. On reentry,
the thief can safely inspect its old array. It wins only if that logical top is
still current and nonempty; success transfers the queue's payload ownership to
the output. A losing thief returns CONFLICT with output unchanged. Both paths
release speculative references. Reading an old array does not imply ownership
of the live slot; array retention and payload retention are distinct obligations.

Destruction requires an external admission gate and joined/quiescent callers;
the controlled test may attempt it while a known paused thief is active and must
observe BUSY without effects. A BUSY result alone is not an admission gate.
Success drops queued payloads once, releases the final array and destroys the
mutex. No operation, including repeated destroy, is valid afterward until new
initialization. Outstanding returned payload tokens remain valid independently.
Initialization failure leaves no usable queue and takes no payload; invalid
parameters leave the control unchanged. Later operational failures preserve
their specified state. There is no asynchronous pthread-cancellation or longjmp
cleanup guarantee.

The decisive test holds a thief after retention, grows2→4, lets another thief win
and drop the same payload, then resumes the loser. Neither old array nor payload
may be destroyed before that last release. Mutants omit the array reference or
payload reference; destructor/allocator counters reject them before any stale
pointer is dereferenced. The demonstration is an early lifetime violation in
original local code, not an arbitrary-code-execution claim. Repair both lifetimes,
then test allocation cuts, capacity/index exhaustion, empty outputs,1,000 unique
owned payloads with real competing thieves and the independent finite deque model.

## Paused pointer snapshot

`MfpPointer` holds one counted immutable owner, a mutex and a nonwrapping even
publication counter. Initialization consumes one existing token only on success.
Snapshot retains the current object under that mutex before publishing a token.
Replacement atomically swaps the root under the mutex and releases the former
root afterward; it consumes the new token only on success. Counter exhaustion
returns OVERFLOW without changing either root or supplied owner. Successful
destruction releases the root, while already returned snapshot tokens survive.
External admission closure and quiescence precede destruction; callback and
no-use-after-destroy restrictions are the same as for the deque.

A real reader obtains a token and pauses. The writer replaces and releases its
old root. The reader must still read the former payload5, then drop it exactly
once; another reader subsequently sees9. Retained snapshots can outlive the
pointer control. The deliberately rejected design retains only a numeric sequence
and raw pointer: the original target demonstrates the destructor occurring on
root release, without dereferencing or even evaluating the stale pointer. A later
sequence mismatch cannot undo that lifetime end. Mutants omit snapshot retention
or admit counter exhaustion. This solution chooses mutex-protected retention and
immutable payloads; it does not claim a lock-free pointer seqlock or equivalence
to the scalar sequence object's retry semantics.

## Deliverables and assessment

Submit the ownership graph, array/reference invariants, last-item ABA argument,
allocation-failure table and callback/admission contract before implementation.
Deliver compiling learner code, exact native/model/destructor evidence, mutations
and a13-dimension cost comparison. All tests must preserve failure-side owners;
early or duplicate destruction fails regardless of throughput. Superficial
completion passes nominal push/pop only. Understanding separately proves why
holding an array does not retain its pointees and why retaining a pointer needs
a protected acquisition. Independent transfer must change the allocator lifetime,
maximum capacity or ownership boundary and show the changed argument without
copying the supplied solution. The delayed assessment remains at least seven days
after assisted work, with assistance recorded; author support execution does not
stand in for a learner attempt.

Hints: first count ownership tokens rather than pointer copies; next separate
array retirement from payload destruction; finally draw last-pop/new-push and
winner/loser completion orders. The assessor analysis is separate. Host tests use
synthetic records and local threads only. No physical timing, unbounded progress,
freedom from starvation, cycle collection or universal mastery is established.
Growth is O(live entries), each retain/release adds atomic traffic, each queue or
root access takes a mutex, and stalled owners can retain memory indefinitely.
Those costs are part of the representation, not hidden implementation details.
