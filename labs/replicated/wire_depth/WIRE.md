# Nested records, bounded decoding and schema-preserving relay

Required successor for X-T76-008, R1–R4 and X-T76-008-DELAYED. Retain the
original scalar wire anchor and the separate independent/delayed assessment.
Families A/B/C/D/E/F/G/H/I/J apply to grammar, representation, resource refusal,
hostile input, evolution and equivalent-contract measurements. Passing author
tests is evidence for these finite cases, not absolute or unaided mastery.

## Prerequisites and environment

Complete PR-T76-08/X-T76-008's original RPMessage and portable ZigZag work,
X-T76-007's canonical-identity analysis and PR-T24-03's actual stream_depth
RfParser/RfMessage. The completed replicated workspace supplies rp_zigzag,
rp_unzigzag, rp_decode/rp_encode and its explicit DOD dependency. The selected
stream functions are rf_parser_init, rf_feed, rf_eof and rf_message_init.
They are real source imports, with explicit completed-workspace CMake paths;
an old io.frame label does not invent a different API. The original
testing.faults artifact supplies failure reasoning, not an assumed runtime API.
All new parser, tree, budget and canonicalization mechanisms are learner work.
Do not import protobuf, a generic serializer or a Unicode normalization library
to implement those mechanisms. Host Python dictionaries/codecs are oracles.

Use ISO C17, 8-bit bytes, exact uint32_t/uint64_t and hosted Linux GCC/Clang.
No native floating arithmetic is required. GNU sanitizers are diagnostic
extensions; they do not enforce lifetime/ownership in ISO C. Hosted stdio,
memcpy and clock are permitted; generated symbols/stack are audited separately.
No strict freestanding, board or live network claim follows from this campaign.
Published C17/C23 access and exact edition comparisons remain separate gaps.

Read R-RP-WIRE's Encoding sections on varints, submessages, duplicate fields
and field order, plus R-RP-WIRE-EVOLUTION's named canonicalization and unknown
field passages. Protobuf accepts some duplicates/merges and does not promise
general canonical bytes. Unknown length-delimited fields cannot reveal whether
their payload is bytes or a submessage. The grammar and identity below are an
original, deliberately bounded protocol, not protobuf conformance. R-RP-UTF8
section3 supplies byte/scalar validation; no complete Unicode normalization
algorithm is imported or claimed.

## Representation and grammar

Callers select this distinct RPW1 schema profile out of band. It is not a
wire-compatible reinterpretation of the original flat RPMessage schema, whose
tag3 could be an unknown integer. Do not reuse an old schema's content-identity
domain or guess a profile from successful parsing. A real negotiated transport
must bind profile/version as well as feature bits; this local frame adapter
assumes the caller already selected RPW1. There is no silent fallback.

RpwDoc owns an arena of at most16 RpwRecords rooted at index0. A record has up
to8 fields. A field contains tag, kind, uint32 value, length and32 inline bytes.
Kind0 is an unsigned integer; kind1 is opaque bytes; kind2 is a child arena
index. Every admitted arena record is reachable exactly once. Cycles, shared
child nodes and unreachable records are refused. Numeric child indexes survive
ordinary struct copies; no internal native pointer escapes. Live public native
objects and all inspected members must be initialized; fabricating invalid
pointer storage is outside the contract.

Every record requires exactly one tag1 unsigned ID and one tag2 unsigned
ZigZag-encoded signed delta. Optional tag3 and tag4 contain nested records.
Tag5 is a required-feature mask; tag6 is an advisory-feature mask. Both are
unsigned values with missing mask equal to zero. Tag7 is opaque bytes. Unknown
advisory tags8..15 may hold an integer or opaque bytes and must be retained.
Unknown tags16 and above are critical and refused. Tag0 is invalid. Duplicate
tags are invalid even if their values agree. These are schema rules, not
cryptographic authorization or semantic truth of a claimed feature.

Wire keys are minimal uint32 varints `(tag<<3)|wire_type`: type0 for integer,
type2 for nested/opaque payload. Type2 carries a minimal uint32 byte length
followed by exactly that many bytes. Other wire types are invalid. All integer
varints have at most5 octets; fifth byte has no continuation and payload<=15.
An extra terminating zero is nonminimal. No signed left shift is used for
ZigZag: call the completed actual primitive and verify its extrema independently.

RpwLimits admits depth1..4, total raw bytes4..256, total fields2..32 and any
uint32 supported-feature mask. Root depth is1. Charge each field once across
the whole tree. The input-byte budget counts physical message bytes once;
nested slices do not get a fresh aggregate budget and are not counted twice.
Enforce all limits before publication. Each opaque field has at most32 bytes.
Required-feature bits outside `supported` reject the whole message, at any
depth. Unknown advisory bits remain stored; a consumer acts only on supported
advisory bits and must not silently reinterpret an unknown required feature.

Example `10 01 08 96 01 1a 04 10 00 08 02` becomes
`08 96 01 10 01 1a 04 08 02 10 00`. Tag3's child ID2/delta0 is separately
canonicalized. `08 80 00 10 01` is rejected. With supported mask3,
`08 01 10 01 28 04` is REQUIRED, while `08 01 10 01 30 04` is accepted and
retains advisory4. Unknown tag8 containing `10 01 08 01` remains those four
bytes: no recursion is authorized for that unknown field.

## Canonical identity, ownership and failure

Decode accepts record fields in any order. Encode orders fields by tag at every
known nested record, emits minimal lengths/values and retains opaque bytes
exactly. Equality is the schema's recursive integer/opaque-byte map equality.
It is intentionally not future-schema semantic equality: two unknown byte
strings that a future version interprets as reordered nested fields are still
different identities today. Presence of optional zero is distinct from absence.
Duplicate rejection makes the semantic map unambiguous; it sacrifices protocols
that rely on repeated-field concatenation or last-value precedence.

All APIs are externally serialized. Control, input and output spans are live,
correctly sized and mutually disjoint, including stream output/consumed fields.
No callback or asynchronous actor accesses this module. Caller retains input
ownership; the decoder copies everything it retains. Returned documents own
all their storage. No allocation, borrowed subview or hidden cleanup occurs.
The structs are inspectable, but C does not enforce these lifetime preconditions.

rpw_decode, encode, update, project and frame stage results. On refusal their
outputs, output lengths and existing document remain byte-for-byte unchanged.
INVALID means malformed grammar/native shape or invalid admission arguments;
LIMIT means depth/bytes/field/opaque-size capacity; REQUIRED means an unknown
critical tag or unsupported required bit; FULL means insufficient output span.
If several defects exist, the first encountered parser check determines status;
do not claim a unique diagnosis for a multiply malformed byte string. A raw
prefix ending after required fields can be a valid shorter message. Framing
establishes where the intended complete message ends.

rpw_update validates the entire document, changes only root ID/delta and
revalidates resulting size before commit. Growing a varint can exceed the byte
budget: rejection must preserve the old document. rpw_project deliberately
returns only root tags1/2 in the actual RPMessage type. Reconstructing from that
projection loses nested, known opaque and unknown fields. It is a comparison
artifact, never a transparent relay implementation.

## Incremental framing and cancellation

rpw_frame emits canonical bytes in actual width1 RfMessage frames of payload
1..8, followed by one empty frame. Maximum256 raw bytes use289 wire bytes.
The zero frame is the explicit document terminator. A nonempty short frame is
not a terminator. Each call to rpw_feed consumes through at most one document
terminator; the caller owns and resubmits any suffix. No pointer to that suffix
is retained. Feed may span many complete frames or split any header/payload.

RpwStream owns up to256 copied raw bytes, the actual RfParser and limits.
Phases are collecting0, done1, failed2 and cancelled3. MORE preserves accumulated
input and leaves the document output unchanged. On an admitted malformed frame,
budget overflow or terminal decode failure, latch failed and report consumed
prefix; output remains unchanged. consumed is progress evidence, not transactional
input rollback. Invalid API arguments do not mutate state or consumed. After
terminal phases, feed refuses STATE; explicit init starts a new independent
stream. Empty input during collection yields MORE/consumed0.

EOF succeeds only after a valid empty-frame terminator. EOF at an ordinary
frame boundary without that terminator fails and latches failure, even if the
accumulated raw bytes would form a valid record. cancel during collection
clears copied bytes and retires the stream; it does not revoke a nonexistent
external borrow or retract previously consumed transport bytes. No I/O is
performed, so socket close/reconnect obligations belong to an explicit adapter.

## Normalization experiment before extending the schema

rpw_binary64 treats uint64 bits as an explicitly specified binary64 encoding:
both signed zeros map to0; any NaN maps to0x7ff8000000000000; all other bit
patterns, including infinities/subnormals, are unchanged. This is an original
application identity policy, not a claim that IEEE arithmetic equates all NaNs
or that an arbitrary C double has that representation. No floating value is
loaded or evaluated. An actual wire extension must add a tag/type and stated
endianness before using this helper; current grammar still rejects fixed64.

rpw_utf8 validates scalar encodings, rejecting truncated continuations,
overlong forms, surrogates and values above U+10FFFF. Empty and U+0000 are valid
length-delimited spans. Text identity, if this profile is selected for a future
field, is exact valid UTF-8 scalar sequence: there is no NFC/NFKC/case folding.
U+00E9 and U+0065 U+0301 therefore remain different byte identities. Unknown
opaque bytes are not implicitly validated as text. Demonstrate why adding
floating/text semantics changes the old integer-only uniqueness proof.

## Deliverables, checks and assessment

D1: grammar, semantic map, canonical uniqueness/idempotence argument and a
counterexample to canonicalizing unknown nested payloads without a schema.
D2: complete wire_depth.c, public state invariants, budget/ownership proof,
actual primitive/parser import ledger and failure-preserving update/relay.
D3: all four anchors, ten semantic controls, independent raw-byte parser and
normalization comparisons, raw compiler/sanitizer outcomes, stack/symbol audit
and minimized disagreements. D4: framed delayed variant, preserved-versus-lost
unknown fields, proposed normalization contract and equivalent cost dossier.

`wire_nested` checks actual flat producer agreement, permutation invariance,
uint32/int32 extrema, duplicate and nonminimal/overflow refusal. `wire_limits`
checks strict depth/global bytes/global fields, exact32-field binary branch tree,
required versus advisory bits, malformed child, invalid arena and every tested
output capacity below the required size. `wire_stream` checks all26 chunk sizes
and26 strict truncations of the22-byte four-record chain/26-byte framed message,
two concatenated documents, consumed prefix, cancellation, malformed length and
byte-budget refusal. `wire_evolution` checks opaque retention, projection loss,
unknown-type ambiguity and the explicit bit/text policy.

The independent Python model uses integer arithmetic, recursive dictionaries
and its own encoder/parser. Its6,102 wire rows include2,225 accepted,
2,082 malformed,1,283 budget-refused and512 required-feature-refused outcomes,
with1,847 distinct canonical values. Another2,112 rows compare bit classes
through Python struct/math and strict Python UTF-8 decoding:899 valid and1,213
invalid byte sequences. These are finite deterministic corpora, not all inputs.
Ten separately compiled semantic controls omit minimality, duplicate rejection,
depth, byte budget, required-bit rejection, transactional output, EOF terminator,
unknown preservation, signed-zero normalization or shortest UTF-8 validation.
Require intended RP_CHECK assertions; compilation errors or sanitizer faults do
not count as semantic rejection. Learner stubs must compile and fail each anchor.

R1 level2 derives exact scope of identity and safe signed arithmetic; level3
finds an unseen semantic-equality/schema counterexample. R2 level2 proves every
parser advance and publication boundary; level3 changes budget/arena layout
without introducing double charging or unreachable state. R3 level2 explains
oracle independence, valid raw prefixes and mutant scope; level3 minimizes a
new adversarial failure. R4 level2 composes the actual frame API and retains
unknown fields; level3 independently designs the delayed negotiation/relay
variant and justifies its normalization policy. Level1 is merely running the
supplied anchors. Ownership violations and unsupported canonical/security
claims fail regardless of throughput.

Hints: first separate known nested records from unknown bytes; next count one
global field budget and a physical outer byte length; finally stage publication
only after the empty terminator. Delayed assessment changes the accepted
required-feature set and packet segmentation, introduces a newly known nested
field formerly opaque, and requires a versioned identity-domain decision before
migration. Provide a counterexample to silently reusing the old hash identity.
Perform it at least seven days later without the prior implementation; author
support does not create that learner evidence.

## Cost, containment and evidence limits

Address all13 shared dimensions: (1) arena and explicit wire representation;
(2) parser/encoder state operations; (3) acyclic reachable uniqueness and budgets;
(4) owned copies/disjoint lifetimes; (5) transactional versus consumed-prefix
failure; (6) record tag scans plus byte work, bounded recursion depth;
(7)16 records/8 fields/32 total fields/256 bytes/32-byte blobs; (8) struct size,
stack usage and generated symbols; (9) copied raw/canonical/frame traffic;
(10) actual same26-byte framing, bulk feed versus byte-at-a-time feed,7 CPU-time
samples of5,000 parses each; (11) explicit feature/unknown-field integration;
(12) canonical integer arithmetic versus compiler/ABI/runtime enforcement;
(13) caller discipline, sanitizers, finite independent tests and residual proof
limits. The separate10,000 encode/decode loop is a workload observation, not the
same comparison as feed segmentation. Report all samples and scheduler/noise
limits. No remote latency, contention or physical cost is inferred.

Security target is only the original local wire_depth process with synthetic
records. Attacker controls bytes, nesting, feature bits, segmentation and EOF;
it cannot fabricate native pointers or control other people's services/data.
Defects expose unauthorized required-feature acceptance, lost future fields,
unbounded work or partial output. Harmless success is the named wrong local
document/admission result. Repair the exact checks, kill semantic controls and
retain boundary regressions. A digest is not authentication; valid negotiation
does not prove a peer implemented or was authorized for a feature. No crash is
equated with arbitrary execution. Run in a local test process; no host security
changes, external services, firmware or production data are involved.

Authored/executable and actual author execution are separate. The matching
source-bound report records compiler profiles, commands, selected dependency
hashes and observed results. Independent review, learner assessment, physical
validation and arbitrary-input proof remain unclaimed.
