# Private T29.03–.06 assessment and transfer analysis

Do not publish this file, reference code, model inputs, discriminating mutations or oracles in learner exports. Retain the original ordinary and delayed IDs. Give the delayed packet at least seven days after the recorded ordinary attempt, using fresh learner code and the same explicit assumptions. Every result below is an author-side solution, not evidence of that independent performance. Score each objective against its own argument and checks.

## 03.a

`capture_begin` acknowledges one admitted event logically; the timer driver first checks INTSTATUS, disables the timer, writes its W1C bit and executes DSB. These are different layers. Successful capture builds an owned28-byte record with up to eight copied input bytes; reserve reads head/tail, fill copies the bounded record, end release-publishes head. No application formatting, allocation, retry for queue space or device wait occurs in the admitted handler. Full queue takes the short refusal path and increments a saturating drop count. The driver retains explicit fatal assertions for broken experiment premises; its terminal failure trap must appear in the full call graph and must not be counted as normal timing.

The variable-length packet owns only an inline prefix. Cases0,1,8,9,UINT32_MAX discriminate zero length/null, exact capacity, truncation and a huge claimed length without huge copying. Require a stable live prefix; a numeric length does not validate an allocation. A pointer-retaining design needs a new lease and cannot earn credit under this copied-record contract. Original helper implementations may appear in generated objects; their fixed arguments and complete linked closure must be checked. `audit_target.py` emits the actual graph, backedges, local stack metadata and a conservative handler increment including the one allowed nested IRQ; it is not a foreground/FP/NMI stack theorem.

## 03.b

Ordinary `active` is producer-owned. Checking it is not itself a reservation. The actual target masks configurable local IRQs across the complete begin operation, then deliberately pends IRQ1 while the service is active and unpublished. The nested begin sees BUSY and does not acknowledge the owned event. `reentrant==1`, `acknowledged==1`, and exactly one record is observed. Counter mutation4 is rejected in the serial public boundary test; the real nested experiment independently checks the selected mask mapping.

Delayed solution: isolate sources into separate queues and retain one register-service owner. `threads.c` verifies2000 records/source, exactly one consumer, per-source order and all eight payload bytes. Its retries are a hosted test driver, not an ISR implementation. A second MCU needs reachable shared storage, compatible layouts, actual atomics/coherence, reset epochs and absence of uncoordinated peripheral access. A lock can be an alternative only with a proved progress/context contract; a preempted lock owner and spinning ISR cannot progress. The explicit state model demonstrates that deadlock without hanging a real process.

## 03.c

Capacity N−1 prevents ambiguity without sharing cursor ownership. Five records with no consumer accept the first three and reject two. Drop-newest does not mutate tail. At saturation, UINT32_MAX means at least that many drops, not an exact total. The deliberate wrapping-counter control fails the saturation check.

The retain-newest solution is a separate `struct xi_latest` representation: a three-record deque with one externally serialized first/count pair. Full push removes the old first, increments drops, then appends the new record. Burst1..5 therefore returns3,4,5. Both push and pop need the same exclusion contract; atomic head alone cannot protect a producer that now modifies reader-owned state. Accept an immutable versioned-slot design with a complete equivalent ownership/retry proof, but not a tail store grafted onto SPSC.

## 04.a

The midpoint is `(9,~7u)`; either scalar can be naturally atomic while their relation is false. A C atomic per field changes individual access semantics, not aggregate transactionality. `interference.py` covers four actors and three masks, including DMA/core1 which ignore both masks. `locks.c` supplies5000 actual mutex-protected observations with both cooperating CPU threads using the same mutex. This does not make DMA honor a CPU lock.

The delayed ownership repair is either shared synchronization used by every CPU accessor in a context that can progress, or private records transferred to a single owner. Hardware access must be scheduled through a separate lease. A clean shared cache line can still be stale on a noncoherent second core; release/acquire alone does not implement a device cache maintenance protocol. Accept a fully specified noncacheable shared region only with its real memory-type, reachability and ABI premises. Do not accept “volatile shared” as that specification.

## 04.b

Binary mask entry returns the original bit; exit restores it. Two nested sections starting0 produce saved0,1 and leave1,0; starting1 produce saved1,1 and leave1,1. Model tokens reject non-LIFO/stale exits before changing state. The assembly interface intentionally has no token ledger: its caller owns that proof obligation.

Threshold combination is `min(nonzero(old,request))`, or0 if both are zero. A request0 does not weaken old0x40. `BASEPRI_MAX` supplies the monotonic raise; ordinary BASEPRI assignment in the negative port breaks the old/request comparison. Early-clobber `=&r` on the saved output matters because MRS writes it before the threshold input is consumed by MSR. A compiler memory clobber orders compiler memory assumptions, not speculative hardware accesses. The port includes the selected architecture barriers and tests actual mask readback. The4096 triples are complete only for the selected16-value set; the frame producer separately validates implemented-bit encodings.

## 04.c

With BASEPRI0x40, IRQ0x20 runs between first and second writes and observes a bad pair; IRQ0x60 remains pending until restore. This controlled property violation is a useful security demonstration, not evidence of privilege escalation. The original source/core ownership still applies: masking CPU IRQs cannot stop an external DMA write. Repair by excluding every relevant local accessor or giving the high IRQ private publication storage. The delayed review must name every shared field the promoted IRQ can reach, including service.active, p_phase, p_index, software acknowledgement/drop counters and raw peripheral registers. Merely changing a priority constant is not a repaired ownership argument.

## 05.a

Slot lifetime alternates producer-owned → published/readable → consumer-held → reusable. Ordinary payload writes happen-before the consumer's reads via release head/acquire head; ordinary reads happen-before the next producer overwrite via release tail/acquire tail. Own-index loads can be relaxed because each role is the only writer. Remote stale observations may refuse progress; coherence and the bounded outstanding set prevent stepping through a live slot. Explain the induction through each physical slot's reuse, not just the first transfer.

The two-producer state counterexample reserves slot0 twice, writesA, writesB, publishes head1 and observes onlyB. This is a defined logical model of the failed premise, not a portable execution claim for racy C. Actual per-source queues fix the premise while requiring one dispatcher. A general MPSC design is an alternative only with distinct reservation and publication phases, full/failure semantics and a progress proof; it is not necessary to claim that generality here.

## 05.b

The word-index target emits actual atomic word loads/stores and barriers, with no libatomic import. The queue API rejects a platform whose ATOMIC_INT_LOCK_FREE is not2. Lock-free in ISO terminology is not a hard latency bound; the selected code's bounded load/store path needs its own generated-code argument. Default alignment follows the atomic type and the declared target ABI; placing the queue in a packed or misaligned byte buffer invalidates admission.

The changed-width fixture is an actual64-bit-index N−1 queue, not an unrelated arithmetic intrinsic. Hosted tests validate3000 records over1000 wraps. Both M4 compilers emit `__atomic_load_8`/`__atomic_store_8`; the strict direct link rejects them. That is a dependency-profile result, not a claim every implementation lacks64-bit atomics. `struct xi_wide` illustrates an explicitly different serialized two-word counter: compute low+n, detect carry via newLow<oldLow, reject high overflow before either store, restore saved mask. It is not falsely presented as a working lock-free64-bit queue. Monotonic-width alternatives must rederive the modulus/capacity relation and all ownership/accessor serialization.

## 05.c

Premature head mutation1 violates publication while the producer still owns the payload. The deterministic phase test sees a nonempty queue too early. GenMC separately diagnoses a non-atomic race in the actual C queue with that defect and with either missing head release (mutation7) or tail release (mutation8). The latter needs reuse, which is why the bounded model attempts three sends/receives at capacity1. Post-join draining checks successful publication count equals consumed count and at least one record was published; no all-empty vacuity. The positive exploration is20 complete executions under the named RC11 model, not every C17 implementation or every queue history.

Tool limitations are retained: GenMC initially crashed while promoting a memory intrinsic in the whole service/frame translation unit. The actual shared queue core was isolated and its real record copy expressed as fixed field/byte operations. Native/target builds use that same core. Selected O0 IR is accepted; an O1 attempt still encounters memory-intrinsic promotion failure. Neither failed attempt is counted as a successful model check, and unrelated service/mask behavior is not attributed to the queue model.

For delayed DMA composition, `dma_handoff.c` imports the frozen actual `pd_submit/poll/finalize`. A clean speculative cache refill obtains0 while DMA owns the line; backing memory becomes42. EN0 alone is insufficient, so the queue remains empty until FIFO/TC quiescence and finalization. Correct post-invalidation makes the copied record42. Omitting it (PD_MUTANT6) makes CPU publication faithfully publish stale0. The harmless assertion distinguishes device visibility from C ordering. The simplified model has whole owned32-byte lines, no dirty unrelated neighbor and one extent; the full imported DMA package carries the broader bank/cache/partial-progress obligations.

## 06.a

The ordinary fixture rejects push4, returns1, then admits5 and returns2,3,5. Empty/full never rely on a mutable count shared by both roles. `oracle.py` uses Python deques, independently checks the declared LCG operation schedule and exact coverage of256 seeds ×64 operations ×three capacities, and validates4096 threshold triples. The algorithmic expected results are not generated through the tested ring code.

The unchanged full-capacity CMX provider is compiled directly with function-section elimination of unrelated components. Capacities1,2,4,8,16 each receive64 rounds, totaling1984 records; both counters start UINT_MAX−3 under quiescence. Outstanding count remains between0 and capacity, which is smaller than the unsigned modulus. Thus the modular difference equals the mathematical distance even across wrap. One-writer atomic coherence prevents an owner from seeing a remote counter move backward in modification order; per-slot handoff prevents advancing through an unread/unreleased full ring. Do not use that proof for arbitrary reset, signed counters or two writers. No new provider is substituted under the existing name.

## 06.b

Public phase tests hold an unpublished producer record and an unreleased consumer record. Mutation2 advances tail early and permits a supposedly full push, so the held-slot assertion fails. All target/native profiles operate multiword records; no single-copy atomicity assumption is available. Explicit field copies and original compiler helpers have different instruction shapes but the same ownership interval. Sanitizer absence is finite execution evidence only; the release/reuse proof is still required. The root API does not promise a borrowed pointer into the queue: `take` copies to independent storage, so retaining that copy after release is valid.

## 06.c

Independently, each burst group has five arrivals and only three slots before its drain, so two drops/group and16 overall; normal foreground drains prevent any drops. The actual target sends all40 events in each declared800000-tick window. Samples contain sequence IDs, drop status, entry/publication brackets/exit/consume times, with no invented records for dropped events. `target_oracle.py` verifies every expected accepted/dropped ID and computes nearest-rank quantiles. It requires burst minimum latency lower bound to exceed normal maximum upper bound in this deliberately separated workload; that is an observed test property, not a general theorem about bursty systems.

The publication instant lies within its read brackets. The counter's25MHz tick is40ns of selected model time; `-icount shift=0` supplies virtual scheduling. The32 read/storage overhead brackets may quantize to zero. `compare_timing.py` compares instrument0/1 images with the same admission and event-window contracts; both still retain entry/exit probes, so neither is a truly uninstrumented firmware measurement. Report raw distributions without subtracting a guessed overhead constant. Physical timing requires the separate board capture protocol and may differ in cache, bus, clock, trace backpressure and interrupt latency.

## Assessor decisions and delayed package

For each original criterion record the exact code/reasoning artifact, its own observed checks, a misconception it rejects, and limits. Alternative valid representations receive credit when they preserve the requested capacity, ownership and failure contract; a changed contract must be explicitly labeled. Superficial completion is one happy-path FIFO run or a mask instruction name. Competent completion includes both publication edges, exact saved-state restoration, failure outputs, object imports and the specific boundary controls. Independent transfer requires deriving the delayed ownership/capacity/visibility contract unaided, diagnosing a withheld mutation, and reproducing evidence under changed compiler or actor assumptions.

Keep delayed tasks as twelve separately scored packets, not one generic extra test. Use the original minimum-days value and record actual attempt dates; never backfill a learner receipt from this reference execution. External review, physical measurements, independent attempts and delayed performance remain pending even when every author campaign passes.
