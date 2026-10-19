# Causal clocks, membership and durable restart identity

This required successor supplies X-T76-003 D4 and X-T76-003-DELAYED support.
Retain the original four rubric identifiers, ordinary clock exercises, source
passages and delayed unaided assessment. The new three anchors are
`clock_identity`, `clock_policy` and `clock_store`; six private compiled controls
test distinct failures. The public `oracle` driver supports an independently
derived causal graph and canonical-byte oracle.

## Prerequisites, environment and problem

Complete PR-T76-03's Lamport, vector and hybrid logical clock arithmetic and
retain X-T76-001's durable-store reasoning. Import the actual `rp_lamport` and
`rp_hlc` producers, the retained DOD dependency required by their containing
source file, and `storage.pages` from PR-T47-10. The durable adapter calls the
real store transaction/commit/recovery interfaces. The clock arithmetic,
membership representation and snapshot format are implemented locally; no
distributed clock or database library substitutes for them.

Use C17 hosted Linux,8-bit bytes, exact uint32/uint64 types, GCC and Clang with
warnings treated as errors. The core is externally serialized. Inputs and
outputs are live, correctly aligned and disjoint; null remote means a local
event. Clock values contain no owned pointers and may be copied. A copied
snapshot does not become current durable authority merely because its fields
are well formed. Byte I/O, storage durability and clock synchronization premises
are separate from ISO C rules. Host diagnostic tools are not target dependencies.

Implement a clock containing nonzero namespace UUID, nonzero self identity,
uint64 Lamport counter, the actual `(uint64 physical,uint32 logical)` HLC, and
up to six sorted `(identity,uint64 counter)` vector components. Identity tags
remain attached to components when membership changes. Provide make, step,
replace, vector order, canonical encode/decode, save/load and two durable
publication wrappers. Derive preconditions and unchanged-output rejection
before coding.

## Identity, order and boundaries

`rpc_make` initializes one zero component and zero clocks. All identities must
be globally unique within the provisioned UUID. External provisioning must not
allocate one identity to two live writers or reuse an old incarnation. This
finite exercise implements retention and durable admission; it does not invent
a consensus service that can allocate identities through arbitrary partitions.

`rpc_step` joins remote components by identity, creates any missing components
within the six-entry bound, then increments self. The namespace must match.
The actual Lamport/HLC functions compute the other timestamps. Stage the entire
result and publish only on success. A sixth-to-seventh identity admission is
FULL; scalar/vector exhaustion is RANGE; neither wraps or discards history.
Malformed representations are INVALID; mismatched namespace is IDENTITY.

`rpc_replace` adds a fresh zero component, retains every old component and both
scalar clocks, then changes self. Replacement itself is not a timestamped event.
An already retained identity is rejected, and capacity exhaustion preserves
output. A later local event increments the new identity while retaining causal
knowledge of the old process. The caller cannot delete retired identities merely
because their old writer is absent. Garbage collection requires a stronger
stability/membership argument that this API does not provide.

`rpc_order` treats a missing identity as zero and returns minus one/equal/plus
one/concurrent for componentwise comparison. The test counterexample resets a
fixed numeric slot to zero, emits one event, and obtains the same vector as an
earlier event. Its equality has lost the restart history. The corrected retained
identity representation orders the old event before the new incarnation event.

## Physical assumptions and finite-counter policy

The allowed-ahead argument is a runtime admission limit: if the maximum known
physical component exceeds the current wall sample by more than that amount,
return SKEW unchanged. Subtract only after ordering the unsigned values. It is
not a measurement that clocks are synchronized, and an attacker can fabricate
a timestamp inside this limit. Accepted remote history is a caller trust premise.

After scalar/vector resource preflight, HLC logical overflow returns RANGE under
REJECT. WAIT_FOR_WALL returns WAIT only when a larger physical sample is
representable; it emits no timestamp and changes no state. The caller may
schedule a retry when wall exceeds the maximum known physical component. This
API never busy-waits. At physical UINT64_MAX there is no such future sample,
so RANGE is permanent. A stalled wall makes waiting unbounded; future remote
messages or resource changes still require readmission on retry.

For the supplied model, true time is nondecreasing and every local wall sample
differs by at most2. Messages refer only to prior events. The maximum sampled
physical time in an event's causal past is at most current true time plus2;
its own wall is at least current true time minus2. Hence accepted HLC physical
time is at most4 ahead of the wall and within2 of true time. This derivation
does not survive unbounded backward wall jumps, fabricated remote timestamps,
different clock-error bounds or arbitrary counter wrap. The separate logical
counter preserves lexicographic order while physical time stays unchanged.

Read the registered Lamport Clock Condition/IR1/IR2 and HLC section2/Figure5,
Theorems1–3/Corollary1. Those passages motivate the order/physical distinction.
The finite capacities, rejection policy, identity registry and persistence
protocol here are original exercise constraints, not claims made by those papers.

## Durable authority and wire representation

`rpc_save` is the privileged snapshot constructor/restore primitive for an
exclusively owned clock store. It validates structural/canonical data but does
not authorize arbitrary rollback of previously emitted time. Normal event and
membership transitions must use `rpc_step_durable`/`rpc_replace_durable`.
Those wrappers first load current stored authority and reject a stale local
snapshot, stage the transition, commit it, and only then publish the output.
No timestamp from a failed call may escape. An error may nevertheless have
persisted the new record; recover/load before retry. Unused clock values after
an uncertain commit are permitted gaps, not duplicate emitted events.

The exclusive already-open store contains three keys C+index0..2, with record
lengths64,64,32. A single store transaction updates all three. The160-byte record
is ASCII RPC1,version1,count in byte5, zero bytes6..7, then big-endian UUID at8,
self16,Lamport24,physical32,logical40. Bytes44..47 are zero. Six identity/counter
pairs occupy48..143; unused pairs are zero. Bytes144..155 are zero; bytes156..159
hold a CRC32 over the first156 bytes (initial/final complement, reflected
polynomial edb88320). Validate checksum, reserved bytes, sorted unique identities,
self membership and exact UUID before output. CRC detects the tested accidental
corruptions; it is not authentication or protection against intentional rollback.
Physical flush behavior remains the imported store contract's premise.

## Deliverables, verification and assessment

- D1: Give a causal graph whose reachability implies increasing Lamport/HLC
  values, plus a pair with increasing timestamps but no causal path. Explain
  how vector comparison depends on stable identity and complete modeled edges.
- D2: Implement the staged operations, retained membership and store protocol.
  Prove vector max/join and self increment, scalar order, fixed bounds,
  representation validity, stale-snapshot refusal and commit-before-publication.
- D3: Predict and execute all anchors and six controls. Run the independent
  Python graph oracle:128 histories,48 events each, all144384 earlier/later
  relations, and128 independent canonical byte records. Inject each37 callback
  boundary in both discard/all-persist modes for event and identity transitions
  (148 cases). Recover and verify old or committed new authority before retry.
- D4: Demonstrate slot-reset loss, retained restart identity, capacity refusal,
  same-physical logical overflow under reject/wait, later-wall recovery, and
  permanent scalar/vector/physical exhaustion. Explain the bounded wall model
  and falsify it with the supplied out-of-bound sample. Compare8 scalar bytes,
  12 HLC bytes and16 bytes per explicitly tagged vector component under clearly
  stated identity/framing overhead; none is a sizeof-struct wire protocol.

Run `metrics` to record actual sizeof/alignment and store read/write/flush
counts for the declared transition. Keep compiler dependency manifests, actual
stack-usage/object-size/disassembly output and exact source-bound logs. The
separate assessor analysis explains failures; it must not enter the learner
package. A semantic assertion must reject each compiled control; sanitizer
termination alone is not a valid mutation result.

All original rubric criteria remain mandatory. A nominal clock increment earns
only superficial completion; competence must explain every refusal, storage
outcome and missing premise; independent mastery requires an unaided delayed
membership/restart investigation and a counterexample under changed assumptions.
Hints progress from labeling vector entries by identity, to staging a full
transition, to identifying which durable operation may authorize publication.
Changed-condition support includes an old message after replacement, a stale
saved local snapshot after a successful transition, and overflow that can be
relieved by wall progress versus exhaustion that cannot.

## Security scope and cost review

Only original in-memory page devices and synthetic clock messages are targets.
The attacker may reorder/duplicate records or change identity/clock fields in
the teaching program. The flaw is accepting a reused identity, unchecked drift,
corrupt snapshot or time published before commit. The harmless success condition
is a wrong local causal comparison or repeated/unsupported timestamp; it is not
code execution. Repair identity retention, bounds and durable admission, rerun
the controls and boundary regressions, and retain trusted provisioning, finite
capacity, authenticated transport and stable-storage limitations explicitly.

| Cost dimension | Required mechanism-specific evidence |
| --- | --- |
| Representation | Actual RpcClock/RpcVector size/alignment and sorted-entry invariant versus fixed wire offsets. |
| Reachable storage | Six retained identities, three store records and imported fixed page/cache arenas. |
| Lifetime | Value copies are safe snapshots; durable wrappers refuse stale authority; store/context outlive calls. |
| Mutation/aliasing | Disjoint output staging and external serialization are caller obligations. |
| Allocation | No heap allocation; fixed vector/store capacity fails before publication. |
| Movement/copying | At most six insertion shifts per new identity plus staged clock/160-byte codec copies. |
| Destruction | No dynamic owners or recursive destruction; close underlying store only after users stop. |
| Dispatch | Direct actual clock/store calls and store I/O callbacks; inspect their generated calls. |
| Concurrency | Causal order is message history, not C thread synchronization; no concurrent core calls admitted. |
| Failure |148 event/identity durability cuts and explicit uncertain-success recovery; no physical theorem. |
| Compilation | Recorded compiler flags, build time and actual dependency closure for every profile. |
| Binary/stack | Object size and compiler stack frames, including160-byte staging and imported store frames. |
| Boundary | C integer semantics, finite counters, byte order, identity provisioning, POSIX host and flush model are distinct. |

Author executions, non-author review, delayed learner assessment, physical
hardware validation and access to final published ISO editions stay separate.
