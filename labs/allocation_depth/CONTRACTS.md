# T16 allocation follow-through contract, version 1

This is the required additive implementation contract for `PR-T16-01` through
`PR-T16-09`, including the historically elective buddy project. The 42 original
`AX-PR-T16-…` criteria and their original `-DELAYED` variants remain the objective,
rubric, hint and transfer identities. Their historical records and workload
estimates are unchanged. Completion requires the criterion-specific reasoning
and changed-condition artifact as well as the code below. A passing author test
is not an independent learner assessment.

## Prerequisites and target

Use the prerequisite artifacts named in each parent assessment. In particular,
derive the object-lifetime and checked-extent arguments from T03/T04 before
creating storage APIs; use T14 representation invariants for the free index and
T15 bounded loops for searches. `allocation.h` imports the actual retained
`algo_pool` declaration from `labs/algorithms/include/algo.h`. The assessor links
the unchanged `assessor/algorithms/algo.c` producer for pool composition. Learners
link their own previously accepted producer, or implement that prerequisite;
the assessor implementation is not supplied in a learner workspace.

C17 remains the execution baseline, with exact-width integers and 8-bit bytes
on the tested Linux x86-64 GCC/Clang targets. The policy is Profile C: implement
allocation, byte copying, placement, tags, lifetime admission and cleanup locally.
Do not delegate them to malloc/realloc, a container, or an allocator library.
The hosted verification harness alone may acquire one backing allocation,
create an anonymous mapping, create threads, time operations, and use mature
comparison tools. Those effects are outside the portable policy.

Core functions use bounded caller-owned objects and no operating-system APIs.
The strict image selects arena, heap, extent and buddy mechanisms; the imported
historical pool translation unit has a broader hosted surface and is deliberately
audited under its existing profile instead of being silently called strict.
Audit each selected object and the final image, including compiler-generated
copy/zero helpers. `tools/run_allocation_followthrough.py` defines exact profiles
and records commands, outputs and source hashes. Its assembler entry is a Linux
x86-64 process ABI adapter, not portable C startup or bare-board execution.

## Common representation and ownership rules

All required control/output pointers designate live, correctly aligned objects.
Input, output and allocator objects are disjoint unless an API explicitly permits
the same handle variable to receive a result. Initializers require quiescence:
no handles, views, callbacks, or other threads may still use the prior instance.
An owner stays at a stable address throughout its handles' lifetimes. An owner
comparison cannot rescue an indeterminate pointer after its C object has died.
Handles detect *logical* invalidation while the owner is live; they do not revoke
raw pointers or prove arbitrary pointers valid.

An aligned declared character array supplies byte storage only. Actual `T[]`
objects supply typed slots. Genuine allocated storage can acquire effective type
through the appropriate typed stores; the harness demonstrates this separately.
The mapping adapter uses the inspected Linux interface and byte accesses, with
no blanket ISO C claim for arbitrary firmware addresses. `struct al_arena` trusts the
caller's stated base-alignment guarantee; it cannot validate arbitrary pointers.
The unaligned-base variant supplies alignment1 and rejects an alignment8 request.

Zero-sized acquisition/resize is rejected without mutation; release is separate.
Bad argument, exhaustion, stale identity, corrupt heap, cleanup failure and
identity exhaustion are distinct statuses. Ordinary admission failures preserve
the complete allocator and output. `AL_CLEANUP` from arena reset/release or class
release reports completed reclamation with one or more failed destructors: it is
not rollback of cleanup side effects. No callback may reenter or mutate its
allocator, retain temporary pointers, throw a nonlocal jump, or concurrently use
its control. Initializers and copy callbacks clean their own partially acquired
external resources before returning failure. Destructors run once and remaining
destructors continue after an error.

## Arena, scopes and LIFO work: T16.01–T16.05

Implement `al_extent`, then `struct al_arena`: capacity is caller-selected, with at most
16 live allocation descriptors and 8 nested marks. `al_extent` checks nonzero
power-of-two alignment, computes padding by unsigned arithmetic, and compares
against remaining capacity before addition. Run the exact 24-byte input
`(3,1),(5,8),(10,4),(1,1)`. Return offsets0,8,failure,13, current requested9,
cursor14 and peak14. Snapshot the failed request. Reset preserves lifetime peak
and clears epoch peak; a new small request must not erase the prior peak.

Each allocation records previous cursor, requested extent, unique serial and
optional cleanup ownership. Marks record owner, unique serial, allocation count
and cursor. Only the current top mark can commit or roll back. Committing an
inner mark preserves its allocations; the outer rollback still owns their
cleanup. A LIFO release must identify the actual top allocation and cannot cross
the active mark's saved count. Invalid/foreign/out-of-order requests do nothing.
Every successful identity creation consumes a 32-bit serial; at its maximum,
creation retires while cleanup remains possible. Quiescent reinitialization is
an explicit new owner epoch, not permission to reuse old handles.

Deliver the pre-M0/A/B lifetime diagram, an escaped-handle rejection trace, and
the persistent-to-temporary borrow or promotion decision. Show a function return
that leaves the arena cursor unchanged by language cleanup. Explain why a
longjmp that skips manual cleanup needs a higher recovery owner or a control-flow
restriction. Use the supplied synthetic cleanup contexts to prove reverse order,
continued cleanup after failure, and exactly-once token disposal. Byte reclamation
alone does not release these resources. Cost is O(1) admission, O(16) handle
lookup, and O(number removed) cleanup; callbacks have separately bounded costs.

## Extents, tags and resize: T16.02, T16.07–T16.08, T16.11

`struct al_extent` is a logical-byte policy with at most16 sorted free intervals. Implement
first fit and best fit, with lowest offset breaking best-fit ties. For holes
`[0,12),[20,40)` and request10, minimum remainder8 absorbs the first hole's
2-byte tail. Test remainders7/8/9, every release order of three8-byte intervals,
overlap rejection and size arithmetic. This logical model has no hidden in-band
header and must not be confused with the tagged allocator below.

`struct al_heap` owns an actual 8-aligned byte array, capacity16..256 in multiples of8.
An allocated block has an 8-byte header, byte payload and 4-byte footer; total
block size is a multiple of8 and at least16. Header fields are little-endian
16-bit total and request, state0/1, reserved zero, markersA5/5A. Footer repeats
total and state and endsC3. Free blocks have request0. The external typed free
index has at most16 increasing offsets; the typed live table binds unique IDs
to allocated-block offsets. Expose bytes only. Requests1..244 and alignments1,2,4,8
are admitted when space permits. No raw header pointer casts are allowed.

Before following any stored extent, validate minimum, alignment, remaining region,
state, markers, matching footer and exact one-to-one live/free membership. The
checker reconstructs the physical partition independently of the free index.
Corruption fixtures operate on original copied objects, never arbitrary memory.
Immediate mode has no adjacent free blocks. Deferred mode permits them until
explicit collect, which restores maximal coalescence. Rebuild the index after
mutation. Admission is staged and committed only after all checks succeed.

Implement shrink, adjacent-hole growth and moving resize. Return a fresh logical
handle even for an in-place resize; all old raw borrows end before the call.
Preserve min(old,new) bytes. Failed destination acquisition or copy callback
preserves old owner, bytes, full metadata and output. Copy callbacks operate on
staged byte storage and roll back their own partial external resources. The
supplied callback acquires a synthetic token, fails after two copied bytes and
cleans that token. Raw byte migration excludes self/interior pointers; the
transfer must choose a documented relocation callback/offset representation or
retain that exclusion. A client descriptor can keep stable identity by updating
its underlying handle only after success; `struct al_stable` implements a three-entry
typed descriptor table with owner/index/generation handles and retirement at3.
The same stable handle survives successful moving resize, failed resize preserves
both table and heap, and release invalidates it. Lookup validates the table before
the underlying bounded heap lookup. The table and heap both remain at live stable
addresses; raw borrows must end before resize, and concurrent access needs an
external synchronization/quiescence contract.

Quantify requested payload, total block charge, retained256-byte region, free
total and largest free extent. Peaks include staged destination storage before
old release on a moving resize. Search counts count successful free-list searches
only, saturating at SIZE_MAX; prove bounds rather than interpreting absence of a failed-search update as
zero work. The validator scans at most16 blocks and16 live descriptors per block;
first/best search is O(free blocks), full snapshot storage is `sizeof(struct al_heap)`,
and copy work is O(retained payload). These costs belong in every comparison.

The explicit variants derive a 4-byte-header/payload5/alignment8/footer4 layout
in32 bytes, wider-offset minimums, and a footer-free allocated-block design.
The latter requires a previous-free indication and a sentinel boundary protocol;
it is a derivation assessment, not a claim that the baseline tags already omit
footers. Compare local logical reuse with ISO realloc lifetime changes using the
exact inspected N1570 and N3096 passages; preserve the published-C17 access gap.

## Pools, classes and buddy work: T16.06, T16.09–T16.10

`struct al_pool` composes the existing16-slot `algo_pool`, admitting only slots0..2.
Slots3..15 are retired. Its wrapper includes owner identity and retires each slot
after generation3, making exhaustion observable without billions of iterations.
Keep the producer unchanged. Demonstrate that the old raw numeric handle can
refer to a matching slot in another pool under its caller-selected-owner
contract, while the wrapper rejects that foreign owner. Audit the state bitmap,
free count, live values and generation together. Compare actual typed int32
slots with byte-only classes; neither grants arbitrary over-aligned typed storage.

`struct al_classes` has four byte slots in each size class8/16/32. Choose the smallest
class, initialize staged bytes, commit ownership only on success, destroy before
reuse, and reject stale/foreign handles. Required requests1,8,9,17 yield payload35,
charged64 and internal waste29. The embedded C arrays reserve all physical storage
for the owner's lifetime: the reported logical class backing224 is active cache
capacity, not an OS mapping or `sizeof(struct al_classes)` measurement. Report metadata
and reserved32-byte slot arrays separately. The four64-byte-slabs variant is a
separate explicit layout model, with one live8-byte slot per slab: payload32,
charged32, retained256. Compaction requires ending raw borrows and updating stable
indirection; merely freeing logical slots does not release an embedded C array.

`struct al_buddy` uses a15-node tree for a64-byte logical region and minimum8. States are
free, split, or allocated; side metadata holds unique IDs. It splits left first,
merges only whole free siblings, and checks hidden descendants are empty.
Implement the9-then8 trace and all subsets of eight8-byte releases. Bound a search
by15 node visits and release by4 path nodes; record metadata and rounded charge.
Derive offset XOR size for validated orders and distinguish owner-relative
address arithmetic from physical-base alignment. Multiple regions never merge
across owners. A bounded request queue can bound admission capacity; it supplies
no hard scheduling deadline without separate service, cache and interrupt bounds.

## Backends and lifetime architecture: T16.12–T16.13

Run identical offset policy on declared bytes, genuinely allocated storage and
an original anonymous private Linux mapping. Construct an `Object` only in the
actual typed array or allocated-storage path. Mapping initialization/teardown,
page alignment and possible system failure are host obligations. The dedicated
firmware variant requires a linker/ABI/board evidence dossier; cross compilation
does not establish RAM accessibility, cache/DMA coherence or physical timing.

The two-worker harness compares worker-private24-byte regions with a shared
metadata mutex. Keep region owners in the joining thread so worker termination
does not destroy a retained allocation. Show24 stranded bytes when one region is
full and the other empty; reject cross-owner operations. Shared allocation and
release run under the mutex, with actual acquisition timing. Do not claim that
protects payload accesses after unlock. Diagnose the isolated `race_target.c`
under ThreadSanitizer; repair ordinary payload access with its own lock or
immutable release/acquire publication and join before destruction.

For the use intervals in the original criterion, choose concrete sizes16,48,24,8
for tokens, AST, response and cache. Derive peak live80, then extend AST lifetime
to8 when a response retains its slice. Give the explicit promotion alternative.
For ten64-byte messages with processing concurrency2, compare128 live with640
retained by a region that never resets. A slow completed response needs bounded
outstanding admission, not just bounded active processing. Compare a long-lived8
object beside ten64-byte requests: one unreclaimed arena retains648; separate
owners can peak at72 under sequential requests. Explain supported acyclic
ownership and why arbitrary cross-region cycles need another reclamation policy.

## Verification, deliverables, rubric and hints

Submit the implementation, operation/state tables, all named ordinary and delayed
analyses, callback/effect ledger, exact source/target manifest, minimized failing
traces and raw resource/timing data. `arena_test.c`, `heap_test.c`, `extents_test.c`,
`pools_test.c`, `buddy_test.c` and `backends_test.c` are executable public anchors.
The independent Python interval oracle compares complete positions/accounting
after every operation; payload tests separately verify preservation and overlap.
Inject every acquisition ordinal in the eight-object construction and every
relevant migration/copy boundary. `minimize_corruption.py` deletes operations
against original correct/flawed checker builds; report its actual minimality
domain. Original mutants remove padding, allow non-top release, omit footer
validation, split a forbidden remainder, publish failed migration, omit owner
validation or merge a live buddy. Each must fail its intended assertion.

Run GCC and Clang debug, optimized and address/undefined sanitizers. Run the
actual thread adapter and isolated race control under ThreadSanitizer separately.
The benchmark keeps size histograms while changing release/order patterns and
reports nine raw batch samples per condition. It toggles an *additional* invariant
pass; mandatory internal safety checks remain in both. Timing includes init,
validation and harness control, not solely allocator instructions. No percentile
from nine batches is a hard tail-latency bound.

The original per-leaf rubrics are mandatory: level0 has no valid evidence;
level1 merely reproduces an anchor; level2 supplies the precise invariant,
boundary/failure test and evidence limit; level3 independently solves the exact
changed-condition task and defends changed costs and assumptions. Require every
objective explicitly, not an aggregate score that hides a missing mechanism.
Hints progress from drawing byte/owner intervals, to separating admission from
publication, to checking the first invariant violation in an independent trace.
Worked implementation and technical answers remain under `assessor/allocation_depth`.
The delayed assessment still requires its original minimum7 days and independent
artifact. Author execution cannot satisfy that learner gate.

## Security scope and evidence limits

Targets are only these original allocator objects and synthetic data in local
processes. The attacker can choose API requests, stale/foreign logical handles,
or corrupt a *copy* of supplied metadata; it has no arbitrary host-address
capability. A meaningful success condition is an incorrectly accepted stale
identity, overlap, or missed corrupted footer, not arbitrary code execution.
Repair validation before following tags, owner/generation checks and staged
commit; retain regression and changed-condition tests. Callers can still violate
the live-pointer, exclusive-access and callback contracts. Sanitizers cannot
make invalid external pointers safe or detect every logical borrow error.

The racy target is an intentional local undefined-behavior diagnostic, run only
with the named sanitizer in its isolated process; no conclusion depends on its
numeric output. No host security settings, external targets or hardware are
changed. Author receipts distinguish actual native execution, object/link audits,
finite models, unperformed external review, unassessed learners and absent
physical observation. A finite passing suite does not prove absolute mastery.
