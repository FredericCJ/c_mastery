# T74 original managed-runtime laboratory contract

This contract, the public header and each X-T74 brief jointly specify the work.
The kernels are original teaching implementations. They do not embed V8, Lua,
CPython or a collector library. Host CTest, Python models and sanitizer runtimes
are verification dependencies. The target core uses only C17 arithmetic,
arrays, structs and pointers, with exact-width types and uintptr_t required by
this profile. Optional types are a documented implementation prerequisite.

All calls are serialized by one owner. Inputs name live, initialized C objects;
output/control objects do not overlap each other or input storage. No callback,
signal handler or second thread mutates a control object during an operation.
Pointers and controls may not outlive their backing storage. Public structs are
inspectable for teaching, but callers must preserve the stated representation
invariants. Range and state checks are not arbitrary-memory-corruption recovery.
NULL required arguments return INVALID, except the void initializer which simply
does nothing. Unspecified padding is never an observable result; byte snapshots
in tests compare unchanged objects, not independently constructed struct padding.

OR_OK=0, INVALID=1, FULL=2, STATE=3, DONE=4, MISS=5. Rejection preserves outputs
and state except property lookup's diagnostic cache counters/state on MISS.
Read each mechanism's explicit partial-progress exception. There is no allocator
in the core; fixed storage exhaustion and injected admission failure stand in
for allocator failure. General allocator-backed variants remain required.

## Value: pointer tags and NaN payload partitioning

`or_number_bits` takes an IEEE binary64 *encoding as uint64*, not a C double.
Any exponent-2047 nonzero-fraction encoding becomes 0x7ff8000000000000. Other
encodings, including signed zero, subnormals and infinities, remain bit-exact.
`or_integer` uses prefix 0x7ff90000 in the upper 32 bits and a modulo-2^32
int32 payload in the lower bits. `or_integer_get` rejects all other prefixes,
including nonzero reserved bits32..47, and decodes negatives arithmetically.
Canonicalizing numeric NaNs prevents them impersonating the integer tag.

`or_pointer_tag` accepts nonnull live void pointers whose uintptr_t conversion
has both low bits zero, tag0..3, and writes the integer representation OR tag.
`or_pointer_untag` requires a word produced by that operation for the same still
live pointer and matching tag. It clears the two bits before converting back.
Forged integer words and stale pointer provenance are outside its precondition;
the tag check is not authentication. Never dereference a tagged integer or
truncate a native pointer into the NaN payload. Actual pointer low bits are
checked; `_Alignas(16)` alone is not a universal assertion about integer mapping.

Required transfer: add a double bridge only after proving size, radix, precision,
exponent and actual representation/endianness assumptions, using character-byte
copy rather than aliasing casts. Compare an explicit discriminated union and a
handle payload representation with equal supported values. ISO C99 introduced
stdint optional exact-width/pointer integer types; C11 alignment facilities do
not grant spare pointer bits. C17 remains the active implementation baseline.
The inspected N1570 C11 draft clauses6.3.2.3 and7.20.1 bound the conversion claims;
exact published C17 and a source-verified C23 representation delta remain gaps.

## Intern: binary symbols with transactional admission

Zero-initialized ORIntern owns eight copied strings of length0..15, including
embedded NUL. `or_intern` returns IDs1..8 in first-distinct-insertion order.
Equal byte sequences and equal lengths share identity; same length alone is
insufficient. NULL+length0 is legal and performs no pointer arithmetic/access.
A hit succeeds even when fail is nonzero or table full. A new entry with fail
nonzero or eight existing entries returns FULL unchanged, including output.
No eviction or ID reuse occurs while the table lives. The linear scan anchor
has O(entries*length) comparison work and fixed copied storage. Build a hashed
alternative with deliberate collisions and atomic resizing failure for the
required comparison; using a ready-made intern/hash library is prohibited.
Lua's inspected short-string interning is an implementation example, not a
promise that every language or every Lua string has pointer identity equality.

## Shapes: immutable layout identities

`or_shapes_init` creates the sole empty shape0 in a16-shape arena. An ORObject
initializes to shape0 and four zero fields. Keys are nonzero uint8 names; an
object has at most four distinct keys. Every nonempty shape records its parent,
added key and ordered descriptor list. Descriptors and arena address remain
stable until all objects/caches are destroyed. IDs are never recycled.
`or_property_set` updates an existing key in place; a new key finds or creates
the transition `(old shape, key)`, then writes the next field and commits shape.
The same insertion order shares a shape. Different insertion order may have
the same property set but a different layout. Exhaustion preserves both arena
and object. Four-field objects can still update existing properties at full
capacity. Deletion, prototypes and dictionary mode require explicit variants.

## Cache: guarded monomorphic and polymorphic property lookup

Zero-initialize ORCache. It retains a borrowed ORShapes owner, two `(shape,key,
offset)` entries, count0..2, sticky mega flag and uint32 diagnostic counters.
Each cache lifetime admits at most UINT32_MAX hits and misses; clients reset
before wrap if they need exact accounting. A different arena resets the cache.
Hits require *both* shape and key and read the current receiver field, not a
cached value. A miss performs descriptor lookup. Successful misses fill two
entries, then a third distinct pair switches to generic lookup. Missing keys
return MISS without changing payload output; misses increment and a foreign
owner may reset. Cache count is bounded; published entries are valid offsets.
No invalidation is needed only because these shape IDs/descriptors never mutate
or recycle. Prototype, deletion and shape retirement extensions must add it.

## Mark: incremental tri-color invariant with two write barriers

ORGraph has16 identity slots, live and root bitmaps, two strong edges per live
node (0=null,1..16 live identity), color0 white/1 gray/2 black and active flag.
`or_mark_begin` validates live edges/roots, refuses an active cycle, whitens all
nodes and grays roots. `or_mark_step` scans up to budget gray nodes, each with
two edges, in ascending identity order; budget0 scans none. It shades white
children and blackens the parent. Return OK while gray remains, DONE and clear
active when none remain. `scanned` reports nodes, not a hardware time bound.

Between steps, `or_graph_store` shades a white target before installing an edge
from a black source in an active cycle; `or_graph_root` shades an added white
root. Root deletion and edge deletion may retain floating garbage this cycle.
There is no sweeping operation between incremental steps. The DONE result is a
marking snapshot and does not authorize later mutation followed by an old sweep;
the serialized caller must sweep immediately or restart/reestablish marking.
The bound is O(V^2+E) with the deliberate gray-array scan, O(V+E) storage. An
explicit gray worklist is a required alternative, including capacity proof.
This incremental single-mutator mechanism provides no concurrent memory-order
guarantee. No allocation is allowed during a cycle in this anchor.

## Nursery: actual semispace copying and second-survival promotion

ORNursery owns two8-cell young arrays and one8-cell old array. active0/1 selects
from-space; used and old_used bound populated prefixes. A reference is0=null,
1..8 a current young index+1,9..16 old index+9. Roots[4] and both fields of every
populated cell contain valid references. Each cell stores int32 payload and
age0 (new) or1 (survived once). Old cells are retained by minor collection even
if otherwise unreachable; full old collection is a separate required variant.

`or_minor(heap, young_capacity, old_capacity, copied)` accepts capacities0..8
with old_capacity>=old_used. It evacuates roots, then old-to-young fields, then
each newly copied object's fields. A forwarding table preserves sharing and
cycles. Age0 survivors go to the other semispace with age1; age1 survivors append
to old. A preexisting old object is not copied, but its young references change.
All roots and fields are rewritten before publishing the new active space.
Unreachable young cells disappear. `copied` counts unique surviving young
objects, including promotions. Insufficient destination capacity returns FULL
with the entire original heap and output unchanged. The reference uses a
bounded whole-state transaction copy; account for its stack/storage cost.
No raw pointer into a young cell may survive collection. Persistent identity is
the payload only in the synthetic fixture; a real adapter must provide handles
or relocated roots. Required variants remove the whole-state transaction copy,
introduce a preflight/admission proof, and integrate the card mechanism.

## Cards: conservative old-to-young remembered roots

ORCards models eight old objects with two fields and four cards (two objects
per card). Targets0..16 use the nursery encoding above. `or_card_store` sets the
object's card if the target is young1..8. Replacing a young reference does not
clear the card; another field on the same card may still point young.
`or_card_scan` visits four fields for each dirty card, unions young target bits
into output, and retains exactly cards still containing young references.
Clean cards are unvisited. A false-positive dirty card costs work but remains
safe; a false-negative loses a root. No writer runs during scanning/clearing.
The paired nursery anchor intentionally scans all old fields as an independent
correctness reference; integrating cards must preserve the same surviving graph.

## Compact: forwarding physical cells through stable handles

ORCompact has16 physical ORPayload cells, a live-slot bitmap,16 handle entries
and outstanding-borrow count. Handle0..15 entries store0 absent or physical
slot+1. Multiple handles may alias one cell. Payload edges name handle IDs1..16,
never physical slots, and must resolve to live cells. `or_compact` refuses any
outstanding raw borrow, then stably packs live cells into the lowest slots using
scratch state, updates *all* handles and returns old-slot→new-slot+1 forwarding
(0 for dead slots). Handle identity and aliasing stay unchanged. There is no
handle issuance/reuse here; integrate generation retirement from memory.pool
before adding it. Forwarding/output storage is distinct from the heap. Pinning
and fragmented placement require an explicit new contract, not ignoring borrows.

## Weak: ephemeron fixed point and finalizer queue

ORWeak embeds a quiescent valid ORGraph, four ephemeron(key,value) pairs, four
weak references, and finalizable/finalized/pending bitmaps within live. Zero in
either pair field disables that pair. Strong root/edge closure is extended by
values whose keys are reachable, repeatedly until no new object appears.
A value's own path back to its key cannot bootstrap this process.

`or_weak_collect` rejects a nonempty pending queue (STATE). It clears weak
references absent from the ordinary/ephemeron closure *before* finalizer
retention. Unreachable finalizable, not-yet-finalized nodes become pending;
their strong closure is retained for this cycle. Other unreachable nodes are
reclaimed; pairs naming reclaimed nodes are cleared. The reported bitmap is
exactly reclaimed identities. `or_final_next` dequeues ascending identity,
marks it finalized before returning it, and leaves output unchanged on DONE.
The caller can run a synthetic finalizer and add a root to resurrect the object;
without resurrection a later collection reclaims it. Each identity finalizes
at most once; this is our policy, not Lua's re-marking behavior. Finalizers do
not run inside collection and cannot reenter collection with pending work.
They must finish before the next collection; external-resource failure is a
separate deterministic owner cleanup obligation, never guaranteed by GC.

## Trial: candidate cycle collection without corrupting true counts

ORCounts represents16 live objects, two counted edges each (parallel edges
count twice), and uint16 true reference counts including external references.
Counts must be at least total incoming modeled edges; no increment API or
concurrent mutation occurs during trial. Candidates is a subset of live.
`or_trial_delete` copies true counts, subtracts only incoming references from
candidate objects, seeds candidates with residual positive counts, then traces
all candidate descendants of those seeds. Candidates not reached are garbage.
Before removing that set, decrement its outgoing counts to surviving objects;
clear its edges/counts and live bits. Survivors with count0 are not recursively
reclaimed by this bounded operation; ordinary RC cascading is a separate
required integration. True survivor counts are not replaced by trial counts.
An outside-candidate reference conservatively protects its target even if that
outside object is itself garbage. The CPython comparison concerns this reachability
calculation, not its production list layout or free-threaded implementation.

## Safepoint: precise maps and derived-reference relocation

ORFrame has eight slots, initialized bitmap, exact root_map and at_safepoint.
Initialized slot tag0 is scalar uint32; tag1 is a reference `(base identity1..16,
offset)`. Uninitialized slots are not inspected, and root_map must be a subset
of initialized with a bit iff each initialized slot has tag1. An equal scalar
bit pattern must not become a root. At a safepoint, forwarding[16] supplies
nonzero destination identities1..16 for each mapped base; lengths[16] gives
pre-move payload extents. Offset<=length permits a one-past derived reference,
which may be represented but never dereferenced. `or_safepoint` validates every
slot and updates every copy of each mapped base transactionally, preserving
offsets and scalar slots. Wrong map/invalid offset/missing forward reject
unchanged; outside a safepoint returns STATE. Forwarding consistency/injectivity
is the collector's precondition, not derived from machine addresses.
This is an explicit interpreter frame map, not an assertion that the C compiler
finds/spills hidden copies. LLVM statepoints require compiler cooperation;
implement and inspect a compiler/IR experiment separately before that claim.

## Security containment and counterexamples

The only targets are these original in-process programs with synthetic integer
payloads. An adversary may choose admitted byte strings, property names/order,
graphs, root changes and interleavings between serialized collector steps. They
cannot corrupt arbitrary host memory or invent valid C pointers. Tests expose
a wrong scalar from a stale cache, a lost synthetic live object, premature
resource-release event, or incorrect relocation identity as harmless success.
Mutants intentionally omit semantic safeguards; they are separately built and
must be rejected by named assertions with exit1, not a compile error or a crash.
Demonstrate prerequisites: mutable graphs, cache reuse across layouts, or an
unreported root must actually occur. Repair the relevant guard/barrier/map and
preserve regression traces; sanitizers alone cannot observe logical reachability.
Do not claim code execution, privilege escalation or attacks on existing VMs.
No production service, real credentials, external target, host-policy change or
board is involved. The residual trust is correct layout, root enumeration,
allocator/handle lifetime and serialized mutation, which these kernels cannot
enforce against arbitrary C callers.

## Build and evidence boundaries

GCC/Clang Debug, Release and ASan+UBSan host campaigns run all11 anchors and22
compiled controls. The Python oracle constructs independent sets, dictionaries
and graph reachability rather than reading expected results from C output.
No thread runs in the core, so TSan is not evidence of a concurrent collector;
the required concurrent variant needs its own synchronization design and tests.
The separate Cortex-M4 freestanding audit links the core with the explicit
byte-loop helper runtime from protocols/targets/runtime.c and inspects undefined
symbols. No malloc, libc mechanism implementation or implicit helper is silently
accepted. Linking is not target execution, timing or physical validation.
Source-bound receipts record actual commands; no finite campaign proves absolute
mastery. Alternatives, real imports, independent assessment, delayed transfer
and resource measurements remain separate required deliverables.
