# Inclusion proofs and staged repair over real framed bytes

This required X-T76-007 successor supplies D4 and the support for
X-T76-007-DELAYED. Retain R1–R4, the original Merkle anchor and HASHING.md's
streaming/HMAC work. Families A/B/C/D/E/F/G/H/I/J apply to proof shape,
canonical messages, hostile replies, staged publication and changed cost models.
Author tests do not substitute for independent learner reasoning or delayed work.

## Prerequisites, sources and permitted dependencies

Complete the original PR-T76-07 bounded tree, X-T76-006 identity/context analysis,
and the streaming SHA implementation in ../hashing/HASHING.md. Import the actual
rph_hash/equal provider; compare u32 leaves against the actual rp_merkle producer
and its declared DOD dependency. Import PR-T24-03's RfParser/RfMessage from the
completed hosted_systems/stream_depth workspace. The selected functions are
rf_parser_init,rf_feed,rf_eof,rf_message_init. This is actual framing composition,
not a fixture claiming that an unimplemented io.frame API exists. No hash,
Merkle, networking or serialization library performs the studied mechanism.

R-RP-MERKLE sections2.1.1–2.1.3.2 were read: raw tree shape/domain separation,
path generation and exact index/path verification. R-RP-SHA supplies the
separately implemented hash. The application-domain wrapper, fixed query/reply
format and repair protocol below are original designs. They are not CT wire
messages or a claim of Certificate Transparency deployment/conformance.

Use ISO C17 hosted Linux,8-bit bytes and exact uint32_t; GCC/Clang. CMake uses
explicit completed dependency workspaces; private assessor implementations stay
separate. Python hashlib and standard I/O/timing are host verification tools.
Strict freestanding linkage and physical/network execution are not claimed.

## Representation, ownership and failure contract

RpkTree owns domain uint32, count0..16 and16 inline leaves, each length0..32 and
32 bytes. Only the first length bytes contribute to a leaf. Initialize complete
native controls before use; active lengths/counts are valid and fields are not
concurrently changed. Empty leaves and an empty tree are distinct. The tree is
an ordered sequence; transport chunk boundaries do not change it, whereas
changing the logical leaf boundaries does.

RpkCommitment is a trusted tuple(domain,count,root32). RpkProof carries domain,
count,index,depth0..4 and up to four sibling hashes. Public native structures
are not wire layouts. RpkCost records client hash invocations, requests, encoded
request/reply bytes and accepted leaf replies. All required pointers are live,
aligned and disjoint; output objects/spans do not overlap inputs/controls.
Callbacks do not reenter or mutate any target/trusted/control object. Native
invalid pointers and arbitrary concurrent access are outside admitted input.

All operational failures preserve target and output/cost objects byte-for-byte.
There is no allocation, persistent store, asynchronous retained buffer or rollback
of external effects. A callback may advance its own synthetic transport state;
that is not undone on failure. The synchronous exchange borrows request/reply
spans only for the call, must return in finite time, stay within reply capacity
and either return0 with exact reply length or nonzero for unavailable response.
It may not retain borrowed stack addresses. No timeout or external liveness
promise can be inferred from this interface.

## Raw tree, explicit commitment and inclusion proof

For raw tree T: empty is SHA256(empty); a leaf is SHA256(00||leaf bytes); an
internal node is SHA256(01||left32||right32). Split n>1 at the largest power of
two strictly less than n. The application commitment is always
SHA256(02||domain BE32||count BE32||raw root32), including domain0. This wrapper
is deliberately separate from the RFC raw tree and from the retained producer's
raw u32 root. Domain/count are therefore bound, not inferred from native layout.

rpk_commit validates all active lengths/counts and stages output. rpk_prove
requires index<count and appends siblings from leaf toward root. rpk_verify
requires the proof's domain/count to equal the independently supplied trusted
tuple, count1..16, index<count and the exact path depth derived from index/count.
It reconstructs left/right orientation from shape, hashes the leaf, folds each
sibling in order and compares the wrapped result. Truncated and extra paths
fail, including extra siblings for a singleton. Empty trees have no inclusion
proof. Proof verification supplies no sender identity, retrieval guarantee or
cryptographic proof from test success. The root must come from an independently
trusted authority; a hostile peer choosing both content and root can be
self-consistent and still unauthorized.

## Exact local framed transport

A request has24 raw bytes: RPQ1,domain BE32,total count BE32,start BE32,span BE32,
nonce BE32. Each eight raw bytes is one real width1 RfMessage frame, so request
wire length is27. Nonce starts at1 and increases per request in a repair; at most
31 tree nodes are requested. The server accepts only a canonical subtree range
of its complete tree, matching domain/count and a nonzero nonce.

A response has96 raw bytes: RPN1 with the same five numeric fields at offsets
4,8,12,16,20; kind at24,length at25,reserved zero26..31;64 payload bytes at32.
A leaf response kind1 contains length0..32 and exactly those bytes, followed by
zero padding through95. A branch response kind2 has length0 and left/right raw
hashes at32/64. It is twelve eight-byte frames,108 encoded bytes. All fields,
range, nonce, kind, length and padding are checked before using the payload.

rpk_serve decodes the request through actual rf_feed/rf_eof, derives the selected
node and constructs real framed replies. It stages output and returns FULL if
reply capacity is insufficient. rpk_repair feeds reply spans in chunk1..108
pieces, including boundaries within headers/payloads and across frames. It
consumes the exact reported prefix, requires complete frame EOF and exact raw
length, and rejects malformed lengths, missing/extra bytes and inconsistent
correlation. Fixed frames are an original transport policy; zero-length or
other-size frames are not valid messages in this protocol even though the
imported generic parser supports some of them.

## Repair and publication

Mode0 compares hashes only for the same domain, total count and canonical range.
If the existing complete commitment equals the trusted one, request nothing.
Otherwise fetch the root node. Its raw child hashes or singleton bytes must
reconstruct the trusted wrapped root. For each child, compare the original
local subtree hash with the expected remote hash; skip only equality under the
stated collision assumption. Fetch mismatching children, authenticate each node
against its already-verified parent expectation, and place verified leaves in
a private staged tree. A changed count/domain forces all necessary content to
be fetched; a trusted empty commitment is checked without a callback.

Mode1 walks all nodes with the same parent/root verification. Mode2 is a direct
full-leaf baseline: fetch each leaf, stage it, and verify the complete wrapped
root at the end. Individual mode2 leaves are not authenticated until that final
check. Every mode recomputes the final staged commitment before publishing the
whole target and cost object once. Missing replies return MISSING, malformed
correlation/framing returns PROTOCOL, and inconsistent hashes return PROOF.
No partially repaired target escapes any failure, including failure after an
earlier leaf was successfully received. This is atomic in-memory publication,
not storage durability, concurrency linearizability or transactional network I/O.

## Reasoning and required deliverables

D1: Derive raw split/orientation and the application wrapper. Explain singleton,
empty, non-power-of-two and maximum cases, why index/count/path length matter,
and why changing logical leaf boundaries or domain changes the commitment.
State the collision assumption and root trust separately from parser validity.

D2: Implement proofs, fixed message codec, real framing and the three repair
modes. Give borrowed-buffer lifetime, canonical range/correlation and staged
publication invariants. Induct from the trusted root through fetched children;
for the full-leaf mode derive why only the final whole-state check authorizes
publication. Show that a missing node can deny progress but cannot justify
publishing incomplete repaired state.

D3: Run merkle_proof,merkle_wire,merkle_repair,merkle_cost. Test all136 leaf
positions across counts1..16, truncated/extra/reordered/changed proof material,
wrong index/count/domain, empty/singleton and cross-check retained raw roots.
Vary all108 chunk sizes and all108 response truncation points. Include bad
nonce, reserved padding, oversized/malformed frame, missing root/child/leaf,
inconsistent root-child and leaf data, and unchanged target/cost on failure.
Ten compiled semantic controls omit leaf domain, reverse orientation, accept
extra paths, reduce an out-of-range index, ignore nonce/padding, publish a leaf
early, omit hash verification, or defeat root/subtree skipping. Require normal
RP_CHECK rejection after successful configure/build; a crash, timeout or
sanitizer finding is not a successful control result.

D4: Compare2,311 independent hashlib roots/paths, including2,284 paths checked
by a separate iterative fn/sn verifier, and2,090 independent selective/direct
repair cases. The repair model enumerates all difference masks through eight
leaves plus changed sixteen-leaf/failure schedules, yielding16,224 successful
requests in this fixed ledger. Expected statuses are1,698 successful,301 missing
and91 inconsistent cases. Preserve exact traces/source/compiler/include/link
pins. Run eight GCC/Clang debug/release/ASan+UBSan/TSan profiles; the serialized
transport has no competing native threads, so TSan does not prove interleavings.

## Cost experiment with equivalent publication contracts

For16 leaves and one changed leaf at index7, mode0 requests5 nodes/675 total
encoded bytes, mode1 requests31 nodes/4,185 bytes, and direct mode2 requests16
leaves/2,160 bytes. All use the same trusted final commitment and unchanged-on-
failure target contract. Client hash counts in this implementation are122,64
and48 respectively. They include final verification and repeated local subtree
hashing, but exclude hashes performed inside the peer. The local CPU timing
includes both client and synchronous peer. Record200 runs/mode and raw CPU
seconds, compiler, optimization and diagnostic environment. Smaller transport
volume does not guarantee lower local CPU time; do not select a deliberately
wasteful full-tree walk as the only baseline. The direct full-leaf comparison is
required. Caching subtree hashes would change representation/ownership costs
and needs a new equivalent-contract experiment.

## Assessment, hints, transfer and security

R1/R2 assess shape/domain/trust and the invariant proof; R3 requires ordinary
plus proof/framing/failure/cost evidence; R4 requires the actual framed repair
and an unseen malformed/missing-node schedule. Level0 is absent/incorrect;
level1 passes nominal examples but cannot explain trust/publication/context;
level2 meets all stated obligations; level3 transfers to changed chunking,
domain/count or peer failures and re-derives assumptions. Require every criterion
at2 and the original independent level3 requirements. Public tests alone earn
at most level1. Seven-day X-T76-007-DELAYED remains learner work.

Hints: (1) draw split shapes for3,5,7,16; (2) derive path directions from count
and index, not untrusted direction flags; (3) retain a complete staged tree;
(4) a short frame prefix is not a complete response; (5) compare transport work
and local hashing separately. Assessor solution is withheld in
assessor/replicated/merkle_repair/ANALYSIS.md.

The target is only the original local peer/repair implementation with synthetic
leaves and private buffers. The adversary controls reply loss, correlation,
canonical bytes and node content but not the trusted commitment or native
pointers. Harmless success is a wrong accepted repaired leaf or partial target
publication under a named mutant. No external CT log, certificate service,
credentials or deployed data is targeted. Repair checks identity/range/shape,
authenticates every necessary node or final staged root, and commits once.
Residual limitations include hash collisions, unauthenticated root selection,
missing-node denial, synchronous callback discipline and finite capacities.
Neither a sanitizer finding nor a wrong tree result proves code execution.

## Thirteen dimensions and evidence limits

Report (1) actual tree/proof/cost layouts, fixed wire sizes and staged stack;
(2) inline ownership and synchronous callback borrows; (3) O(N) root/proof build,
O(log N) proof verification and repeated-subtree repair work; (4) shape/range/
root trust invariants; (5) missing/protocol/proof failure publication; (6) wrapper,
framing and staged-copy overhead; (7)16 leaves,32 bytes each,depth4 and31 requests;
(8) repeated local/peer hashes versus transferred bytes; (9) actual CPU samples
without network latency/WCET or physical claims; (10) code/stack/helper audit;
(11) actual SHA, retained rp_merkle comparison and RfParser/RfMessage imports;
(12) ISO unsigned arithmetic and typing versus runtime guards, collision and
callback/trust premises; (13) caching, new domains/counts, changed transport and
asynchronous lifetime alternatives. Recursion depth is bounded by the selected
16-leaf tree; timing is not constant-time or security proof.

Authored executable support. Only a current source-bound report and guard records
actual author execution; no independent human review, learner mastery, live
network, physical durability or target-board run is inferred. Published C17/C23
access gaps remain. Finite differential/model tests cannot prove collision
resistance or absolute mastery.
