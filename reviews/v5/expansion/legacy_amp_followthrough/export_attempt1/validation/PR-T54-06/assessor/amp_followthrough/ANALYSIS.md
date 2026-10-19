# T54 private assessor derivations

These author derivations are separate from the incomplete public learner package. Evidence kinds are restricted to each exact mechanism.

## AX-PR-T54-01-T54.01.a

The M7 image occupies flash08000000 and private DTCM20000000; M4 uses flash08100000 and private SRAM130000000. The shared control is a separate8KiB reservation at30040000. The two private maps do not overlap. The selected master ledger rejects treating DTCM as peer-accessible SRAM.

Changed condition: The changed image reserves control at38000000 in D3 SRAM4 and updates the MPU base. DMA payload stays in D2 SRAM3 at30042000; moving control does not silently move the DMA bank. RM0399rev4p142 supports the peer SRAM4 route; physical power/clock behavior remains unobserved.

Evidence selectors: gcc-O0-images transport-gcc-O0 transport-gcc-O3.

Scope: Native range/owner ledger plus actual separate ELF placement; no bus observation.

## AX-PR-T54-01-T54.01.b

The builder compiles each core with its selected ISA, C17 and strict warnings, links explicit original objects with arm-none-eabi-ld, checks no unresolved symbols, and rejects runtime archives. Original byte helpers and the actual context/scheduler objects are source-bound providers.

Changed condition: The uint64 division object imports __aeabi_uldivmod at both optimization levels. Linking that object without an admitted provider fails with that exact symbol. This successor rejects the new dependency; it does not quietly enable libgcc.

Evidence selectors: transport-gcc-O0 dual-pipeline-clang-O3 dependency-gcc-O0-imports dependency-clang-O3-reject.

Scope: Actual objects, linked maps and intentional unresolved-helper rejection.

## AX-PR-T54-01-T54.01.c

M7 owns shared initialization, RCC allocation, cache policy, DMA2stream0 and supervisor state. M4 owns acquisition and source/sink tasks. Each core owns its NVIC bank and outgoing doorbell. A second claimant is refused by af_owner_claim; common addresses do not serialize register writes.

Changed condition: The native owner transfer assigns telemetry to M4. The interrupt-bank derivation changes HSEM IRQ125/C1bank to IRQ126/C2bank, COREID3 to1, while M7 retains shared RCC authority. The local-compute firmware moves the compute task to M4 and eliminates cross-core request traffic; cost observations are separately recorded.

Evidence selectors: gcc-O0-images gcc-O0-doorbell qemu-local-gcc-O3.

Scope: Ownership derivation and actual selected bank/model/task-placement checks.

## AX-PR-T54-02-T54.02.a

AfBoot has six fixed32-bit atomic words, version2 and layout41463220. af_boot_ack acquires readiness, then rejects wrong version, layout or epoch before any AfRing payload is accessed. The stale-header and omitted-layout-check control distinguish the failure.

Changed condition: Increasing the payload-layout identity by4 is rejected while the queue remains untouched. Both images have static offset/size checks; adding a wider field requires a new layout value and new maps, not a cast over retained bytes.

Evidence selectors: gcc-O0-boot gcc-AF_MUTANT1 gcc-O0-recovery.

Scope: Concrete stale-prefix/layout tests and target static ABI checks.

## AX-PR-T54-02-T54.02.b

Both owner acknowledgement and peer readiness waits use unsigned elapsed counts. Zero intervals and intervals at least2^31 are refused; a delta at least2^31 is ambiguous and fails closed. The exact deadline is expired even if an acknowledgement appears then.

Changed condition: The native case starts atFFFFFFFC and observes2: the modular delta is6. Recovery tests also cross wrap. This argument requires monotonic32-bit observations less than half a cycle apart; DWT reads in firmware need the board frequency/running-clock premise and are not calibrated by AN386.

Evidence selectors: gcc-O0-boot gcc-O0-recovery.

Scope: Checked wrap arithmetic and compiled bounded wait paths; no physical timer calibration.

## AX-PR-T54-02-T54.02.c

Unexpected M4 restart with an already-acknowledged ready epoch fails closed. A controlled drained restart first withdraws readiness, waits for the peer park acknowledgement and the DMA task quiet acknowledgement, then initializes rings and publishes a strictly newer epoch. BOOT_C2 only releases cold boot and is never used as a running-core stop.

Changed condition: Cuts before and after every one of four header publication stages replay completed stages inertly, reject skipped stages and withhold traffic until exact acknowledgement. Native reset-with-pending ledger tests classify uncertainty; the emulator executes the distinct drained epoch1to2 task rendezvous.

Evidence selectors: gcc-O0-recovery qemu-pipeline-gcc-O0 ledger-gcc-O0.

Scope: Actual header transitions, native pending-work classification and single-core emulated rendezvous; no physical reset execution.

## AX-PR-T54-03-T54.03.a

At head1/tailFFFFFFFE, distance is3. Capacity16 admits at most13 additional records. Two producers can both read the same head and overwrite the same slot; this API therefore forbids a second producer and any reset while active.

Changed condition: af_batch validates all1..4 records and space before writing slots, then publishes one release head. Consumer acquisition and release tail protect payload visibility and reuse. The actual pthread campaign transfers100000 variable-size records in batches4 across wrap.

Evidence selectors: gcc-O0-ring gcc-O3-threaded clang-O3-threaded.

Scope: C17 ownership argument plus real single-producer/single-consumer threads, not an all-interleaving proof.

## AX-PR-T54-03-T54.03.b

The event counterexample publishes head, observes stale slot0, then writes42. With payload-before-release and an acquire reading that publication, payload42 is ordered before consumption. Volatile alone creates no C synchronizes-with edge.

Changed condition: GCC/Clang O0/O3 images retain DMB in the ring paths and DSB around hardware boundaries. Disassembly is inspected alongside the source proof; neither emitted barriers nor native threads establish a physical noncoherent cache result.

Evidence selectors: analyses transport-gcc-O0 transport-clang-O3.

Scope: Finite event counterexample and actual compiler ordering artifacts.

## AX-PR-T54-03-T54.03.c

Four batches fill16 slots; record17 is refused without advancing head. Drain preserves numbered payloads across uint32 wrap. Stale epoch is rejected without consuming the slot.

Changed condition: Lengths0..32 are admitted. Every insufficient receive buffer preserves tail and untouched output bytes; retry with sufficient capacity reads the same record. The two-thread run varies every length and validates each payload byte.

Evidence selectors: gcc-O0-ring clang-O3-ring clang-O3-threaded.

Scope: Actual C queue/payload behavior; physical caches remain separate.

## AX-PR-T54-04-T54.04.a

Three queued records survive one pending interrupt. Service masks, clears the local bit, drains a bounded amount, rearms, and rereads queue state. Existing periodic task service also does not require one interrupt per packet.

Changed condition: The register model injects a producer publication at every mask/clear/rearm boundary and after the final check. Either a record was consumed, the queue remains runnable, or an enabled pending interrupt exists. The model assumes hardware retains masked interrupt status as documented; board delivery latency is separate.

Evidence selectors: gcc-O0-doorbell clang-O3-doorbell.

Scope: Exact coalescing/rearm register model and task service code; no silicon interrupt timing.

## AX-PR-T54-04-T54.04.b

Failed ownership readback produces no release write. COREID3(M7) and1(M4) and process0 must match. Mask/clear at58026500/504 belongs to M7;58026510/514 belongs to M4. The actual target wrapper saves/restores PRIMASK for the one local outgoing owner.

Changed condition: Tests move outgoing semaphores0..3 and corresponding incoming4..7, deriving RLR80+4s, release4s and bit1<<s. The firmware uses independent8/9 assignments. Wrong-bank and failed-take-release mutants are rejected.

Evidence selectors: gcc-O0-doorbell gcc-AF_DOOR_MUTANT1 gcc-AF_DOOR_MUTANT2.

Scope: Actual adapter function executed against bounded register model; manual offsets and image code separately bound.

## AX-PR-T54-04-T54.04.c

A budget2 drains the three-record example in visits2+1 and sets runnable after the first visit. A mutant that clears runnable loses this obligation. Firmware tasks bound their work and yield through the original scheduler.

Changed condition: For arrivals at most3 per service interval and service budget2, backlog grows by1 per interval until finite capacity rejects; no finite steady bound exists. For a burst3 followed by at most1 per interval and budget2, maximum queued backlog is3 and it drains to at most1 by the next interval. The finite pipeline model records exact saturation/rejections under a downstream pause.

Evidence selectors: gcc-O0-doorbell gcc-AF_DOOR_MUTANT3 analyses.

Scope: Concrete budget tests and explicit arrival/service algebra; no unstated stability assumption.

## AX-PR-T54-05-T54.05.a

3004001F plus2 spans two32-byte lines: [30040000,30040040),64bytes. The cache model shows an unrelated dirty neighbor lost when the whole line is invalidated.

Changed condition: At64-byte lines the same original interval is one64-byte line; changing the address to3004003F makes two lines ending30040080. af_cache_span checks addition and rounding overflow. Buffer placement uses complete disjoint64-byte extents; it never grants ownership merely by rounding a borrowed subspan.

Evidence selectors: gcc-O0-cache analyses.

Scope: Native overflow/range tests and explicit dirty-neighbor model.

## AX-PR-T54-05-T54.05.b

The early-doorbell model observes stale bytes until clean. The frozen Pd path cleans source and destination, invalidates destination, performs release and enables DMA; after quiet completion it invalidates destination and acquires before publication.

Changed condition: The receive model starts dirty, models a speculative stale refill during DMA, and requires post-completion invalidation. Buffer tests consume the actual frozen Pd/lease composition and reject mutants omitting destination preclean or final invalidation. Unrelated dirty neighbors are excluded by the whole-line allocation contract.

Evidence selectors: gcc-O0-buffers gcc-PD_MUTANT3 gcc-PD_MUTANT6 analyses.

Scope: Actual Pd callback order and bounded cache-state counterexample; not physical DMA visibility.

## AX-PR-T54-05-T54.05.c

Transport images preserve the cache-disabled comparison. Cached pipeline images map only8KiB control noncacheable, initialize the selected M7 D-cache and exercise the actual DMA/cache adapter on disjoint payload lines at30042000. Their maps and code are audited; physical data/ordering observations remain not-run.

Changed condition: In the coherent model the external agent sees the shared memory directly, removing explicit clean/invalidate copies. The exclusive DEVICE lease, completion ordering and stale-generation rejection remain necessary. Coherent native success is not evidence about H745 cache lines.

Evidence selectors: dual-pipeline-gcc-O0 transport-gcc-O0 analyses.

Scope: Actual cached-path image construction and coherent/noncoherent premise comparison; no board result.

## AX-PR-T54-06-T54.06.a

AfBuffers implements FREE,FILLING,DEVICE,COMPLETED,CONSUMING,FREE around actual Pd/Xd providers. DEVICE refuses filling, consumption and release. The two physical source/destination pairs are fixed and disjoint; one stream has one serialized task owner.

Changed condition: At most one buffer can be device-owned and the other staged or retained. A third fill is refused. Completion permits the next staged submission while the completed other buffer remains borrowed by its consumer; reuse requires its release. This is bounded ping-pong storage, not an unbounded queue.

Evidence selectors: gcc-O0-buffers dual-pipeline-gcc-O0.

Scope: Concrete ownership adapter and actual linked DMA2 port; native device observations are simulated.

## AX-PR-T54-06-T54.06.b

The actual adapter requests stop, retains DEVICE/STOPPING ownership while EN or FIFO permits writes, and ignores an old token. EN0 without FIFO quiet is insufficient. Monotonic tokens prevent old completion publication into the next request.

Changed condition: A quiet transfer with TE returns XD_ERROR, zero guaranteed bytes and64 possibly touched bytes. The storage can be released because writes have ceased, but af_consume rejects it as a successful payload. The next generation completes normally after required post-invalidation.

Evidence selectors: gcc-O0-buffers clang-O3-buffers.

Scope: Mechanism-specific Pd/Xd completion and visibility checks; no timeout-as-quiescence inference.

## AX-PR-T54-06-T54.06.c

The selected H745 DMA2 policy accepts whole-line spans only in the SRAM3 payload bank and refuses DTCM20000000. Target configure uses DMA2 base40020400, stream0, FIFO mode and checked NDTR.

Changed condition: The F439 DMA2 policy admits20000000..20030000, rejects the H745 destination, and refuses4097-byte extent beyond its4096-byte contract. The original target provider derives different RCC/base/error handling for each board; this package executes validation and binds that source, while physical bus/error observations remain open.

Evidence selectors: gcc-O0-buffers gcc-O0-images dual-pipeline-clang-O0.

Scope: Actual selected engine validation plus linked target source; no physical reachability measurement.

## AX-PR-T54-07-T54.07.a

1000 minus900 equals100 arithmetically but has no latency unit or meaning without a mapping. The firmware records per-core local counts; common external capture is required for cross-core latency.

Changed condition: Two exchange samples admit rate1.9 with offset[3,40], rate2 with[-20,30], and rate2.1 with[-33,10]. Nonnegative asymmetric delays yield these inequalities; choosing the midpoint as exact would discard justified uncertainty.

Evidence selectors: analyses.

Scope: Exact rational interval derivation from declared synthetic exchanges, not measured oscillator calibration.

## AX-PR-T54-07-T54.07.b

48000/480000000 is1/10000second,100microseconds. FFFFFFF0 to20 gives48 modulo32-bit ticks under an admitted no-extra-wrap interval. A declared24-cycle probe changes48000 to47976,99.95microseconds; the declared cost is illustrative, not measured on this board.

Changed condition: 24000counts at480MHz followed by24000 at240MHz total150microseconds. Applying480MHz to all48000 falsely yields100microseconds. The analysis retains exact fractions and the capture schema requires segment frequency and overhead bounds.

Evidence selectors: analyses gcc-O0-boot.

Scope: Exact arithmetic and explicit frequency/probe premises; no invented timing sample.

## AX-PR-T54-07-T54.07.c

The1000 ordinary synthetic samples have maximum106units. No universal bound follows; the report records that WCET is not established. The physical capture plan requires raw samples, resolution, interference, omissions and exact image identities.

Changed condition: A previously absent burst produces1800-unit samples. The old maximum/average cannot characterize the new workload. The target interference variant adds bounded work only in the M4 acquisition interrupt path; AN386 executes this path but does not measure H745 interrupt timing.

Evidence selectors: analyses qemu-burst-clang-O3.

Scope: Finite counterexample plus actual emulated interference execution, not a physical WCET observation.

## AX-PR-T54-08-T54.08.a

af_effect serializes a latest-only operation receipt in survivor-owned memory. Cuts before effect, after effect before reply, and after reply all permit an exact duplicate to return the same result without a second addition. Changed payload conflicts and an old receipt outside the one-entry horizon is stale.

Changed condition: Peer reset preserves the still-live supervisor receipt; full-machine reset does not. A new application session prevents old-session replay, but recovering already performed external effects needs persistent application records outside this in-memory contract. The code claims neither power-fail atomicity nor general exactly-once execution.

Evidence selectors: gcc-O0-receipt gcc-O0-recovery.

Scope: Actual application receipt transition/cut tests with explicit retention horizon.

## AX-PR-T54-08-T54.08.b

Header tests reject old epoch8 acknowledgement after publication of9; sequence reuse cannot override epoch mismatch. af_ledger_restart retains each old record epoch, and terminal results with the wrong current epoch are stale. MAX epoch retires instead of wrapping.

Changed condition: Recovery cuts before final ready and before matching acknowledgement leave the traffic gate closed. Trusted peer and device quiescence are prerequisites for ring reset; a numeric generation alone proves neither.

Evidence selectors: gcc-O0-boot gcc-O0-recovery ledger-gcc-O0.

Scope: Concrete epoch/gate and per-record stale-ack checks; physical reset authority not inferred.

## AX-PR-T54-08-T54.08.c

The firmware executes a cooperative park/quiet/new-ready handshake; the AN386 task experiment verifies epoch1to2 while all accepted IDs of the first epoch have terminal records. An unexpected M4 reset is detected and fails closed rather than claiming this controlled restart succeeded.

Changed condition: The native ledger admits100, completes90 and drops5 after admission. Trusted restart leaves the other5 uncertain in their old epoch; late old results cannot complete newly admitted epoch2 work. No queued effect is silently retried or declared undone.

Evidence selectors: qemu-pipeline-gcc-O0 ledger-gcc-O0.

Scope: Actual single-core emulated drained rendezvous and separate native pending-work recovery model.

## AX-PR-T54-09-T54.09.a

The discrete event implementation has costs10/20/30 and waiting capacities2/4/8; arrivals80 apart complete in60. The actual task pipeline uses exactly those queue limits, though its machine costs are measured separately and are not assumed to be10/20/30microseconds.

Changed condition: A downstream pause through400 with arrivals every5 saturates all three queues. The saved model output lists exact completed, dropped and outstanding IDs with start/finish times. Backpressure retains an in-service item until the next stage admits it.

Evidence selectors: analyses qemu-pipeline-gcc-O0.

Scope: Executable event schedule and actual bounded task queue composition; costs are explicit model premises.

## AX-PR-T54-09-T54.09.b

The target hook captures at most1 or4 records, with no parsing or blocking call; producer, worker and sink have distinct original scheduler stacks. The M7 worker can delay downstream response independently of M4 task priorities.

Changed condition: The model separately pauses stage0 for M7 interference and stage1 for M4-only interference, recording each queue occupancy. The actual bounded M4 interrupt burst executes in the AN386 same-task-code variant; physical per-core growth and latency must be captured on H745.

Evidence selectors: analyses qemu-burst-gcc-O0 dual-pipeline-clang-O0.

Scope: Actual interrupt/task separation plus explicit per-stage interference schedules; no cross-core measured bound.

## AX-PR-T54-09-T54.09.c

The exact ledger starts with100 accepted records and ends with90 completed,5 explicitly dropped after acceptance and5 uncertain at old epoch1. All100 identities remain classified. The separate live pipeline counts offered = completed + admission-rejected only after its drained final boundary.

Changed condition: Moving compute to M4 removes cross-core request messages for the same512 offered IDs and same bounded ISR burst. The event model retains equal results and changes traffic8to0 on its slow workload. Actual queue maxima and task-switch counts are recorded for each emitted machine image, without comparing unrelated clock origins.

Evidence selectors: ledger-gcc-O0 analyses qemu-local-gcc-O3.

Scope: Exact accepted-input ledger and actual same-correctness moved-task experiment.

## AX-PR-T54-10-T54.10.a

Reports hash source/providers, ELF, maps, disassembly, logs and commands. Each image records its actual CPU, role, layout, memory intervals and physicalnot-run status. Native and AN386 identities are not substituted for a board serial/revision.

Changed condition: Both compilers and O0/O3 produce separate preserved artifacts; the source guard rejects changed provider/receipt hashes. Historical pilots remain separate, including the initial header-selection and linker-script failures.

Evidence selectors: transport-gcc-O0 transport-clang-O3 qemu-pipeline-clang-O3.

Scope: Source-bound native/image/emulator receipts and immutable attempts.

## AX-PR-T54-10-T54.10.b

The dual pipeline composes original per-core task scheduling, bounded acquisition2/shared4/response8, HSEM hints, M7 DMA/cache leases and a drained epoch restart. AN386 executes the same task pipeline on one core and validates512 offered IDs with disjoint completed/rejected bitmaps. H745 parallel execution and external timing remain not-run.

Changed condition: The local-compute variant moves the compute service to M4 and retains512 offered identities and the same burst4 contract. It eliminates the request crossing; image sizes, stack allocations, queue maxima and raw scheduler counts are retained separately. No native timing is promoted to board CPU/I/O cost.

Evidence selectors: dual-pipeline-gcc-O0 qemu-burst-gcc-O3 qemu-local-clang-O3.

Scope: Actual source composition, separate dual-image audit and single-core machine execution.

## AX-PR-T54-10-T54.10.c

All physical_status fields remain not-run. The capture template requires board and silicon identity, both flashed ELF hashes, protocol/layout, clock configuration, trace provenance and raw per-ID queue/reset/cache/DMA observations before a physical claim can be considered.

Changed condition: After separately authorized dedicated-board access, flash the exact pair, check probe readback hashes, capture baseline/overload/interference/restart/cache cases and run the capture validator. A failed assumption must revise the contract and trigger fresh dependent images; a completed template alone is not trusted measurement.

Evidence selectors: analyses dual-pipeline-clang-O3.

Scope: Prepared board experiment and strict missing-evidence rejection; no physical or independent learner completion.

