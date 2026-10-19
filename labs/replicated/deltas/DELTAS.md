# Three-replica durable causal delta intervals

This required successor implements X-T76-005 D4 and supplies its original
X-T76-005-DELAYED changed-condition support. Retain R1–R4 and the ordinary
observed-remove proofs. Families A/B/C/D/E/F/G/H/I/J apply to exact tags,
causal admission, failure publication, restart and transfer. Author executions
are not independent learner assessment.

## Prerequisites, references and dependencies

Complete PR-T76-05's actual RPSet producer, PR-T76-03 incarnation reasoning,
X-T76-001's page-store commit/recovery analysis, and the preparatory SHA slice
in [HASHING.md](../hashing/HASHING.md). Import `rp_set_add/remove/merge/contains`,
the containing source's declared DOD dependency, the actual PR-T47-10
`storage.pages` implementation, and the first-principles streaming SHA provider.
No CRDT, database or hash library substitutes for these mechanisms. Learner
dependency workspaces are explicit; assessor sources remain private.

Read R-RP-DELTA: section2's noncorrupt asynchronous-network/durable-transition
model; section6 Definitions4–6 and Proposition2; section6.1 Algorithm2 and its
durable-counter/full-state-fallback rationale. The named passages and rendered
Algorithm2 were inspected. Read the existing R-RP-CRDT observed-remove set
passage. Our fixed capacities, transport epochs, receiver admission checks and
page-store adapter are original bounded additions. They are not a claim that
the paper specifies our wire layout or physical storage behavior.

## Environment and representation

Use ISO C17 hosted Linux,8-bit bytes, exact unsigned widths and GCC/Clang.
All operations are externally serialized. Three original simulated replicas
use separate owned page images and typed local message queues. Delivery may
be lost, duplicated, delayed or reordered. Messages come from identified,
noncorrupt peers; Byzantine alteration is outside this protocol's premise.
The public packet is a typed in-process value, not a portable wire struct.
Canonical snapshots below are byte encodings. Do not label this harness as a
socket transport, a distributed deployment or physical power-failure test.

`RpdSet` composes two actual RPSets: the first supplies writer0/1 slots and the
second supplies writer2 in slot0; second.slot1 remains zero. Each writer has
eight never-reused tags, for24 lifetime adds across values0..3. For each value,
membership is adds minus removed. Tags must not appear under two distinct
values; masks are bounded by known frontiers. A remove delta carries exact
observed tombstones plus known counter frontiers, so it remains structurally
self-contained before its matching add arrives. All tombstones remain retained.

`RpdState` owns namespace UUID, fixed self0..2, positive transport epoch,
uint64 transition sequence and the set. `RpdReplica` exclusively borrows one
already-open actual store, owns that state, a four-entry consecutive delta
history, and three-entry sent/acknowledged/received marks. No dynamic allocation
occurs. Controls and store callbacks are stable, live, aligned, nonreentrant,
externally serialized and not copied while authoritative. Public fields are
inspectable, not a license to change state/history. Packets and acknowledgments
are independent value copies; they retain no buffers. Inputs/outputs are
disjoint, except a set output may exactly equal its set input and `rpd_open`
may replace a quiescent retiring replica control. Set output and delta output
must be distinct. Partial object overlap is invalid.

## Operations, state transitions and persistence

`rpd_set_add` uses the actual selected writer slot and refuses its ninth tag
with FULL. Remove applies the actual observed-remove operation to both blocks.
Join uses both actual OR/maximum merges and verifies the combined tag layout.
All set failures preserve both outputs. Malformed objects/indices are INVALID;
the checks cannot validate forged native pointers or global identity ownership.

`rpd_create` requires a fresh exclusive store with no D0/D1 records. Existing
records return STATE, preventing accidental counter/epoch reset. It commits
epoch1,sequence0 and empty set before publishing the replica. Namespace is
nonzero and the fixed writer identity must be unique among live replicas for
that namespace. A replacement deployment must preserve its durable tag
frontiers; choosing a new namespace is an explicit administrative operation,
not an excuse to reinterpret surviving old messages.

`rpd_open` loads the canonical state, requires matching namespace/self, refuses
epoch UINT64_MAX and commits epoch+1 before returning authority. It retains
the set, tag frontiers and transition sequence, but clears volatile delta and
acknowledgment history. Old controls are retired before recovery; an uncertain
open does not grant authority. A second open attempt may advance the epoch
again. This is acceptable: epochs need uniqueness and monotonicity, not density.

Every operational API checks that the control's canonical durable state matches
the actual store. A stale copied control is STATE. This extra guard does not
make concurrent owners legal. An already-active store transaction or poisoned
store is refused without aborting someone else's transaction.

`rpd_local(add,value)` creates a delta; even an empty successful removal is
one local transition. Before publication, stage set and sequence+1, refuse
UINT64_MAX, then commit both together. On success append the delta indexed by
the prior sequence to the volatile history. If it is full, evict the oldest
delta only. Evicting transmission history is safe because a later sender can
fall back to full state; it does not remove CRDT tombstones. Failure preserves
the replica's published state/history, although the store may be poisoned and
the new durable state may have committed. IO/UNCERTAIN requires store recovery,
then open; never reinterpret unchanged memory as durable rollback.

The128-byte snapshot is RPD1,version1,self,zeros6..7,UUID BE64 at8,epoch BE64
at16,sequence BE64 at24. The first set starts at32 and the second at56;
each occupies24 bytes: four BE16 add masks, four BE16 removed masks, two
one-byte frontiers, six reserved zeros. Bytes80..95 are zero. Bytes96..127
are SHA256 of bytes0..95. Decode checks all layout/state/hash constraints
before publication. Two64-byte values at keys{'D',0}/{'D',1} are written in
one real store transaction. The imported successful-flush contract and actual
redo protocol provide the stated atomic model; the hash is not authentication.

## Causal interval and session protocol

`rpd_send(peer,current_peer_epoch)` obtains the peer epoch from an identified
current session. A matching acknowledgment at the local current sequence yields
EMPTY and leaves output unchanged. Otherwise it sends `[ack,sequence)` joined
from the retained consecutive deltas, if the peer's acknowledged epoch matches
and that start remains retained. Missing history, a new peer epoch or an older
acknowledged prefix selects the entire current state with full1,start0.
Successful send records the largest sent endpoint for that peer session before
returning the copied packet. The containing local state, endpoint and epoch
are from one serialized operation; there is no hidden asynchronous sender.

`rpd_receive` checks namespace, distinct sender/target, current target epoch,
positive epochs, interval order, canonical set and full/start relation. Older
sender epochs are STALE. A non-full interval requires an already-known matching
sender epoch and start at most the receiver's joined endpoint; otherwise it
returns CAUSAL. This explicit check complements the sender's acknowledgment
discipline. Full state initializes the new sender-session mark. It joins into
current state; if the state grows, the real store commits set+sequence before
any ACK is published. If the body is already subsumed, no new transition is
needed. Received marks advance by maximum only after successful admission.

The ACK identifies namespace, sender/recipient identities, both session epochs
and joined endpoint. `rpd_ack` accepts only current local/peer epochs and an
endpoint no greater than the recorded sent endpoint or current sequence.
Duplicate/out-of-order ACKs take maximum; delayed ACKs from a retired session
cannot skip new history. Lost ACKs may cause duplicate interval/full delivery,
which is harmless under idempotent joins. On restart, lost receiver marks force
a full-state handshake; durable state still contains every previously
acknowledged update. No caller may supply an invented current peer epoch as
though the session had authenticated it.

Derive the causal condition directly: the receiver must subsume the sender
state at the interval's start. An ACK is issued only after durable admission,
so the sender's chosen prefix is retained across receiver restart; if its
volatile mark is lost, the new epoch forces full state. Sequence does not reset
across sender restart, and old session ACKs are rejected. The independent oracle
checks actual set inclusion against the sender's stored prefix, not merely
equality of numeric ACK fields. Eventual convergence still requires sufficiently
many successful exchanges after operations stop; no finite disconnected
schedule or bounded execution establishes fairness.

## Deliverables, expected results and verification

D1: Implement the scaffold and explain exact tag allocation, two-block mapping,
observed removal, remove-before-add and concurrent third-writer add. Prove the
24-tag bound and retain tombstones instead of asserting an unproved stable
frontier. An old add cannot resurrect a removed tag with retained history.

D2: Implement canonical snapshots, real transactional publication, current
authority checks and durable epoch/sequence preservation. For a successful
operation report store writes/flushes separately from volatile history cost.
An incomplete/uncertain commit gives no usable new output or ACK. Store
recovery can yield the old or new state as the declared persistence model allows.

D3: Run sets/wire/durable/protocol anchors and ten compiled semantic controls.
Flip all1,024 snapshot bits individually, plus a reserved-byte change with a
recomputed digest. Inject each of37 actual write/flush callbacks for local
update, received update and restart, using both discard-pending and
persist-all-pending outcomes:222 cuts. After recovery, only cuts at commit
write36 or final flush37 with pending persistence may expose the new transition.
Every failed operation preserves caller-visible state/output/ACK. The initial
pilot accidentally included store-open recovery writes in the next fault
counter; it was corrected to reset the counter at the operation boundary before
collecting these results. A matching final report, not the pilot, is evidence.

Inject durable sequence/epoch values at UINT64_MAX-1, explicitly as test setup
rather than a claim to execute that many transitions. Observe one last valid
advance, then unchanged refusal without a write/flush or consumed add tag.
At sequence exhaustion a subsumed received body still permits an ACK; growing
state does not. Refuse a stale copied authority and a saved old-sender-epoch
packet after a new full-state handshake with the receiver epoch unchanged.

D4: Use the independent Python tagged-set/session model and actual C/store
driver:32 seeded histories,3,104 transitions,9,312 canonical snapshot
comparisons and413 independent causal-base checks. Deliver delayed messages,
drop ACKs, reopen either side, exhaust retained delta history and require full
fallback. Preserve all status counts and raw commands/source/toolchain/include
closures. Explicit protocol anchors separately force a causal-gap refusal.
Keep GCC/Clang debug/release/ASan+UBSan/TSan results distinct; this synchronous
model contains no native competing threads, so TSan is not a distributed proof.

## Security scope and counterexamples

Only original in-process replicas, synthetic namespaces and owned page images
are targeted. The adversary schedules old messages/ACKs and the named storage
failures, without forging peer identities or altering the process's pointers.
The deliberately flawed tombstone experiment clears both add and remove masks
while keeping the frontier, then replays a saved original add: membership
resurrects. That wrong local set result is the harmless success condition.
Repair by retaining tombstones, or separately prove a global stable frontier
covering every possible old sender/message before reclaiming them. This
implementation chooses retention and quantifies finite exhaustion.

Other controls omit tombstone merge, remove unobserved future tags, ignore
checksum/reserved bytes, publish state or ACK before commit, suppress full-state
fallback, retain a restart epoch, ignore the causal condition or accept retired
ACK epochs. A normal semantic failure is required; a crash, timeout, compile
error or sanitizer finding is not an intended kill. Retain residual noncorrupt
peer, session identity, global writer uniqueness, key-free checksum and physical
durability limitations. No code execution, external attack or Byzantine
consensus claim follows from these demonstrations.

## Assessment, hints, variants and thirteen costs

Use R1/R2 for the set/lattice and causal proof, R3 for ordinary plus expanded
boundary evidence, and R4 for durable third-replica/session support. Level0 is
missing/wrong behavior; level1 passes nominal cases but cannot explain the
admission/publication invariant; level2 satisfies D1–D4 and all stated failures;
level3 independently changes the disconnected peer, delivers old ACKs after
both sides reopen, and re-derives full fallback and ownership. Require every
criterion at2; record assistance and delayed unaided work separately.

Hints: (1) distinguish CRDT tag counters from anti-entropy sequence and transport
epoch; (2) an ACK certifies a durably joined prefix; (3) dropping a volatile delta
can be repaired by full state, but dropping a tombstone may lose semantics;
(4) model sender-prefix inclusion independently of numeric counters. Separate
assessor reasoning is `assessor/replicated/deltas/ANALYSIS.md`.

For X-T76-005-DELAYED use an unseen three-node partition/restart/delivery order
and change which peer loses volatile history. Preserve original IDs and learner
delay; author runs supply support only. Revisit assumptions if storage can
roll back a successful flush, writer IDs are reused, messages are corrupted,
or peers are permanently removed without a stability proof.

Thirteen cost dimensions: (1) measure actual two-set/state/replica/packet/ACK
layouts; (2) exclusive store/control ownership and copied messages; (3) fixed
four-value joins and at most four retained deltas; (4) monotone tags, sequence,
epoch and exact causal prefix; (5) transactional refusal and uncertain durable
outcomes; (6) delta versus full-state work including hash and store operations;
(7) bounded inline history and24 lifetime tags, with no hidden allocator;
(8) sequential small bitmaps versus repeated4096-byte page traffic;
(9) host execution samples are not network latency/WCET or physical flush
durability; (10) compiler code/stack/helper audit; (11) actual RPSet, SHA and
storage composition; (12) ISO arithmetic versus runtime guards, mathematical
reasoning and peer/storage premises; (13) changed session/storage/fairness
assumptions and alternatives to indefinite tombstone retention. No native
scheduler or asynchronous buffer lifetime exists in this typed local transport.

## Validation scope

Authored executable support; final status is in a matching
`replicated_deltas_validation.json` and its guard. No independent human review,
learner mastery, physical power-cut, real network or target-board run is
claimed. Published C17/C23 access gaps remain. The finite oracle and commit
cuts support the declared model, not all physical storage failure subsets.
