# Original runtime and pinned FreeRTOS comparison

## Identity, families and objective scope

This required successor assesses AX-PR-T52-11-T52.11.a/b/c and their original DELAYED variants. Families A, B, C, D, F, G, I and J apply: explain the distinct semantics, build separate adapters, provoke bounded failures, reject faulty mappings, audit dependencies and change port/rate. Timing distributions and physical interrupt latency are separate work; no checksum implies those claims.

## References and prerequisites

Read the exact V11.2.0 commit `0adc196d4bd52a2d91102b525b0aafc1e14a2386` passages registered in `research/v5/expansion/legacy_realtime_freertos/resources.json`: POSIX port restrictions and allocation, queue wait/timeout, priority inheritance/disinheritance, default conversion and ARM_CM4F startup/interrupt validation. Require completed PR-T52-10, PR-T50-12 and PR-T51-06 artifacts, the actual CMX queue/mutex implementation and public headers. Import those implementations explicitly. Do not copy the vendor kernel into the original runtime or implement a synthetic replacement and call it CMX.

## Environment and dependencies

Use C17, GCC/Clang hosted Linux for the POSIX port, and GCC/Clang Cortex-M4 soft ABI for the original image and softfp for the ARM_CM4F port in an owned QEMU MPS2-AN386 image. The comparison kernel is an explicit external dependency with its clean pinned Git identity. No flashing or host scheduling/security changes occur. The original adapter links actual CMX only. The vendor adapter links FreeRTOS only. M4 links only explicitly supplied course byte helpers and the provided linker script; inspect every symbol and map for hidden runtime dependencies. Minimal freestanding declaration headers supply declarations only; they are not a hosted C library. The vendor startup enables CP10/CP11 before entering softfp-compiled C; the original image uses the soft ABI and rejects generated VFP-register instructions. Integer-only source can still acquire VFP-register copies under a softfp optimization, so source types alone do not prove the instruction contract.

`configMAX_PRIORITIES=5`, preemption/time slicing enabled, tick rate100 or1000, 32-bit ticks, mutexes/notifications enabled, timers disabled, static object allocation enabled and dynamic kernel allocation disabled. Original CMX urgency1 is highest; FreeRTOS priority3 is highest among the workload workers; controller priority4 coordinates outside that comparison. ARM SysTick/PendSV are set lowest by the actual port, SVC highest; `configMAX_SYSCALL_INTERRUPT_PRIORITY=128`, test IRQ priority160. Use the port's assertion checks; an unsupported tick rate is a configuration refusal.

## Problem and concrete supplied inputs

Implement `fc_ticks`, `fc_priority` and `fc_run` in the public mapping/adapter scaffolds. Preserve explicit input and output ownership. Run an eight-slot copied-unsigned queue with values0..255, count256 and sum32640; an out-of-band `UINT_MAX` sentinel is protocol data, not a FreeRTOS close operation. Before that workload, fill all eight slots, attempt a timed send, drain exactly0..7, and attempt a timed empty receive. Use21ms at100Hz and1000Hz through the adapter. A failed receive leaves the caller's sentinel999 unchanged.

For inversion, urgency3 owns a contested mutex and an unrelated second mutex. Admit urgency1 and urgency2 together under a scheduler barrier. Urgency1 waits for the contested mutex; the inherited owner must progress before the unrelated middle task. Record owner-release/high-acquire/middle-work order1,2,3. Observe effective priority before release, after giving the contested mutex while still holding the unrelated mutex, and after releasing both. Use the actual original scheduler to choose each ready task; the CMX adapter is a serialized transition driver, not a native pthread/PSP port.

## Why this mechanism is necessary

Correct count/sum alone misses queue-full behavior, retained ownership, priority direction, tick quantization, timeout outcomes and hidden allocation. FreeRTOS retains inherited priority while any mutex remains held in this selected path; CMX recomputes inheritance from current wait relationships. The two valid nested-mutex results deliberately differ. Do not conceal the difference by changing the workload. A binary semaphore substitute does not provide mutex inheritance.

## Required reasoning and failure contract

`fc_ticks(ms,rate,out)` accepts rate1..10000 and products fitting unsigned32. It computes mathematical ceil(ms*rate/1000), preserving `*out` on invalid/overflow/null output; zero milliseconds maps to polling0. State why the vendor's default `pdMS_TO_TICKS(21)` is floor2 at100Hz while the selected adapter returns3. Three tick boundaries still do not guarantee30ms since an arbitrary call phase; the common contract is a count of observed scheduler boundaries. The original deadline is absolute and CMX returns BLOCKED before a later task.result; the vendor call returns after resumption. Never re-enter a completed CMX wait as though it were its result collector.

All queue values are copied; caller objects/tasks and static storage outlive operations. FreeRTOS has no matching queue-close primitive here: first drain the sentinel protocol, collect worker completion, remove worker tasks, then delete idle queue/mutex objects. This is a bounded quiescent shutdown adapter, not cancellation of unknown blocked users. Original close returns CLOSED on an empty drained queue. Finite traces do not establish arbitrary multi-mutex inheritance equivalence. Complexity/storage analysis must distinguish CMX finite task scans, FreeRTOS priority/event lists and the port's own structures.

## Implementation and experiment deliverables

Supply separate original/vendor images, exact configuration/source/compiler identities, linker maps, symbols, disassembly, stack-usage files and raw results. The M4 vendor image also performs a real emulated NVIC interrupt: a low worker pends IRQ0 after the controller blocks, `xQueueSendFromISR` wakes that controller, and `portYIELD_FROM_ISR` must transfer before the low worker executes its next instruction. This is a software-pended emulator IRQ, not a physical device or latency measurement. The POSIX port uses host pthreads/signals; its event/thread support calls malloc and libc, even with all kernel objects static. State this explicitly.

## Verification and independent expected results

Run public checks and the independent Python integer oracle with exact input rows; retain negative results for overflow, invalid urgency/rate and unchanged outputs. Match count256/sum32640, full/empty timeout outcomes, at least the selected number of observed tick boundaries, trace1,2,3, final restored priority and drained shutdown. Expect nested_retained0 for CMX and1 for this FreeRTOS path. The linked M4 vendor image must contain the actual SVC/PendSV/SysTick handlers and have no allocator, unresolved symbol or unapproved runtime helper; a static config macro alone does not pass. Original artifacts must contain no FreeRTOS symbols, and vendor artifacts no CMX symbols.

Compile and run priority-direction, floor-conversion, binary-semaphore and omitted-ISR-yield controls. Only the M4 vendor port exercises the last control. Require successful compilation followed by the specific semantic check; a hang, crash, sanitizer finding or link error is not a passing negative experiment. Both unsolved adapter and mapping scaffolds must compile and be rejected. A substituted unsolved CMX producer must also fail.

## Per-objective assessor rubric

For .a, level0 omits workload/assumptions; level1 obtains checksum but confuses a virtual port with MCU timing; level2 explains each queue/priority/tick/allocation/ISR/shutdown difference using the supplied distinct observations; level3 transfers the explanation to the exact MCU configuration and distinguishes static kernel objects from static whole-image allocation. For .b, level0 lacks separate adapters; level1 uses config flags without ownership/maps; level2 executes full/empty/inversion/quiescent cleanup and validates source separation; level3 changes conversion/rate without changing the promised external contract and diagnoses the first divergent result. For .c, level0 asserts equivalence; level1 compares aggregates only; level2 retains ordered trace, nested-mutex counterexample and discriminating mutants; level3 repeats port/optimization and states precisely which semantic claims survive and which physical costs remain unmeasured. Independent seven-day learner work is still required; author execution is not learner acceptance.

## Graduated hints

1. Write down which queue operation owns each integer while a sender blocks.
2. Separate the rounding calculation from the kernel call; test1ms and21ms before starting a scheduler.
3. Arrange readiness through notifications and a suspended scheduler so the medium worker cannot run before the high worker attempts its lock.
4. Inspect `uxMutexesHeld` before assuming disinheritance follows every give. Assessor code and technical analysis remain under `assessor/legacy_realtime` and are not exported.

## Changed conditions and independent transfer

For .a-DELAYED, execute the ARM_CM4F image with static tasks/control/storage and inspected vector/BASEPRI/IRQ contract. For .b-DELAYED, replace default floor conversion with checked ceiling at100Hz and1000Hz; retain the same boundary-count promise, and explain why a wall-time not-before promise would require an additional clock/phase contract. For .c-DELAYED, compare GCC/Clang optimization and POSIX/M4 observations; retain the real nested-mutex divergence, changed ISR evidence and source separation. Wait at least seven days for the unaided learner assessment; no author run fabricates that delay.

## Security and isolation

Only original synthetic tasks and the pinned dedicated teaching kernel run. Host tests are bounded child processes; hardware-mode tests are owned QEMU images with no networking. No third-party operational target, credentials, host-policy changes or physical flashing is involved. Timeout supervision is a harness failure signal, not evidence the workload recovered correctly.

## Validation status and remaining limits

Authored executable successor; exact observed profiles/counts belong to its source-bound campaign and guard. No physical measurements, independent human review or unaided learner attempt are asserted. The original CMX image executes its actual transition/scheduling functions on Cortex-M4 but does not turn the serialized driver into a new original PSP-context port. The ARM_CM4F comparison validates the selected port and finite synthetic workloads, not every interrupt priority, floating-point task context, sleep-state or fault-recovery configuration.
