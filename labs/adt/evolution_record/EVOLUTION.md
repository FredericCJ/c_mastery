# T67.09: Lossless duplicates, canonical transcripts and authorization

This required successor retains X-T67-009/R1-R4 and its delayed variant.
Implement the supplied record stub, retaining the actual PR-T67-09 TLV reader
and writer. Complete X-T67-008, PR-T03-02 `bytes.cursor` and PR-T20-01
`codec.records`. The parser calls actual `struct f_cursor/f_take`; the two-edition
relay imports `ar_encode/ar_decode`. Authentication composes the preparatory
first-principles `replicated/hashing` stage, whose integer/byte prerequisites
are T03/T04. That stage can be implemented before the later T76 Merkle work;
this is not a cyclic prerequisite on completing the whole T76 project.

Supply completed dependency roots through `--dependency foundations=...`,
`api_depth=...` and `replicated=...`, or the matching explicit CMake root
variables. No assessor implementation enters a learner workspace. The header,
stub, tests and original intentionally vulnerable `target.c` are public; the
correct implementation and technical analysis remain assessor-only.

## Execution environment and source authority

Use hosted ISO C17 with exact uint32 and 8-bit bytes, GCC/Clang debug, release
and ASan/UBSan. Build mechanisms from integer, byte and fixed-array operations;
no target serialization, sorting, cryptographic or container library replaces
them. The actual completed SHA-256/HMAC implementation is an explicit learner
component dependency. Python hmac/hashlib and models are host verification
dependencies. Record actual object helpers, stack use, binary size and compiler
dependency closure. No freestanding, board or physical timing claim is made.

The inspected Protobuf guide distinguishes binary unknown-field retention from
loss through fieldwise reconstruction. Its encoding guide states scalar
duplicate and field-order behavior. Those are comparison facts, not this
format's wire specification. This original bounded format deliberately uses
its own duplicate policy and canonical form. HMAC is a keyed authentication
code, not a public-key digital signature. Existing registered hash passages
and actual imported source bound that dependency; no new cryptographic proof
or production-security audit is inferred.

## Wire representation and owned admission

Wire bytes are count:u8 followed by exactly that many retained ADT TLVs
`tag:u8,length:u8,payload`. Count is0..8. Each payload is0..16 bytes, so the
whole record is1..145 bytes. An empty record is the single byte00; empty input
is malformed. Tag1 has exactly two bytes, tag2 has none; tags3..127 are advisory
unknowns. Tag0 and128..255 fail. This narrows each record field to16 bytes;
the unchanged original single-TLV producer still supports255 payload bytes
and is tested at that maximum. Never silently replace its historical contract.

Known tags1/2 are singular: any duplicate is rejected, even byte-identical ones.
Unknown tags may repeat. Their occurrence order within a tag is meaningful and
is preserved exactly. Parsing uses a local FCursor, takes each complete header
and payload, calls actual `ad_evolve_read` on that exact TLV, and copies the
payload into an owned `struct ade_record`. Validate the entire count, lengths, known
forms, duplicate policy and exact end position before publishing. Every refusal
leaves the previous output unchanged. Truncated or trailing input is invalid.

All pointers refer to live aligned objects and real extents; input/control/
output/key/tag/service storage is disjoint and externally serialized. Public
record fields are inspectable values, not permission to forge an invalid state.
All active fields must be initialized. Record copies own their inline arrays
and retain no wire pointer. The original `struct ad_evolving` view still borrows input:
mutating that buffer changes its payload, while mutating the source after
successful `ade_read` cannot change the copied record. No allocation/destructor,
asynchronous cancellation or hidden resource ownership is involved.

## Forwarding and canonical ordering

`ade_write(record,canonical,out,capacity,used)` validates the full value and
capacity before any output byte changes. `canonical` is0 or1; other values are
INVALID. Insufficient real storage returns CAPACITY with both buffer and used
unchanged. Preserve the count and every tag/length/payload. Mode0 emits original
order byte-for-byte. Mode1 performs a stable insertion sort by numeric tag,
then emits the complete record. Equal unknown tags keep their relative order;
deduplication or payload-based ordering is forbidden. Sorting different tags
does not change this record's abstract meaning, but swapping two occurrences
of the same unknown tag can change a future reader's meaning.

Example input fields9:[04],1:[00,07],9:[05],3:[] canonicalize to
1:[00,07],3:[],9:[04],9:[05]. Reparse/recanonicalize is idempotent. Reversing
the two tag9 occurrences produces a different canonical byte sequence. A
future last-occurrence interpretation therefore remains stable through relay.
Do not import Protobuf's scalar last-wins policy as permission to discard
unknown occurrences in this original format.

The actual codec adapter places an `ar_encode` format1 or format2 record in
opaque tag3, forwards/reorders it, then calls `ar_decode` with the externally
known codec version. Format1 signed16+XOR and format2 signed32+XOR have their
own contract. The relay never interprets that payload, changes its checksum,
or treats XOR as authenticity. The canonical component identity stays
`codec.records`; this explicit successor implementation is recorded separately.

## Authentication transcript and higher-level policy

`ade_tag` authenticates the bytes `CMASTERY-ADT-RECORD-v1` followed by one zero
byte and the entire canonical record. The terminating zero is part of the
domain separator. The fixed synthetic teaching key is32 bytes; the imported
`rph_hmac` computes HMAC-SHA-256. The transcript includes count, all lengths,
all known and unknown fields and every duplicate in stable occurrence order.
`ade_check` compares all32 bytes using actual `rph_equal`. Invalid inputs or
authentication mismatch publish nothing. The source loop's fixed work is not
an ISO guarantee of constant machine timing; no new side-channel claim is made.

If an asymmetric signature replaces the keyed code, sign and verify exactly
this transcript with an explicitly agreed algorithm, key identity, format
version and policy. A relay must not reorder same-tag duplicates, drop unknown
data or reconstruct only recognized fields. This exercise executes HMAC and
specifies signature participation; it does not claim to implement public-key
signatures or key distribution. Canonicalization cannot supply authorization
for unknown semantics, replay protection, confidentiality or freshness.

`ade_apply` first authenticates the whole record, then applies a trusted local
policy: exactly one tag1 amount is required, tag2 may occur once, and every
unknown tag is refused even with a valid authentication code. Amount is unsigned
big-endian16. On success add it to `struct ade_service.total`, leaving privileged
unchanged. Missing amount/unknowns give POLICY; uint32 addition overflow gives
RANGE. State changes only after every check. Zero amount is allowed. No error
may increment either counter. The policy is not supplied by attacker bytes.

## Concrete bounded security experiment

Target: only public original `target.c` and corrected `ade_apply` in this local
test executable. Environment: isolated local process, synthetic key00..1f,
synthetic amount7 and ordinary/privileged counters; no sockets, other data,
credentials, deployed software or host-setting changes. The attacker controls
the incoming record's advisory fields and occurrence order but does not receive
the service key or a signing operation in the modeled request interface.
The test harness provisions known messages/tags as fixtures, not real secrets.

The flaw projects to recognized fields for authentication, then forwards the
unprojected record to a newer endpoint. The newer endpoint treats the last
one-byte tag9 as an administrator flag. Start with authenticated tag1 amount7;
append tag9:[00] and tag9:[01] without changing its original authentication code.
The vulnerable local target accepts and changes synthetic privileged0→1.
This is the exact harmless security-property violation. It requires projection
of unknown data plus a downstream interpretation change; it is not a forgery
of HMAC, arbitrary code execution or host privilege escalation.

Repair1 commits every unknown occurrence in the authentication transcript: the
old code now yields AUTH and unchanged state. Repair2 separates authentication
from authorization: even a freshly authenticated complete attack record yields
POLICY. A legitimate known-only record still adds7. Regression tests reverse
unknown duplicate order, permute different tags, alter the last authentication
byte, omit amount, and force counter overflow. Residual limits are replay/key
management, arbitrary future policies, deployment side channels and unavailable
cryptographic review. Never claim a crash demonstrates this property.

## Deliverables, verification and rubric

D1: define known/unknown/required semantics and the exact duplicate relation.
D2: derive bounds, lifetime transitions, stable canonical form and transcript
bytes; supply implementation and source import map. D3: record baseline and
composed tests, all truncations/capacities, independent oracle comparisons,
generated-code evidence and eight separate compiled mutant rejections.
D4: provide the two-version relay, local security witness and repaired policy,
then the thirteen cost/enforcement rows below. Retain predictions and exact
commands/source hashes rather than assuming a family PASS assesses everything.

The independent Python model parses a separate grammar, uses stable tuple
sorting, and computes HMAC with the host standard library. It compares full
output buffers, unchanged used values, authentication bytes and both service
counters, including seeded and enumerated malformed/duplicate cases. Four
anchors run on six compiler profiles; four compiling stubs must fail. Eight
controls detect duplicate-known acceptance, unstable equal-tag sorting,
unknown-authentication omission, one-byte authentication comparison,
authenticated-unknown authorization, failure-state mutation, original unknown
payload loss and original malformed-known acceptance. Intended rejection must
be an ordinary semantic assertion, never a sanitizer or compilation failure.

R1 distinguishes malformed known/required fields from tolerated advisory data.
R2 requires exact retention and ownership, capacity proof and actual codec/cursor
imports. R3 requires full byte relay and all original limits, including255-byte
single-TLV and borrowed mutation, plus independent composed results. R4 requires
duplicate identity before canonicalization, exact authentication transcript,
the original local policy violation and both repairs. Level0 violates a
contract; level1 passes ordinary examples; level2 supplies each argument and
bounded experiment; level3 handles unseen ordering/version/policy conditions
unaided and explains which former equivalence no longer holds.

For X-T67-009-DELAYED, give an old relay two tag9 occurrences with opposite
values, permute different tags, and change the downstream schema to interpret
the final occurrence. Predict exact forwarding, canonical bytes and policy
outcomes before running. Repeat with a zero-length unknown and maximum-sized
opaque payload. Preserve the original seven-day unaided assessment delay;
author implementation is readiness evidence, not learner performance.

Hints: (1) define equal-tag ordering before choosing a sort; (2) authenticate
the information the receiver will act upon; (3) valid authentication does not
grant permission to interpret an unknown field; (4) model ownership separately
from a borrowed-view round trip.

## Thirteen cost/enforcement dimensions

| Dimension | Evidence and limit |
| --- | --- |
| Representation | Measured owned-record size/alignment, count and fixed tag/length/byte fields. |
| Reachable storage | Eight inline16-byte payloads; staged145-byte wire and bounded authentication state. |
| Lifetime | Original TLV borrows; successor copies; codec payload remains owned through both relays. |
| Mutation/aliasing | Disjoint inputs/outputs; canonicalization sorts indices, never source fields. |
| Allocation | No mechanism allocation; Python/harness storage is host-only. |
| Movement/copying | Full staged write and owned parse; exact copied byte count is bounded by145. |
| Destruction | No heap destructor; imported keyed state clearing retains its own documented limits. |
| Dispatch | Direct real cursor/TLV/codec/hash calls; no hidden serializer provider. |
| Concurrency | Externally serialized service and record access; no concurrent authorization claim. |
| Failure | Admission/capacity/authentication/policy/overflow preserve outputs and service state. |
| Compilation | Actual strict diagnostics, compiler input closure and clean/unchanged build observations. |
| Binary/stack | Object/link/stack reports include imported primitives; no freestanding exemption for helpers. |
| Boundary | Stable within-tag order versus cross-tag equivalence; MAC versus digital signature and authorization. |

Parsing/forwarding costs O(F+B), canonical insertion sorting O(F²+B), and HMAC
O(B) with fixed key, where F≤8 and B≤145. All storage bounds are fixed here.
Seven1000-iteration samples report maximum145-byte canonical relay separately
from full authentication, with exact byte/checksum results. Those perform
different work; no relative speedup claim is allowed. Available host evidence
does not establish all inputs, independent assessment or physical behavior.
