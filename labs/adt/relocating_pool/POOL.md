# T67.05: Pool identity, relocation and copied concurrent access

This required successor implements X-T67-005-R4 and its delayed variant without
changing the retained fixed-address `ADPool` contract. Complete R1–R3 using the
actual `ad_pool_*` functions first. Retain `memory.pool`, the ownership ledger and
the original independent occupancy model; implement this adapter locally. The
public scaffold is `relocating_pool.c`, the header and five `adt.pool_*` tests.
The reference implementation and technical answers remain assessor-only.

## Representation and admitted objects

Use C17 hosted Linux and POSIX mutexes. A live `AdpPool` owns one heap allocation
containing four typed slots and one initialized mutex. Slots are FREE, LIVE or
RETIRED; each has a positive uint64 generation and a uint32 payload. A handle is
an ordinary copyable triple `(nonzero pool identity, index, generation)`. There
is no raw slot pointer or borrowed payload in this API. `get` copies a value
while holding the mutex. A copied handle is neither a second allocation owner
nor an independently authorized release: after one release, both copies stale.

The `AdpIds` issuer is externally serialized. Initialize it at a positive first
identity, issue successive identities, issue UINT64_MAX once, then return FULL
without changing the output forever. Resetting an issuer while issued handles
can survive violates its precondition. Nonreuse across process restart requires
an external durable authority, which this module does not supply. Public C
representations remain forgeable; callers must not write live control fields.

All nonnull object pointers denote correctly aligned, live objects of their
declared types. Inputs, outputs and controls occupy distinct storage; outputs
are exclusively writable for the call. Output descriptors may be uninitialized
only when used strictly as outputs. Init/move destinations own no initialized
mutex or allocation. A valid initialized pool may be closed, but no operation
except a fresh init is allowed after successful destroy. Never assign, memcpy,
reinitialize or relocate a live pthread mutex. Handle copies are allowed.

Allocation callbacks return either NULL or unique storage sufficiently sized
and aligned for every ordinary C object type used here. They are matching,
normal-returning operations with live contexts, do not reenter any pool using
the same allocator or mutate its controls, and do not retain pool storage.
They may be called under the mutex. A failed allocation creates no ownership.
The allocator must itself support all simultaneous calls from distinct pools
that share it. Native tests use one pool, so its callbacks are serialized.

## Operations, publication and failure

`init` requires a positive identity and generation; ordinary generation starts
at1, while changed-condition fixtures use MAX-1. Allocate and initialize four
FREE slots, then initialize a fresh default mutex. Allocation failure returns
NOMEM with no owner. Mutex-init failure returns SYNC and frees the staged block.
Failed constructor storage has no usable pool state and can be retried. Exact
mutex-init resource failure is an unforced platform boundary, not a tested
allocator failure. Other operations require a successfully initialized object.

`acquire(value,out)` chooses the first FREE slot and publishes one LIVE handle;
if none exists, return FULL unchanged. `get` and `release` validate identity,
index<4, LIVE and exact generation in that order before accessing a slot.
Invalid handles return STALE; no stale pointer is dereferenced. Successful get
copies the payload. Release clears the payload and increments generation; at
MAX it permanently retires the slot. Capacity therefore can fall below4.
No operation wraps generation, revives a retired slot or silently evicts a LIVE
slot. `snapshot` copies the identity and all four slots under the mutex.

`compact` allocates a replacement block while holding the mutex, copies all
four slot values/states/generations, publishes the new block and deallocates
the old one. Allocation refusal preserves the old block, all handles and all
payloads. This is actual relocation of owned storage; its identity survives.
No client can retain an address into that block. The test allocator keeps live
allocation counts and checks failed compaction leaves exactly one block.

`move(destination,source)` is a different operation: every source caller and
waiter must already be joined or otherwise quiescent, and destination storage
must be exclusive and unowned. Create a fresh destination mutex, transfer the
block, identity and callback ownership, and leave the source closed with its
original mutex still alive. Existing handles work with the destination. The
old source returns CLOSED until explicitly destroyed. A failed fresh mutex
init leaves source ownership unchanged and creates no destination pool.

`close` runs under the mutex, frees the block once and publishes CLOSED. It is
idempotent and may compete with admitted get/release/compact calls. A call that
wins the mutex first may succeed; a later one observes CLOSED without changing
its outputs. The shell and mutex must remain live throughout that competition.
Only after every caller/waiter is joined may `destroy` destroy the closed mutex.
Destroying an open pool returns STATE. Destroy does not infer quiescence from
an unlocked mutex, and cannot diagnose a violation of the caller's lifetime
precondition. `move` and `destroy` must not race other calls.

Null required arguments, zero constructor identity/generation and self-move
return INVALID. For admitted non-constructor failures, outputs and owned state
remain unchanged except documented idempotent close. SYNC represents an actual
pthread lock/init/destroy failure. A successfully locked valid default mutex is
unlocked by its owning thread; its normal successful unlock is a platform
premise, not a fallible transaction after publication. There is no cancellation,
signal-handler, robust-owner-death or asynchronous callback support here.

## Required construction and falsification

Submit the source, state transition derivation, identity/lifetime assumptions,
linearization points and exact cleanup ledger. The model compares the original
pool as well as the successor; do not replace both by the same implementation.
Concrete checks include cross-pool rejection, release/reacquire rejection,
MAX-1→MAX→RETIRED, all-retired FULL, allocator failure, payload compaction,
old-shell closure and handle survival at a newly initialized destination mutex.

Run three real pthread readers performing10000 copied reads each while a writer
releases the original handle, makes1000 new incarnations, compacts100 times and
closes. The original handle can return123 before release, STALE afterward or
CLOSED after close; it must never return a later incarnation's value, and
refusals preserve the sentinel987. Join all threads before destroy. Separately,
use the supplied fully atomic handshake to obtain999 from a later incarnation
after a reader already checked generation1. This is a defined identity failure
in original teaching code, not a data race, memory exploit or proof that atomic
generation checks protect lifetime. No external targets or data are involved.

Run GCC and Clang debug, optimized, address/undefined-behavior instrumentation
and thread instrumentation profiles. Reject ten compiled controls with ordinary
ADO_CHECK assertions, not build failure or sanitizer termination. The controls
independently omit identity or generation checks, wrap/excess-increment a
generation, change identity during move, corrupt compaction, publish on get
refusal, leak on close, and omit each retained pool check. Compiling learner
stubs must fail all five public anchors. The finite Python occupancy models
enumerate short traces and seeded longer traces; actual native observations
remain separate from sequential enumeration and are not all-interleaving proofs.

Compile wrong-category C and Rust marker probes and their positive controls.
Explain why distinct C structures require a diagnostic for implicit mismatch,
yet copying or constructing their public fields is legal. Rust PhantomData
can affect typing/lifetime/drop checking without storing a runtime marker, but
using a named marker type is not generative branding for every runtime pool.
The probe does not implement a Rust pool or establish a concurrency theorem.

## Assessment and changed conditions

R1 requires all four dimensions: nominal category, runtime pool identity, slot
incarnation and resource ownership. R2 requires the actual retained fixed-address
transition model and its no-address-reuse premise. R3 requires independent model
comparisons, wrong-category diagnostics, public-representation counterexamples
and independently killed original controls. R4 requires actual block and shell
relocation, failure preservation and the concurrent close/join lifetime protocol.
Passing public tests alone remains level1. Level2 requires justified admitted
states and all required artifacts. Level3 requires an unseen changed condition,
the invalidated old argument and a corrected design, with assistance recorded.

For X-T67-005-DELAYED, withhold prior code and change the initial generation to
MAX-1, force the next compaction allocation to fail, move the shell after readers
join, then restart readers using old numeric handles on the new shell. Have the
learner explain which calls need quiescence and which merely need the mutex.
Also change the ID allocator to restart at1 and require a concrete old-handle
identity collision explanation; adding a mutex does not solve restart identity.

Hints: (1) draw separate ownership arrows for shell/mutex/block/handle; (2) first
make a copied get linearize inside one lock; (3) transfer the block without
copying the mutex, and count the old shell's remaining destruction obligation.
Independent delayed learner assessment and non-author review remain unobserved.

## Cost and enforcement review

| Dimension | Required evidence in this module |
| --- | --- |
| Representation | Public pool/handle/snapshot sizes; valid states and positive IDs/generations; ABI padding is implementation-specific. |
| Reachable storage | One four-slot block plus the shell/mutex; two blocks during successful compaction. The mutex's internal storage cost is not assumed zero. |
| Lifetime | Constructor, retirement, close, quiescent shell move and joined destruction; explain the old shell's remaining mutex obligation. |
| Mutation/aliasing | Copied reads under lock, disjoint outputs, exclusive destination, no raw slot pointer; real competing readers and a defined check-then-read counterexample. |
| Allocation | Exact failed-init/failed-compaction and allocate/free counts; allocator callback context/nonreentrancy premises. |
| Movement/copying | Actual four-slot compaction, fresh destination mutex, preserved numeric handles, closed old shell; no mutex copying. |
| Destruction | At most one block free at close; allocator and mutex scheduling latency are not bounded by this C model. |
| Dispatch | Direct API calls, indirect allocation callbacks and actual mutex symbols in disassembly/dependency observations. |
| Concurrency | Native copied-read/release/compact/close histories, mutex linearization, and join before destroy; no lock-free or all-history claim. |
| Failure | Unchanged output/state at admission and allocation refusal, retired capacity, constructor cleanup and explicit unforced mutex-resource failure. |
| Compilation | Recorded clean builds and an actual unchanged Ninja rebuild; diagnostic negative/positive controls. Environment/cache costs are recorded, not generalized. |
| Binary/stack | Linked sizes, object disassembly, unresolved symbols and compiler stack-usage records; no portable exact stack bound. |
| Boundary | C17 hosted/POSIX profile, fixed versus numeric identity, no durable ID authority, no raw borrow serialization or physical target claim. |

For latency, retain seven100000-get samples after documented initialization;
no warmup is removed from these raw samples. Both paths return123 in serialized
use, but the baseline has no concurrent safety contract. Report that difference
beside the values; these are not universal overhead or equivalent-concurrency
claims. Each operation is externally compiled without link-time optimization.
First-free acquisition examines at most4 slots; compaction copies4. Allocation,
lock scheduling and system noise dominate boundaries outside that finite work.
Label runtime checks, platform contracts, diagnostics and caller discipline
separately; ISO C provides neither linear use nor generative/runtime identity.
Validation receipts distinguish authored, built and actually executed profiles.
No hardware behavior, published C17/C23 text inspection, durable ID authority,
non-author review or learner mastery is inferred. POSIX.1-2017 destruction and
alternative-representation text was inspected through an identified reprint;
the official current endpoint was inaccessible. The glibc2.42 lock documentation
was inspected, but its empty init/destroy entries are not normative evidence.
