"""Concrete thirteen-dimension T76 analysis bound to observed author reports."""
import json
from pathlib import Path
from common import ROOT,dump,sha
DIMENSIONS=['Representation','Reachable storage','Lifetime','Mutation/aliasing','Allocation','Movement/copying','Destruction','Dispatch','Concurrency','Failure','Compilation','Binary/stack','Boundary']
DATA={
1:('followthrough','replicated_durable_current_validation.json',[
'struct rp_log retains16 events/17 roots; struct rpl_log adds UUID/schema, kinds and a borrowed store. Canonical events64 bytes; page format4096 bytes,67 pages.',
'Each live root shares old HAMT nodes in a bounded arena. Snapshots store event position and explicit present-mask/value projection; the authoritative prefix still owns event records.',
'Create/open requires a live exclusive store. The log control/escaped roots may not relocate. Recovery constructs an unpublished index before exposing it.',
'All calls serialized; no external store writer while a log is live. Snapshot validation checks prefix authority before publication, not merely a checksum.',
'Actual d_hamt_update allocation cuts and store page transactions are composed. A failed append restores count/used/root authority; unreachable scratch bytes are not promised identical.',
'Encoding copies64 canonical octets per event; recovery rebuilds index roots from admitted events. Snapshot loading compares declared contents against authoritative history.',
'No heap destructor in the finite log; closing or recovering the backing descriptor/store remains an adapter obligation. Resetting memory does not erase durable records.',
'Direct real HAMT/record-codec calls, indirect byte read/write/flush callbacks. Prefix callbacks are bounded per page; imported helpers remain in compiler/link closure.',
'Exclusive writer/serialized recovery model. It is not a concurrent shared-store transaction protocol or distributed consensus.',
'All139,502 append and139,502 recovery byte/flush prefix cuts are specified and executed in the current deep report; uncertainty can poison state and requires reopen.',
'followthrough','replicated-durable','RPL1 UUID/schema/position and checksummed store generations are explicit. A checksum is not authentication and a Linux file experiment is not physical power-loss evidence.']),
2:('outbox','replicated_outbox_validation.json',[
'Sender source, receiver inbox and acknowledgment log are three distinct struct rpl_log/store objects. Request64→72 framed bytes; reply24→27.',
'Historical source/inbox/ACK prefixes retain identity and previous query results. Rebuildable read totals do not replace authoritative event history.',
'Local AF_UNIX descriptors and exclusive file stores outlive each synchronous request. Child termination preserves only what the actual store/file contract committed.',
'Controls and backing stores are distinct, live and serialized. Exact original key/value/kind must agree on duplicate identity; copied controls are not new authorities.',
'Actual durable/HAMT failure paths are retained. Host file creation and process creation are harness resources, separately cleaned up and never target-library shortcuts.',
'Transport owns staged complete request/reply bytes through prefix reads. Read output publishes only after exact frame completion/EOF; partial external progress is not rolled back.',
'Close descriptors after children reaped and I/O stopped. Durable inbox/ACK history is intentionally retained; dropping it changes replay safety.',
'Actual struct rf_parser and struct rpl_log imports plus bounded POSIX calls. The test file backend loops over explicit accepted prefixes and fdatasync outcomes.',
'Two original local processes and failure/retry boundaries, not concurrent arbitrary clients. Original projection pthread tests separately serialize through a mutex.',
'Sender commit, receiver commit before reply and sender before acknowledgment process exits are distinct; external effect counterexample remains duplicate-prone outside the receiver transaction.',
'outbox','replicated-outbox','UUID, position, content and historical total correlate trusted peers; these checks are not cryptographic origin authentication or exactly-once unrelated device effects.']),
3:('clocks','replicated_clocks_validation.json',[
'Finite retained identity/vector control; actual size/alignment observations accompany8-byte scalar,12-byte HLC without identity and per-identity vector growth.',
'Replacement retains old incarnation components rather than resetting a reused slot. Fixed capacity limits the number of lifetime identities.',
'Durable incarnation creation precedes publication. Exhaustion retires identity/counter capacity; restart does not silently reuse timestamps.',
'Serialized control, disjoint staged outputs and exact stored-current canonical comparison. Caller-owned historical copies cannot mutate the current store.',
'Clock arithmetic allocates no heap; durable encoding uses the actual page store and injected failures. Growth refusal preserves the current identity vector.',
'Canonical identity-bearing snapshots are copied/staged; vector comparisons inspect all retained identities. A scalar wire size is not a substitute for its causal information.',
'No recursive destructor; retained causal history cannot be discarded without a separate stability argument. Closing the store is outside the scalar-clock primitive.',
'Direct checked arithmetic and imported rp_hlc; persistence is through declared page callbacks. No hidden wall clock is read by the pure update.',
'Independent graph reachability supplies the causal oracle. A timestamp order is not a proof of happens-before; actual histories expose nonconverse pairs.',
'Scalar/vector MAX refuses before HLC WAIT; logical overflow may wait only below physical UINT64_MAX. Reopen resolves uncertain incarnation persistence.',
'clocks','replicated-clocks','Explicit wall/skew/rate premises bound HLC closeness; process-local numerical time is not automatically portable or synchronized physical time.']),
4:('membership','replicated_membership_validation.json',[
'Six identity-tagged components implemented through two actual struct rp_pn blocks; canonical snapshot160 bytes and authenticated component packet64 bytes.',
'Retired identities remain in state. Compared with a per-event log, retained state grows with lifetime writer count, while uncheckpointed deduplicating logs grow with event count.',
'Administrative replacement chooses a fresh identity; revocation affects future admissions and does not erase already joined contributions.',
'Serialized counters, immutable packet bytes during admission and trusted permit/key memory. Raw public fields are not tamper-proof objects.',
'No heap allocation in joins; sixth identity capacity is real. Union preflight prevents partial seventh-identity publication; key/permit storage remains caller-owned.',
'Sorted identity union stages a complete result; snapshots encode canonical values and MAC packets bind exact interpreted bytes. Native padding is never signed.',
'No asynchronous destructor. Key clearing is scoped to explicit live spans, and erasing one copy cannot prove all secret copies gone.',
'Actual PN joins and first-principles SHA/HMAC calls. Runtime MAC/identity/cap checks add work absent from an unchecked merge; compare equal policy contracts.',
'External serialization and unique writer ownership are required. Two independent writers reusing a component lose intended increments even when merge obeys max laws.',
'Tag failure, wrong namespace/writer, policy cap, revocation, capacity and overflow preserve state. Authenticated UINT32_MAX demonstrates semantic entitlement is separate from authentication.',
'membership','replicated-membership','RPA1 schema, namespace/writer binding and finite cap are explicit. Synthetic keys/permissions are a local teaching policy, not a complete Byzantine membership service.']),
5:('deltas','replicated_deltas_validation.json',[
'Three trusted replica identities,24 lifetime add tags, retained tombstones, four delta-interval history slots and canonical128-byte durable state.',
'No tombstone garbage collection is inferred. Every removed tag is retained; queue eviction causes full-state fallback rather than silently dropping causal history.',
'Durable incarnation/counter identity changes on reopen. Acknowledgments name an incarnation/sequence prefix; old acknowledgments cannot authorize a new-epoch interval.',
'Externally serialized typed packets and controls. Incoming interval requires the actual causal base already incorporated; receipt is not proof merely from a larger sequence number.',
'No core heap; finite tag/history/counter capacity refuses explicitly. Persistent epoch/state changes use the actual store transaction and fault injection.',
'Intervals copy a bounded retained union; fallback sends the full state. Cost counts include retained tombstones and canonical wire snapshot, not just live set elements.',
'Clearing tombstones without stability resurrects an old add. The implemented choice retains them; no distributed GC or failure detector is claimed.',
'Actual struct rp_set component operations and page callbacks. Independent mathematical tagged-set/history model supplies the oracle.',
'Disconnected/restarted three-replica schedules are finite modeled transitions; no asynchronous transport/liveness guarantee or consensus is implied.',
'Full history, stale incarnation, missing causal base, duplicate delivery and counter MAX have separate outcomes; unchanged duplicates need no fresh sequence.',
'deltas','replicated-deltas','Typed packet delivery has explicit trusted membership. It is not a wire parser or authenticated hostile-peer transport; those mechanisms are separate exercises.']),
6:('sequence_depth','replicated_sequence_depth_validation.json',[
'Sixteen tree nodes plus16 early-delete slots; alternate placeholders consume node capacity. OT retains16 operations/17 contexts and32 Unicode scalars.',
'Tree descendants survive ancestor tombstones. OT context vectors and transformed request log remain necessary until the stated compaction/quiescence boundary.',
'Compaction changes a nonwrapping epoch only after all three replicas agree and transport is drained. Old messages reject; a timeout alone is not quiescence.',
'Serialized tree/server/client controls; unique immutable IDs and valid Unicode scalar payloads. State/output commit together; unadmitted events preserve prior state.',
'No heap in these finite structures; early dependencies, node/history/full document capacities are distinct failure points.',
'Snapshot368 bytes includes digest; compaction produces a new live chain/identity map. The queue-versus-placeholder comparison charges different retained metadata.',
'Recursive rendering depth is bounded by16 admitted nodes. Tombstone compaction is not safe destruction until the explicit old-message lifetime premise holds.',
'Actual struct rp_sequence ordering and rp_ot_insert are composed with context control; multi-operation transformation uses already-transformed committed suffix operations.',
'Fixed three clients, at most one pending request/client and a central serialization point. Pairwise OT equations do not prove a general distributed protocol.',
'Missing parents, delete-before-insert, conflicting duplicates, invalid scalars and capacity are transactional. Three-operation correct axb versus context-blind abx is an executed counterexample.',
'sequence_depth','replicated-sequence_depth','Unicode scalars are not normalized grapheme clusters. Epochs/context vectors state the admitted protocol identity; no durable OT log or Byzantine peer claim.']),
7:('merkle_repair','replicated_merkle_repair_validation.json',[
'Sixteen leaves of0..32 bytes, trusted domain/count/root commitment, proof depth0..4. Request24→27 bytes and reply96→108 framed bytes.',
'Entire candidate tree and proof siblings remain local until validation. Repair cannot publish a partially filled tree after a missing or inconsistent peer response.',
'Synchronous callback borrows request/response buffers only for the call. The trusted root must remain fixed across the full reconstruction.',
'Disjoint live state/output and serialized callbacks; malicious synthetic peer controls response bytes but not the trusted commitment.',
'No heap allocation. Fixed staging and proof depth bound stack; encoded capacity failure is explicit rather than a hidden allocator fallback.',
'Selective traversal skips only proven-equal ranges; full node and direct leaf modes reconstruct the same committed result. Counts include framing and final root verification.',
'No asynchronous teardown. Recursive tree/proof work has depth at most4 for16 leaves; callback completion is a prerequisite to buffer reuse.',
'Actual SHA primitive, recursive tree hash and actual struct rf_parser framing. Hash counters are client-side; timing includes the peer callback computation.',
'Serialized synthetic peer interaction, no concurrent target mutation. No network consensus or availability guarantee follows from proof validity.',
'Malformed/truncated/extra/reordered path, wrong domain/count, absent node and inconsistent payload reject unchanged target/output.',
'merkle_repair','replicated-merkle_repair','RFC-style raw tree and explicit original domain/count envelope differ. Finite tests cannot prove collision resistance or authenticate where the trusted root came from.']),
8:('wire_depth','replicated_wire_depth_validation.json',[
'Arena16 records×8 fields, global fields<=32, raw bytes<=256, depth<=4 and opaque bytes<=32. Complete framing includes an explicit empty terminator.',
'Stream owns partial bytes and parser state; decoded records own unknown opaque fields. A field-wise reconstruction that copies only known fields loses future information.',
'MORE retains partial owned state; done/failed/cancelled phases are explicit. Init resets only after the caller has stopped feeding the former stream.',
'No retained borrowed input; output published only after complete validation. Mutable caller input after feed cannot alter already copied prefix bytes.',
'No heap; global arena/field/byte/depth budgets are independent. Per-record admission cannot evade the global field or nesting bound.',
'Canonical encode sorts known records recursively, retains opaque unknown bytes exactly, and does not guess whether unknown bytes contain a nested message.',
'Cancel clears owned pending bytes and retires the stream. There is no asynchronous callback to drain in this adapter; live external feed calls must be serialized.',
'Actual flat codec comparison and struct rf_parser composition. One-byte versus bulk feed preserves the same parser/termination/budget semantics.',
'Externally serialized stream; caller retains the unconsumed suffix after the first document terminator. No concurrent feed or lock-free claim.',
'Minimal varints, duplicate/critical tags, unsupported required bits, truncated EOF, budgets and output capacity reject with specified failed-phase/progress behavior.',
'wire_depth','replicated-wire_depth','Distinct RPW1 schema selected out of band; it is not a silent reinterpretation of old tag3. Bit-level NaN/negative-zero and exact UTF8 policy do not imply NFC or a future float/string wire extension.']),
9:('replay_depth','replicated_replay_depth_validation.json',[
'Eight ticks, two players, nine full state snapshots, known/predicted input distinction and pending struct ar_effects. Canonical state512 bytes; input16→18 framed bytes.',
'All history remains owned until explicit new timeline. Pending effect identity and separate durable inbox prefix survive lost feedback; no external device action is part of the state.',
'Core state copies are values; durable wrapper exclusively borrows a store and sink log cannot relocate escaped HAMT roots. Process children are reaped before descriptor release.',
'External serialization, disjoint outputs and exact stored-current authority. Confirmed known inputs are immutable, so pending effect snapshots cannot be changed by later corrections.',
'No core heap. Store writes and sink HAMT allocation use actual imported failure behavior; finite horizon/revision/head refuse rather than silently dropping obligations.',
'Correction copies state and recomputes a suffix, but full invariant validation remains O(head). Notices replace speculative ranges; committed effects remain separate.',
'No core destructor. A blocked I/O callback or unjoined child prevents safe adapter cleanup; the supplied harness has explicit finite poll/call limits.',
'Actual rp_step and ar_decide/retry/feedback, actual hash/stream/store. Dependency audit rejects hidden time; the hidden random-state counterexample shows why symbols alone do not enforce purity.',
'Actual two-process local correction delivery; core serialized. Confirmed effects publish through ordered idempotent inbox, not arbitrary concurrent clients.',
'WAIT, revision MAX, score overflow, conflicting input, malformed bytes and296 page boundaries preserve the stated outputs; uncertain disk-new/local-old requires reopen.',
'replay_depth','replicated-replay_depth','Actual PPC64 ELFv2 big-endian core/codec execution checks canonical identity. POSIX files/processes are separately hosted; no physical power-loss or exactly-once device guarantee.'])}

def main():
    build='research/v5/modern/replicated_build_cost_validation.json';bd=json.loads((ROOT/build).read_text());assert bd['passed']
    mechanisms=[];reports={build:sha(ROOT/build)}
    for i,(module,filename,notes) in DATA.items():
        path='research/v5/modern/'+filename;d=json.loads((ROOT/path).read_text());assert d['passed'];reports[path]=sha(ROOT/path)
        rows=[]
        for dimension,note in zip(DIMENSIONS,notes):
            if dimension=='Compilation':note=f'Actual GCC/Clang Release clean, unchanged and copied-header-change builds for {note} are in the build-cost report. One observation per condition, background/cache effects retained; no universal compile-time bound.'
            if dimension=='Binary/stack':note=f'Current compiler/link/stack observations for {note} are source-bound in its campaign or companion ordinary-depth campaign. Native size/stack is ABI-specific; generated helper audit is separate from source-level allocation claims.'
            rows.append(dict(dimension=dimension,analysis=note))
        measurements=[r for r in d.get('observations',[]) if r.get('kind')=='metrics']
        builds=[r for r in bd['measurements'] if r['module']==module]
        mechanisms.append(dict(exercise=f'X-T76-{i:03d}',dimensions=rows,report=path,observed_metrics=measurements,build_observations=builds,limits=['No physical timing, external review or learner result is inferred.','Operation counts depend on the exact finite correctness/failure contract; timing samples include observer/cache/scheduler uncertainty.']))
    dump(ROOT/'research/v5/modern/replicated_cost_matrix.json',dict(schema_version=1,status='authored-analysis-bound-to-observed-reports',mechanisms=mechanisms,current_reports=reports,contract='labs/replicated/COST.md',limits=['Published C17 and C23 access gaps remain.','Some pure value mechanisms have no destructor/allocation/concurrent publication; their scoped non-applicability does not close those mechanisms elsewhere.']))
    print(json.dumps(dict(mechanisms=len(mechanisms),dimensions=sum(len(x['dimensions']) for x in mechanisms))))
if __name__=='__main__':main()
