# T66 ownership laboratory contract

This family uses hosted ISO C17, exact-width integers and C17 atomics on Linux
x86-64. The pthread test harness supplies concurrency. The component implements
every ownership, allocation-policy, borrowing and copying mechanism locally;
libraries may not implement them. Atomic lowering, lock-free status and emitted
helpers are implementation observations. This is not a strict freestanding
image. Compile GCC/Clang debug, optimized and separate ASan/UBSan builds; TSan is
a separate diagnostic experiment whose availability must be reported.

## Objects and permitted calls

All pointers passed to the API designate live objects of the declared type and
extent. Distinct owners, outputs, control blocks and callback contexts do not
overlap unless an operation explicitly transfers ownership. Numeric and null
checks cannot validate arbitrary pointer provenance or a dangling pointer.
Objects begin zero-initialized except explicitly initialized atomic controls,
cell extent/value, and pin records. The array allocator has eight real
`int32_t[16]` backing arrays; it does not cast an aligned byte buffer to arbitrary
types. A live allocation owns its requested prefix. Pool reuse ends the
protocol lifetime of old borrows even though the actual typed backing array
continues to exist.

The imported `struct v4_allocator` declaration is copied byte for byte from the accepted
`labs/algorithms/include/v4_algo.h`. `take(context,count)` returns a distinct
live region suitable for count int32_t values, or null with no acquired region.
The caller checks count*sizeof(int32_t) before calling. `release` receives the
exact live base pointer once. Backends do not retain external owner pointers,
reenter the active operation, longjmp, block, or fail during release. The
instrumented backend is deliberately bounded; a hosted backing allocator is
permitted only as a separately documented adapter. The fixture checker verifies
the import's bytes. Integrate with the learner's containers.array callbacks,
memory.pool and testing.faults artifacts; preserve their contracts and record
the exact adapter revision.

## Ownership invariants

An `struct ow_owner` owns exactly block[0..count), with count in0..3, a valid allocator
when nonempty, and distinct non-null blocks. Construction publishes only after
all three acquisitions. Destroy pops before release, clears consumed pointers,
and is idempotent on empty state. Move requires an empty destination and poisons
the source to empty. C structure assignment can still duplicate ownership:
debug state and a diagnostic attribute do not create a move-only type.

Cleanup entries execute in reverse registration order if their always/error/
success policy applies. Finish pops before invoking a callback and rejects
reentrancy; callbacks cannot register or exit nonlocally. Publish transfers a
fully constructed staged owner before releasing the former published owner.
Both owners must be valid and disjoint; release cannot report failure. An
external side effect is not undone by swapping an in-memory owner.

Borrow records describe half-open numeric subranges of a single declared
extent. Active overlapping ranges may coexist only when both are readers;
empty ranges touch no element. Four slots bound simultaneous guards. A token
contains slot and serial; release checks both. Serial exhaustion retires a
slot, and no wrap permits stale-token reuse. Split consumes a writer and creates
two disjoint writers transactionally, including capacity failure. The token
protocol does not prevent a client from retaining an illicit raw alias.

Cell access goes through a live guard for exactly [0,1); writes require a writer.
Replace requires no active guard and returns the old value only on success.
This dynamic protocol is single-threaded. A const pointer to the cell does not
prove the absence of another mutable alias.

## Reference counts and concurrency

A newly initialized shared control has strong=1, weak=1. Weak includes one
implicit reference while strong is nonzero. Explicit weak references retain the
control, not the value. Last strong release destroys the value once, then drops
the implicit weak reference. Last weak release reclaims the control once.
Upgrade cannot resurrect strong=0. Retain/downgrade reject count overflow before
mutation. Destruction and reclamation callbacks cannot reenter that control.
The control remains a live object through its last permitted callback.

The non-atomic version requires external exclusion. The atomic version requires
each operation to begin with an owned strong or explicit weak reference as
appropriate. Double-drop, fabricated ownership and access after final weak
release are precondition violations, not recoverable races. Successful acquire
CAS increment, release decrement and acquire fence on the last release support
the stated lifetime protocol. Explain the synchronization chain, initialization
publication and why an immutable value or separate synchronization is still
needed. An atomic counter does not make every field or payload race-free.
Overflow refusal does not establish bounded destruction latency or cycle
collection. Strong cycles remain retained; cycle collection belongs to T74.

## Copy, address and checked-boundary protocols

Copy-on-write controls remain caller-owned and live while referenced. Clone
increments a non-atomic count; shared mutation allocates/copies a fresh buffer
before changing either owner. Failure leaves both logical values and ownership
unchanged. Unique mutation needs no new allocation. Private mutable borrows
cannot survive a sharing operation. Explicitly account for the old control's
remaining lifetime and backing storage release.

Pinning checks bind one live object address to a logical thread ID. The ID is a
test input, not authentication; a real thread adapter must obtain identity from
its environment. Submit retains the address until terminal completion. Cancel
only requests cancellation; it does not permit rebinding. Completion here is
an explicit backend quiescence event supplied by the teaching test. Compose with
io.async using its actual terminal-release contract, and reject a backend that
claims completion before it stops accessing storage.

The checked façade protects two eight-element logical tenants within one real
sixteen-element typed array. Its intentionally flawed counterpart performs
defined unsigned wrap and masks every access into that real array. Thus a test
can demonstrate cross-tenant integrity loss without an out-of-bounds C access.
The repaired boundary checks offset<=8 and count<=8-offset before forming the
trusted-core index. It does not isolate malicious code in the same address
space and does not claim arbitrary code execution, privilege escalation or
protection from pointer forgery. Only synthetic local data are permitted.

## Assessment and evidence

Each exercise's D1–D4 and objective-specific rubric remain mandatory beyond its
public CTest. Use the shared exercise contract and modern-cost-contract for
reasoning, experiments and all thirteen resource/enforcement dimensions. Mutant
rejection requires exit1 with OW_CHECK at the intended anchor; crashes, hangs,
compiler failures and sanitizer aborts do not count. The independent set oracle
checks finite borrow combinations, not a proof of all C lifetimes or schedules.
Preserve commands, source/input/toolchain identities, unexpected observations,
and minimized disagreements. Delayed transfer is attempted without the earlier
solution after at least seven days, recording assistance and reopening failed
claims. Authored, executed, reviewed and learner-assessed evidence are separate.
