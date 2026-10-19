# Exact T55 private assessor derivations

Preserve evidence kinds: finite model, executed native mechanism, measured host distribution and selected target link are distinct.

## AX-PR-T55-01-T55.01.a

The forward write/release-head/reads-from-acquire/read and reverse read/release-tail/reads-from-acquire/overwrite chains order every conflicting ordinary payload access. Single-writer indices permit own-index relaxed loads only. The batch hook sees the opposite side refuse until the final publication.

Changed condition: CfRing copies all accepted records before one release head and copies all returned records before one release tail. A second writer remains forbidden. Both early-publication and early-reuse controls fail the corresponding actual synchronized boundary assertion.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-core-batch", "clang-tsan-core-batch"], "models": ["publication"], "providers": ["spsc"]}`.

## AX-PR-T55-01-T55.01.b

The actual four-slot run is among2048 checked wrapped histories. The fifth insertion refuses and all four values/inverses drain in order; occupancy never exceeds4. Both native compilers also run the masked8-bit representation.

Changed condition: The8-bit representation masks counters and differences; the independent4/5/8-bit model enumerates translated occupancy states under capacity<half-modulus. A complete producer lap aliases empty and is excluded by occupancy/observation premises, not by the mask alone.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-core-ring", "clang-O3-core-ring"], "models": ["width"], "providers": []}`.

## AX-PR-T55-01-T55.01.c

Removing the synchronizing release/acquire edge can leave conflicting ordinary payload accesses unordered. Finite x86 silence cannot establish portability. The relation model labels this failure without executing a native race; current source-bound AS target receipts distinguish compiler/helper behavior.

Changed condition: Compile identical CfRing operations with CF_PAD0/1 and compare one producer/consumer, limit31, identical body size, two allowed CPUs and five native repeats. Report layout and raw distributions independently of ASan/TSan evidence; a64-byte alignment declaration is not a physical cache certification.

Exact evidence selectors: `{"measurements": [{"affinity": 2, "kind": 0, "kind_name": "queue", "pairs": 1}], "mechanisms": ["gcc-O3-core-ring"], "models": ["publication"], "providers": ["spsc"]}`.

## AX-PR-T55-02-T55.02.a

The repeating1024-round model has1024 competitor completions and zero losing-call completions. It supplies a starvation prefix and repeatable interference relation, not a wait-free own-step bound. Spurious failure alone need not correspond to another caller succeeding.

Changed condition: The frozen allocator probe actually imports malloc/free and fails strict no-runtime closure. A stopped allocator owner can delay the complete operation even when its central CAS loop has a conditional lock-free argument; actual imported libatomic/progress premises remain separate.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-core-cas"], "models": ["progress"], "providers": ["history"]}`.

## AX-PR-T55-02-T55.02.b

Actual CMX ticket0 and1 are reserved; only1 publishes, so pop returns RETRY with output99. Publishing0 then produces11,22. The frozen AR successor distinguishes EMPTY, PENDING, completed value and acknowledged cancellation.

Changed condition: AR pauses owner0 after its last payload copy, records a cancellation request, and still returns PENDING. Only the owner acknowledgment creates a tombstone that the consumer may skip. Late and stale cancellation, unchanged output, capacity and terminal tickets are checked.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-protocol-baseline", "clang-tsan-reservation-tests"], "models": [], "providers": ["reclamation"]}`.

## AX-PR-T55-02-T55.02.c

The checked counter starts UINT_MAX-2, consumes three injected spurious failures, increments twice and refuses a third unchanged. A real stale expected value is refreshed by failed CAS. The counter uses relaxed atomics because it publishes only a number, not ordinary payload.

Changed condition: Frozen actual host queries report16-byte pointer/tag pairs not lock-free and4-byte compact words lock-free for both compilers. Actual M4/M0 pair helper imports and strict link rejections are retained. Compact nonwrapping generations are an admitted alternative with explicit lifetime exhaustion, not an invented128-bit guarantee.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-core-cas", "clang-O3-core-cas"], "models": [], "providers": ["identity"]}`.

## AX-PR-T55-03-T55.03.a

The synthetic sequence removes A/B, reuses A with nextC, and permits raw numeric A->B CAS; B is no longer live. A generation-sensitive comparison rejects the old incarnation, but numeric equality never authorizes a dangling dereference.

Changed condition: The different-type ledger reuses exclusively owned allocated storage from NodeA to NodeB. Address equality does not authorize the old typed access. This separate effective-type/lifetime defect is analyzed without executing invalid native accesses or treating union storage as an ownership mechanism.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-protocol-baseline"], "models": ["aba"], "providers": ["identity"]}`.

## AX-PR-T55-03-T55.03.b

Starting1, the exact tags are1,2,3,0,1. Actual original CMX generation1..65535 is exercised and the next allocation moves to another slot instead of reissuing the first identity.

Changed condition: The model computes ceil(rate*pause)+burst and requires it strictly below2^g. Examples distinguish safe1<4, unsafe4=4, safe101<256 and unsafe256=256. Without a bounded pause and reuse burst no finite tag can exclude every recurrence; permanent retirement sacrifices availability.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-protocol-baseline"], "models": ["tags"], "providers": ["identity"]}`.

## AX-PR-T55-03-T55.03.c

A handle valid at the first check may be released/reused before use. AP instead publishes its hazard then rechecks the source before dereferencing and retains protection through the final access. Generation validation and lifetime pinning are distinct.

Changed condition: The checked synthetic trace validates generation1, retires/releases it, creates generation2 and rejects the proposed old access. The actual AP load/protect/validate pause case forces source replacement and retries before hazardous access; no native stale pointer is dereferenced.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-protection-validation", "clang-tsan-protection-validation"], "models": ["check_then_use"], "providers": ["identity", "reclamation"]}`.

## AX-PR-T55-04-T55.04.a

The CfPool executable accounts dummy, FREE, RESERVED, LINKED and RETIRED through failed link, cancellation, publication, removal and exclusive reclaim. The distinct model partitions1+7+8+3+13=32. AP tests provide actual hazard-retained exhaustion.

Changed condition: Four paused owners holding two distinct retired nodes each retain8, leaving23 storage slots after the dummy. There is no active caller if all four are stopped. The existing four-participant executable uses three holders and one actor instead, retains6 and admits25; this changed actor count is explicitly preserved.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-core-pool", "clang-O3-protection-retention"], "models": ["capacity"], "providers": ["reclamation"]}`.

## AX-PR-T55-04-T55.04.b

An actual failed link comparison leaves the full reservation incarnation owned. The owner can cancel it once or retry; after successful root publication the ticket is LINKED and cancellation refuses. A stale repeated cancellation cannot release a newer incarnation.

Changed condition: cf_cancel is one bounded full-state RESERVED compare-and-exchange, followed by nonwrapping release. The owner alone calls it before publication and after its last private write. Asynchronous cancellation in the successful-link/state-transition interval is outside this contract. The linked-cancel mutant fails the exact preservation check.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-core-pool", "gcc-CF_MUTANT_CANCEL_LINKED", "clang-CF_MUTANT_CANCEL_LINKED"], "models": [], "providers": []}`.

## AX-PR-T55-04-T55.04.c

Original CMX/AIPool fields require external serialization. AP/CfPool obtain ownership through atomic full-state CAS; a tag alone supplies no concurrency safety. The ordinary helper is never subjected to native conflicting unsynchronized accesses.

Changed condition: With32 slots including the dummy and skewed demand12,4,4,4, isolated partitions admit19 and refuse5 despite12 free slots; shared/stealing admit24. Actual four-worker repeated take/cancel measurements retain scans, retries and elapsed time under all three policies, with every allocation returned and the same total capacity.

Exact evidence selectors: `{"measurements": [{"kind_name": "pool"}], "mechanisms": ["gcc-O0-core-pool"], "models": [], "providers": ["identity", "reclamation"]}`.

## AX-PR-T55-05-T55.05.a

Actual AP validation/protection fixtures force replacement before hazard publication and retain an old dummy during the last access. Its finite SC protection model and omitted-validation/ignored-hazard controls remain source-bound.

Changed condition: AP_RELAX_ALLOC changes only a candidate state load; successful full-state SC acquisition still authorizes payload initialization. Weakening hazard publication/scan to release/acquire is rejected: both loads can read initial values without synchronization. The SC cycle argument is shown explicitly and is not transferred to that different memory order.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-protection-validation", "clang-tsan-protection-protection"], "models": ["hazard_weakening"], "providers": ["reclamation"]}`.

## AX-PR-T55-05-T55.05.b

Actual AP collector pauses holding state2 while another thread advances the slot through reuse to state6. Its full-state CAS cannot free that later retirement; the frozen untagged-release control fails the intended assertion.

Changed condition: The actual reduced AP generation limits1 and3 execute until63/127 total admissions and terminal retention; the new generation-limit1 private pool retires all31 non-dummy slots. Availability ends instead of tags wrapping into stale authority.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-protection-collector", "clang-tsan-protection-generation", "gcc-O0-core-pool"], "models": [], "providers": ["reclamation"]}`.

## AX-PR-T55-05-T55.05.c

Actual AP retention and registration cases hold retired nodes, report FULL, reclaim only after valid hazard clearing and reject an old registration lease before it can clear a replacement owner. Finite memory safety need not imply unlimited admission.

Changed condition: The existing departure test joins the owner before successor cleanup. New actual12-retirement comparisons retain one hazard versus epochs with quiescence every3/6/12 updates. All counts and collection points are recorded; data structures and metadata sizes are not falsely equated.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-protection-retention", "clang-tsan-protection-departure", "gcc-O0-epoch-retention"], "models": [], "providers": ["reclamation", "grace"]}`.

## AX-PR-T55-06-T55.06.a

The actual AP hook pauses after link while Tail still denotes the dummy. A second enqueue helps and completes; its dequeue returns the first value before the original publisher resumes. Tail movement is not mandatory enqueue publication.

Changed condition: The emitted three-call history includes the stopped publisher spanning both completed helper operations. CH checks it and the independent permutation oracle validates the actual log; the test is a real paused thread, not a manufactured queue-state prefix.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-protocol-pause", "gcc-history-oracle", "clang-history-oracle"], "models": [], "providers": ["reclamation"]}`.

## AX-PR-T55-06-T55.06.b

The stopped reader protects dummy0 while another dequeuer removes11 and a collector cannot reuse it. After resumption the reader revalidates Head, retries and returns22; each value has one successful Head-CAS removal.

Changed condition: The actual overlapping two-dequeuer trace includes both original enqueues and unique event ordinals. Independent FIFO checking admits the successful competitor before the resumed loser, while the protected retired dummy remains unavailable until the slow operation exits.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-protocol-pause", "clang-tsan-protocol-pause", "gcc-history-oracle"], "models": [], "providers": ["reclamation"]}`.

## AX-PR-T55-06-T55.06.c

The actual four-thread run has two producers of20000 distinct checked values, two consumers,40000 singleton seen entries, joined workers and one remaining dummy. It proves only that finite multiplicity/payload execution.

Changed condition: Actual paused and independently concurrent four-call provider histories retain invocation/response intervals. The new independent oracle checks67500 FIFO/stack/pending cases and replays returned witnesses; none of those finite cases is advertised as universal FIFO or weak-memory proof.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-protocol-stress", "clang-tsan-protocol-stress", "gcc-history-oracle", "clang-history-oracle"], "models": [], "providers": ["reclamation", "history"]}`.

## AX-PR-T55-07-T55.07.a

The actual CfEpoch/AG composition advances0->1 then returns PENDING with the old reader active. Nested inner leave does not release it. After the outer exit, retirement0 becomes eligible; arbitrary objects still need removal and exclusive reuse authority.

Changed condition: The12-retirement experiment records each retired stamp and the exact operation3/6/12 at which the final old section exits and collection occurs. Earliest reuse in this declared workload is the corresponding quiescent boundary, not a bare global increment.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-epoch-registration", "clang-O3-epoch-retention"], "models": [], "providers": ["grace"]}`.

## AX-PR-T55-07-T55.07.b

AG active departure returns BUSY, and17 versions retain16 old objects plus one current until the real reader exits. The existing joined-departure boundary remains distinct from timeout/failure suspicion.

Changed condition: Under the same12 logical retirements, periodic quiescence at3/6/12 gives epoch peaks3/6/12 while one hazard retains one distinct old object per interval. The exact native ledgers include collection points and distinguish logical retention from representation cost.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-versions-pressure", "clang-tsan-epoch-retention"], "models": [], "providers": ["grace", "reclamation"]}`.

## AX-PR-T55-07-T55.07.c

Unsynchronized CMX/AG ordinary metadata would violate its serialization contract. CfEpoch acquires one C17 flag before all ordinary fields and release-clears it afterward; unsuccessful try calls return BUSY without access or output publication.

Changed condition: Fixed AG tickets and sections retire at nonwrapping exhaustion. Actual concurrent two-reader20000-section tests use the gate; stale tickets reject, active departure refuses, nested accounting persists and terminal registration falls back to another slot. A stopped gate owner can prevent successful service, so no lock-free epoch claim is made.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-tsan-epoch-threads", "clang-tsan-epoch-threads", "gcc-O0-epoch-registration"], "models": ["epoch_gate"], "providers": ["grace"]}`.

## AX-PR-T55-08-T55.08.a

Actual frozen CMX waits for reader1 of2 even after reader0 leaves1. Actual AG collects retired1 after the old reader leaves while the new reader remains; neither result changes the other API contract.

Changed condition: The new handoff experiment keeps at least one newer reader active for1000 arrivals. Retired1 is already collected while global-zero polling remains false. The repeated nonzero-reader state permits an indefinite extension; the finite count alone is not an infinite liveness proof.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-versions-versions", "clang-O3-epoch-arrivals"], "models": [], "providers": ["grace"]}`.

## AX-PR-T55-08-T55.08.b

The frozen nonnested AG API rejects a second enter. Valid read copies return consistent immutable versions and reject expired borrows; direct mutation of published bytes is a contract violation rather than a supported experiment.

Changed condition: Nested AG/CfEpoch mode increments depth and keeps the original section identity/announcement until the outermost leave. Actual inner-exit tests show retirement0 remains ineligible, then becomes eligible after the last exit; depth and section exhaustion never wrap.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-versions-epoch", "clang-tsan-epoch-registration"], "models": [], "providers": ["grace"]}`.

## AX-PR-T55-08-T55.08.c

Actual AG admits16 retirements in17 slots, refuses the next update, retains old bytes and recovers after the reader exits. This serialized user-space teaching object is not Linux RCU and supplies no wait-free update bound.

Changed condition: The native AG/RW comparison uses32-byte payloads, reserved capacity17, two readers, one writer,1000 updates and joined shutdown. It reports reader/writer raw distributions, FULL retries, peak retained versions and actual storage sizes. The RW lock reuses one slot under exclusive locking; AG retains immutable versions and its metadata, so those costs are explicit.

Exact evidence selectors: `{"measurements": [{"kind_name": "rw"}], "mechanisms": ["gcc-O0-versions-pressure"], "models": [], "providers": ["grace"]}`.

## AX-PR-T55-09-T55.09.a

Actual original CMX rejects enqueue7 complete before8 followed by dequeue8. CH reproduces this FIFO result and supplies a legal witness when returning7. A distinct event schedule permits genuinely overlapping reorderings.

Changed condition: The same complete insertion7,insertion8,remove8 history is valid for the stack and invalid for FIFO; remove7 reverses those outcomes. The independent list oracle tests both specifications and replays every accepted witness.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-protocol-baseline", "clang-O3-history-tests", "gcc-history-oracle"], "models": [], "providers": ["history"]}`.

## AX-PR-T55-09-T55.09.b

CH rejects overlapping calls from one participant. A response timestamp equal to another invocation provides no strict coarse-time precedence, so a history may admit a different order. The same-participant nonreentrancy contract is never inferred from mere numeric IDs.

Changed condition: Recorded ordinal events disambiguate that equal timestamp boundary and force the known response-before-invocation edge; the formerly admitted reversed FIFO history rejects. Duplicate/inconsistent events reject. The actual queue log obtains ordinals through atomic increments rather than inventing them after observing results.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-history-tests", "clang-tsan-protocol-pause"], "models": ["coarse_time"], "providers": []}`.

## AX-PR-T55-09-T55.09.c

The original CMX complete-history interface remains bounded at8 and rejects9. It has no pending field; an invented timestamp sentinel does not add pending semantics. New CH explicitly represents pending calls and admits at most10.

Changed condition: CH selects legal completion/removal subsets, preserves all completed calls, derives pending results and publishes only a valid witness. The missing-pending control fails a completed removal that requires a pending insertion. Nine ordered complete calls pass the extension; a one-node search budget returns LIMIT unchanged. Independent67500-case verdict/witness checks record actual search costs.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-protocol-baseline", "gcc-O0-history-tests", "gcc-CH_MUTANT_DROP_PENDING", "gcc-history-oracle", "clang-history-oracle"], "models": [], "providers": ["history"]}`.

## AX-PR-T55-10-T55.10.a

SPSC is measured only at1+1. AP and mutex adapters use equal8-byte index/check messages, identical immutable-body reads, upper occupancy31, fixed caller storage and joined exact-once transfers. AP pool admission may conservatively fail and collection consumes work; the mutex logical-capacity check and condition-blocking adapter are reported distinctly.

Changed condition: Actual native five-repeat comparisons change body size8/256 and pin only the participating threads to one/two already-permitted CPUs. They retain every enqueue/completion latency and correctness ledger. Body size changes the referenced payload workload, not AP layout; no SPSC multi-producer substitution is made.

Exact evidence selectors: `{"measurements": [{"kind_name": "queue", "pairs": 1}, {"kind_name": "queue", "pairs": 4}], "mechanisms": ["gcc-O0-protocol-stress"], "models": [], "providers": []}`.

## AX-PR-T55-10-T55.10.b

ASan/UBSan and TSan invocations are labeled correctness-only and excluded from native summaries. The controlled2ms paused participant retains an AP old dummy until its real resume, and all workers later join. Raw per-operation and per-worker data exposes the stall beyond a mean throughput.

Changed condition: Five native repeats produce min/median/p95/p99/max, paired ratios and explicitly exploratory empirical resampling intervals. Eight workers on one/two permitted CPUs are oversubscribed; actual observed mutex-try faster cases are retained alongside slower cases. Shared-host noise, differing synchronization/collection costs and virtual CPU topology prevent a universal ranking.

Exact evidence selectors: `{"measurements": [{"kind_name": "queue", "pause": 1}, {"kind_name": "queue", "pairs": 4}], "mechanisms": ["clang-tsan-protocol-stress"], "models": [], "providers": []}`.

## AX-PR-T55-10-T55.10.c

A stopped blocking logger can keep a failed enqueue call from returning, invalidating an end-to-end lock-free claim. The explicit premise model includes allocator, logger, collector, helper and scheduling assumptions rather than treating central CAS as the entire operation.

Changed condition: The new single-producer diagnostic ring admits4 of9 records and drops5 new diagnostics without allocation or waiting. Full preserves unread records; the drop counter and no-retry policy bound this path. Multiple producers need separate rings or another admitted protocol. Selected M4 images close all new core symbols without hidden runtime providers.

Exact evidence selectors: `{"measurements": [], "mechanisms": ["gcc-O0-core-diagnostic", "clang-O3-core-diagnostic"], "models": ["progress"], "providers": ["history"]}`.

