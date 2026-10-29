# Actual producer bindings and mismatch policy

The source-bound author runner links the following existing original producer
implementations; no copies are substituted inside the continuation. Learners use
their accepted producer revisions and supply corresponding hashes. An author
reference result is not an assertion that learner artifacts were supplied.

| Artifact | Actual operations and consumer use | Boundary |
|---|---|---|
| firmware_io.gpio | fw_gpio sets the initialization status pin | Educational register numbers, not STM32 addresses |
| firmware_io.uart | fw_uart transmits0xA5 only after TXE | Polled header only; DMA loan is separate |
| firmware_io.timer | fw_timer sets1kHz from1MHz, duty500/1000 | Exact divisibility; no physical clock measurement |
| firmware_io.dma | fw_dma starts, requests cancellation, observes idle with the original ticket | Reset cannot erase old ownership |
| firmware_io.watchdog | fw_watchdog feeds only on successful quiescent completion | Progress fact comes from this adapter, not arbitrary activity |
| execution.tasks | cmx_init/task_add/schedule/invariant create and inspect three model task lifetimes | Scheduler-state execution, not actual stack/context switching |
| sync.copied_queue | cmx_queue_send/recv copy an internal branded-slot index | Serialized model; no ISR call without the selected saved-mask adapter |
| containers.ring | algo_ring_init/put/take hold bounded result history | Diagnostic overflow counts loss, does not reverse completed work |
| adt.brand | ad_pool_acquire/get/release retain each message until consumed | Owner object stays live/stable; raw indices are internal only |

`struct pf_system` is nonmoving. Its fixed task stacks are aligned owned arrays used for
model stack-admission metadata; this does not execute a machine stack switch.
Message-slot identity and recovery-capability identity are checked separately.
On queue admission failure, release the newly acquired slot. On processing a
pending/reset message, release only its message slot. The DMA allocation remains
owned by PFDriver until its own quiescence protocol finishes. Producer return
codes are converted explicitly; no generic 'nonzero means done' conversion is
permitted. Queue timeout on an empty immediate receive becomes PT_PENDING.

The hosted competing-consumer variant uses a real pthread mutex around the whole
queue-and-dispatch transition. The Cortex-M4 saved-mask backend is a different
execution contract and is compile-audited only. A multicore/priority-specific
port must rederive serialization and interrupt admissibility. The B06 H745 cache
port is separate selected-target evidence; a firmware_io model idle integer is
not a physical proof of DMA bus quiescence or cache coherency.

`cache_follow.c` additionally calls the actual B06 `fw_cache_range` policy before
`pf_driver_start`. The host test records clean/DSB/ISB before the FW_DMA start
write, rejects a borrowed interior pointer, and preserves the allocation through
pending quiescence. The bus mapping is a declared selected-target premise;
truncating a64-bit host pointer is not an address translation. The original B06
MMIO port and its source inspection retain their own compile/physical limits.

The target compilation uses declarations from the privately extracted Newlib
headers because the complete existing algorithms translation unit includes
`math.h` and `string.h`. It links no Newlib archive. Unused numeric operations
are removed by function-section garbage collection; no floating-point kernel
is claimed in this composition. The supplied `runtime_mem.c` implements the
retained ring's byte initialization and the disjoint-copy contract with explicit
byte loops. Together with the existing declared Arm memory helpers, these are
the entire permitted target runtime. Inspect the final undefined-symbol list
and disassembly at both optimization levels; a successful hosted link cannot
establish this closure.
# Active-object task/queue adapter

active_import.h additionally consumes execution.tasks and sync.copied_queue
through actual cmx_init/task_add/queue_send/queue_recv and the original HSM
through pt_dispatch. A physical receive moves one copied value into an owned
pending slot; dispatch failure retains that logical head. Capacity counts both
queue and pending slot. All accesses, including queue.count admission, occur
inside one supplied lock. This version-specific representation use must be
revisited if the imported queue representation changes. The two-consumer host
test uses a mutex; the separately compiled IRQ lock applies only to a saved-mask
uniprocessor port, not a general SMP synchronization claim.

