# Data layout and persistent structures: contract version1

This contract is incorporated by every T70 exercise. `include/dod.h` fixes the public names and representation limits. C17 is the active implementation edition. Target objects are live, initialized, exclusively owned, unmodified except through the API and not copied by struct assignment after initialization. Input/output/control objects are disjoint unless an operation explicitly permits sharing. Null required control/output pointers are rejected with D_INVALID; void initializers/reset and d_small_data require a live nonnull object. A null byte pointer is admitted only with zero length. Functions never validate arbitrary C pointer provenance or caller-forged representations. No thread safety, automatic destruction, borrow checking, reflection or compiler-enforced immutability is claimed.

All outputs and observable abstract state remain unchanged on rejection unless explicitly stated. Unused pool cells and instrumentation counters are outside the abstract-state guarantee. All indices are unsigned and validated before indexing. Host tests may use allocation, stdio and Python; the mechanisms may use integer operations, loops, typed objects and supplied allocator callbacks. No container, ECS, persistent-collection, rope or SIMD library may implement the assessed mechanism. Host allocator adapters are distinct from the dependency contract of a freestanding consumer. Before a freestanding claim, compile/link that consumer with its baseline restrictions and inspect symbols, relocations and compiler helpers; the hosted campaign does not establish this.

## Columns and SWAR

DColumns contains at most4 records in parallel typed arrays; count0 is valid. `d_columns_step` admits ticks0â€“10, computes mathematical x+velocity*ticks and rejects the entire update if any int32_t result is out of range. Cold values and velocities never change. Inputs outside that range are D_INVALID. Requires exact-width uint32_t/int32_t/int64_t; these are target availability requirements, not universal C guarantees.

`d_swar_add` adds four independently saturating unsigned8-bit lanes, where lane0 is bits0â€“7 of a uint32_t, independent of memory byte order. No intrinsics, vector extensions or scalar loop may be used in the submitted SWAR implementation. The scalar loop is permitted in the independently derived oracle. Lane overflow saturates at255; carries must not cross lanes. No constant-time security claim follows from branchless source alone.

## Byte-only small buffer

DSmall initially has inline capacity16, length0 and no heap. It stores counted bytes, including zero, with maximum length128; no terminator is implied. Append input must be disjoint from all storage owned by the receiver. It admits length first, grows by min(128,max(required,2*capacity)), copies before freeing old heap, and publishes only after allocation succeeds. Missing allocator callbacks on a required spill returns D_FULL; a callback returning null returns D_FAILED. A callback must return a fresh disjoint array of at least the requested bytes or null. The pair owns allocation/release, must remain live through reset, may not reenter/mutate the object, and may not retain released storage. The component cannot inspect whether the callback lied.

Truncate admits0<=new_length<=old_length and retains capacity. Compact succeeds only at length<=16 and returns heap ownership after copying live bytes inline. Move requires a distinct initialized empty destination with no heap; it transfers allocator/heap ownership or copies live inline bytes, leaving source in its initial empty state. Self move rejects. Reset releases heap exactly once, is idempotent, and invalidates all borrowed views. Append growth, compact and move can also invalidate views. Raw assignment is not this move protocol. No arbitrary T object is placed into the declared unsigned-char inline array; an aligned address alone would not establish the required effective type/lifetime rule. `dod.small_buffer` exports this byte-storage contract to the closure exercise, which must encode/decode captures or use concrete typed alternatives.

## Sparse stable slots and archetype rows

DSparse has4 concrete typed slots and a dense array of slot IDs; dense[rank[id]]==id for each live ID. Add uses the lowest available slot. Generations1â€“3 may be issued and removal increments generation; generation4 retires a slot permanently. A DHandle contains stable owner identity, slot and generation. Owner storage must outlive all handles, and resetting/reusing that address invalidates the protocol: generation checks cannot repair an ended owner lifetime. Removal swap-erases only dense IDs; typed slot addresses remain stable while live. Get copies value; a raw pointer retained beyond remove is not revoked by generation checks. Exhaustion returns D_FULL and stale handles return D_STALE without output modification.

DEcs has4 non-recycled entity IDs0â€“3 and3 tables with masks1(position),2(velocity),3(both), each capacity2. Nonexistent entities have mask0. Set creates or migrates to masks1â€“3, or updates an existing row in place. Supplied position/velocity initialize the corresponding typed arrays; fields absent from the mask are ignored semantically. Destination capacity is admitted before modifying either table. Source swap-erasure repairs the moved entity's row mapping. All prior row borrows are invalid after structural migration. Step visits only table3 and adds velocity to position, committing all or none on int range overflow. No deletion, cached query invalidation, parallel scheduler or hot reload is implied by this bounded anchor; the exercise specifies extensions explicitly.

## Persistent indexed tree and HAMT

DTree and DHamt are caller-owned typed arenas with64 usable nodes; node0 denotes empty. Roots must be0 or roots returned for that same live arena, not arbitrary internal-node indices or roots from another arena. Returned snapshots remain immutable until arena reset. Reclamation is region-wide only after every snapshot, zipper and reader is quiescent. There is no garbage collector, concurrent reclamation or reference-counting guarantee.

DTree represents8 integer cells, default0, by a depth3 binary trie. Each update clones exactly the selected leaf and3 ancestors, shares other subtrees and returns a new root. DHamt represents keys0â€“31, hash(key)=key&15, low2-bit chunk first, a second2-bit chunk, then an equality-checked collision bucket of at most2 keys. Internal bitmap bits select compact children in increasing slot order; count equals bitmap population. Update clones leaf+2 ancestors; key equality, not hash equality, decides replacement. Missing key is D_STALE. The model deliberately forces full-hash collisions k and k+16; it is not an adversarially robust general-purpose hash function.

For both pools, fail_after=0 disables injection; fail_after=k makes the kth allocation of an update fail. Each update resets its allocation instrumentation. Failed update restores used and leaves output root and all previously reachable nodes unchanged. Unpublished cells above used can be overwritten; their bytes and instrumentation are not rolled back. Node exhaustion returns D_FAILED. No user callbacks run during cloning. Costs must distinguish lookup depth, fresh nodes/update, retained versions and region reclamation latency.

## Rope DAG

DRope is a caller-owned64-node arena. Leaf copies0â€“8 bytes; empty is root0 and allocates nothing. Concatenation accepts same-arena roots, returns the nonempty input on empty concatenation and otherwise allocates one ordered concatenation node. Total expanded byte length is at most128. Sharing a root with itself is legal; cycles are impossible because new nodes refer only to older nodes. Snapshot roots retain their byte sequence. Flatten first admits output capacity, then writes the exact counted byte sequence with no terminator and sets written. Capacity rejection preserves both output bytes and written. Input/arena/output storage are disjoint. Region reset requires all users quiescent. This anchor does not balance, split, index Unicode scalars or reclaim individual nodes. Those are separately specified required learner investigations and transfer obligations; a passing concat test does not establish them.

## Required persistent ownership extensions

`dod_persistent.h` supplies X-T70-006/007. DZipper borrows one live DTree arena
and snapshot. Open focuses its root; down records the current node/direction in
one of three crumbs and follows a possibly empty child. Up restores a crumb.
Writing is valid only at depth3 and reconstructs a leaf and three parents,
sharing untouched siblings. It publishes only the new root, not a changed
zipper. Each failed clone restores the arena's used mark and output root;
allocation instrumentation records attempts. Do not reset the arena while a
zipper or root is live. The census accepts at most16 roots and reports unique
reachable nodes and nodes with multiple incoming root/edge references. It is
not the count of logical occurrences in every expanded version.

HAMT deletion uses the actual original arena/lookup/update contracts. Missing
keys return D_STALE. Delete copies only surviving changed nodes, removes empty
children from compact arrays and clears their bitmap bits. A node with no
children normalizes to root0. Deleting the only entry needs no new node.
Allocation failures restore used/output and preserve every published version.
The tests enumerate all1024 ordered key pairs including identical keys, full
hash collisions and different bitmap positions, then all three failure cuts
for a surviving collision bucket. Arbitrary hash dictionaries and constant-
hash buckets beyond the original two-entry bound require separate policy.

DRcTree is a separate caller-owned64-node typed pool of depth3 persistent trees
with owned byte-string leaves of length0..32. Root0 denotes eight empty values.
Every returned nonzero root owns one reference; each internal edge owns one.
Retain rejects above32 references before modifying state. A caller may not
forge or duplicate an owning root without retain, use an internal node as root,
or use a root after its final release. C integer representation does not enforce
those ownership rules. No concurrent mutation, cycles or weak references exist.

Clone returns1 and transfers one fresh disjoint allocation (including an empty
string), or0 and transfers nothing. Drop is nonfailing and exactly once per
successful clone. Callbacks/context remain live and cannot reenter, change
the pool, retain borrows or perform a nonlocal jump. An update clones the new
leaf text, allocates its node and three ancestors, and retains each shared
sibling. Allocation/clone/refcount failure releases the constructed prefix and
its retained edges, preserving old roots and output. fail_at injects the kth
node-allocation attempt of the update. Counters and free-node bytes are outside
abstract-state preservation. A borrowed get result lasts only while a retained
root keeps its leaf alive. Release decrements one owning reference and reclaims
zero-reference nodes recursively, with maximum depth4 in this model.

DRcRetire starts zero-initialized. Retire transfers one root reference into its
pending stack without increasing its count; the caller must cease owning that
root reference. Drain consumes at most budget pending references, returns
done=1 only after the queue is empty, and frees each zero-reference node once.
Budget0 is allowed and makes no progress. A separately retained root remains
readable throughout. No new node is created during draining; at depth4 the
explicit stack fits its65 entries. Deferred work retains resources and can
exhaust the pool. It cannot be discarded unfinished. A callback's latency is
not bounded by the number of node visits. All these APIs are single-threaded.

## Required text-editing extensions

`dod_text.h` extends X-T70-008. Byte index rejects index>=length unchanged.
Split admits0..length and returns distinct left/right output roots. It shares
whole untouched subtrees, splits an intersected leaf, and copies the path.
Capacity failure restores the arena mark and both outputs. Balance flattens
into128 temporary bytes and rebuilds a midpoint-balanced tree with leaves of
at most8 bytes; it preserves original snapshots, admits all node allocations
transactionally and may fail if the old versions leave too little arena space.
This is an explicit copying rebuild policy, not a claim to implement the rope
paper's incremental rebalancer. Its height is logarithmic in leaf count;
copy traffic and retained old nodes must be included in comparison.

DGap owns128 bytes with half-open gap[low,high). Logical content concatenates
bytes before/after the gap. Insert admits position0..length and disjoint counted
input; delete admits a range wholly inside the document. Admit range/capacity
before moving the gap. Empty insertion/deletion is valid. Movement copies in
the direction that preserves overlap. Flatten checks capacity before writing
bytes or count. Moving the gap costs distance moved, not necessarily document
length, while a remote edit can move nearly the entire document.

DPieceTable borrows immutable original bytes and an exclusively writable add
buffer, each at most128 bytes. These buffers and the stable table control are
disjoint and remain live while any snapshot refers to them. Insert input is
disjoint from every owned/borrowed buffer. Its32 pieces hold source/offset/length;
adjacent contiguous pieces of the same source coalesce. Existing added prefixes
never change. Stage the full descriptor list before appending input; exhaustion
of document size, add-buffer space or piece count rejects unchanged. Deletion
does not recover add-buffer capacity. Flatten visits pieces in logical order.
Views cannot outlive backing storage; public struct copying alone grants no
permission to reuse or modify another snapshot's bytes.

Compact writes a new immutable original into a separate caller-provided buffer
of at least current document length, initializes a distinct table with a new
separate add buffer, and leaves all old buffers/descriptors intact. The caller
must retain them until every old snapshot is quiescent. Insufficient supplied
memory rejects; no hidden allocation occurs. This explicitly trades additional
storage for snapshot preservation and does not promise in-place compaction.

`d_utf8_offset` validates the complete counted byte string and maps a scalar
index to a byte offset, allowing the index just after the last scalar. Malformed,
truncated, overlong, surrogate and above-U+10FFFF encodings reject unchanged;
a valid string with an excessive scalar index returns D_RANGE. Validation never
stops early merely because it has found the requested offset. Byte offset is
not a grapheme, normalization or line-break contract. To edit scalar ranges,
`d_gap_scalar_edit` and `d_piece_scalar_edit` validate both endpoints and the
complete inserted bytes, then transactionally replace the scalar range.
They preserve the entire document and backing append extent on invalid UTF-8,
range or capacity failure. Retain validated backing storage against mutation
for the whole operation; the single-owner precondition supplies this exclusion.
RFC3629 sections3/4 supply the inspected sequence constraints, not a complete
Unicode text-processing specification.

## Evidence and assessment

## Required layout, lane and identity extensions

`dod_ext.h` is required support for X-T70-001/003/004/005. The existing common
ownership/null/unchanged-output assumptions apply. `D_ROW_FIELDS` generates
both DRow members and DColumns8 arrays and is expanded again for field copying.
This prevents one form of duplicated field-list drift; it does not prove the
semantic correspondence of two manually written operations. Columns and rows
admit0..8 records. Row and column stepping share the original mathematical
int32 overflow/ticks0..10 contract, committing all positions or none. Conversion
does not allocate. Output row capacity is checked before writing any row.

DTiles uses three lanes per tile and at most three tiles, so the final tile is
partial for counts1,2,4,5,7,8. `has_cold` is0 or1. Missing cold columns decode to
zero, while present columns preserve values. The counted logical rows define
the state; unused lanes are not rows. Encode/decode with insufficient capacity
rejects before output modification. Submit the address equations i/3 and i%3,
prove the last valid lane, and explain what schema evolution must update beyond
the field list. The supplied small models do not establish a performance win;
warm/cold large-population measurement remains required evidence.

`d_rows_step_many` and `d_columns_step_many` admit0..1048576 rows under the same
ticks0..10 and full-update overflow contract. The column arrays are disjoint.
Both implementations first validate every mathematical result, then update
positions in a separate pass; the source must remain stable between passes.
They do not allocate or copy cold fields. `tests/measure.c` uses these actual
kernels and separate equal-result sequential/indexed whole-record consumers.
It shuffles run order, verifies checksums outside timing, and distinguishes
warm observations from64MiB eviction-conditioned observations. No cache state,
fixed CPU frequency or universal layout advantage is guaranteed. The compiler
reports and source-bound sample distributions are required interpretation
artifacts, not automatic mastery. Indexed whole-record access changes which
fields share fetched cache lines and can reverse a sequential-layout result.

`d_swar16_add` adds two unsigned16-bit logical lanes in a uint32_t, independently
saturating at65535. It requires exact-width uint32_t and no lane loop, vector
intrinsic or library arithmetic in the implementation. The independent scalar
oracle may loop. Reserved high-bit addition and low-part sum must reconstruct
the result without cross-lane carry; saturation depends on both original high
bits and low-part carry. Bit numbering is independent of memory byte order.
The public tests cover1296 endpoint products and65536 correlated lane pairs;
these are not an exhaustive proof of all2^64 operand combinations.

DSegment contains two real DSparse pages. All resources are nonnull borrowed
identities until successful add transfers exactly one destroy obligation.
The owner remains at a stable address and outlives handles and borrows. Add
chooses the first page with an issuable slot; permanently retired generations
do not become available. Capacity/retirement rejection leaves resource ownership
with the caller. Remove invalidates the handle before the exactly-once, no-fail,
nonreentrant destroy callback. Callback context and resources must be disjoint
from the owner. It cannot retain borrowed slot storage or perform a nonlocal
jump. All live handles must be removed before discarding the owner.

Get copies a value. Borrow returns the address of a typed slot member, valid
only until removal; no long-lived access is authorized by the API. The test
also exposes the C limitation safely: the slot object's lifetime continues
after logical removal, so its old address can observe a replacement value even
though the old handle fails. This does not authorize access to a destroyed
external resource. No concurrency or reclamation safety follows from a token.

DQueryWorld embeds the original DEcs and starts at version1. Every successful
set increments version, even an in-place value change. At UINT_MAX mutation
rejects before changing ECS state. DQuery starts zero-initialized and may cache
one owner/version/required/excluded tuple. Required and excluded masks must be
subsets of3 and disjoint. Query returns increasing live IDs whose mask contains
all required bits and no excluded bit; required0 is allowed, nonexistent
mask0 entities are excluded. Cached empty results are versioned too. No direct
mutation of embedded ECS or manual version reset is permitted. A cache may be
reused for another still-live owner, but owner lifetime must exceed its use.
Successful query replaces the whole cache; invalid inputs leave it unchanged.
Get is bounded by4 entity tests on a miss, constant on a hit. Versioning adds
invalidation cost and finite mutation lifetime; it is not an unbounded epoch.

For each extension submit its ownership/state argument, exact public oracle
outputs, both mutant diagnoses, changed-condition variant and thirteen-part
cost record. The assessor analyses remain separate. Existing objectives/R1â€“R5
assess these extensions explicitly; passing the new anchors supplies author
execution evidence, not independent learner performance.

Each anchor is paired with a compiled semantic mutant. Mutant rejection requires an intended DOD_CHECK assertion and exit1; a crash alone is insufficient. GCC/Clang and optimization/sanitizer profiles are reported separately. A test pass is bounded executable evidence, not acceptance of the whole written exercise or proof of mastery. Every objective needs its own rubric evidence, 13-dimension modern-cost analysis, changed-condition defense and a delayed attempt at least7 days after recorded study. Publication excludes assessor implementations, analyses and withheld variants.

## Fixed and zero-inline quota buffers

`dod_buffers.h` supplies DInline:16 actual unsigned-char elements plus length, zero initialization, append and truncate. Append admits a null source only at length0, rejects insufficient capacity before writing and never spills. Compare its capacity contract and actual owner footprint with DSmall; it is not a drop-in substitute for successful17-byte inputs.

DZero has no inline storage. It owns one exact-length allocation from a stable DQuota, whose four actual unsigned-char[128] rows provide typed backing. The shared **logical requested-byte quota** is128; the physical backing is512 bytes plus metadata and must be reported separately. Every nonzero append allocates a new exact combined length while the old allocation remains live, copies old and added bytes, then publishes and releases old. Total128 and null/zero rules follow DSmall; failed admission/injection leaves the owner and bytes unchanged. This strong guarantee can reject growth64 to128 because old64+new128 exceeds the simultaneous quota. Zero append is a no-op without allocation.

`d_zero_pair_append` requires distinct initialized owners in the same quota and disjoint stable inputs. It prevalidates both totals, stages both new allocations and publishes only after both succeed. Failure releases any staged allocation and preserves both old byte sequences/pointers. Allocation-attempt counters and peak observed quota are not rolled back. Input overlap with either owner is outside this contract. Move requires a distinct initialized empty destination and valid source; it transfers the source quota identity/storage and empties the source without allocating. Reset releases the current allocation once and is idempotent. All borrowers are invalidated by successful append, move or reset; no global alias tracking is supplied. Quota allocation attempts stop at SIZE_MAX; release count saturates while release still completes. The family remains single-owner/non-reentrant.

For two16-byte owners, appending16 to each stages32 then32 bytes: logical peak96, final64. Fail-at-first/fail-at-second preserve both16-byte owners; the latter temporarily reaches64 and releases the staged row. These are the required X-T70-002-DELAYED rollback and move mechanisms, with assessor source separate from learner stubs. The `dod.buffers` controls reject over-capacity fixed admission and premature first-buffer publication.

## Signed sixteen-bit saturating lanes

`d_swar16_signed` interprets each halfword of a uint32 word as the two's-complement encoding of a mathematical signed16 value and returns two independently saturated sums in the same encoding. It never requires an out-of-range signed cast or signed overflow. First add the low15-bit fields (which cannot carry out of a16-bit lane), then combine high-bit parity for a modular lane sum. Overflow occurs only when operand signs agree and the sum sign differs. Expand each overflow bit to its lane mask; choose0x7fff or0x8000 from the operand sign. No host-endian serialization claim follows from numeric lane placement.

The C oracle converts each lane arithmetically, sums in int32 and clamps; tests cover2401 edge combinations and100000 deterministic mixed-word pairs. A separate Python oracle checks all2401 exact outputs. `dod.swarsigned` controls omit saturation or allow cross-lane carry. The measurement producer additionally compares actual signed/unsigned kernels with scalar saturation under equal value contracts, keeping copy instrumentation out of timed binaries.
## Caller dictionary and bounded collision investigation

`dod_dictionary.h` extends X-T70-007 with a distinct persistent map. A `DHKey`
owns exactly two copied octets. `DHDictionary` supplies a borrowed context,
`uint32_t` hash and equality callbacks. Equality must be an equivalence relation;
equal keys must hash equally. Callbacks must terminate, preserve dictionary and
map state, never reenter the map, and retain no temporary key address. Their
context and semantics stay valid and unchanged until every root is discarded.
This is not verified by the C callback type. The teaching dictionary folds ASCII
A-Z only in the first byte and compares the second exactly; its normal hash
uses the canonical first byte in the high octet and the second in the low
octet. The constant-zero hash obeys the same semantic contract.

The region contains512 typed nodes plus unused sentinel0. A root is0 or a root
returned by this initialized map; arbitrary internal indices are not valid
roots. Root indices, context and map storage are borrowed, not transferable to
another map. No operation reclaims old nodes. Reinitialization invalidates all
roots and is permitted only after their users quiesce. There is no concurrency
or raw-pointer revocation guarantee. Caller outputs and metrics must be live,
distinct storage outside the map; `root` is passed by value so updating a caller
root variable in place is allowed. A null optional metrics pointer is allowed.

Sixteen two-bit levels consume all32 hash bits. Every internal bitmap describes
compact children in increasing logical-slot order. A terminal bucket owns up
to8 distinct logical keys and values. Equal-key replacement keeps the original
stored key representation. Children precede their parent in the arena, proving
acyclicity. Lookup publishes its value only on D_OK; absence is D_STALE and
preserves output. Update and deletion publish only a complete new root.
Deleting an absent key succeeds with the original root and no allocations;
empty nodes normalize to0. A full collision bucket rejects a new unequal key
with D_FULL, even if the region has spare nodes. It still accepts replacement.

Set `fail_at` to a one-based node allocation attempt within the next mutation
(zero disables injection). `attempts` resets at each valid mutation and counts
attempted node allocations. Region exhaustion or an injected failure returns
D_FAILED; output and `used` are unchanged, every prior node remains unchanged,
and bytes beyond `used` are unreachable scratch. Cost metrics record completed
work even on these failures: visited existing nodes, equality calls and
successful clones before rollback. Invalid control/output pointers or an index
beyond `used` return D_INVALID without publishing metrics; structural root
validity and callback semantics remain preconditions.

Successful insertion or replacement clones17 nodes. Deletion clones at most17;
deleting the only key along a branch may return0 without cloning. Hashing is
called once per valid operation. Lookup traverses at most17 existing nodes and
performs at most8 equality calls. An eighth constant-hash key remains usable;
the ninth distinct key is rejected after8 comparisons and no allocation. This
bounds work in the original local target, subject to terminating callbacks,
and exposes capacity denial rather than solving adversarial hash selection.
No networking, credentials or external targets participate. The observable
security-property failure in the equality mutant is overwrite of a different
synthetic key; the repair compares actual keys and preserves old roots.
Regression traces include canonical-equal/different-bytes pairs, unequal
same-hash pairs, all17 clone failures, ninth-key rejection and region exhaustion.
No arbitrary execution or constant-time security claim follows.

The reference and two semantic controls are separate from learner sources.
`dod.dictionary` checks these invariants; the independent Python canonical-map
oracle compares4096 version/query results and9 collision-work records. These
finite cases do not prove arbitrary callback correctness or independent mastery.
## Recycled entities and deferred command batches

`dod_entities.h` composes the actual `DSparse` identity pool and `DQueryWorld`
archetype/cache implementation. It does not substitute ID equality for handle
identity. Initialize stable world storage once; moving or reinitializing it
while handles, commands or query caches survive is forbidden. A handle borrows
that world, carries its slot and generation, and never authorizes access after
removal. Generations1..3 are issued and generation4 retires a slot. No operation
wraps generation or version. Version exhaustion rejects mutation with D_FULL.

Creation takes mask1..3 and two `int` component values. It stages both identity
allocation and table admission; no slot/generation is consumed if the table is
full. Output must be distinct live storage outside the world and is unchanged
on failure. Component bits1/2 mean position/velocity; unused stored values are
not part of the logical entity. A mask0 command deletes. Deletion repairs the
swapped archetype row, clears the entity mask, then removes the sparse handle.
The implementation's staging copy contains no self pointers; staged handles
are deliberately rebound only after validating the supplied original owner.

`d_entity_apply` accepts0..4 commands, a null array only at count0. The array
must be stable outside the world for the call. Commands execute in order on a
private world copy. A stale handle, invalid mask, version exhaustion or any
intermediate table-capacity rejection preserves the entire original logical
world, generations and version. Failure is not waived because a later command
would free capacity. Successful publication invalidates all old raw row borrows
and query iterators; unchanged generation-checked handles still identify live
entities. Collect commands while iterating and apply only after row borrows
have ended. Failed batches leave those borrows valid. Storage for4 commands and
one full staged world is a disclosed fixed cost, not zero overhead.

Initialize `DEntityQuery` to zero before first use. Its borrowed owner/cache
fields must not be forged. Required/excluded masks use the existing query
contract; successful query results enumerate live handles in ascending slot
order. A repeated query may reuse the match cache only at the same world
version. Copying the cache does not extend its owner lifetime. Query output
must not overlap the world. Callers must requery after a successful mutation;
retained handles are still checked at command application. These serialized
interfaces provide no concurrent mutation or reclamation permission.

`dod.entities` checks destination-full rollback after an earlier successful
staged migration, deletion/reuse, an old command rejected for a recycled slot,
owner mismatch, version exhaustion, all12 issuable slot/generation pairs and
deletion followed by a now-stale command within one rejected batch. The
independent dictionary oracle covers729 six-operation scripts /4374 prefixes.
Two compiled controls ignore generations or publish a successful prefix.
Use only the original local synthetic world; the harmless security observation
is a stale command changing a new entity in the faulty control. Repair requires
both owner/generation validation and atomic batch publication. No crash is
needed and no privilege-escalation claim follows.
## Layout conversion, structure measurements and rope statistics

`d_rows_to_arrays` and `d_arrays_to_rows` use the actual D_ROW_FIELDS schema for
all x/velocity/cold fields, count0..1048576. All input and output spans are
disjoint; each has at least count elements. Capacity<count returns D_FULL before
any write; excessive count returns D_INVALID. Null spans are allowed at count0.
There are no numerical conversions or allocations. The companion layout study
times both directions as well as equivalent field-update/whole-record workloads,
with poisoned destinations and post-timing independent checks. Conversion cost
must be charged when choosing a layout for a short-lived workload.

`d_rope_stats` accepts an initialized DRope and a valid root, writing disjoint
output only on success. It also rejects reachable nodes that violate older-node
edges, leaf bounds or cached lengths. It reports leaf depth0, unique reachable
nodes/leaves, logical byte length, distinct stored leaf bytes,8-byte leaf-slot
slack and allocated nodes unreachable from this root. The Fibonacci minimum is
F(depth+2), with F0=0/F1=1, saturated at129 because admitted strings have at most
128 bytes. Empty root0 is balanced with minimum0. The root-only balanced flag
does not assert that all descendants are balanced. The traversal memoizes shared
nodes for physical storage counts; logical sequence length still counts repeated
occurrences. The fixed arena has no external allocator fragmentation metric.

The companion node and piece-table traces preserve old roots/descriptors.
Snapshot compaction reuses the existing caller-owned `d_piece_compact` contract:
all new backing is disjoint from old backing and descriptors; old arrays remain
live and unchanged while any old descriptor refers to them. With a sixteen-byte
result and a48-byte future append area, exactly64 additional backing bytes are
supplied. A fifteen-byte snapshot destination fails without writes. New edits
do not affect old snapshots; only explicit caller retirement of every old
descriptor permits old backing to be overwritten. There is no automatic lease
tracking or reference-count reclamation in this borrowed piece-table API.

`structure_measure.c` is a host-side experiment, with its own runtime/stdio/time
dependencies. It never becomes a target dependency. It compares identical
two-entity result streams with an original component sparse-set baseline and
separately compares synchronous/budgeted destruction of the same reference-
counted tree. Source-statement copy observations exclude whole-world staging,
metadata and hardware traffic; raw timing omits observer hooks. The report
preserves all samples and limits. These measurements establish no universal
layout ranking, physical timing guarantee or independent learner completion.
## Insertion order over stable slots

`dod_ordered.h` supplies X-T70-004-DELAYED. `DOrdered` embeds the actual DSparse
pool and four previous/next index pairs, with sentinel4 and head/tail indices.
Mutations use only the adapter; callers may use `d_sparse_get` to validate a
returned handle but must not mutate the embedded pool directly. Owner storage
stays at one address until all handles/iterators/borrows are discarded. A new
entity appends to the linked tail even when it reuses a low-numbered slot.
Removal unlinks it and invokes actual generation advancement/retirement.
Unrelated removal never moves a surviving payload; dense iteration order in
DSparse is deliberately not the insertion order.

`d_order_begin` initializes a borrowed iterator. Next returns D_OK and has_value1
with a handle and a borrowed pointer into the live slot, or has_value0 at end
without changing handle/value outputs. Required controls/outputs must be live,
disjoint storage outside the owner/iterator except the intended iterator
mutation. A successful add/remove increments the owner version and invalidates
all iterators; next rejects a changed version with D_STALE and no outputs.
Version exhaustion returns D_FULL before any mutation. Payload pointers remain
borrowed only until their entity is removed or the owner is retired. Stable
addresses alone do not authorize reading a later occupant through an old borrow.

The declared cost counters record actual linked-field writes through a helper
and successful iteration steps. Each successful add/remove makes four linked-
field writes; DSparse's own metadata and payload stores are separately excluded.
Four slots and three issued generations bound lifetime linked writes by96.
Iteration visits exactly the live linked members, with one step per returned
value; its diagnostic counter saturates at UINT_MAX. Handle lookup checks the
same constant number of owner/slot/generation fields as DSparse. No allocator
is called. Pointer chasing, eight index fields plus head/tail/version, and
iterator invalidation are the additional representation costs. These operation
counts do not establish wall-clock speed against dense scanning.

`dod.ordered` and its independent ordered-list oracle cover deletion at every
position, recycled insertion at the tail, stable surviving payload addresses,
stale iterators, all twelve issuable entities, retirement, version exhaustion
and saturated diagnostic counters. Controls prepend new values or omit removal
invalidation. The local synthetic failure is a wrong sequence or a stale
iterator success, not an external exploit or a claim of concurrent safety.


## Exact history obligations and cross-representation editing

The original anchors now also link tests/histories.c. The small signed-column product enumerates counts0..4, every x/velocity assignment from{-1,0,1}, ticks{0,1,10}:22143 cases, plus both overflow directions at each of four failure positions. Every untouched velocity/cold/unselected element is checked. Byte-buffer tests compare every initial length0..128, append length0..129 and next-allocation failure choice:33540 cases. They check capacity/pointer preservation on rejection, actual malloc/free ownership, self/nonempty move, all compact lengths0..16 and maximum128. The8-bit lane checks add524288 selected-lane/neighbor-boundary pairs and100000 seeded mixed words to the existing exhaustive lane checks.

The sparse-history alphabet is precisely add(value0..3) or remove(the latest issued token for value0..3), with duplicate values permitted by DSparse. Enumerate every prefix through length7:2396744 transitions. A separate logical model tracks physical slot admission, generation, liveness, values and every issued token. After each action check all tokens, the entire live set, dense/rank in both directions and generation retirement. An unissued token is represented safely and must reject; the foreign-owner test uses a second live pool. This alphabet does not enumerate arbitrary invalid pointers or concurrent access. The independent Python dictionary oracle additionally checks every length3 script prefix.

The persistent-history alphabet updates index{0,3,7} with value{0,1,-1}. All prefixes through length5 produce66429 successful transitions. Before each success inject each of the four allocation cuts, for265716 rejections. Compare every retained root's eight values with independent copied arrays, published node fields with the prior state and independently traversed graph edges/reachability; logical allocation attempts need not roll back. All roots are retained in this experiment, so every committed node must remain reachable. Python list snapshots independently check6561 actual retained-version rows; sparse traces add1536 rows. Exact finite coverage does not establish correctness for arbitrary capacity or all C implementations.

`d_rope_splice` makes the original rope participate in the same insert/delete scripts as the gap buffer and piece table. Given a valid root, byte offset, removed count and disjoint counted input, replace that byte range and return a new root. Offset beyond length or removal beyond the suffix returns D_RANGE; result length above128 returns D_FULL. Empty replacement of an empty range returns the same root without allocating. It calls actual split twice, builds inserted leaves of at most8 bytes, and concatenates the surviving prefix/insertion/suffix. The operation retains old nodes and snapshots, and on any pool exhaustion resets only the unpublished arena suffix, leaving output and old roots unchanged. Input may not alias storage that allocation can overwrite. No arbitrary C object or Unicode grapheme semantics is implied.

The new root may share unaffected subgraphs. Work is bounded by split depth plus inserted bytes and metadata allocation; the64-node pool can exhaust even when logical result fits128. Tests run identical243 five-operation edit scripts against all three implementations and a flat independent sequence, checking every retained rope snapshot after every prefix. Exhaustion is injected by filling the node pool to each spare capacity0..16; two semantic mutants leak an unpublished suffix or drop an inserted byte. Full128, invalid offsets/counts, empty edits and whole deletion are separate boundaries. This copied midpoint insertion builder is not a claim to implement the original rope paper's forest rebalancing algorithm.
