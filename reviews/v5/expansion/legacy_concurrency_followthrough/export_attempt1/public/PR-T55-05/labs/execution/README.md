# Original execution laboratory

Build a small C17 task runtime in layers. `src/` is the unsolved learner scaffold; `include/` defines the contract and `tests/` supplies observable gates. The independent implementation under `assessor/execution` is instructor material and is never a learner dependency. A successful named gate is an anchor, not completion of every experiment in its project brief. Read `family.json` for exact scope.

The laboratory has three evidence levels: a hosted deterministic state-machine harness, original Cortex-M code that actually switches PSP stacks in QEMU, and physical F439/H745 artifacts requiring board observations. Hosted tests cannot establish interrupt latency or physical cache coherence. A strict image audit cannot establish board execution. The supplied H745 implementation covers cold-boot AMP using noncacheable shared SRAM; cached/DMA/recovery extensions remain learner work.

From a learner workspace on Linux/WSL:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build -R '^execution.admission$' --output-on-failure
```

The scaffold builds and fails semantic gates. Implement one stage, predict the trace, run its gate, retain the failure, then consult one hint tier in `HINTS.md`. Do not replace learner files with assessor files. Instructors may explicitly configure a separate build with `-DCMASTERY_ASSESSOR_ROOT=/absolute/repo/assessor/execution`. `-DCMASTERY_SANITIZERS=ON` instruments hosted C code; these libraries are not linked into freestanding images.

`CMASTERY_TARGET` selects the runner context. Supported names are `host`, `cortex-m4-qemu`, `stm32f439`, and `stm32h745`; `family.json` maps stage-specific target gates. CMake exposes all available tests for instructor campaigns, while the curriculum runner selects the project gate. Missing cross tools make those tests unavailable, never passed. Read `targets/README.md` before a target run.

## Contract and order

Start with T50 admission, context switching, states and timer races. T51 adds serialization and blocking mechanisms to that core. T52 analyzes bounded scheduling models before measured target experiments. T54 uses two independent UP instances communicating through explicit AMP ownership. T55 studies concurrent queues and reclamation separately from the serialized scheduler. The x86 SMP port is in the kernel laboratory; this Cortex port does not claim a shared SMP scheduler.

| Part | Supplied executable gate | Learner evidence beyond the gate |
|---|---|---|
| T50 core | static stack admission, lifecycle, RR/FP/EDF, modular timers; actual cooperative/preemptive/FPU guests | ownership diagram, disassembly, timing, stack-fault experiments, hardware tickless programming |
| T51 synchronization | latch, semaphore, mutex/PI, condition, queue, events, notifications, barrier, once, RW lock, teardown | adversarial timelines, invariant argument, primitive port wrappers, failure policy |
| T52 real time | RM/DM order, response time, finite EDF admission, immediate ceiling, sporadic server, CBS, partition/global EDF comparison | measured costs/jitter/blocking, assumption dossier, overload and RTOS/Linux comparisons |
| T54 AMP | original dual image, shared SRAM map, SPSC packets, HSEM IRQ/doorbell, cold release/ack | physical boot, cache/DMA variants, clock calibration, independent peer recovery and traces |
| T55 concurrency | SPSC, identity pool, reservation-stall example, hazard MS queue, serialized epoch tracker, coarse RCU, history checker, pthread stress | linearization/progress/reclamation argument, stalled participant and exhaustion measurements |

All pointers refer to initialized, correctly aligned objects with lifetimes covering every reference and outstanding waiter. APIs do not validate arbitrary memory. Reinitialization while referenced is forbidden. Every scheduler/object operation in `cmx.c` executes inside one serialization domain: saved interrupt mask on UP, or a correctly designed scheduler lock on SMP. None of those functions acquires a lock internally. The hosted harness supplies serialized calls. `concurrent.c` and the RCU experiment use their explicit C atomic contracts instead.

Task-entered operations accept READY/RUNNING slots; a WAITING task cannot call another blocking operation. Raw task IDs are slot indexes, not generation-safe public capabilities. A reap increments the slot generation; adapters exposing long-lived handles must compare the generation. Every fixed pool, waiter sequence and generation has bounded exhaustion behavior. Wait sequence saturation rejects new waits with FULL instead of silently wrapping FIFO order. Reinitialize only after proving all tasks/objects quiescent.

`CMX_BLOCKED` in the model means completion occurs later through `task.result` (and `task.value` for receive/join). Do not immediately re-enter the call. Cortex blocking wrappers suspend and return the completed result after dispatch. These wrappers reject handler mode and any nonzero PRIMASK, BASEPRI or FAULTMASK. Nonblocking give/close may be serialized in handler context; rescheduling occurs at the next dispatch point. Task creation is startup-only in this port. Supply a permanent idle task. Idle must yield in a cooperative build and may WFI in the periodic-preemptive build.

Absolute uint32 deadlines and EDF ordering require every compared interval to be strictly less than 2^31 ticks, with no tick jump of that size. A result that completed first under serialization wins a deadline/signal race. The exact deadline is due. Immediate resource acquisition may succeed even with an expired wait deadline: deadlines limit waiting, not successful nonblocking acquisition. Counters are logical ticks unless calibrated separately.

Semaphores directly transfer a permit to an enrolled waiter. Mutexes are owned, nonrecursive and detect wait-for cycles; PI propagates transitively and is withdrawn when donors leave. Immediate ceiling boosts an owner on acquisition. It is not the original PCP system-ceiling admission rule. Condition wait atomically enrolls and releases the owned mutex; signal, close, timeout and cancellation all require reacquisition before return. That reacquisition can exceed the original wait deadline. Always loop over the protected predicate.

Queue capacity zero is a rendezvous, capacity one a mailbox, and positive capacity a bounded channel. Close rejects new sends and drains accepted items. Abort discards accepted items and wakes both directions CLOSED; it cannot revoke an item already transferred to a receiver. Event flags coalesce, notifications count, and neither is a mutex. A barrier cancellation breaks the entire generation. A failed once initializer wakes waiters RETRY; each waiter must re-enter before becoming initializer. The RW lock favors waiting writers, rejects upgrades, and re-admits readers when the last pending writer withdraws. Writer preference can starve readers under an unbounded writer stream.

Exit refuses owned mutex/once/RW resources. Cancel wakes waits; it is not forced task destruction. Cooperatively release resources, exit, join, switch away and reap before reclaiming stack/object storage. Robust-owner recovery is a separate project policy. The stack canary/range checks detect some corruption after switching; they are not an MPU guard or a proof against every overflow.

## Concurrency and measurement limits

The SPSC queue requires exactly one nonreentrant producer and consumer and a power-of-two capacity at most 16. Release publication and acquire observation protect non-atomic payloads. Native unsigned atomics must be lock-free on the selected target; inspect the final image for hidden atomic runtime dependencies.

The Michael–Scott experiment has 32 nodes and four exclusively owned, nonreentrant participant IDs, each with two hazard slots. Node identity and payload must stay stable while published/protected. Publish hazard, re-read the source pointer, and retry before dereferencing. Link-CAS publishes enqueue; successful head-CAS removes dequeue. A pool state contains a retirement generation so two collectors cannot free a later incarnation using an old retirement observation. Saturated generations are retired permanently. Storage exhaustion can return FULL despite available abstract capacity; count allocation, collection and stalled participants when making end-to-end progress claims.

All queue atomics in the hazard experiment are sequentially consistent for an auditable baseline. The four-pthread test exchanges 40000 distinct values and checks every value exactly once. This is finite stress, not proof or comprehensive weak-memory exploration. The epoch tracker is externally serialized teaching metadata. The RCU experiment has one serialized writer, immutable roots and four active-reader slots; stalled readers can delay reclamation indefinitely. It is not Linux RCU. The history checker accepts only bounded complete histories (at most eight operations); incomplete histories and larger searches require extensions.

Response-time and admission tests are bounded arithmetic models with declared finite horizons. Account for interrupt/dispatch/communication overhead in supplied costs; no target WCET is inferred from them. The comparison workload is exactly 256 values (0..255), an eight-slot channel, close/drain, count 256 and checksum 32640. Hosted comparison prints logical counts; the integrated Cortex guest runs actual tasks. Match these counts to Linux/FreeRTOS before comparing timing distributions on like hardware and clocks.

The instructor negative controls corrupt transitive donation, SPSC capacity, condition mutex reacquisition, FP restore, and startup BSS clearing. Each must produce a named assertion or actual guest failure. BSS is poisoned before guest reset so omitted clearing is observable. `tests/controls.py` records the rejected defects; a build failure does not count as successful defect detection.

## Reading anchors

Use the curriculum's T50–T55 reading dossiers alongside: Arm Cortex-M exception entry/EXC_RETURN and programmer manuals; the ST STM32H745 device definitions and AN5617/AN4839; Liu and Layland on fixed-priority/EDF analysis; Sha, Rajkumar and Lehoczky on priority inheritance; Abeni and Buttazzo on CBS; Michael and Scott on nonblocking queues; Michael on hazard pointers; and the C11/C17 memory-model clauses. Exact inspected register sources and validation scope are recorded under `research/v3/execution_sources.md`. Read source contracts before imitating implementation patterns.
