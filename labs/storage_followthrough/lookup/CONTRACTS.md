# Ordered lookup, normalization and callback reentry

This required successor addresses the three ordinary and three delayed
`PR-T47-03` criteria. Reuse the learner's actual `PR-T47-01` codec, `PR-T47-02`
cache and `PR-T47-03` lookup/scan implementation. The full integration fixtures
additionally require `PR-T47-04/05` insertion/deletion and the valid opener;
early hand-constructed-tree reasoning remains possible before those stages.
The actual retained store is imported unchanged, not replaced by a library map.
The active language mode is C17 with eight-bit unsigned page/key bytes; host-side
Python sorting and normalization are independent verification, not C dependencies.

## Binary ordering and ordinary scan

Retain the exact unsigned length-delimited comparator. Zero-length keys are
invalid for get/put, embedded zero is ordinary data, and the shorter of equal
prefixes sorts first. Octets128..255 follow0..127 regardless of signed `char`.
An internal separator is the minimum key of its right child; equality must choose
that child. The compiled original wrong-separator control changes equality to
descend left and must fail after a successful build.

The fixture inserts92 independently defined records in permutation `(j*37)%92`.
Record0 has key00, record1 key0000. For record `i+2`, `0<=i<90`, the three-octet
key is `([0,1,128,255][i/23],0,i%23)` and the one-octet value is `i+2`. Compare
every lookup and the complete scan with Python's independent byte ordering.
Check the actual tree/leaf/allocation invariant after each insertion; after a
copied scan, the reentry workload checks it after every deletion as well.

For get, NOT_FOUND, invalid empty key and insufficient capacity preserve both
caller value bytes and returned length. A successful zero-length stored value
would not require a value copy; do not confuse it with a missing key. For scan,
an empty lower bound means start before all keys. Test exact prefix0000, exact
key00000a, between-key00000a00 and after-allffff. Callback nonzero is a normal
early stop, not corruption. A callback that accepts zero records still receives
the first candidate so it can decline it; no candidate means no invocation.
Keep accepted-record count distinct from invocation count. Retain data only by
copying it while the callback borrow is live. The retained callback contract
does not authorize reentrant store operations.

## Changed collation: an exact normalization domain

Implement `sl_normalize` for **printable ASCII octets0x20..0x7e only**, length1..16.
Only `A`..`Z` become `a`..`z`; punctuation and other admitted octets are unchanged.
Reject NUL, control octets, high-bit bytes, empty keys and longer keys. This is
explicitly neither locale-dependent text processing nor Unicode case folding.
Stage sixteen output bytes (zero beyond length) and the length; reject unchanged.
Input may overlap the output byte array; the separate length output is live and
disjoint from bytes/input/control. Native addresses remain caller preconditions.

`SlText` is a borrowed facade over one live externally serialized actual store.
`sl_text_bind` checks the actual tree, scans existing keys and rejects a store
containing noncanonical text keys. It does not silently migrate mixed policy
data or resolve collisions. After binding, all mutations must use this policy;
raw bypass is a programmer-discipline counterexample, not an ISO C-enforced ban.
`sl_text_put/get/scan` normalize before calling the actual operation; an empty
scan lower bound remains a before-all sentinel. A canonical collision such as
Alpha/ALPHA denotes one key and uses the retained replacement semantics.
Original spelling is not retained. Binding output preserves on rejection.
Facade and engine lifetimes are explicit; no ownership transfer or allocation
occurs. Its scan callback retains the original no-reentry rule.

## Changed callback lifetime: owned immutable snapshots

`SlSnapshot` contains at most `31*7=217` fully copied `SlRecord` values, a count,
source generation, whether capture was from an active transaction, and a ready
flag. This loose bound follows from at most31node pages each with seven entries;
internal pages actually reduce leaf capacity. No pointer into a source page,
engine, transaction scratch or cache is stored. Source generation is descriptive,
not a unique snapshot identity: two captures during one transaction may differ
while carrying the same generation. A staged snapshot is not durable evidence.

`sl_snapshot_capture` runs under the source store's serialization. First validate
the entire actual tree/leaf/allocation invariant with `store_check`; then copy the
complete ordered scan into private staging. No client callbacks occur during
capture. Any corruption/IO/capacity failure leaves the destination unchanged;
publish the whole snapshot only after success. Destination is live and disjoint
from engine/backend and is not currently borrowed. Replacing an unborrowed
snapshot releases no external resource because it owns only inline values.

Once capture returns, the immutable copied snapshot is independent of subsequent
source updates and lifetime. `sl_snapshot_scan` applies unsigned lower-bound
ordering to these copies. It calls the client at most the supplied budget;
budget0 makes no calls. `calls` counts actual invocations, including one returning
nonzero to stop normally. Invalid admission leaves that output unchanged. The
callback receives one borrowed record valid only for its call. It may perform
serialized operations on the original store, or capture into another disjoint
snapshot, because no engine call/borrow remains outstanding. It must not mutate
or destroy the current snapshot, retain the borrowed pointer, reenter a scan of
that same snapshot with conflicting output state, jump out or destroy the count
output/control. Context/output/current snapshot are disjoint and live through
return. Concurrent mutation is not authorized by this synchronous contract.

The actual reentry fixture captures92 staged records, then deletes each source
key from the callback and checks the live source invariant. During the first
callback it captures another91-record snapshot. The outer snapshot still yields
all92 original records and values while the source ends empty. This supplies a
new lifetime/serialization mechanism, rather than simply permitting callbacks to
reuse transient leaf pointers. It costs bounded deep-copy memory and does not
implement shared-page version pinning or concurrent readers.

## Structural falsification

Cross-check actual tree traversal, linked leaves and allocation bitmap after
updates. From a valid active fixture independently inject: a child32 outside the
31node range; a leaf self-cycle; allocated but unreachable page31; and a shared
subtree by making two root children name the same child. The shared subtree has
no directed cycle but violates exclusive tree ownership. The actual checker
rejects the second visit through its visited-page set and also requires exact
bitmap reachability and matching leaf order. Invalid child/cycle scans reject
within the finite page bound. The snapshot precheck prevents a superficially
successful leaf scan from laundering unreachable/shared tree state.

All corruption uses original copied caller-owned teaching objects. Tests do not
access third-party images, raw devices or arbitrary pointers. A synthetic invalid
tree must not be promoted to a demonstrated memory exploit. Keep the first failed
invariant and exact bounded traversal evidence separate from a generic assertion.

## Required assessment and verification

| Original criterion | Concrete implementation or analysis and pass evidence |
|---|---|
| `AX-PR-T47-03-T47.03.a` | Derive unsigned/prefix ordering, all92record results and every separator-equality descent; reject wrong-direction control |
| `...a-DELAYED` | Implement exact ASCII normalization plus real canonical-store bind/put/get/scan; demonstrate collision replacement, mixed-policy refusal, unchanged malformed rejection and idempotence |
| `AX-PR-T47-03-T47.03.b` | Execute lower bounds before/at/between/after, stop after zero/one/three accepted records, preserve get outputs on refusal, and explain callback borrow/copy lifetime |
| `...b-DELAYED` | Implement owned snapshot capture and real reentrant deletion/nested capture after the source scan returns; prove current snapshot remains immutable and source operations are serialized |
| `AX-PR-T47-03-T47.03.c` | Run the actual invariant check after each update; inject dangling child, cycle and unreachable allocation, record bounded refusal and unchanged snapshot |
| `...c-DELAYED` | Inject acyclic shared subtree and explain why tree ownership/reachability rejects it even if a leaf-chain scan alone appears ordered |

D1 is the predicted order, ownership and changed-policy argument; D2 is actual
C source/composition; D3 is exact source-bound campaign, independent oracle and
minimized counterexamples; D4 is separately administered delayed transfer after
the existing seven-day minimum. Rubric0 violates contract,1 reproduces nominal
examples,2 supplies independent ordinary evidence and diagnosis,3 independently
derives normalized identity or copied snapshot reentry and changed invariants.
Passing author campaigns do not administer a learner assessment.

Graduated hints: compare unsigned octets then length; draw the equal separator's
right interval; write which call owns each callback pointer; normalize before any
map operation; end the source scan before reentry; use a copied immutable value
set if shared-page lifetime would otherwise escape; visited-node uniqueness and
absence of cycles are different properties. The separate assessor analysis is
not exported with the learner package.

Six anchors cover binary lookup, normalization, ordinary scans, snapshot reentry,
exact lower-bound equality and structural rejection. Five compiled controls omit
folding, publish partial normalization, omit the full tree precheck, skip exact
lower equality or use the retained wrong-separator implementation. All must
compile and reject semantically; learner stubs compile and fail all anchors.
The independent Python oracle enumerates65,793 empty/one/two-octet inputs for the
specified normalization and all92binary records. It does not prove arbitrary
Unicode behavior, file inputs or histories. Six C17 GCC/Clang profiles cover debug,
release and address/undefined-behavior sanitizers; physical and human-independent
evidence remain separate.

The snapshot uses at most217 inline records of82bytes plus native metadata;
capture additionally stages one complete snapshot on the call stack and executes
the retained bounded checker/scan. Record exact compiler stack and object imports;
do not infer a microcontroller stack budget. Normalization is O(key length),
snapshot scan O(records*key bound), capture O(reachable pages + records) apart from
the retained checker costs. The facade adds no allocation; copy cost buys source
lifetime independence. Current evidence lives in
`research/v5/expansion/legacy_storage_followthrough/lookup_validation.json`.
Exact published C17 access remains a gap; active compilation remains C17.
