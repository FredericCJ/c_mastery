# Runtime representation, weak identity and foreign borrows

This required extension implements the changed conditions in X-T74-001,
X-T74-002 and X-T74-008. It shares their objectives, rubrics and delayed IDs.
It does not replace the eleven original kernels. Implement the functions in
`include/object_runtime_ext.h` through `src/object_runtime_ext.c`; the supplied
CTest cases and host Python oracle are verification artifacts. Separate assessor
implementations are excluded from learner packages.

The first stage is `ext_values`, requiring X-T74-001's arithmetic/representation
knowledge. Complete the typed moving pool before weak interning and foreign
borrows. Weak interning consumes the actual typed allocation callbacks from
X-T66-009 (`ownership.allocator`); foreign borrowing consumes the actual
X-T66-008 `OwPin` operations. Supply an accepted ownership learner workspace as
`CMASTERY_OWNERSHIP_ROOT`, with its existing include/import layout. Assessor
dependency selection is explicit through `CMASTERY_OWNERSHIP_ASSESSOR_ROOT`.
The original linear intern kernel is the semantic comparison for copied binary
strings. No hash, intern, pool, moving-collector or ownership library may replace
the mechanism. The real pthread worker is a hosted verification backend, not a
freestanding target dependency. The target audit of the old allocation-free
core does not validate this extension.

All controls are initialized, live and stationary; outputs and inputs are
distinct storage. One controller serializes heap, intern and lease operations.
Foreign workers only read the pinned byte span. They do not inspect or mutate
heap metadata. Callback allocators do not reenter or retain temporary arguments.
Public structures are inspectable, but callers may not fabricate their states.
Initialization requires a fresh object owning no resources; reinitializing an
active heap/table is forbidden. All failures preserve outputs and control state;
allocator diagnostic attempts may advance and privately acquired storage may be
released during rollback. No API checks arbitrary pointer validity.

## Inert values and a separately admitted native bridge

`ORXValue` is a discriminated union. Tag1 selects int32, tag2 selects a binary32
encoding held as uint32, and tag3 selects a two-byte logical handle. Each tag
requires its corresponding initialized union member. The six-octet wire layout
is tag, four big-endian payload octets, then generation. Tag1 uses modulo-2^32
encoding and arithmetic signed decoding. Tag2 canonicalizes every exponent255,
nonzero-fraction word to 0x7fc00000 while preserving both zeros, infinities and
other encodings. Tags1/2 require generation0. Tag3 requires payload1..8 and
generation1..3; its in-memory slot is payload minus one. All other tags/reserved
values fail unchanged. A decoded handle is inert data, not permission to read
an object and never a native pointer.

The binary32 path performs no floating arithmetic and requires neither `float`
nor pointer integer conversion. A capability-pointer implementation can retain
ordinary C pointers in trusted controls and use these logical wire handles;
the design does not assume spare pointer bits or a particular pointer width.
No capability architecture was executed. Compare the representation and memory
cost with the original NaN-boxed encoding on the same admitted values.

`orx_double_bits` and `orx_bits_double` are an explicitly narrower native
binary64 bridge. Require CHAR_BIT8, sizeof(double)8, radix2, precision53,
maximum exponent1024 and the target's documented IEEE binary64 object format
with ordinary little- or big-endian octet order. The implementation additionally
checks these constants and the actual representation of1.0; that finite probe
alone is not a proof of the format for every value. Mixed-endian, padded,
trapping or undocumented representations are unsupported. Access bytes only
through unsigned char lvalues; do not alias double with uint64 pointers. The
decoder canonicalizes all NaN encodings before constructing the double and
therefore never materializes the supplied signaling-NaN representation. Do not
enable fast-math assumptions in this experiment. Show signed zero, finite
values, infinities, subnormal boundaries and canonical NaN round trips; disclose
floating environment and compiler options.

## Typed pool, stable logical handles and pinned packing

`ORXHeap` owns eight genuine typed cells and eight independent identity entries.
Each cell holds0..15 copied bytes and two optional strong handle edges. NULL
with length0 is legal. Allocation chooses the first unused identity whose
generation is1..3 and the first free physical cell, with one external root.
Generation0 is invalid; reclamation increments to the next generation, and4
permanently retires the identity slot. Different live handles cannot alias a
physical cell. The null edge is `{0,0}`; other edges require a live generation.
An external copy of handle bits does not retain a root. Retain/release changes
the separate root count, rejecting saturation255 and underflow unchanged.

Collection starts from positive external root counts and outstanding foreign
pins, follows both strong edges, and reclaims precisely the complement. It
clears dead cells, retires their generations and increments the collector epoch.
An epoch starts at1; a collection at255 fails unchanged instead of wrapping.
Dead cycles are reclaimed; a live root protects its whole reachable graph.
No arbitrary finalization or allocation happens during this collection.

Packing preserves logical identities and payload/edge values. Pinned cells keep
their physical addresses, and the implementation must not even rewrite their
bytes: an actual foreign worker can read them concurrently. Visit other live
identity entries in slot order and place them into the earliest remaining
physical positions, staging before mutation. Report the number whose physical
position changed. This is packing around pins, not a contiguous-prefix promise.
Derive the holes caused by pinned positions and compare copying two-byte handles
with pinning larger payload storage. Read returns a copy, never a persistent raw
pointer. The original `ORCompact` remains the whole-heap-borrow-rejection
alternative and comparison point.

## Weak hash interning with real allocator failure

`ORXIntern` borrows its heap and a genuine `V4Allocator`. Buckets are allocated
int32 objects, never a character-array-to-struct cast. A bucket stores zero or
`generation*8 + slot + 1`. Hash starts at0 and folds each byte as
`(hash*33 + byte) mod256`; use linear probing and compare full length and bytes.
Empty strings and embedded NUL are ordinary keys. Deliberate collisions are
part of the supplied fixture, and are not equality evidence.

The initial table has no allocation. The first miss stages four buckets. A
miss exceeding half occupancy stages twice the capacity, up to16. Rehash uses
the live handles' current cell positions. A hit retains the existing symbol and
does not allocate, even when the next allocator attempt is configured to fail.
A miss privately acquires the replacement bucket array before pool admission;
if either acquisition fails, the old table/heap/output remain unchanged and any
private array is released. Publish the new table only after both are ready.
Explicit reserve admits only capacities4/8/16; requests no larger than current
capacity are no-ops. There is no shrinking policy.

The table itself does not root symbols. After any collection, get/reserve reject
until `orx_intern_sweep` removes stale entries and rebuilds probing in the same
allocation. Sweep updates the observed epoch and reports the removed count.
Packing alone does not change identity or epoch. A retained symbol survives
movement unchanged; an unrooted dead symbol is removed and reinterning the same
bytes gets a new generation, not its old identity. Closing releases only bucket
storage; caller roots remain theirs. The table must close before its allocator
or heap expires. Derive load-factor search cost, full-table bounds, staging peak
allocation and retained dead-entry space between collection and sweep.

## Foreign bytes: request cancellation, then acknowledge quiescence

Four central lease slots each have generations1..3, then retire4. Begin checks
the object and pin-count saturation, initializes/submits the actual T66 `OwPin`,
increments the object's pin count and publishes a ticket plus const byte span.
The ticket names the heap, lease slot and generation. It is safe to copy ticket
bits because completion validates central state; copying active heap/lease
controls is prohibited. The span must not be used beyond completion.

Cancel validates the controller identity and requests cancellation through
`ow_pin_cancel`. It does not revoke the raw pointer, decrement pins or establish
that the backend stopped. Finish requires an externally established backend
quiescence acknowledgement: in the supplied real pthread experiment this is
successful join. Finish invokes `ow_pin_complete`, drops the pin and retires or
advances the lease identity. A stale ticket cannot finish a later operation.
There is no forced thread cancellation, hostile callback containment or progress
guarantee for a backend that never returns. The API cannot infer quiescence from
the ticket; premature finish violates its precondition.

The original, separately compiled backend receives only bytes/length and a
mutex/condition handshake. Hold it after admission, release external roots,
collect, pack other cells, request cancellation, and collect again. Only after
the backend reads the original bytes and joins may finish release the pin.
Demonstrate a queued cancellation separately from a stale completion. Mutant
checks must clean up any started threads before their intended failing
assertion. No sanitizer diagnostic is used as a substitute for the ownership
assertion.

## Local authority demonstration and verification

After X-T74-008's pool is available, return to X-T74-001-R4 and its delayed
serialized-input variant. `orx_import` accepts a decoded tag3 only when a trusted
grant names both the same live heap and exact generation handle. Success retains
one root. A matching numeric handle from another heap grants nothing. The test
target consists solely of the original local teaching heap with synthetic byte
strings; attacker input is six wire octets and no trusted grant. The harmless
violation is an unauthorized increment of that target's root count. The two
intentionally flawed controls omit grant admission or arena identity. Repair
must reject both unchanged, alongside a permitted positive control. These are
cooperative in-process grants, not unforgeable capabilities against arbitrary
memory writes; no external service, credentials, deployed code or real data is
targeted. A decoded NaN word also remains a number encoding and can never select
the handle union arm or authorize a host pointer.

The Python oracle derives10,768 rows independently:10,000 intern operation
observations with real failure positions,256 physical packing masks, and512
binary32 special encodings. CTests add cycles, generations, saturation,
transaction rollback, native bridge and actual foreign-worker schedules.
Each named new anchor has two compiled semantic mutants and a compiling learner
stub that fails its intended assertion. Record exact compiler commands, standard
mode, sanitizer evidence and source pins. These finite observations are author
falsification support. They do not establish independent delayed retention,
arbitrary malformed memory safety, all thread interleavings or physical target
behavior. The other T74 investigations remain required.

Submit source, invariant/ownership proofs, failure-prefix tables, independent
oracle comparison, minimized mutant witnesses and the13-dimension cost review.
For the existing rubric, level1 only reproduces nominal identities; level2
derives why private rehash state and quiescent pins preserve the contract and
checks counterexamples; level3 independently adapts a changed capacity, wire
format or backend acknowledgement protocol and revisits every bound.

Hints: first draw separate identity, physical-position and root/pin tables;
then derive what information a serialized value lacks; finally enumerate the
private-acquire/publication/release frontier for a resizing miss. For pinning,
look for writes that appear harmless because they preserve byte values but
still conflict with a foreign reader. Full solutions are assessor-only.
