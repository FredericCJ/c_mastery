# Owned resources and synchronized copy-on-write snapshots

This is required X-T66-007 support, including X-T66-007-DELAYED. Use hosted
ISO C17 with the original `struct ow_cow` and `struct ow_allocator` contracts. The resource
successor below is a first-principles representation change: `struct ow_cow`'s scalar
copy cannot express fallible owned-resource cloning. POSIX supplies only mutexes
and test threads; it does not implement COW or reclamation. All pointers and
contexts designate live, correctly typed disjoint objects. The store is fresh,
nonowning and externally exclusive at init, then noncopied and nonmovable until
destroy. A token is an ownership capability, not permission to mutate a public
field. Input resources and all callback contexts remain valid during calls.

## Representation and operations

Each allocated block owns three resources. Each resource has a distinct owned
`int32_t` allocation of length1..16, cloned from one supplied seed. The seed
remains caller-owned. A raw allocation callback supplies the properly aligned
block; each resource-clone callback acquires and populates its own allocation.
The actual harness uses `ow_allocator` for those typed resources, including
its finite8-block capacity. There is no byte-buffer-to-arbitrary-type cast.

A clone callback gets an empty destination. Success yields a valid independent
copy of the same length/content. Failure may leave a valid partial owner; even
that failed element must be dropped. Drop is nonfailing, accepts empty, empties
the owner and releases through the matching allocator. Callbacks must return
normally, never reenter or mutate the store, never retain borrowed inputs, and
follow the declared lock order. They run under the store mutex after init.
Cancellation, owner death, nonlocal exits and mutex-backend failure during an
admitted operation are outside this contract.

`ocr_init` stages the block and all three clones before publishing ready=1.
Clone failure rolls back in reverse resource order, including a failed partial
owner, and releases the raw block. The store remains nonowning and its other
fields are unchanged. If the mutex initialization itself fails, no resource
escapes; the mutex representation has the platform's failed-init semantics and
must not be treated as a live initialized mutex. Reinitializing a live store
would lose ownership and is prohibited.

The store owns one reference to `current`. Up to four loan slots own additional
references, each with an active bit and a monotonically increasing serial.
`ocr_snapshot` acquires the mutex, selects `current`, increments its count and
publishes `{owner,slot,serial}` before unlocking. This selection-and-retain is
one protected operation. A separate atomic increment after an unprotected raw
pointer load would not establish the control lifetime. FULL and DEAD preserve
the output. The final UINT_MAX serial is issued once and release retires the
slot permanently; cleanup remains available at the limit.

`ocr_read` validates owner, slot and serial under the same mutex and copies one
element to the output. Invalid index or stale token leaves the output unchanged.
No pointer or mutable borrow escapes. `ocr_release` consumes exactly that live
loan, clears its registry entry and drops the block reference. Copied stale
tokens are rejected while the store remains live, but C does not prevent the
first use of an illicit copy from consuming the original ownership. The caller
must preserve actual token ownership and store lifetime.

`ocr_write` changes a unique current block in place with no clone/allocation.
If any snapshot shares it, allocate a fresh block and clone all three resources
before changing the chosen fresh element. Only after complete success publish
the new current pointer and drop the store's old reference under the mutex.
Failure preserves the old current, every snapshot and all logical values;
allocator counters and callback effects remain observable. Invalid indices
do no work. Immutable old snapshots survive subsequent writes.

`ocr_close` stops new snapshots/writes and drops the current owner. Existing
snapshots still read and release. `ocr_destroy` requires all external callers
joined/quiescent; a live loan returns BUSY. It finally drops any current owner
and destroys the mutex. Zero loans alone does not prove no caller can enter.
No token may be used after successful store destruction.

## Concrete inputs and checks

Seed resources are `[1,2]`, `[3,4]`, `[5,6]`. Unique write resource0/element0=7
must keep clone_calls=3 from init. Acquire an old snapshot, then write9: old
reads7, new reads9, and clone_calls=6. Releasing the last old snapshot reclaims
its three resources and raw block. Test raw-block failure, typed allocation
failure at positions1..3, and callback failure after acquiring each position.
All refusal paths preserve both current and snapshot values and restore live
resource counts. Check four-slot fullness, stale generation, final serial,
close/read/release and joined destruction.

The original scalar anchor uses actual `ow_cow_clone/write/drop`, unique/no-new-
allocation, failed shared detach and UINT_MAX refusal. A separate defined,
single-threaded raw-alias demonstration writes44 through a retained scalar
backing pointer after sharing and observes the alias change. This intentionally
bypasses the COW API contract; it is not C undefined behavior or concurrent data
race evidence. It falsifies an overstated claim of language-enforced independence.

The independent Python oracle enumerates3,275legal clone/write/drop/failure
histories of up to5operations over3owners, comparing logical copied lists and
separate sharing/allocation accounting against the actual scalar producer.
Six C17 profiles execute six anchors, six compiled controls and compiling stubs.
Controls isolate shared mutation, omitted partial-owner cleanup, ignored serial,
post-close admission, and actual retained OW_MUTANT7/17 defects. Require the
intended `OW_CHECK`, not a diagnostic abort. Emergency process cleanup in the
test harness runs only on abnormal assertion exit; successful tests assert all
real drop/release counts and do not rely on it.

The concurrent variant starts two actual readers (400 snapshots each) and one
writer (500 attempts), while an additional old snapshot is retained. Read twice
around `sched_yield` through each token and require identical values. Writes may
return NOMEM when retained versions exhaust the real finite allocator. Require
all800reader snapshots and500writer outcomes accounted, no changed old value,
joined callers and exact final reclamation. Do not impose a scheduling-specific
success/refusal distribution. Separate GCC/Clang TSan runs test these observed
histories; they do not prove fairness or every interleaving.

## Reasoning, assessment and variants

D1/R1 explains logical value independence versus representation sharing. Rust
`Cow`'s borrowed-or-owned representation and `to_mut` comparison is bounded to
the registered inspected passages; this reference-counted snapshot store is not
an equivalent language mechanism. D2/R2 derives reference ownership, mutex
publication, partial-clone rollback and no escaped mutable borrows. D3/R3 supplies
the oracle, failure ledger, compiled controls, lowering/cost evidence and raw-
alias counterexample. D4/R4 and the delayed variant independently rebuild the
owned-resource and concurrent-reader change after the required delay, changing
the partial failure point or retained-version pressure before seeing feedback.

Hints: include the failing resource in the initialized frontier; select and
retain while holding one lock; close removes only the store's owner. The private
assessor analysis gives the derivation. Nominal outputs alone are superficial;
level2 requires invariants and exact rollback/reclamation evidence, and level3
requires unaided transfer plus a counterexample to the former assumptions.

Complete all13modern cost dimensions: unique writes O(1); detach O(sum resource
lengths) plus3callback costs and1raw block; reads/counters O(1), with mutex waiting
unbounded without a scheduling premise. Retained versions trade allocation and
memory traffic for immutable snapshots; finite allocator refusal is meaningful.
Record actual store/token sizes, stack, code/disassembly, imports, compile and
incremental-build time,7samples, allocation counts and callback/diagnostic costs.
Runtime serial/bounds checks cannot enforce C ownership, prevent forged fields,
or validate arbitrary pointers. This is hosted, not a strict freestanding image.

Only original local programs and synthetic resources are used. No external
targets, credentials or system security changes. Author reports distinguish
authored/executed support from independent human or learner evidence. Published
C17 remains inaccessible; no physical timing or absolute mastery claim follows.
