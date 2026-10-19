# Changed-page redo under an equal transaction contract

## Prerequisites, imports and environment

This required changed-condition exercise implements
`AX-PR-T47-11-T47.11.c-DELAYED` from PR-T47-11. Complete PR-T47-01, 06, 07,
08, 09 and 10 first, including the actual page callback contract and recovery
argument. Reuse your completed `store_io` declaration and the separately
implemented failure medium in `../durability/medium.h`; its public stub is
supplied. The reference harness explicitly imports the corresponding medium
implementation. No implementation of a storage engine is called by SDW1.

Use hosted ISO C17, eight-bit bytes, uint32_t/uint64_t and 4096-byte pages.
GCC and Clang debug, optimized and address/undefined-behavior sanitizer profiles
are separate observations. Host verification may use Python zlib as an
independent checksum oracle; target code implements its checksum from bit
operations. This exercise has no physical-device execution claim.

## Representation and publication protocol

Implement `delta.h` in `delta.c`. Both policy values expose exactly 32 mutable
raw pages and atomic transactions over those pages. Full policy logs all32
pages when any page changes; sparse policy logs the pages actually touched by
a byte change. A page restored to its original bytes may remain marked dirty.
Both use the same capacity, checksum, generation, failure and recovery rules.
Neither implements the CST2 B+tree or reads its format. Compare their measured
traffic under this shared contract, then explain why merely omitting CST2
pages would violate CST2's page-generation invariant.

The exclusive image has at least67 pages: metadata slots0/1, homes2..33,
afterimages34..65, publication marker66. SDW1 records occupy4096 bytes and use
little-endian integers. Offset0 is uint32 magic0x53445731;4 is kind1 metadata
or kind2 marker;8 is uint64 generation. Metadata offsets16/20 are32/0.
Marker offsets16/20 are nonzero dirty mask/popcount. Offsets24..31 are zero;
32..159 contain32 uint32 checksums of the complete prospective raw pages;
160..4091 are zero. Offset4092 is reflected CRC32 of bytes0..4091 with
polynomial0xedb88320, initial and final XOR0xffffffff. Raw payload pages have
no header and use that same checksum over all4096 bytes.

Format is a destructive operation on a fresh disposable image with no live
handles. Zero homes and invalidate marker, flush, then publish each generation0
metadata copy with its own flush. Opening initializes fresh control storage,
chooses the greatest valid metadata generation, rejects equal-generation
metadata disagreement, and considers only an exactly-next newer valid marker.
Validate every prospective page, from WAL or unchanged home as appropriate,
before the first home write. Install marked pages, flush homes, publish the
alternate metadata slot, flush, then invalidate marker and flush. Opening
checks all committed home checksums before admitting use. Older markers do not
override newer metadata. Invalid marker bytes are not a committed transaction.

Begin first completes any outstanding checkpoint, then gathers32 pages into
scratch. Writes require an active transaction and copy their admitted range.
Commit first invalidates the prior marker and flushes; writes selected
afterimages and flushes; writes the complete marker and flushes. Only after
successful final flush may the control expose the new generation and WAL
mapping. Checkpoint follows the install sequence above. An empty transaction
performs no publication I/O. UINT64_MAX rejects a nonempty commit before I/O;
it does not prevent empty transactions, reads or cleanup.

## Ownership, failures and costs

Control and backend are live, externally serialized and exclusively owned.
Do not copy an active public control or mutate its fields. Constructors require
fresh storage; no escaped handle survives formatting. Callbacks may partially
modify buffers or persistence before reporting failure; successful flush must
persist every preceding accepted write. Callbacks cannot reenter, retain the
borrowed page buffer or mutate the backend independently between validation
and installation. Application input/output ranges are disjoint from the
control, scratch and callback-owned medium. Null input is legal only at length0;
offset4096 is legal only with length0. Caller provides a full4096-byte output.

Admission failures preserve the live transaction. Read errors preserve caller
output. A failed begin may change private inactive scratch but does not create
an active transaction. Aborting discards scratch changes. A commit error before
marker publication returns SD_IO; a marker write/final flush error returns
SD_UNCERTAIN. Both poison the handle and terminate its transaction. Reopen from
actual persisted state; do not retry a nonidempotent operation on that handle.
Checkpoint error also poisons. A successful read alone does not repair poison.
An unsuccessful constructor leaves an unusable control, not an old live handle.
Corruption returns SD_CORRUPT and may require external repair. CRC is detection,
not authenticity; an attacker who can reseal metadata is outside this profile.

Derive costs with P=32 pages, B=4096 bytes and D touched pages. Begin reads PB
and scratch holds PB bytes plus checksums/control. Both commit checksum all P
pages, O(PB), while sparse commit writes(D+2)B and full writes(P+2)B, each with
three flushes. Checkpoint reads(1+P+D)B and writes(D+2)B, with three flushes;
full substitutes D=P. There is no claim that checksum CPU cost, storage-device
traffic, durability latency or elapsed time falls in the same proportion.

## Concrete workload and falsification

Initialize all bytes zero except first bytes of pages0/1/31 =10/20/30.
Commit the equal update11/0/77. Independently expect every byte of all32 pages
after recovery. The sparse mask is0x80000003; full mask is0xffffffff. Sparse
commit has8 write/flush boundaries; full has37. For each boundary run seven
specified medium persistence choices (none, even pages, prefix2048, suffix2048,
all pages, odd pages, middle512 at offset512). Apply the same315 cuts to
checkpoint. Repeat interruption at all8x8 and37x37 checkpoint/reopen pairs,
first persisting none then all, and require the complete new image after a
final recovery. These finite choices are not all possible devices or tears.

Reject a corrupted last selected WAL page and, for sparse mode, an unchanged
home page before home publication. Inject each of35 preflight read failures,
including partial read output. Check page31, bounds including SIZE_MAX,
null-plus-zero, empty commit, abort, unchanged failed read output and a valid
maximum-generation disk fixture. Four compiled controls omit final flush,
reclaim marker early, omit candidate validation or forget page31's dirty bit.
Require a named semantic assertion, not a process fault or sanitizer complaint.

Run the CMake/CTest suite and independent `verify_storage_delta.py --driver`
oracle. The oracle checks315 exact commit outcomes, both complete marker pages
and equal-workload traffic. For3 useful changed bytes, expect sparse commit
5 writes/3 flushes versus full34 writes/3 flushes; checkpoint36/5/3 versus
65/34/3 reads/writes/flushes. Logs report accepted page traffic, not media traffic.

## Deliverables, rubric and transfer

D1: derive record offsets, state machine and the home-before-metadata and
marker-before-reclaim proof obligations. Explain a plausible incorrect shortcut
that preserves ordinary runs but loses recovery authority.
D2: implement the complete raw-page transaction API with byte-format fixtures,
range and generation checks, an explicit exclusive-backend premise and costs.
D3: provide source-bound ordinary/cut/repeated/corruption/mutant/oracle results,
including the same32-page outputs for both policies and every failed run.
D4: compare costs without silently changing correctness, capacity or flush
semantics, and identify which claims require a physical device experiment.

Superficial completion is a sparse loop plus passing normal writes. Satisfactory
work explains and passes the complete supplied protocol cases. Independent
assessment requires reconstructing the invariants from an unfamiliar trace,
finding an unannounced publication defect and defending the equal-contract
comparison without a worked implementation. Graduated hints: first draw the
authoritative marker/metadata choices; next validate untouched pages as well as
WAL pages; finally trace a cut between home flush and metadata flush. Assessor
analysis is separately packaged. Delayed extension: change to64 pages and a
two-word mask, or add a bounded page-existence bitmap; redesign the record and
whole-image validation before measuring, rather than changing only constants.
Another transfer is a typed tree format with per-page generations; state and
test the additional invariant rather than claiming this raw-page engine is it.

Validation begins as authored. Source-bound campaign/readiness records identify
the actual executed configurations and controls. Passing finite tests is author
support, not independent learner assessment, physical validation or absolute
mastery. All corrupt images are original synthetic data in disposable local
memory; no deployed storage is accessed.
