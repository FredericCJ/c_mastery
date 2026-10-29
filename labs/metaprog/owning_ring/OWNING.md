# Required T68.01 owning-ring transfer

This extends X-T68-001-R1 through R4 and X-T68-001-DELAYED. Retain the earlier
`containers.ring` artifact and its FIFO/full/empty proof; the generated mechanism
is implemented here, with no call to a library ring. The retained scalar/pair
`mp_u32_*` and `mp_pair_*` implementation is exercised separately. Compile ISO C17
with extensions disabled on hosted Linux. Allocation is performed only by the
supplied clone callbacks/harness; the ring core itself neither allocates nor
implements an allocator. This is a serialized ownership exercise, not a concurrent
queue. Real pointers, live objects and admitted callback behavior are premises.

## Representation, operations and exact failure contract

`struct mpo_blob` owns its byte allocation; `struct mpo_words` owns its uint32_t allocation. An
empty owner has a null pointer and zero extent. A live owner has an extent valid
for its allocation. The two rings each contain four actual elements, head, count
and matching typed clone/drop operations. Derive `head < 4`, `count <= 4`, and
logical element i at `(head+i)%4` for i<count; every other slot owns nothing.
Representationally identical bytes do not authorize copying an owning ring.

Initialization requires a fresh or fully cleared, nonowning destination. It may
not overwrite an active ring. Missing required pointers/callbacks return INVALID
without publication; otherwise init publishes empty storage. Subsequent operations
require successful initialization and unmodified valid control state. All rings,
callback contexts, inputs and outputs must be live, properly aligned and mutually
disjoint. `clear` requires a nonnull valid ring. An input owner remains borrowed
and unchanged for push; it need not transfer ownership to the ring.

Push first rejects full with FULL and zero callbacks. Otherwise clone receives
an initially empty staged owner. Zero means successful, distinct ownership; nonzero
means failure and may leave a valid partially initialized owner. Drop must accept
both empty and partial owners, cannot fail, and must leave empty. Failed clone
therefore invokes drop exactly once and returns CLONE, preserving all ring fields
and existing owners; callback effects and allocation attempts are not rolled back.
Success transfers the staged owner into the vacant tail and increments count last.
There is no subsequent fallible operation. Invalid input pointers return INVALID.

Pop output must own nothing, but initialized sentinel metadata is allowed. EMPTY
preserves it and the ring. Success moves the front owner into output, clears the
slot, advances head and decrements count. The caller now destroys or transfers it.
Clear destroys remaining owners in FIFO order and advances head once per owner;
repeated empty clear makes no calls. Zero-size values remain distinct owners if
their callback allocates a zero-size representation. Callbacks may not reenter or
mutate this ring, retain borrowed inputs/staged pointers, throw/longjmp, or access
owners after drop. The context outlives every possible callback.

`struct mpo_erased_ring` has four pointers to actual, empty typed objects and a fifth actual
typed scratch object. All five must be pairwise disjoint and disjoint from control,
inputs and outputs; pointer equality checks cannot prove full object disjointness.
Its matching erased clone/drop/move callbacks are caller obligations. Move cannot
fail, consumes source and writes an empty destination. No character array is cast
to a declared owning struct. Scratch is empty between calls. Full/empty, partial
clone cleanup, FIFO clear and pop ownership are exactly the typed contract.
`mpo_manual_*` is a handwritten Blob specialization with that same contract.

## Construction and concrete workload

Implement the unguarded private template twice, with prefix, ring, element and
operation types. Assert associated element agreement with `_Generic`, require
capacity four, reject occupied private helper macro names, and undefine parameters
and helpers. ISO C diagnoses a violated constraint; macros do not prove clone/drop
laws, callback lifetime, ownership or automatic hygiene. Export only declarations
from the guarded public header; put definitions in one translation unit.

Push owners holding bytes 1,2,3,4; pop/destroy 1; push 5; clear must destroy
2,3,4,5. A word owner holding UINT32_MAX,7 must retain 7 after the borrowed input
changes to 99. Inject clone refusal before allocation and after allocation; both
return CLONE with no live allocation and unchanged empty ring. Fill the ring and
verify a fifth push performs neither allocation nor cleanup. Empty pop preserves
the null-pointer/length99 sentinel. Compare all three implementations against
the supplied independent event model over ABFGPC histories: successful pushes of
1/2, before/after-allocation failures, pop+destroy, and clear. The model tracks a
logical queue and destruction events, not the implementation's tail formula.

## Required evidence and assessment

D1: retain two expansions, the associated-type/static-capacity checks and the
explicit ownership representation. D2: derive rollback and transfer invariants;
produce C associated-type/private-name failures and a positive control, and Rust
associated-type mismatch/positive compilations. Explain why Rust trait signatures
still do not prove an arbitrary clone's resource law. D3: pass four anchors, reject
all eight compiled semantic controls normally, match every oracle transition,
link two declaration-only clients successfully, and show duplicate definitions
and an incorrectly guarded template fail at link. D4: implement the owning and
erased changed condition, compare identical callback traces, and report the costs
below. A sanitizer finding or abnormal process exit does not kill a semantic
control. A leaked partial owner, changed failure output, extra full-push callback
or reversed FIFO destruction fails irrespective of timing.

Score each original rubric separately: level1 is an ordinary scalar ring; level2
requires these proofs/evidence; level3 requires the delayed owning variant without
copying the prior solution and an explicit counterexample to the old value-copy
proof. Author solutions and runs do not establish a learner's level. Hints: first
draw the four ownership edges; next enumerate clone's two failure points; finally
locate the last fallible operation before publication. Assessor analysis remains
outside learner packaging. At delayed assessment change the owner to a two-resource
object whose second acquisition fails and supply its cleanup trace; the learner
must adapt the same contract, not merely change capacity.

## Thirteen cost and enforcement dimensions

1. Representation: record sizeof/alignment of both owning elements, typed rings
and erased control plus five separately typed objects. Count scratch storage;
pointer-only erased control is not its total footprint.
2. Operations: push/pop O(1) in capacity plus clone/drop/move costs; clear O(n)
callback invocations. Extent-dependent clone is O(bytes/elements), not constant.
3. Invariants: prove FIFO occupancy, vacant emptiness and unique ownership;
ordinary C typechecking cannot establish allocation uniqueness.
4. Ownership: explicit clone/move/drop and context lifetime; no automatic RAII.
5. Failures: preflight full/invalid, clean partial clone, then nonfallible publish;
callback side effects are observable and not undone.
6. Allocation: core zero calls; actual harness one allocation per accepted clone,
including injected postallocation failure. Erased scratch is caller supplied.
7. Stack: typed push has one staged element, erased push uses caller scratch;
record actual compiler stack reports and distinguish callbacks' frames.
8. Traffic/cache: typed slots inline; erased slots add pointer indirection and
may be scattered. Payload clone/free dominates this small owned-byte workload.
No cache-miss or false-sharing measurement is claimed.
9. Dispatch/code: template specializes two types, but typed clone/drop remain
function-pointer calls. Erased move adds a callback. Compare one generated Blob
instance with one handwritten Blob instance, then record the second type's code
growth separately; do not compare two instances against one as pure overhead.
10. Build: record preprocessing token/byte volume, distinct exported symbols,
O0/O3 section sizes and clean versus unchanged Ninja builds. No universal compiler
size/time guarantee follows from these local observations.
11. Runtime: seven 10000-round samples each enqueue and pop/destroy four one-byte
owners, with identical checksum100000 and 40000 drop calls. Keep allocation,
ownership and callback semantics equal for typed/manual/erased comparisons.
12. Enforcement/portability: ISO C17 typed signatures and static assertions versus
callback discipline; Rust associated types checked by its compiler. Host pointer
width, ABI, allocator and optimizer affect costs. Public N1570 is a draft, not an
inspection of the inaccessible published C17 or C23 standards.
13. Composition/limits: repeated inclusions need distinct prefixes and no leaked
private names; public declarations support multiple clients. No synchronization,
exception unwinding, automatic destruction or arbitrary native-pointer validation
is supplied. Security work stays in original local synthetic programs; a leak is
not evidence of code execution. Physical behavior and unaided assessment remain
separate from source-bound hosted author validation.
