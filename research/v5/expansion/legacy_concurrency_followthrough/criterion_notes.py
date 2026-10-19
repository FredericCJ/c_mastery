"""Thirty exact T55 reasoning and changed-condition packets; no shared generic pass."""
def evidence(*mechanisms,models=(),providers=(),measurements=()):
 return dict(mechanisms=list(mechanisms),models=list(models),providers=list(providers),measurements=list(measurements))
NOTES={
'01.a':(
'Draw both transfers of ownership for each slot. Extend the diagram to an all-or-none batch before writing either publication store. Use actual paused copy hooks to distinguish copied bytes from admitted bytes.',
'The forward write/release-head/reads-from-acquire/read and reverse read/release-tail/reads-from-acquire/overwrite chains order every conflicting ordinary payload access. Single-writer indices permit own-index relaxed loads only. The batch hook sees the opposite side refuse until the final publication.',
'CfRing copies all accepted records before one release head and copies all returned records before one release tail. A second writer remains forbidden. Both early-publication and early-reuse controls fail the corresponding actual synchronized boundary assertion.',
evidence('gcc-O0-core-batch','clang-tsan-core-batch',models=['publication'],providers=['spsc'])),
'01.b':(
'Predict every slot and modular distance starting at UINT32_MAX-1; retain sentinel outputs on empty and unread records on full. State the width before using the arithmetic.',
'The actual four-slot run is among2048 checked wrapped histories. The fifth insertion refuses and all four values/inverses drain in order; occupancy never exceeds4. Both native compilers also run the masked8-bit representation.',
'The8-bit representation masks counters and differences; the independent4/5/8-bit model enumerates translated occupancy states under capacity<half-modulus. A complete producer lap aliases empty and is excluded by occupancy/observation premises, not by the mask alone.',
evidence('gcc-O0-core-ring','clang-O3-core-ring',models=['width'])),
'01.c':(
'Separate a C synchronization proof, the actual selected atomic lowering, and an observed timing result. Never use an undefined ordinary-payload run as a reliable numeric counterexample.',
'Removing the synchronizing release/acquire edge can leave conflicting ordinary payload accesses unordered. Finite x86 silence cannot establish portability. The relation model labels this failure without executing a native race; current source-bound AS target receipts distinguish compiler/helper behavior.',
'Compile identical CfRing operations with CF_PAD0/1 and compare one producer/consumer, limit31, identical body size, two allowed CPUs and five native repeats. Report layout and raw distributions independently of ASan/TSan evidence; a64-byte alignment declaration is not a physical cache certification.',
evidence('gcc-O3-core-ring',models=['publication'],providers=['spsc'],measurements=[{'kind_name':'queue','kind':0,'pairs':1,'affinity':2}])) ,
'02.a':(
'Track successful requests separately from successful CAS interference, then include every allocation and diagnostic path in the complete operation.',
'The repeating1024-round model has1024 competitor completions and zero losing-call completions. It supplies a starvation prefix and repeatable interference relation, not a wait-free own-step bound. Spurious failure alone need not correspond to another caller succeeding.',
'The frozen allocator probe actually imports malloc/free and fails strict no-runtime closure. A stopped allocator owner can delay the complete operation even when its central CAS loop has a conditional lock-free argument; actual imported libatomic/progress premises remain separate.',
evidence('gcc-O0-core-cas',models=['progress'],providers=['history'])),
'02.b':(
'Keep reservation order, publication order, cancellation request and final access acknowledgment in separate columns. A later completed reservation does not authorize skipping an unfinished predecessor.',
'Actual CMX ticket0 and1 are reserved; only1 publishes, so pop returns RETRY with output99. Publishing0 then produces11,22. The frozen AR successor distinguishes EMPTY, PENDING, completed value and acknowledged cancellation.',
'AR pauses owner0 after its last payload copy, records a cancellation request, and still returns PENDING. Only the owner acknowledgment creates a tombstone that the consumer may skip. Late and stale cancellation, unchanged output, capacity and terminal tickets are checked.',
evidence('gcc-O0-protocol-baseline','clang-tsan-reservation-tests',providers=['reclamation'])),
'02.c':(
'Write the numeric overflow policy before the retry loop. Inject permitted weak failure separately from actual stale-expected CAS failure and inspect the selected representation.',
'The checked counter starts UINT_MAX-2, consumes three injected spurious failures, increments twice and refuses a third unchanged. A real stale expected value is refreshed by failed CAS. The counter uses relaxed atomics because it publishes only a number, not ordinary payload.',
'Frozen actual host queries report16-byte pointer/tag pairs not lock-free and4-byte compact words lock-free for both compilers. Actual M4/M0 pair helper imports and strict link rejections are retained. Compact nonwrapping generations are an admitted alternative with explicit lifetime exhaustion, not an invented128-bit guarantee.',
evidence('gcc-O0-core-cas','clang-O3-core-cas',providers=['identity'])),
'03.a':(
'Draw A->B->C with allocation incarnations. Keep the stale successor B even after the head address becomes A again, then distinguish abstract membership from C object type/lifetime.',
'The synthetic sequence removes A/B, reuses A with nextC, and permits raw numeric A->B CAS; B is no longer live. A generation-sensitive comparison rejects the old incarnation, but numeric equality never authorizes a dangling dereference.',
'The different-type ledger reuses exclusively owned allocated storage from NodeA to NodeB. Address equality does not authorize the old typed access. This separate effective-type/lifetime defect is analyzed without executing invalid native accesses or treating union storage as an ownership mechanism.',
evidence('gcc-O0-protocol-baseline',models=['aba'],providers=['identity'])),
'03.b':(
'Enumerate the entire two-bit cycle and count reuses during the longest admitted reader pause, including burst allowance rather than only average rate.',
'Starting1, the exact tags are1,2,3,0,1. Actual original CMX generation1..65535 is exercised and the next allocation moves to another slot instead of reissuing the first identity.',
'The model computes ceil(rate*pause)+burst and requires it strictly below2^g. Examples distinguish safe1<4, unsafe4=4, safe101<256 and unsafe256=256. Without a bounded pause and reuse burst no finite tag can exclude every recurrence; permanent retirement sacrifices availability.',
evidence('gcc-O0-protocol-baseline',models=['tags'],providers=['identity'])),
'03.c':(
'Put a scheduling boundary after successful generation validation and before any proposed payload read. Identify the event that grants continuous access authority.',
'A handle valid at the first check may be released/reused before use. AP instead publishes its hazard then rechecks the source before dereferencing and retains protection through the final access. Generation validation and lifetime pinning are distinct.',
'The checked synthetic trace validates generation1, retires/releases it, creates generation2 and rejects the proposed old access. The actual AP load/protect/validate pause case forces source replacement and retries before hazardous access; no native stale pointer is dereferenced.',
evidence('gcc-O0-protection-validation','clang-tsan-protection-validation',models=['check_then_use'],providers=['identity','reclamation'])),
'04.a':(
'Assign all32 slots exactly one owner/state. Count private reservations separately from published queue elements and do not infer free-node availability from logical emptiness.',
'The CfPool executable accounts dummy, FREE, RESERVED, LINKED and RETIRED through failed link, cancellation, publication, removal and exclusive reclaim. The distinct model partitions1+7+8+3+13=32. AP tests provide actual hazard-retained exhaustion.',
'Four paused owners holding two distinct retired nodes each retain8, leaving23 storage slots after the dummy. There is no active caller if all four are stopped. The existing four-participant executable uses three holders and one actor instead, retains6 and admits25; this changed actor count is explicitly preserved.',
evidence('gcc-O0-core-pool','clang-O3-protection-retention',models=['capacity'],providers=['reclamation'])),
'04.b':(
'Follow the private reservation through a failed actual root CAS. Before adding cancellation, name the last point at which the owner can prove the node was never published.',
'An actual failed link comparison leaves the full reservation incarnation owned. The owner can cancel it once or retry; after successful root publication the ticket is LINKED and cancellation refuses. A stale repeated cancellation cannot release a newer incarnation.',
'cf_cancel is one bounded full-state RESERVED compare-and-exchange, followed by nonwrapping release. The owner alone calls it before publication and after its last private write. Asynchronous cancellation in the successful-link/state-transition interval is outside this contract. The linked-cancel mutant fails the exact preservation check.',
evidence('gcc-O0-core-pool','gcc-CF_MUTANT_CANCEL_LINKED','clang-CF_MUTANT_CANCEL_LINKED')),
'04.c':(
'Separate a numeric generation from a concurrent ownership transition. Compare equal total node storage under shared scans, per-worker partitions and explicit stealing.',
'Original CMX/AIPool fields require external serialization. AP/CfPool obtain ownership through atomic full-state CAS; a tag alone supplies no concurrency safety. The ordinary helper is never subjected to native conflicting unsynchronized accesses.',
'With32 slots including the dummy and skewed demand12,4,4,4, isolated partitions admit19 and refuse5 despite12 free slots; shared/stealing admit24. Actual four-worker repeated take/cancel measurements retain scans, retries and elapsed time under all three policies, with every allocation returned and the same total capacity.',
evidence('gcc-O0-core-pool',providers=['identity','reclamation'],measurements=[{'kind_name':'pool'}])),
'05.a':(
'Write load-source, publish-hazard, recheck, dereference in order against removal/retirement/scan, then identify precisely which proposed relaxed operation changes the proof.',
'Actual AP validation/protection fixtures force replacement before hazard publication and retain an old dummy during the last access. Its finite SC protection model and omitted-validation/ignored-hazard controls remain source-bound.',
'AP_RELAX_ALLOC changes only a candidate state load; successful full-state SC acquisition still authorizes payload initialization. Weakening hazard publication/scan to release/acquire is rejected: both loads can read initial values without synchronization. The SC cycle argument is shown explicitly and is not transferred to that different memory order.',
evidence('gcc-O0-protection-validation','clang-tsan-protection-protection',models=['hazard_weakening'],providers=['reclamation'])),
'05.b':(
'Give every retirement an incarnation, including the collector snapshot. Pause a collector across collection, reuse and another retirement rather than merely toggling a RETIRED bit.',
'Actual AP collector pauses holding state2 while another thread advances the slot through reuse to state6. Its full-state CAS cannot free that later retirement; the frozen untagged-release control fails the intended assertion.',
'The actual reduced AP generation limits1 and3 execute until63/127 total admissions and terminal retention; the new generation-limit1 private pool retires all31 non-dummy slots. Availability ends instead of tags wrapping into stale authority.',
evidence('gcc-O0-protection-collector','clang-tsan-protection-generation','gcc-O0-core-pool',providers=['reclamation'])),
'05.c':(
'Keep registration ownership, hazard slots, retained nodes and active callers separate. A timeout cannot prove that a paused participant will never publish again.',
'Actual AP retention and registration cases hold retired nodes, report FULL, reclaim only after valid hazard clearing and reject an old registration lease before it can clear a replacement owner. Finite memory safety need not imply unlimited admission.',
'The existing departure test joins the owner before successor cleanup. New actual12-retirement comparisons retain one hazard versus epochs with quiescence every3/6/12 updates. All counts and collection points are recorded; data structures and metadata sizes are not falsely equated.',
evidence('gcc-O0-protection-retention','clang-tsan-protection-departure','gcc-O0-epoch-retention',providers=['reclamation','grace'])),
'06.a':(
'Locate the successful next-link CAS separately from the optional Tail update. Record actual invocation/response ordinals while the publisher is stopped at that exact boundary.',
'The actual AP hook pauses after link while Tail still denotes the dummy. A second enqueue helps and completes; its dequeue returns the first value before the original publisher resumes. Tail movement is not mandatory enqueue publication.',
'The emitted three-call history includes the stopped publisher spanning both completed helper operations. CH checks it and the independent permutation oracle validates the actual log; the test is a real paused thread, not a manufactured queue-state prefix.',
evidence('gcc-O0-protocol-pause','gcc-history-oracle','clang-history-oracle',providers=['reclamation'])),
'06.b':(
'Track the old dummy hazard and successor payload hazard through competing Head CAS operations. Explain why the loser must reread rather than publish a copied stale result.',
'The stopped reader protects dummy0 while another dequeuer removes11 and a collector cannot reuse it. After resumption the reader revalidates Head, retries and returns22; each value has one successful Head-CAS removal.',
'The actual overlapping two-dequeuer trace includes both original enqueues and unique event ordinals. Independent FIFO checking admits the successful competitor before the resumed loser, while the protected retired dummy remains unavailable until the slow operation exits.',
evidence('gcc-O0-protocol-pause','clang-tsan-protocol-pause','gcc-history-oracle',providers=['reclamation'])),
'06.c':(
'Keep the40000-value multiplicity ledger separate from the sequential FIFO specification and progress argument. Gather short complete histories before recording throughput.',
'The actual four-thread run has two producers of20000 distinct checked values, two consumers,40000 singleton seen entries, joined workers and one remaining dummy. It proves only that finite multiplicity/payload execution.',
'Actual paused and independently concurrent four-call provider histories retain invocation/response intervals. The new independent oracle checks67500 FIFO/stack/pending cases and replays returned witnesses; none of those finite cases is advertised as universal FIFO or weak-memory proof.',
evidence('gcc-O0-protocol-stress','clang-tsan-protocol-stress','gcc-history-oracle','clang-history-oracle',providers=['reclamation','history'])),
'07.a':(
'Announce epoch0, advance exactly once and distinguish a numeric epoch from a particular removed object and its retirement stamp.',
'The actual CfEpoch/AG composition advances0->1 then returns PENDING with the old reader active. Nested inner leave does not release it. After the outer exit, retirement0 becomes eligible; arbitrary objects still need removal and exclusive reuse authority.',
'The12-retirement experiment records each retired stamp and the exact operation3/6/12 at which the final old section exits and collection occurs. Earliest reuse in this declared workload is the corresponding quiescent boundary, not a bare global increment.',
evidence('gcc-O0-epoch-registration','clang-O3-epoch-retention',providers=['grace'])),
'07.b':(
'Ask whether the stalled reader may still resume before changing registration metadata. Treat retention and explicit FULL as valid bounded outcomes.',
'AG active departure returns BUSY, and17 versions retain16 old objects plus one current until the real reader exits. The existing joined-departure boundary remains distinct from timeout/failure suspicion.',
'Under the same12 logical retirements, periodic quiescence at3/6/12 gives epoch peaks3/6/12 while one hazard retains one distinct old object per interval. The exact native ledgers include collection points and distinguish logical retention from representation cost.',
evidence('gcc-O0-versions-pressure','clang-tsan-epoch-retention',providers=['grace','reclamation'])),
'07.c':(
'Identify every ordinary field touched by enter, leave, registration and advance. Specify one complete synchronization domain rather than changing only the epoch counter type.',
'Unsynchronized CMX/AG ordinary metadata would violate its serialization contract. CfEpoch acquires one C17 flag before all ordinary fields and release-clears it afterward; unsuccessful try calls return BUSY without access or output publication.',
'Fixed AG tickets and sections retire at nonwrapping exhaustion. Actual concurrent two-reader20000-section tests use the gate; stale tickets reject, active departure refuses, nested accounting persists and terminal registration falls back to another slot. A stopped gate owner can prevent successful service, so no lock-free epoch claim is made.',
evidence('gcc-tsan-epoch-threads','clang-tsan-epoch-threads','gcc-O0-epoch-registration',models=['epoch_gate'],providers=['grace'])),
'08.a':(
'Separate publication of root2 from the set of readers that could still hold root1. Compare a conservative global-zero predicate to retirement-specific eligibility.',
'Actual frozen CMX waits for reader1 of2 even after reader0 leaves1. Actual AG collects retired1 after the old reader leaves while the new reader remains; neither result changes the other API contract.',
'The new handoff experiment keeps at least one newer reader active for1000 arrivals. Retired1 is already collected while global-zero polling remains false. The repeated nonzero-reader state permits an indefinite extension; the finite count alone is not an infinite liveness proof.',
evidence('gcc-O0-versions-versions','clang-O3-epoch-arrivals',providers=['grace'])),
'08.b':(
'State whether one participant may nest, and preserve immutable published bytes. The C struct representation cannot itself prevent a caller from mutating shared payload.',
'The frozen nonnested AG API rejects a second enter. Valid read copies return consistent immutable versions and reject expired borrows; direct mutation of published bytes is a contract violation rather than a supported experiment.',
'Nested AG/CfEpoch mode increments depth and keeps the original section identity/announcement until the outermost leave. Actual inner-exit tests show retirement0 remains ineligible, then becomes eligible after the last exit; depth and section exhaustion never wrap.',
evidence('gcc-O0-versions-epoch','clang-tsan-epoch-registration',providers=['grace'])),
'08.c':(
'Count the finite version budget while one reader remains active, then compare equal read/update requests under a reader-writer lock without concealing its different storage policy.',
'Actual AG admits16 retirements in17 slots, refuses the next update, retains old bytes and recovers after the reader exits. This serialized user-space teaching object is not Linux RCU and supplies no wait-free update bound.',
'The native AG/RW comparison uses32-byte payloads, reserved capacity17, two readers, one writer,1000 updates and joined shutdown. It reports reader/writer raw distributions, FULL retries, peak retained versions and actual storage sizes. The RW lock reuses one slot under exclusive locking; AG retains immutable versions and its metadata, so those costs are explicit.',
evidence('gcc-O0-versions-pressure',providers=['grace'],measurements=[{'kind_name':'rw'}])),
'09.a':(
'Choose the sequential object specification before searching any history. Nonoverlap order and log-row order are different constraints.',
'Actual original CMX rejects enqueue7 complete before8 followed by dequeue8. CH reproduces this FIFO result and supplies a legal witness when returning7. A distinct event schedule permits genuinely overlapping reorderings.',
'The same complete insertion7,insertion8,remove8 history is valid for the stack and invalid for FIFO; remove7 reverses those outcomes. The independent list oracle tests both specifications and replays every accepted witness.',
evidence('gcc-O0-protocol-baseline','clang-O3-history-tests','gcc-history-oracle',providers=['history'])),
'09.b':(
'Validate participant ownership before searching. Record coarse timestamps separately from unique invocation/response event ordinals.',
'CH rejects overlapping calls from one participant. A response timestamp equal to another invocation provides no strict coarse-time precedence, so a history may admit a different order. The same-participant nonreentrancy contract is never inferred from mere numeric IDs.',
'Recorded ordinal events disambiguate that equal timestamp boundary and force the known response-before-invocation edge; the formerly admitted reversed FIFO history rejects. Duplicate/inconsistent events reject. The actual queue log obtains ordinals through atomic increments rather than inventing them after observing results.',
evidence('gcc-O0-history-tests','clang-tsan-protocol-pause',models=['coarse_time'])),
'09.c':(
'Read the retained checker limit before invoking it. Pending completion is a new specification, and exhausting a search budget is an undecided result.',
'The original CMX complete-history interface remains bounded at8 and rejects9. It has no pending field; an invented timestamp sentinel does not add pending semantics. New CH explicitly represents pending calls and admits at most10.',
'CH selects legal completion/removal subsets, preserves all completed calls, derives pending results and publishes only a valid witness. The missing-pending control fails a completed removal that requires a pending insertion. Nine ordered complete calls pass the extension; a one-node search budget returns LIMIT unchanged. Independent67500-case verdict/witness checks record actual search costs.',
evidence('gcc-O0-protocol-baseline','gcc-O0-history-tests','gcc-CH_MUTANT_DROP_PENDING','gcc-history-oracle','clang-history-oracle',providers=['history'])),
'10.a':(
'Create a comparison table before timing: participants, payload, admission semantics, maximum occupancy, allocation, retries, reclamation and shutdown.',
'SPSC is measured only at1+1. AP and mutex adapters use equal8-byte index/check messages, identical immutable-body reads, upper occupancy31, fixed caller storage and joined exact-once transfers. AP pool admission may conservatively fail and collection consumes work; the mutex logical-capacity check and condition-blocking adapter are reported distinctly.',
'Actual native five-repeat comparisons change body size8/256 and pin only the participating threads to one/two already-permitted CPUs. They retain every enqueue/completion latency and correctness ledger. Body size changes the referenced payload workload, not AP layout; no SPSC multi-producer substitution is made.',
evidence('gcc-O0-protocol-stress',measurements=[{'kind_name':'queue','pairs':1},{'kind_name':'queue','pairs':4}])),
'10.b':(
'Retain tail latency, retries, per-participant service and retirement pressure. Do not use sanitizer timing or a mean as a production performance claim.',
'ASan/UBSan and TSan invocations are labeled correctness-only and excluded from native summaries. The controlled2ms paused participant retains an AP old dummy until its real resume, and all workers later join. Raw per-operation and per-worker data exposes the stall beyond a mean throughput.',
'Five native repeats produce min/median/p95/p99/max, paired ratios and explicitly exploratory empirical resampling intervals. Eight workers on one/two permitted CPUs are oversubscribed; actual observed mutex-try faster cases are retained alongside slower cases. Shared-host noise, differing synchronization/collection costs and virtual CPU topology prevent a universal ranking.',
evidence('clang-tsan-protocol-stress',measurements=[{'kind_name':'queue','pause':1},{'kind_name':'queue','pairs':4}])),
'10.c':(
'Audit the entire success and failure path, including the act of reporting exhaustion. A callback invoked only on FULL is still part of that operation.',
'A stopped blocking logger can keep a failed enqueue call from returning, invalidating an end-to-end lock-free claim. The explicit premise model includes allocator, logger, collector, helper and scheduling assumptions rather than treating central CAS as the entire operation.',
'The new single-producer diagnostic ring admits4 of9 records and drops5 new diagnostics without allocation or waiting. Full preserves unread records; the drop counter and no-retry policy bound this path. Multiple producers need separate rings or another admitted protocol. Selected M4 images close all new core symbols without hidden runtime providers.',
evidence('gcc-O0-core-diagnostic','clang-O3-core-diagnostic',models=['progress'],providers=['history']))
}
