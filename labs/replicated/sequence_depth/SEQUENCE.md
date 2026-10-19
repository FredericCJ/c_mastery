# Unicode sequence state and controlled operational transformation

This required X-T76-006 successor implements D4 and the support needed for
X-T76-006-DELAYED. Retain R1–R4 and the original two-insert diamond. Families
A/B/C/D/E/F/G/H/I/J apply to dependency admission, scalar boundaries, canonical
state, compaction fencing, transform context and independent transfer. The
finite author campaign does not assess a learner's unaided understanding.

## Prerequisites and inspected sources

Complete PR-T76-06's RPSequence/rp_ot_insert implementation and its ordinary
oracle; X-T76-005's delayed-history and identity reasoning; and the preparatory
SHA provider in ../hashing/HASHING.md. The actual RPSequence and rp_ot_insert
functions, containing RP source's DOD dependency, and streaming SHA source are
compiled imports. No editor, CRDT, Unicode codec or transformation library
implements the mechanism. Host Python/stdio and diagnostic runtimes are
verification tools. Explicit completed learner workspaces supply dependencies;
private assessor sources are not learner imports.

R-RP-CRDT section3.5.1/Specification19 supplies the causality/tombstone comparison;
section4.1 supplies the stability premise. R-RP-OT sections4.1–4.4 and5–6
supply transformation/context/quiescence and the warning that a pairwise diamond
is insufficient. These named PDF passages were read. Our parent tree, bounded
queue/placeholder comparison and central prefix protocol are original designs,
not claimed implementations of every RGA or dOPT detail. R-RP-UTF8 section3
supplies scalar-to-octet rules and the surrogate exclusion. No grapheme,
normalization, display width or culturally appropriate ordering is inferred.

## Environment and common ownership

ISO C17 hosted Linux,8-bit bytes,32-bit unsigned, exact uint32_t/uint64_t,
GCC/Clang and a serialized local harness. All public controls are live, aligned,
initialized by their API and otherwise unmodified. Partial overlap is invalid;
output objects/spans are disjoint from controls and inputs. Tree values are
self-contained copies; server/client copies do not create independent authority
in the same session. There are no callbacks, dynamic allocations, asynchronous
borrows or locks in this adapter. Standard C typing does not prove valid control
invariants, global identities, session authority or transport quiescence.

All operational failures leave controls and output objects/spans unchanged.
Admitting a missing dependency is successful queueing, not visible insertion.
Null required controls/outputs are INVALID. Render permits null output only
with capacity0; a nonempty result then returns FULL without touching length.
Tree init and server/client init replace unowned value storage. Tests use only
original local models and synthetic characters/identities.

## Parent tree, early deletion and Unicode

RpsTree contains16 item records plus the actual RPSequence topology. Each item
has positive globally unique per-epoch unsigned ID, parent<ID, Unicode scalar,
inserted flag and removed flag. Root is0. IDs cannot be reused within an epoch.
Unknown parents are retained as pending inserts; a staged rebuild admits their
children only after the entire parent chain is present. Acyclicity follows from
the strict ID decrease toward root. The underlying RPSequence stores item
indices as byte tokens; rendering maps tokens back to full scalars and encodes
UTF-8. It never truncates a code point into the producer's byte field.

Mode0 uses a separate16-entry early-delete queue: an unknown delete reserves
one ID there; its later insert consumes that entry and remains tombstoned.
Mode1 reserves a tombstoned placeholder in the16-item table instead. Both
admit a child before its parent and delete before insert. Their resource
behavior differs:16 unknown deletes exhaust the placeholder table, whereas
mode0 may still admit unrelated inserts until its item table fills. Repeated
identical insert/delete is idempotent; conflicting reuse returns CONFLICT.
The17th required item or early-delete record returns FULL. No silent eviction
or promise of eventual delivery follows from queued admission.

Resolved siblings use the actual descending-ID depth-first traversal.
Tombstoned nodes hide their own scalar but retain descendants. For example,
delete2; insert(3,parent2,U+1F600); insert(2,parent1,U+20AC) renders empty until
insert(1,parent0,'a'). After insert(4,parent0,'b'), output is 'b','a',U+1F600:
62 61 F0 9F 98 80. Surrogates D800..DFFF and values above10FFFF are INVALID;
U+0000 is a legal scalar and output is length-delimited, not a C string.
Render stages up to64 octets; insufficient output capacity preserves bytes and
length. It never writes a partial multibyte scalar.

## Canonical snapshots and fenced compaction

The368-byte snapshot is RPS1 at0,epoch BE64 at4,mode/count/waiting-count at12..14,
zero15;16 slots at16, each16 bytes: ID BE32,parent BE32,scalar BE32,inserted,
removed,zero2. Present items are sorted by ID. At272 are16 sorted early-delete
IDs encoded BE32. Unused slots are zero. At336 is SHA256 of the first336 bytes.
Restore validates flags, bounds, IDs, parent/scalar rules and canonical bytes
before publishing, then rebuilds actual topology. A checksum is not origin
authentication. Restoring an old snapshot cannot itself grant current group
session authority; the coordinator must admit its epoch and provenance.

rps_compact receives three distinct live replica controls and the local
coordinator's exact outstanding copied-message count. It requires zero
outstanding messages, no unresolved insert/delete placeholders, identical
canonical admitted state at all three sites (mode may differ), and epoch below
UINT64_MAX. BUSY/GAP/RANGE preserves all three controls. The caller has closed
admission while checking and executing the barrier; the numeric count does not
prove remote quiescence. No disconnected/undetectably failed fourth peer is
allowed by this fixed membership model.

On success, render visible scalar tokens, build a fresh chain with IDs1..N and
parent0..N-1 in each replica, discard old tombstones and advance epoch once.
Empty and all-deleted states compact to an empty new epoch. Existing visible
order is preserved; old IDs/anchors/cursors are invalidated. Every delayed old
insert/delete has the old epoch and is STALE. Reparenting deleted nodes in the
same epoch or merely deleting tombstones is not equivalent. The scaffold does
not implement distributed failure detection, consensus or physical durable
compaction; those premises are not supplied by three matching hashes.

## Prefix-context operational transformation

RpsServer retains at most16 original requests,16 transformed commits,17 exact
state vectors and17 document snapshots. Three RpsClients retain their committed
prefix, vector, up to32 scalars, received commits and at most one pending local
request. A request includes session epoch, exact base revision, three-component
base vector, site0..2, next per-site sequence and insert/delete operation.
Clients generate from a committed prefix; they do not optimistically display a
pending edit. Receiving other committed edits keeps the local request pending.
This trades responsiveness/availability for a precise context contract.

rps_prepare accepts insert(kind1,gap0..length,scalar) or delete(kind2,index below
length,scalar0). It leaves the displayed document unchanged and owns a copy of
the request. BUSY refuses a second local outstanding operation. The caller may
retransmit the retained request after a lost reply; it must not invent a new
sequence for the same pending operation.

rps_submit requires an exact stored prefix vector, an existing base revision,
next per-site sequence, and no unacknowledged earlier operation by that same
site in the suffix. Missing/future/incompatible context is GAP. An exact
previous request returns its historical commit; same(site,sequence) with any
changed context/payload is CONFLICT. A new operation after16 commits is FULL,
with server and output unchanged. That bounded session must be retired; this
adapter does not silently discard its transform history or clear a client's
pending request. The application owns refusal/session teardown and cannot
assume a FULL request was applied.

The server validates the operation in its original base document, then rebases
it over each *transformed* suffix commit, whose coordinate system is the next
common prefix. Insert/insert calls actual rp_ot_insert with distinct site
priorities; lower site number wins an equal gap. Insert over delete decreases
position only when the deleted index is strictly earlier. Delete over insert
increases position when insertion is at or before its target. Delete over
delete becomes a no-op for the same target, or moves left for an earlier
delete. No-op commits still consume operation identity and advance the vector.
The server stages the new document, original request, transformed commit and
vector before publication. Capacity failure is atomic.

rps_deliver requires the next revision and exact pre-vector; out-of-order
commits return GAP unchanged for the caller to buffer/retry. Duplicate commits
are accepted only when their complete retained content matches; conflicting
replay is CONFLICT. A client's own commit must match its pending sequence.
Only that own completion clears pending. A retired epoch is STALE. The fixed
sequencer and clients are trusted; hostile wire bytes require a separate
parser/authentication layer. This protocol has no leader failover or durable
server recovery, and cannot reuse the same epoch after losing the log.

## Required reasoning and counterexamples

Prove tree admission closure, strict-parent acyclicity, deterministic sibling
order, hidden-anchor traversal and staged output. Compare item versus
pending-delete accounting under a missing dependency flood. Derive the scalar
and UTF-8 byte bounds and show why code points are not grapheme clusters.
Explain exactly which barrier assumptions justify tombstone reclamation and
why old message epochs and stale restored snapshots still require authority.

For transformation, define each operation's coordinate context, exact prefix
vector and per-site identity. Induct over retained transformed suffix commits;
then induct over client prefix delivery. The argument establishes this central
prefix protocol under its stated trust/session premises, not arbitrary
peer-to-peer dOPT or every intention-preservation property.

Concrete three-operation counterexample: A inserts 'a' at0 from empty; B
concurrently inserts 'b' at0. After seeing A but not B, A requests 'x' at1.
The sequencer applies A then B, whose transformed position is1. Rebasing x over
that transformed B leaves x at1, producing axb. Applying the individually valid
pairwise rule to B's original position0 instead moves x to2, producing abx.
The actual producer demonstrates both positions. A correct pairwise function
used with the wrong context is still wrong.

## Deliverables, verification and pass/fail

D1: Implement both early-delete representations using the actual sequence
producer; submit invariants, ownership/identity rules, capacity accounting,
Unicode explanation and canonical snapshot derivation.

D2: Implement the three-control compaction barrier and the complete sequencer/
client protocol, including original versus transformed history, exact vectors,
retransmission, no-op deletes, pending ownership and terminal log exhaustion.
Provide the three-operation counterexample before examining its observed output.

D3: Run seq_tree,seq_snapshot,ot_context,ot_delivery. Reject ten compiled
controls by normal RP_CHECK assertions: lost early delete, surrogate acceptance,
ignored outstanding transport, reused compaction epoch, ignored state vector,
omitted rebase, out-of-order client admission, conflicting duplicate admission,
wrong duplicate delete and premature pending release. Configure/build failures,
sanitizer findings and process signals do not count as semantic rejection.
Flip all2,944 snapshot bits and a reserved byte with a recomputed digest. Test
empty, unresolved, identical and differing compaction inputs, finite epoch,
full logs/queues, hidden descendants and output preservation.

D4: Compare96 independent tagged-tree histories/9,312 transitions and96 marker-
token OT histories/15,456 transitions against the actual C producer. The Python
model retains deletion targets by stable token identity and moves an explicit
gap marker through retained edits; it does not call the C transform. Every OT
history has at least three admitted operations. Preserve raw status counts,
canonical bytes, command/toolchain/include/link identities and negative controls.
Run all eight GCC/Clang debug/release/ASan+UBSan/TSan profiles. This serialized
harness has no native competing threads; a TSan pass is not an interleaving proof.

Each R1–R4 criterion requires its reasoning and relevant D1–D4 evidence.
Level0 is missing/contradictory; level1 is nominal output without the causal or
ownership argument; level2 meets the complete stated criterion; level3 solves
an unseen delivery/compaction/context variant and identifies the changed proof.
Require all criteria at2 and the original independent level3 requirements.
Do not award level3 merely for reproducing supplied public traces.

## Hints, delayed variants and security scope

Hints: (1) use the producer's byte as an item token, not the Unicode payload;
(2) an unknown deletion needs retained identity even before its insertion;
(3) a compaction epoch invalidates old anchors only if every actor obeys it;
(4) write the document context beside every original and transformed edit;
(5) retain the exact request until its own commit arrives. Assessor analysis is
in assessor/replicated/sequence_depth/ANALYSIS.md.

X-T76-006-DELAYED changes the disconnected/missing parent, Unicode scalar
boundaries, compaction delivery order, or which writer generates the third
context-dependent operation. The seven-day unaided attempt remains learner
work. Investigate the original paper's decentralized control assumptions and
explain why this constrained prefix protocol neither proves nor replaces them.

The attacker controls only original synthetic message ordering, repeated IDs,
invalid scalar/snapshot bytes and named semantic mutants in a local process.
The properties at issue are history preservation, accepted-state validity,
context-correct edits and single pending ownership. A harmless success condition
is resurrection, wrong scalar output, abx instead of axb, or premature pending
release within this teaching target. Exploitability requires the exact missing
check/premise; a native invalid pointer is outside admitted input. Repair the
invariant, require the matching regression/control, and retain fixed membership,
trusted sequencer, non-durable sessions, finite capacity and barrier limits.
No third-party editor, service, credentials or data is touched.

## Thirteen costs and validation limits

Record (1) actual tree/client/server layouts and368-byte snapshots; (2) inline
value ownership versus unique session authority; (3) reconstruction/traversal,
canonical sort and suffix-transform costs; (4) identity/context/epoch invariants;
(5) queued admission, capacity refusal and unchanged outputs; (6) byte-token
indirection and retained snapshot overhead; (7)16 nodes/early deletes/log entries,
32 document scalars and64 UTF-8 bytes; (8) copying17 document versions versus
replaying a retained log; (9) raw host execution observations without network
latency/WCET claims; (10) actual code/stack/helper audit; (11) actual RPSequence,
rp_ot_insert and SHA integration; (12) ISO arithmetic/typing versus runtime
checks, external proofs and caller/session discipline; (13) changed membership,
failover, Unicode graphemes, optimistic UI and distributed stability alternatives.
A straightforward rebuild makes at most N passes with N candidates and O(N)
producer lookup per candidate, O(N^3) upper bound; actual recursive render scans
siblings over N nodes, O(N^3) loose bound with depth at most16. Canonical sort is
O(N^2). A server rebase is O(L), followed by document and retained-state copying;
this implementation copies the fixed server object, so count that cost explicitly.

Authored executable support; only the matching source-bound report/guard records
actual author runs. Independent human review, learner mastery, remote-network
execution, durable OT recovery and physical validation are not claimed. Published
C17/C23 access gaps remain. No finite test set proves absolute mastery.
