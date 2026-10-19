# T16 assessor analysis (withheld)

The original42 criteria and their delayed tasks are the grading authority.
`bindings.py` binds each separately to actual executable observations or an
explicitly identified derivation. Require learners to reproduce the reason under
changed assumptions; handing in these author answers is not independent evidence.

## Admission and storage legality

For power-of-two alignment, unsigned `(-cursor)&(alignment-1)` is the least
nonnegative padding. Subtract from remaining capacity in two admitted stages;
checking a wrapped `cursor+padding+bytes` after evaluating it loses the proof.
The finite exhaustive test enumerates capacities0..32/cursors/bytes1..33 and
alignments1..16 with independent repeated-increment rounding. The SIZE_MAX case
is arithmetic only and does not create an enormous object.

An offset multiple of8 does not imply an actual pointer is aligned8. Our arena
requires a true caller-proved base alignment and rejects stronger requests.
Declared byte storage remains declared byte storage, even when `_Alignas` holds.
The actual hosted allocated Object uses typed stores into malloc-supplied space;
the typed int32 pool uses real int32 members. Mapping-backed byte use rests on
the selected Linux contract; firmware requires its own linker/ABI/board evidence.

The synthetic4-byte-header layout begins payload at8, ends payload at13, footer
at17, and next block at24. Its7 trailing alignment bytes are part of charge.
With region32/minimum8, remainder8 is reusable; minimum16 absorbs it. A different
layout's two32-bit links plus tags needs16 bytes, versus24 for64-bit links under
the stated8-byte rounding. These examples are not the concrete ALHeap format.

## Lifetime and callbacks

An integer cursor alone cannot tell whether a mark belongs to this owner or to
the current nesting history. Unique serials plus strict top-mark identity make
rollback order explicit. Reused offsets do not revive handles. Retiring identity
creation at UINT32_MAX avoids wrap aliasing; a quiescent new instance requires
all old references to have ended. C does not invalidate raw byte pointers on a
logical rewind, so raw-view use requires discipline independent of handle checks.

Before M0 survives both rollbacks; A survives rollback M1 but not M0; B survives
neither. Committing M1 transfers its cleanup responsibility to the outer scope.
Callbacks cannot retain a borrowed B pointer for later use: promotion/ownership
transfer changes the contract and may fail before publication. C return and
longjmp concern language control flow, not a custom arena's cursor. Our API
excludes nonlocal exit through it; an explicit higher recovery owner is the
alternative. Cleanup failures do not justify skipping remaining resources or
repeating a destructor. The log2,3,1 and injected cleanup error demonstrate this.

## Placement, structural invariants and resizing

First fit chooses the earliest sufficient extent; best fit chooses the smallest,
then earliest. The exact first-fit fixture charges12, not10, because remainder2
cannot satisfy minimum8. Alternating free holes can have total24 and maximum8,
so request9 fails although aggregate space exceeds it. Coalescence depends on
adjacency in the *same* region, not just aggregate bytes or numerical addresses.

ALHeap validates the physical chain before every mutation. Header total is
checked against remaining capacity before the footer index is formed. Exact
matching free-index order catches duplicates even if every canary remains intact.
Exact live membership prevents a fabricated allocated tag from borrowing an
unrelated descriptor. Immediate normalization rejects adjacent free blocks;
deferred mode deliberately weakens that one invariant until collect.

Full metadata/byte snapshots implement strong failure behavior at a bounded
stack cost; they are not free. Moving resize temporarily owns both extents. The
copy callback receives staged bytes only, and rolls back external partial
resource effects itself. A returned error discards the snapshot, including a
new ID and index changes. In-place success still produces a fresh logical ID.
The actual three-entry stable descriptor table changes the underlying ID after
success while preserving its separate owner/index/generation identity. Failure
preserves table and heap; release increments/retire-generates its identity.
Raw interior pointers must already have ended. Byte copies do not repair
self-pointers; the contract explicitly excludes them absent a relocation layer.

Footer-free allocated blocks require a separate previous-free bit in the next
header and a boundary sentinel. Only when that bit is set can the predecessor's
footer contain a valid size to follow. Split/merge/allocate/free must update both
neighbors. This variant is a derived design with explicit obligations, not an
executed alternative implementation. Do not grant its implementation facet from
the baseline full-footer C code.

## Pool, class and buddy distinctions

The retained producer's raw handle contains generation and index. Equal raw
handles can designate different owners; this is a counterexample to interpreting
its old API as global identity. The wrapper adds a live stable owner pointer.
It reserves only three slots and retires after generation3; public tests check
old/new values, double release, foreign owner and actual exhaustion. Moving the
control or ending it invalidates the owner precondition; looking at a dead pointer
to detect this would itself overstate safety.

Embedded links require room and valid type/alignment. An external bitmap avoids
overwriting payload but costs side storage; an actual typed union can explicitly
separate live payload and free-link states. A1-byte byte payload cannot hide a
machine pointer. For stricter alignment use a matching real typed array or an
admitted allocation backend, not a cast to the existing byte array.

Class requests1,8,9,17 sum35; slot charges8+8+16+32=64. Four64-byte slabs each
holding one8-byte object retain256 despite only32 live/charged bytes. Separating
long/short lifetimes can release whole backing slabs only when those backends
actually support release. Our embedded arrays do not. Reusing cached bytes does
not initialize a new resource owner. Initializer failure rolls back its synthetic
token before allocator publication; old destruction and new initialization are
distinct log events. Expensive shared resources need an independently retained
cache owner and per-request reset/borrow boundary.

Buddy siblings differ by exactly one validated order bit of the relative offset.
Adjacent blocks of different order are not mergeable siblings. The15-node tree
bounds one allocation traversal by15 visits and one release by4 nodes; invariant
validation, callbacks, mutex scheduling, compiler helpers and backend costs are
additional. Multiple owners remain distinct even if their physical regions touch.
An8-aligned base suffices for8-aligned payload offsets; a64-byte physical base
alignment is not needed for the logical split tree. Base3 cannot support that
payload promise without correcting/reducing the admitted region.

## Experimental limits and architecture choices

The Python placement oracle uses ordered logical intervals and reconstructs
positions/accounting independently of C headers. Generated C tests separately
check payload patterns and pairwise byte occupancy. Each comparison covers its
finite grammar, not every possible malformed C object or lifetime violation.
Delta debugging yields the one-operation corrupted-footer trace `c` in the
declared alphabet. It is not a proof of globally minimal arbitrary input.

Thread-private region metadata removes one sharing point but can strand capacity.
The real workers retain owners in main; a worker's termination therefore does
not destroy its region. Shared metadata locking does not order later unlocked
payload writes; the racy target's TSan report supports that specific observation.
Immutable release/acquire publication orders initialization; join ends the reader
before owner destruction. Neither guarantees scheduler fairness or bounded wait.

The explicit token/AST/response/cache sizes16/48/24/8 give live curve24,72,56,56,
80,32,8,8,8,8 and peak80. Extending AST retention through8 changes the curve,
even if peak stays80. Ten64-byte messages with two active can have128 live while
a never-reset request region retains640. An indefinitely retained completion
requires bounded outstanding admission or cancellation/ownership transfer.
The8-plus-ten64 example retains648 in one arena versus peak72 with independent
release. Cross-region owning cycles violate the chosen acyclic lifetime contract;
tracing or explicit graph teardown is a different mechanism, not free arena reset.

Benchmarks retain raw batch samples and equal request sequences, status checks
and final data contracts. The toggle adds another structural validation pass;
the core's mandatory checks remain. Counts and retained bytes are more portable
than timings. Batch elapsed time includes reset, copying, validation and clock
overhead. No physical cache explanation or hard worst-case latency follows.

N1570 is C11 draft, N3096 is C23 draft. Their realloc wording differs on nonnull
zero-size calls; the null-pointer path follows malloc. Failed allocation preserves
old ownership. On success the standard allocation's old lifetime ends even if
the returned address compares numerically equal in a separate valid analysis.
Do not inspect the former indeterminate pointer after success. Exact published
C17 wording remains an access gap, and the local API consistently rejects zero.
