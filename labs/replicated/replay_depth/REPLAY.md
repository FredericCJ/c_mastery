# Required rollback replay successor — X-T76-009

This contract extends PR-T76-09 and all four original rubric criteria. It does
not replace the original single-player reducer, six HAMT allocation cuts or
checkpoint proof. Families A–J are used for the mechanisms below, not as a
mechanical exercise quota. R-RP-ROLLBACK's inspected *How Does It Work* passage
motivates complete saved state, prediction and replay without rendering;
R-RP-EVENTS's registered External Updates and Reversing Events passages bound
the effect argument. Neither source specifies this original finite protocol.

## Prerequisites and dependency boundary

Retain X-T76-008 and PR-T76-09's `struct rp_state`, `rp_step`, `rp_checkpoint`, actual
T70 `persist.hamt`, T48 `storage.pages`, and PR-T24-03 `io.frame` parser.
PR-T20-01 covers T20.03: compose its actual `ar_double` through `ar_decide`,
`ar_retry`, `ar_feedback` from the T20.04 effect adapter. Do not replace those
implementations by lookalike local helpers. The pure session stage needs only
the reducer and effect decision code. Durable and process stages additionally
need X-T76-001's `struct rpl_pages`/`struct rpl_log`, the completed SHA-256 exercise, and the
learner's local-process I/O skills. Their public interfaces live in
`include/replicated.h`, `hashing/hashing.h`, `followthrough/durable.h`,
`../storage/include/store.h`, `../api_depth/architecture/{arch,effects}.h`, and
`../hosted_systems/stream_depth/stream.h`. First-principles implementations are
required; host Python `hashlib` is an independent oracle, never a target library.
Assessor source is selected only in author campaigns. Exported learners supply
completed workspaces through the explicit CMake `CMASTERY_*_ROOT` variables.

Use ISO C17, 8-bit bytes, 32-bit unsigned/int and exact uint32/uint64 types.
The hosted adapter uses POSIX `socketpair`, `fork`, `pread`, `pwrite`,
`fdatasync`, `poll` and wait status; these are not ISO C guarantees. The
PowerPC64 Linux ELFv2 experiment uses a static freestanding image, a tiny raw
write/exit assembly harness and explicit audited memory helpers. It tests byte
order under emulation, not an operating-system-free board or physical device.
C11 draft text is not relabeled as the inaccessible published C17 text.

## State and operations

Implement the public `replay_depth.h` interfaces in the five supplied compiling
stubs. A session owns a nonzero timeline, nonwrapping uint32 revision starting1,
head0..8, horizon1..8, all input history and nine complete two-player snapshots.
Each initial player is `(tick0, position0..15, score0..65535, explicit seed)`.
Every player transition calls actual `rp_step`. Local/remote moves are -1,0,1.
An unknown remote input predicts the latest preceding known remote move,
initially0. A correction changes that tick and all subsequent unknown
predictions, restores the preceding complete snapshot and recomputes the suffix.
The confirmed prefix stops at the first unknown input. Advance refuses WAIT
when `head-confirmed==horizon`; it does not evict unconfirmed history. A full
eight-tick timeline refuses FULL. A fresh timeline is explicit, not wraparound.

Repeated known input with the same timeline/tick/value succeeds without revision
change; a different value conflicts. Foreign timeline is STALE. Unknown input
outside the retained horizon is STALE. Revision MAX permits duplicate input and
effect cleanup, but refuses a new advance/correction. Score overflow refuses
without publishing session or notice. Native controls are live, initialized,
disjoint objects, externally serialized; no callbacks may reenter or retain
borrowed storage. Invalid pointer fabrication is outside the API. Nullable
required pointers reject before access where specified by the implementation.

`struct rpr_notice` replaces speculative presentation for inclusive `[from,to]`; from0
means no replacement. It includes old/new revision and final presentation
values. The adapter discards the old speculative range before presenting the
new one. This is replacement data, not revocation of already committed effects.
The presentation integer is `pos0+16*pos1+256*(score0+score1)`; actual
`ar_decide` doubles it with its checked arithmetic. Only a confirmed tick may
allocate effect identity `(timeline,tick,value)`. A pending prepare returns the
same request. Exact successful feedback clears pending; failure/pending retains
it and returns IO. Feedback checks all identity fields. No core operation calls
an external effect, time, randomness, allocator or I/O.

## Canonical snapshots, transport and durability

The snapshot is exactly512 bytes. Prefix0..479: `RPR1`, version1, horizon/head/
confirmed at5/6/7; timeline BE64 at8, revision BE32 at16; initial positions20/21,
scores BE16 at22/24, seeds BE32 at28/32. Local+1, effective remote+1 and known
bytes occupy36..43,44..51,52..59, with unused entries zero. Effect next BE32 at60,
pending at64; pending ZigZag value/id BE32 at68/72, otherwise zero. Snapshots
0..head start80, each two `(tick,position,score,seed)` BE32 tuples. All remaining
bytes are zero. SHA-256 of the prefix is stored480..511. Decode validates digest,
reconstructs the complete history and compares canonical encoding before output.
This detects inconsistencies; unkeyed SHA does not authenticate an adversary.

Remote input is16 bytes: `RPI1`, timeline BE64 at4, tick1..8 at12, movement+1 at13,
zero14/15. Actual `struct rf_parser`/`struct rf_message` carry it in exactly two eight-byte frames
with one-byte lengths:18 wire bytes. Reject alternative segmentation, truncation
and trailing bytes. Stream harness reads one18-byte record at a time; it does
not confuse a partial `recv` with message end. Each local callback may accept a
prefix; finite retry/call limits are explicit. No borrowed input survives a call.

`struct rpr_durable` exclusively borrows an already-open store with eight reserved
two-byte keys `{'R',0..7}`, each holding64 snapshot bytes. Create requires those
keys absent. Every operation verifies stored canonical state against its local
copy, stages the transition, writes all eight keys in one actual store transaction,
then publishes session/output. An unchanged retry performs no commit. A failed
or uncertain transaction leaves local outputs unpublished; it may still have
persisted. The poisoned store must be reopened and its state loaded before retry.
The test's pre-checkpointed transaction has37 write/flush boundaries. A preceding
nonempty WAL can cause extra checkpoint I/O before the transaction;37 is not a
universal operation cost. Failure at a supplied page write models discarded or
fully persisted accepted writes, not arbitrary physical sector behavior.

`rpr_sink_apply` consumes a separate, live, exclusive `struct rpl_log` with matching
timeline and schema `0x52505245`. Its prefix holds SET events key=tick1..8 and
the exact even effect value. Admit only the next tick; exact historical retry
returns `newly_applied=0`; changed content conflicts; a gap is STALE. Durable
append precedes `newly_applied=1`. The log may not be copied/relocated and its
borrowed store must remain live; no other writer mutates it. This is an
idempotent durable intent ledger, not exactly-once audio, network or device work.

## Concrete experiment and deliverables

D1 implement/invariant ledger: use initial players `(0,0,0,1)` and `(0,7,0,9)`,
local `[1,0,-1,1]`, remote `[-1,1,0,-1]`. Predict all four before delivering remote
ticks2,1,4,3,2. After tick2, unknown ticks3/4 predict1 and remote position at4 is10.
Confirmed prefix progresses0,2,2,4; the final duplicate changes nothing. Show
equivalence to immediate complete-input replay, while presentation notices differ.

D2 publish/failure dossier: preserve outputs on range/WAIT/conflict/refusal;
show pending confirmed effects cannot be invalidated by later corrections.
Exercise revision MAX, score overflow, zero/full states and all296 transaction
cuts: four operations ×37 boundaries × two persistence policies. A failed final
flush may leave the new durable state despite unchanged local state. Retain the
original six HAMT allocation cuts and prove old roots remain valid on rollback.

D3 falsification: run six anchors, twelve separately compiled semantic controls,
all9,841 ternary histories length0..8 plus768 independent two-player arrival
orders against Python integer/byte construction; compare every predicted,
corrected, pending and acknowledged snapshot. Flip all4,096 snapshot bits and
reject an altered reconstructed snapshot even after recomputing SHA. Inspect
actual undefined symbols/call sites. The deliberately flawed hidden-state
reducer must produce different replay output; direct external-effect repetition
must increment its synthetic counter twice. These counterexamples do not imply
memory corruption or arbitrary code execution.

D4 transfer: run the two-process AF_UNIX pilot with a barrier delaying corrections,
three-byte transport progress and short positional file callbacks. Terminate the
sink child with exit42 after durable append but before feedback; reopen both
stores and demonstrate stable retry without duplicate intent. Execute128 oracle
traces in an actual big-endian PPC64 ELFv2 image at O0/O3, inspect ELF/link maps
and undefined symbols, and reject the deliberately wrong-endian serializer.
Retain a separate counterexample for an external effect outside that transaction.
Record machine, tool versions, exact commands, raw evidence, source hashes and
every limitation. Sanitizers and finite checks are evidence, not proofs of mastery.

## Assessment, hints and changed conditions

R1 passes only with actual imported decisions and a complete effect/lifetime
boundary; a duplicate-free nominal run is superficial. R2 requires the suffix,
confirmed-prefix and commit arguments plus the original HAMT transaction proof.
R3 requires independently predicted bytes/failure results and meaningful mutant
rejection, not a process crash. R4 requires actual composition and changed-target
observations, with physical claims withheld. Level3 requires an unaided changed
case with the prior proof revisited; the author campaign earns no learner level.

Hints: (1) list every bit needed to reproduce a step, including seeds and effect
progress; (2) distinguish known from predicted input even when their values agree;
(3) stage the canonical state and effect identity before any commit, then separate
uncertain persistence from caller publication. Separate assessor analysis is in
the private package. Delayed X-T76-009-DELAYED remains at least seven days after
assisted work: a sealed arrival order, horizon and ambiguous feedback point must
be solved anew. Support is supplied for these changed assumptions; its execution
cannot stand in for the learner's delayed attempt.

## Cost, enforcement and security boundary

Submit all13 dimensions from modern-cost-contract.md: representation (fixed
history/512-wire), operations (suffix work plus validation full-fold), invariants
(known prefix), ownership (store/log/borrow graph), failure (uncertainty), lifetime
(explicit timeline/reset), allocation (none in core, actual persistent arena in
sink), layout/traffic (native sizes versus wire), complexity (O(head) validation
and suffix; constant finite cap does not erase scaling), dispatch (direct core,
callback page backend), scheduling/latency (blocking local harness not deadline
guarantee), integration (real T20/T24/T48/T70), enforcement (C types and checked
arithmetic versus discipline, external source/link audit and runtime checks).
Measure seven repeated CPU samples of equal eight-tick workloads; include stack,
code and link output. No physical latency or purity theorem follows from samples.
Contrast explicit state with language-level pure/effect types: ISO C cannot forbid
hidden globals or I/O, as the hidden-state negative specimen demonstrates.

Security target is only these original local processes, private unlinked files
and emulator images with synthetic data. A teaching adversary can reorder,
duplicate, corrupt or omit framed inputs and select failure boundaries; no
external service, real credentials or deployed device is touched. The defect is
premature/duplicate publication or unauthorized history acceptance under a missing
identity/canonical/commit check. A harmless success is a wrong synthetic state or
counter value. Preconditions include the exact weakened check and valid local
object lifetimes; a sanitizer finding is not demonstrated code execution. Repair
the specific invariant, retain rejection/regression controls, and state that
unkeyed hashes, trusted-peer transport, finite horizons and external effects remain
limits. Hardware flashing and host policy changes are outside this exercise.

## Evidence status

Author support is implemented; exact executed configurations are recorded only in
matching source-bound host and target reports. No physical run, independent human
assessment, learner mastery or seven-day delayed attempt is inferred. The target
experiment executes the pure/session/codec slice; POSIX persistence remains a
separate hosted experiment. Published C17 text access remains an explicit gap.
