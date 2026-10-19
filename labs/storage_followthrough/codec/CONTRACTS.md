# Storage page format and fresh-image successor

This required support implements the three ordinary criteria and three delayed
tasks of `PR-T47-01`; it adds no historical hours or acceptance claims. Read the
retained `labs/storage/FORMAT.md`, `include/store.h` and original project first.
Prerequisites are the learner's `PR-T47-01` byte codec and, for the fresh-image
integration stage, `PR-T47-09` opener/recovery artifact. The early codec stage does
not require a completed tree engine. The supplied assessor adapter imports the
actual retained store; learner builds must select their explicit provider with
`CMASTERY_STORAGE_ROOT`. It never substitutes an external database library.

## Representation and version policy

The retained CST2 file remains unchanged. Build under ISO C17 with eight-bit
octets, exact-width unsigned integers, serialized live objects and caller-owned
storage. `memcpy` and `memset` are permitted byte utilities; they do not implement
the page-format validation or CRC mechanism. The current campaign is hosted;
no strict freestanding, device, foreign ABI or hardware result follows.

Implement the original **CST3L leaf page component**. It is a separate version,
not an in-place migration of the full CST2 database. Internal CST2 nodes cannot
be upgraded by this component. Unknown versions reject rather than reinterpret.
All integer fields are little endian, with no native structure overlay:

| Octets | Field |
|---|---|
| 0..3 | Magic `0x43535433` |
| 4..7 | Version `3` |
| 8..15 | Exact caller-supplied generation, including zero or `UINT64_MAX` |
| 16..19 | Exact logical page ID, 1..31 |
| 20 | Leaf marker 1 |
| 21 | Count 0..7 |
| 22..23 | Zero |
| 24..27 | Next leaf ID 0..31; zero terminates |
| 28..31 | Payload floor 128 |
| 32..87 | Up to seven eight-octet entries: key offset, key length, value offset, value length, each unsigned 16-bit |
| after the live table through 127 | Zero |
| 128..4091 | Payload; every unreferenced octet is zero |
| 4092..4095 | CRC-32/IEEE over octets 0..4091, reflected polynomial `0xedb88320`, initial all ones, final complement |

Keys have 1..16 octets, values 0..64. Each offset is in 128..4092 and length is
at most `4092-offset`. A zero-length value may point one past payload; a key may
not. All positive-length spans are pairwise disjoint, including key/value pairs
within one entry. Keys are strictly ordered by unsigned lexicographic comparison,
with a shorter equal prefix first. Duplicate keys reject. Noncanonical placement
and zero-filled holes are allowed; the encoder packs key then value consecutively
from 128, preserving record order. This makes accepted representation and canonical
serialization distinct. CRC consistency does not establish structural validity
or authenticity; adversarial examples recompute CRC after corrupting metadata.

`sf_leaf_encode/decode` preserve outputs on refusal. Decode requires exactly 4096
accessible bytes; no pointer arithmetic on a shorter span is permitted. Caller
objects and byte spans must be live and correctly typed; no function validates
arbitrary forged native addresses. Complete validation/staging permits byte input
and output representations to overlap. Successful node output has unused member
arrays zeroed; native padding has no serialized meaning. `STORE_INVALID` covers
API admission/encoder input failures; admitted malformed bytes return
`STORE_CORRUPT`. `sf_leaf_upgrade` first calls the actual CST2 decoder, then emits
CST3L for leaves. `sf_leaf_downgrade` validates CST3L before the actual CST2 encoder.
Converting a canonical CST2 leaf to CST3L and back preserves all 4096 bytes. An
arbitrary accepted noncanonical CST2 page may normalize unused bytes; do not claim
identity for that different premise.

## Fresh creation and failed open

Implement `sf_format_fresh` over the actual `store_format` and `store_open`.
`SfFactory.acquire` returns zero with no transferred ownership, or nonzero with
one exclusive fresh disposable 67-page image and a unique nonzero identity. The
factory, not data in the file, establishes this authority. An established image
must never be supplied. The formatter cannot infer freshness from arbitrary bytes.
The callbacks/context and candidate IO remain live through all use; callbacks
do not reenter, retain borrowed control, jump out or destroy the formatter state.
`discard` consumes the candidate exactly once, is nonfailing, and may retain a
forensic copy before disposing storage. It must not recycle an established image.

The result descriptor owns no prior image. Acquire failure returns `STORE_FULL`
unchanged; format or verification failure returns the actual error, discards the
candidate and leaves output unchanged. Successful format plus verified open
publishes the candidate once; the caller then owns its eventual discard. Never
retry the failed candidate. Acquire a distinct fresh identity for a later attempt.
No generation transaction or atomic rollback is promised for initial formatting.
The same-process test's factory can recycle its private byte array only after
discard and reassignment to a new identity; it never presents the old candidate
as a recovered established database.

Compare established-image reopen on wrong checkpoint magic, invalid leaf identity
with recomputed CRC, a short/error block read, and one torn checkpoint beside a
valid newest checkpoint. Require corruption/IO failure or the valid generation;
record every write and prove the original bytes were not silently reformatted.
The opener may mutate its unpublished handle on failure; the byte preservation
claim concerns the image and successor result descriptor, not that handle.

## Required work, evidence and scoring

| Original criterion | Concrete work and pass evidence |
|---|---|
| `AX-PR-T47-01-T47.01.a` | Derive full empty and maximum CST2 pages independently; compare all 4096 bytes including holes and independently computed CRC. Decode intentionally unaligned spans. Explain why packing/native endianness does not define a file. |
| `...a-DELAYED` | Implement CST3L and the leaf upgrade/downgrade boundary. Test unknown version rejection, full-width generation, canonical round trip and internal-node rejection. Explain the missing whole-image upgrade protocol. |
| `AX-PR-T47-01-T47.01.b` | Reseal independent count/identity/key/value/child-reference mutations; prove unchanged result and rejection before following disk references. Separate checksum and structural predicates. |
| `...b-DELAYED` | Implement bounded disjoint spans, legal zero-length one-past values, noncanonical placement, equality rejection and staged overlap-safe output. Compare the independent set-of-octets oracle and minimize an overlap failure. |
| `AX-PR-T47-01-T47.01.c` | Use an established nonempty actual store, then show failed opens preserve image bytes and cannot fabricate an empty success. Show valid checkpoint fallback only under its precise generation premise. |
| `...c-DELAYED` | Inject every one of 38 write/flush format failures and every verification-read failure; distinguish failed candidate disposal, fresh identity retry and unchanged result. Diagnose omitted discard, including the allocation/resource leak it would cause. |

Deliver D1 the byte table, extent/ownership proof and predicted errors; D2 the
learner C source and build closure; D3 exact source-bound runs, independent Python
oracle, minimized failures, compiler/optimization/sanitizer comparison and cost
analysis; D4 a separately administered delayed transfer after the existing
minimum seven days. Passing author runs does not administer D4 to a learner.

Use the existing 0..3 rubric: 0 violates the contract; 1 reproduces a nominal page
without causal explanation; 2 supplies the ordinary independent construction,
diagnosis and evidence; 3 derives the changed-version/span/fresh-identity rules
without copying the worked solution and states their limits. A valid checksum,
matching return code, sanitizer silence or family-wide pass alone earns no
criterion. The separate assessor analysis is not part of learner exports.

Graduated hints: (1) draw byte intervals before casting or copying; (2) subtraction
proves an extent only after admitting its offset; (3) zero-length spans contribute
no occupied octets; (4) gather all fields and validate ordering before publishing;
(5) a fresh creator owns disposal, whereas an opener has no authority to erase an
established image. Stop before the next hint when assessing independent transfer.

## Validation and cost limits

The six named anchors are `original`, `migration`, `variable`, `atomic`, `fresh`,
`reopen`. Five separately compiled semantic controls remove disjointness, truncate
generation incorrectly, publish before order validation, accept the wrong version,
or omit disposal. Controls must compile and fail a named assertion; a build failure,
signal, sanitizer diagnostic or setup error is not a semantic rejection. Learner
stubs must compile and fail each anchor. The oracle uses Python integer sets and
`zlib.crc32`, independently of the C implementation; it exhausts its selected
1024 single-entry offset/length combinations and checks four full page images,
not all possible files. Multi-entry order and output-overlap tests are separate.

With page size P and at most 14 spans, decode costs O(P*S + S^2) plus CRC and key
comparison; encode is O(P + payload). Maximum serialized payload is 560 bytes.
The implementation stages a 4096-byte page or a typed node, never calls an
allocator for the codec, and the format/open integration uses the retained
engine's larger stack/control workspace. Audit actual compiler stack reports and
object imports instead of inferring a microcontroller budget. This hosted support
does not claim physical power-loss durability, exhaustive corruption detection,
cryptographic authentication, human review or learner mastery. All images are
original synthetic local test data; no raw disks or deployed devices are accessed.

Current source-bound status is reported in
`research/v5/expansion/legacy_storage_followthrough/codec_validation.json`; pending
or failed evidence stays pending/failed. Published C17 text is an access gap; the
active language mode remains C17, and older committee-draft passages retain their
own edition labels.
