# T54 AMP followthrough

Implement the incomplete public policy files using the exact original project and criterion in `criteria.json`. Predict a counterexample before consulting any private assessor work. `run.py` compiles one selected mechanism; its default sources and provider manifest are deliberately incomplete. Accepted earlier components may be supplied explicitly. Changing a provider changes the evidence identity.

The C baseline is C17. Host GCC/Clang, pthreads, sanitizers, Python and QEMU are verification tools. Target images contain explicit original objects only: no libc, libgcc, libatomic, compiler-rt, HAL, OpenAMP or operating-system implementation is admitted. `target/build.py` requires an explicit source tree and defaults to the public incomplete provider manifest. A changed64-bit division operation deliberately fails dependency closure until an original provider is implemented and audited or the operation is rejected.

## Boot and queue ownership

AfBoot is a fixed six-word prefix. Both images must agree on32-bit atomics, offsets, version and layout before accessing payloads. One supervisor owns ready/version/layout/epoch; the peer owns acknowledgement/park publication. AfBootOwner starts zeroed. Construction and ring reset require the peer and every other possible writer, including DMA, to be quiet. Numeric booleans are assertions supplied by a trusted port, not proofs by themselves. A timeout does not supply either assertion. EpochUINT32_MAX retires.

Elapsed32-bit observations must be monotonic modulo2^32, with nonzero intervals below2^31 and observations less than half a wrap apart. `af_boot_status` expires at the exact deadline even if an acknowledgement is then present. A completed boot stops using that handshake deadline as a service deadline. A missing/changed clock invalidates the timing premise.

AfRing has one producer and one consumer, no reentrancy, no concurrent reset/copy, capacity16 and maximum payload32. All caller buffers, control objects and outputs remain live, properly aligned, disjoint and exclusively owned for each call. Batch size is1..4; all records are validated before payload writes and a single release head publication. The consumer acquires head and releases tail after copying. Rejected short receive preserves the slot and output. `af_queued` is a snapshot for the producer, consumer or a quiescent observer; it is not a transactional third-party snapshot during arbitrary concurrent progress. Transport epoch and application operation identity are distinct.

## Notification and cache

HSEM registers require word accesses. Only matching lock/COREID/PROCID0 may release. One serialized local user owns an outgoing read-take semaphore; the target saves/restores PRIMASK. Each receiving core uses its own mask/status/clear bank. Notifications coalesce; head/tail remain authoritative. The service adapter masks, clears, drains a bounded budget, rearms and rereads state. Remaining backlog remains runnable. The target pipeline polls bounded queues as task work and uses HSEM as a hint.

Control occupies a distinct8KiB noncacheable MPU reservation, normally SRAM3/D2 at30040000 or the changed SRAM4/D3 at38000000. DMA payload remains at30042000..30042100. The cached variant enables a selected32-byte-line/four-way M7 D-cache only from the declared cold disabled-cache state, invalidates all sets, and uses the actual frozen cache/DMA providers. It never discards a live dirty cache. The baseline cache-disabled image is a separate comparison. Whole-line ownership is required before clean/invalidate; rounding a subspan does not create that ownership. Coherent machines may remove explicit maintenance, but retain ownership and completion ordering.

## DMA and application results

AfBuffers is initialized once from a zeroed object and a trusted cold/fenced device. It owns one serialized DMA stream and two fixed disjoint source/destination64-byte pairs. Slot0 is30042000/30042040; slot1 is30042080/300420C0. Only FREE→FILLING permits caller writes; DEVICE forbids reads, writes, reuse and release. A second buffer may be staged while one is device-owned; the third admission is refused. Configure/refusal is nonpublishing, and callbacks cannot reenter the adapter. The provider descriptor is copied, but payload bytes remain borrowed.

Timeout requests stop and retains ownership. Actual Pd/Xd completion requires matching token, EN clear, FIFO quiet, terminal status and visibility completion. A quiet error returns storage with a possibly touched extent; it does not turn that extent into valid payload. COMPLETED error storage may be released, while successful consumption requires a full XD_COMPLETE result. Tokens never wrap. The two controller policies have different reachable regions, alignment and extent bounds; host pointers do not demonstrate either bus route.

AfReceipt is a serialized, latest-only application receipt in survivor-owned memory, with checked int32 addition. Identical latest operation retries return its saved result; changed payload conflicts, older IDs are stale and MAX retires. It survives only when that actual memory and owner survive. It is not a durable power-failure transaction or a receipt history for multiple clients.

AfLedger is a serialized bounded128-record survivor ledger initialized with a nonzero epoch. IDs are admitted in increasing order once. Completion or declared drop after acceptance is terminal. A restart requires both quiescence premises and marks unresolved accepted records uncertain without altering their old epoch. Old replies do not close new work. Full-machine reset persistence is outside this in-memory contract.

## Executable target experiments

The transport image provides the boot/queue/cache/DMA comparison. The pipeline image composes the original Cortex-M scheduler and context assembly: M4 acquisition ISR → private queue2 → producer task → shared request queue4 → M7 worker task → response queue8 → M4 sink. Every task has caller-owned512-word stack storage. Interrupt capture is bounded to1 or4 records, parsing/compute occurs in tasks, and each visit yields. The M7 device task alone owns DMA; the supervisor waits for its explicit parked/quiet acknowledgement before reset.

After256 offered IDs have terminal records, readiness is withdrawn, M4 parks, DMA becomes quiet, queues are reinitialized under that fence and epoch2 starts. After512 offered IDs, completed/rejected bitmaps must be disjoint and cover every identity. This drained cooperative restart is distinct from an unexpected peer reset, which fails closed. The native ledger separately exercises unresolved work at restart. A local-compute build moves computation to M4 and eliminates the request crossing. The M4 interference build adds bounded work in its SysTick hook; this is not a physical latency estimate.

AN386 runs the same task bodies on one emulated Cortex-M4 to exercise actual SVC/PendSV/SysTick and bounded queues. It does not emulate an H745 pair, DMA, cache coherence or physical oscillators. H745 images record maps, machine code and resource sizes but remain physicalnot-run. The declared10/20/30microsecond pipeline is an executable event model, not a substitution for measured task costs. DWT-origin subtraction across cores is invalid without calibration.

## Evidence and independent work

Keep compiler/options, original and changed images, source/provider hashes, complete per-ID classifications and raw logs. An author/native/model/emulator pass does not establish a delayed independent learner attempt or external review. The original minimum-seven-day variant remains unadministered until observed independently. Use `BOARD_CAPTURE.md` and the capture schema for later separately authorized physical observations; no sample is supplied as physical evidence here.

Example hosted attempt: `python labs/amp_followthrough/run.py --group boot --build build/my-amp-boot`. Its initial semantic rejection is expected. Do not replace the incomplete policy with assessor files and call that a learner solution.
