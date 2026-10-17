# Original mechanism laboratory contract

Use ISO C17 with extensions disabled and -Wall -Wextra -Wpedantic -Werror.
Public src files are compiling unsolved learner stubs. Reference solutions and
semantic mutants reside only under assessor/. Explicit assessor selection must
never be used as learner evidence. Python, CMake, compilers and diagnostic
runtimes are host verification tools, not implementations of the taught mechanism.

Unless an operation explicitly states otherwise, all pointer/control/output
objects must be live, correctly aligned, initialized to their required
representation and disjoint from input/control storage. Nonzero extents require
real backing objects of that extent; numerical range checks cannot establish
that a fabricated pointer is valid. Null allowance is stated per operation.
Output failure atomicity refers to admitted inputs. There is no general promise
to diagnose invalid native pointers, data races or lifetime violations.

Views borrow stable backing storage. The caller retains ownership and must not
destroy, relocate or mutate it while the view's validated interpretation is in
use. Value copies of handles/views do not extend lifetime. Public representation
fields remain constructible in C; they are not unforgeable proof objects.
Operations are single-threaded unless explicitly synchronized by an external
contract; adding atomics to one counter does not provide reclamation safety.

No library may replace the specific constructor, validator, property runner,
token expander or graph mechanism being implemented. Declared prerequisite
components may be reused through their documented contracts. At final
composition, retain the actual learner source identities and an adapter test;
merely listing an import does not establish working integration.

All security investigations use only these original local teaching programs,
synthetic bytes and local state. A harmless violation is an invalid input
accepted, a wrong synthetic value returned, or a stated property falsified.
This does not establish arbitrary code execution, privilege escalation, or a
property of any deployed program. Never use third-party services or real data.

Every project requires D1-D4 from its exercise, all thirteen dimensions in
modern-cost-contract.md, a prediction before execution, exact tool/source
identity and independent expected results. Level2 requires the entire written
contract and original implementation/analysis; public tests alone earn at most
level1. Level3 requires a delayed unaided changed-condition attempt after at
least seven days. Record assistance and distinguish self-report from review.
Author campaigns are not learner acceptance, independent human assessment,
physical validation or proof of absolute mastery.

## PR-T76-01: Positioned event logs and persistent snapshot roots

Implement RPLog and rp_log_init/append/get. The original in-memory log contains at most16 events (position1..16,key0..31,int32 value) and17 snapshot roots. Each append first updates the imported persist.hamt arena, then publishes the event and root together. get(position,key) queries the exact retained prefix; position0 is empty. Invalid key/position, log full and injected HAMT allocation failure preserve published state and output. Replay an admitted prefix into a fresh fixed-address arena; values are latest event payload per key, not accumulated balances. No filesystem durability is implied.

Retain the actual PR-T70-07 learner artifact and its64node/3nodes-per-update contract. Prove16 appends use48nodes, every published root remains immutable and an interrupted failed append cannot advance count. Initialization requires no surviving borrowed roots; do not copy/relocate a live arena and interpret roots in another owner. Account for unpublished cells and allocation counters separately from reachable state. State a release/publication order for a future concurrent adapter.

## PR-T76-02: CQRS projection admission and atomic position advancement

Implement RPProjection with sixteen retained applied events, a contiguous count and int64 total of int32 event values. Admit positions1..16 in order. A byte-independent semantic duplicate (same position,key,value) succeeds without mutation; conflicting payload at an old position returns RP_CONFLICT; a gap returns RP_GAP. Publish total, retained event and cursor as one serialized transition. Supplied pthread adapter holds one mutex across this entire call; two threads each deliver the same ordered1..16 stream.

Prove the invariant count=k implies exactly the first k events contributed once and applied[i].position=i+1. A 16-term int32 sum fits int64 on the declared target. Retain history for conflict checks rather than trusting a reused position from a new log. The caller must associate a projection with one log identity. Explain why a mutex around cursor alone is insufficient, and why an external email/file effect cannot become atomic with this state by naming it CQRS.

## PR-T76-03: Lamport, vector and hybrid logical clock boundaries

Implement checked Lamport receive max(local,remote)+1, fixed3-replica vector merge-then-increment and partial-order comparison (-1 before,0 equal,1 after,2 concurrent). Implement HLC Figure5 using uint64 physical and uint32 logical components, local/send events via remote=NULL and receive events via a remote pair. Apply every max/tie branch; backwards wall time is allowed. Counter overflow returns RP_RANGE with output unchanged; do not wrap and silently violate causality.

Show two concurrent events with ordered Lamport timestamps, two incomparable vectors and every HLC equality case. Separate physical-clock closeness claims from the synchronization assumptions in Corollary1. On local overflow choose explicit failure; neither arbitrary sleep nor silently increasing physical time is equivalent. Outputs are ordinary C structs, so validity and per-replica serialization remain caller obligations.

## PR-T76-04: Join-semilattice counters and deterministic registers

Implement RPPN with3 positive and3 negative uint32 components, checked local increments and int64 query sum(P)-sum(N). Merge by componentwise maximum, allowing self-merge. Implement RPRegister merge by lexicographic(timestamp,replica0..2); identical identity with different payload is RP_CONFLICT. A canonical initial register is(0,0,0), and generated writes use positive timestamps. Inputs must derive from one writer per replica identity; public fields are not Byzantine proofs.

Prove idempotence, commutativity and associativity on admitted compatible states, plus inflation of each grow-only component. Explain why sum of received states double-counts retransmission, why finite overflow must reject, and why a reused writer identity with forked state violates the ownership model. For registers identify conflict cases outside the unique-event assumption and the total-order tie rule.

## PR-T76-05: Observed-remove sets and replayable delta state

Implement RPSet for values0..3 and2 replica writers, each with8 never-reused add identities. adds[value] and removed[value] are16-bit tag sets; next[replica] reserves the next local identity. add returns a delta containing its new tag and counter frontier; remove returns tombstones only for tags actually observed. Merge ORs add/tombstone sets and takes frontier maxima. Empty removal succeeds. Exhausted identity space returns RP_FULL; no reset/reuse while any state/delta survives.

Define membership as adds minus removed. Derive why a concurrent unobserved add survives removal and why an observed add cannot resurrect when its delta arrives again. Treat one writer per replica as a premise. The bounded delta carries exact tag tombstones, so remove-before-add delivery is representable; do not infer this for all causal CRDTs. Explain the delta-interval causal precondition from the paper and specify acknowledgment/stability requirements before reclaiming history.

## PR-T76-06: Replicated sequence identities and contextual transformation

Implement RPSequence with16 nodes{id,parent,byte,tombstone}. id is globally unique and positive, parent<id; parent0 is the root and every nonzero parent must already exist. Concurrent siblings render in descending ID order by depth-first traversal; tombstoned parents remain traversable. Duplicate identity with identical parent/value is idempotent; conflicting reuse fails. Missing dependencies return RP_GAP unchanged. Render stages output and fails transactionally if capacity is insufficient. Also implement two-concurrent-insert position transformation with distinct priority tie-breaks; no full network OT control algorithm is supplied.

Derive acyclicity from parent<id and closure from causal admission. Prove insertion-order-independent rendering for a fixed admitted node set, including descendants of deleted anchors. Explain that this original tree contract is RGA-inspired, not a claim to implement every detail of Specification19. For OT derive the two-branch diamond under distinct priorities and identify the state-vector/log context needed beyond it.

## PR-T76-07: Merkle commitments and bounded anti-entropy

Implement local SHA-256 for byte lengths0..65 directly from RFC6234 equations, and RFC9162-style Merkle roots over0..8 uint32 leaves encoded big-endian. Empty root is SHA256(empty), leaf is SHA256(00||four bytes), internal node is SHA256(01||left||right), split at largest power of two strictly below count. Implement recursive equal-subtree skipping and return a bitmask of differing leaf positions for equal-length trees. No target hash library, network service or certificate-transparency implementation may replace these mechanisms.

Derive padding across55/56/63/64/65-byte boundaries, uint32 modular arithmetic, round constants and endian conversion. Prove the tree shape for non-power-of-two counts and that a subtree comparison is meaningful only with the same position/range/domain. Account for hash collision and corrupt-peer assumptions; a matching root alone authenticates no sender and guarantees no retrievability.

## PR-T76-08: Field tags, minimal varints and canonical bytes

Implement RPMessage with up to8 unique fields, tags1..15 and uint32 values. Wire keys are(field<<3)|0 and all values use minimal base128 uint32 varints. Fields1 and2 are required; field1 is an ID and field2 is a ZigZag-encoded signed delta; unknown advisory3..15 fields are retained semantically. Reject duplicates, other wire types, critical tags>=16, overlong/nonminimal/truncated/overflow varints, missing required fields and excessive field count. Decode accepts any field order; encode sorts tags, preserves unknown values and publishes bytes/length only on success. This is a stated teaching subset, not general Protocol Buffers compatibility.

For five-byte uint32 varints require the final payload<=15 and no continuation. Derive ZigZag using unsigned conversion/shift and a sign mask; decode odd values as -(value>>1)-1 to include INT32_MIN safely. Prove canonical decode/reencode idempotence and uniqueness for the admitted semantic map. Explain the tradeoff of rejecting duplicate fields compared with protocols that merge repeated occurrences.

## PR-T76-09: Pure reducers, explicit effects and rollback replay

Implement RPState{tick0..8,position0..15,score0..65535,random:uint32}. rp_step consumes movement-1/0/1, wraps position on a16cell ring, advances LCG random=random*1664525+1013904223 modulo2^32, adds(random&3) with checked score limit and returns RPEffect{tick,score} as data. No clocks,I/O,allocation or callbacks occur in the reducer. rp_replay stages at most8 steps and effects, preserving outputs on failure. Restore the state before a changed input and replay the suffix; compare with a fresh fold of corrected inputs. rp_checkpoint composes two actual HAMT updates for position/score under one root publication and injects failure at each of6allocation ordinals; it is a projection checkpoint, not the full replay snapshot (tick/random remain in the explicit RPState snapshot).

Prove determinism from complete state/input and defined unsigned arithmetic. Identify every effect boundary and distinguish speculative requests from committed presentation. Derive rollback of the HAMT used cursor after a failed second update while preserving old reachable roots; staged unreachable arena cells and allocation counters may differ. All arena users must be quiescent during the transaction, and roots from unpublished intermediate updates must never escape. Score overflow or invalid movement must not partially publish a state/effect.

## Exact shared assumptions and composition

The target has 8-bit bytes, uint32_t/uint64_t and 32-bit int. Arithmetic in
uint32_t is modulo2^32; signed-overflow behavior is never an oracle. The active
edition is ISO C17. C11 already supplies the required unsigned/representation
mechanisms; no C23 feature or stronger guarantee is assumed. Published C17/C23
wording remains an explicit access gap, not an inspected source.

Zero-initialized projection, PN counter, set and sequence are admitted empty
states. Log initialization calls the imported HAMT initializer. Public structs
must remain constructor/operation-admitted: routines do not authenticate forged
control state. PN/set merge allows exact self-alias; other input/output/control
objects are disjoint unless a value parameter makes overlap irrelevant. NULL
data is allowed only at zero byte/element length; output/control pointers are
always live. All functions are serialized core operations. The projection test
has a real pthread mutex around the entire shared transition and joins threads
before destroying storage. This demonstrates that adapter only; unsynchronized
core calls are outside the contract. No callbacks occur in core mechanisms.

Single-writer replica identities cannot be cloned into independent writers.
All log positions refer to one externally identified immutable history. Every
HAMT root belongs to its live stable-address arena. Quiescent checkpoint staging
may roll back arena.used on failure only because no intermediate root escapes;
unreachable cells and diagnostic allocation counters need not be byte-identical.
The persist.hamt producer is linked from an explicitly selected completed
workspace, not reimplemented or silently replaced by an assessor library.

Default source is the unsolved learner scaffold. In an initialized standalone
workspace provide --dependency dod=COMPLETED_DOD_WORKSPACE, which selects
CMASTERY_DOD_ROOT. Author validation explicitly selects assessor implementations
for both families. Common assessor root and family-specific assessor root are
supported; the latter needs CMASTERY_DOD_ASSESSOR_ROOT explicitly. Never count an
assessor-selected run as learner evidence.

The supplied memory anchors execute real HAMT composition. Durable storage.pages,
codec.records, io.frame, ipc.local and the full prior T20 learner reducer adapter
are required named investigations with explicit acceptance obligations, but are
not claimed executed by this campaign. Retain their source identities, interface
adapters, short-I/O/crash/rollback tests and residual limits in learner evidence.
Security success conditions are synthetic invariant violations only; all state,
message and local transport experiments remain confined to original teaching
programs. No remote services, real data or host security changes are authorized.
