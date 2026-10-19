# Cache identity, replacement and generation follow-through

This support implements all three ordinary and three delayed `PR-T47-02` tasks.
Prerequisites are the learner's original four-entry page cache and `PR-T47-01`
byte codec. Actual commit/checkpoint/deletion integration additionally requires
`PR-T47-05`, `PR-T47-07` and `PR-T47-08`; component replacement-policy work does
not wait for those integrations. Use C17, eight-bit page bytes and the exact
`store.h` API. The imported retained engine is unchanged. Public stubs and tests
are distinct from the private reference implementation.

## Clean storage, identity and admission

`ScCache` owns four complete clean 4096-byte entries and bounded policy metadata.
It owns no backend, dirty node, heap allocation or outstanding borrow. Initialize
fresh/unowned caller storage with `sc_init`; invalid policy preserves the object.
All calls are externally serialized. Cache/control/output/backend storage must
be live and disjoint; callbacks must not reenter, mutate cache or view, destroy
owners, retain borrowed output or jump out. Cached bytes are copied to the caller,
so no lease or reclamation protocol is implied.

A trusted `ScView` binds synchronous read callback/context, nonzero image identity,
full64-bit generation, current root1..31 and `wal`0/1. The caller derives these
from the actual current backend authority and refreshes them after every view
transition. Identity is unique for the lifetime of retained entries; a newly
created image must not reuse it. Generation wrap/reuse is prohibited by the
producer. These are cooperative C preconditions, not authenticated capabilities.
Passing a stale trusted view is a counterexample to automatic freshness.

Logical page0..31 maps to physical `2+logical` for home or `34+logical` for redo.
A hit requires equality of the entire `(image identity, generation, physical
page)` tuple. This component caches raw bytes; a hit does not certify CRC, page
kind, a valid tree or a transaction. The actual codec/engine still validates
those properties. No write entry point exists; transaction-owned dirty typed
scratch stays exclusively in the original store and cannot be written through
this clean cache. The callback must return zero only after a complete page read;
on nonzero it may have copied a partial prefix into the temporary page.

Policies all contain exactly four pages:

| Policy | Placement and replacement |
|---|---|
| `SC_DIRECT` | Slot `logical % 4`; exact tuple still required |
| `SC_TWO_WAY` | Two sets selected by `logical % 2`, two ways per set; choose the lowest empty way, otherwise least recently used way; successful hits update the recent bit |
| `SC_PIN_ROOT` | Current root always uses slot0; other pages map to `1 + logical % 3`; metadata is separate from the four payload pages |

On a miss, increment backend read-attempt count, read into a private page and
publish entry/replacement state/output only after success. IO failure preserves
every entry, replacement bit and caller output; the failed attempt still counts.
A hit increments only hit count and copies a full page. At `UINT64_MAX`, refuse
the relevant counter increment with `STORE_FULL` before IO or publication. This
is a changed finite-counter contract, not an assertion that the retained engine
already has the same instrumentation policy. Bad API arguments return
`STORE_INVALID`. No function examines forged pointer validity or repairs mutated
live controls.

## Publication, invalidation and a falsifiable correction

Run the actual retained store: prime a home leaf, begin a transaction, change its
value in typed scratch, commit, read through WAL, checkpoint and read again.
Assert no backend write occurs merely from reading the dirty scratch. Check
both actual transition postconditions: commit and checkpoint clear all entries.
The controls materialize copies of the exact retained source in the build
directory, remove one or both named invalidation calls, and leave the original
provider unchanged. A build error cannot stand in for a rejected mechanism.

The historical task requests separate stale-generation counterexamples for each
omission. In this particular retained implementation, exact physical tags and
the other invalidation can prevent a stale-value counterexample for a single
omission. Do not invent one: the separate controls fail the explicit empty-cache
postcondition; their bounded stale-read comparisons pass. The combined omission
keeps an old home entry when the changed page is not read through WAL before
checkpoint. The raw page API then exposes old generation0 instead of1. Actual
`store_get` subsequently rejects it as `STORE_CORRUPT`, preserving result/length,
because the retained decoder checks generation. This is an observed stale-cache
and availability defect, not demonstrated stale successful value publication.

Derive the masking argument: before commit, old WAL entries have been cleared
by the preceding checkpoint if that invalidation remains; remaining home tags
cannot hit WAL. If commit invalidation remains, no old home entry can survive
into checkpoint; remaining WAL tags cannot hit home. This argument depends on
the retained valid single-handle API sequence, exact physical tags and no external
backend mutation. It is not a general license to omit invalidation in a shared
or asynchronously changing cache. The original assessment text and its historical
evidence remain intact; this successor supplies the bounded correction.

For physical reuse, run the actual tree: insert20 keys, cache leaf2 at generation1,
delete all20 and commit (allocation bitmap returns to3), then insert20 different
keys and commit at generation3. The freed logical page2 is allocated again;
its redo physical slot repeats. The successor must miss its generation1 entry
and copy generation3 bytes. Also change image identity and home/WAL view separately.
The trusted view must be refreshed from the current store; native object address
alone cannot identify an image or generation.

## Objective evidence and rubric

| Original ID | Required demonstration |
|---|---|
| `AX-PR-T47-02-T47.02.a` | Derive the trace1,1,5,1: three reads, one hit; exact physical tags, separate dirty scratch, no read-induced write; actual cache/entry/scratch byte sizes |
| `...a-DELAYED` | Implement two-way LRU and compare1,3,1,5,1: three reads, two hits; retain ownership contract. Implement the parent project's pinned-root comparison without claiming extra page capacity |
| `AX-PR-T47-02-T47.02.b` | Execute actual commit/checkpoint, detect each omitted invalidation postcondition and preserve the bounded masking proof. Demonstrate the combined stale generation and actual decoder containment |
| `...b-DELAYED` | Execute real deletion/free/reallocation and verify full generation/image/physical identity; distinguish a trusted stale-view misuse from a cache defect |
| `AX-PR-T47-02-T47.02.c` | Compare observed backend attempts/hits with the independent model; report four payload pages, metadata, scratch and temporary page separately from device traffic |
| `...c-DELAYED` | Scan pages0..7 twice: direct and two-way16reads/0hits, pinned15reads/1hit. Compare16point reads of root1:1read/15hits for all. State the workload and cold-start assumptions |

Deliver D1 predictions plus identity/ownership/masking and cost proofs; D2 source
for the policies and exact existing-engine composition; D3 source-bound runs,
independent Python traces, minimized controls and object/stack audit; D4 the
existing separately administered delayed assessment. Score0 for incorrect
contract/result,1 for nominal reproduction,2 for correct ordinary evidence and
causal diagnosis,3 for independent changed-policy/generation/workload transfer.
A learner who asserts a single-omission stale value despite contrary source and
executed controls has not demonstrated this criterion.

Hints: first write the complete tag, then the set/way; derive the authority of
dirty nodes before adding replacement policy; count attempted backend reads at
the callback boundary; never update replacement state after partial read; trace
which physical tags can survive each publication transition. Keep the assessor
analysis separate, and reveal hints gradually.

## Verification and limits

Nine anchors cover direct/two-way/pinned policy, identity, failure, transitions,
actual page reuse and scan/point workloads. Eight compiled controls drop physical,
generation or image tags, publish a failed read, evict the most recent way, or
omit each/both invalidations. Three additional comparisons show the two isolated
omissions remain value-correct for the selected trace and the combined defect is
contained by actual page decoding. All learner stubs must compile and fail the
nine named anchors. The independent Python list/tag model enumerates7203
four-operation histories and28812transitions across the three policies; it does
not call C to compute expected results or prove arbitrary histories.

Payload memory is16384bytes; report actual native metadata and transaction
scratch separately. A hit/miss search costs at most two tuple comparisons; copying
or reading costs one4096-byte page. Two-way LRU needs two one-bit logical states
(the teaching representation uses unsigned members). There is no timing or real
filesystem/device traffic claim. Hosts, synthetic images and compiled controls
are local original teaching targets. No deployed disk, real data or settings are
modified. C17 is the active compile mode; exact published standards and physical
drive behavior remain separate evidence gaps. Current execution is recorded in
`research/v5/expansion/legacy_storage_followthrough/cache_validation.json`;
source-bound author support does not establish human review or learner mastery.
