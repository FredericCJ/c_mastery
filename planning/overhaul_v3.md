# C Mastery 3 implementation record

## Authorized outcome

Implement the approved second overhaul: T49 operating-system mechanisms and Linux
observation; T50 original freestanding task runtime; T51 synchronization and IPC;
T52 advanced real-time scheduling; T53 coherent x86 SMP; T54 physical STM32H745
asymmetric multiprocessing; T55 lock-free structures and reclamation; T56
concurrent OS services and integrations. Baseline is commit `40bf1b2`; preserve
all 48 prior domains, 337 project identities and 1,605 objective identities.

User selected broad OS depth, strict independence from libc/libgcc/compiler-rt/
libatomic, required NUCLEO-H745ZI-Q hardware, and advanced RT mechanisms including
ceilings, budget servers and partitioned scheduling. Existing F439 remains the
single-core MCU target. x86 QEMU must execute with 1, 2 and 4 CPUs. H745 uses
separately linked M7/M4 images, static task/peripheral ownership and original
shared-SRAM queues; it does not assume undocumented cross-core CAS guarantees.

## Required delivery

Preserve unsolved learner scaffolds, staged hints and separate assessor solutions.
Add bounded 4–12-hour projects with independent stage gates, failure drills,
references to inspected passages and delayed transfer. Reuse earlier components
without charging their original implementation twice. Supply original C and small
assembly boundaries; permit compiler type headers and inspected inline intrinsics.
Existing GRUB handoff is declared pre-runtime infrastructure. FreeRTOS/Linux are
separate comparisons and cannot satisfy an original-runtime implementation gate.

Implement shared task/wait/scheduler contracts with architecture ports; semaphore,
mutex, condition, bounded messages, events, counted notification, barrier, once,
RW lock; fixed priority/EDF/CBS; SMP APIC/IPI/per-CPU mechanisms; H745 boot epochs,
cache/DMA ownership; histories/ABA/hazard/epoch experiments; bounded process,
thread, demand-zero/COW and service extensions. Keep the existing T48 ABI route
as a compatibility gate. A dedicated service serializes access to the T47 store.

## Work ownership

- Root: edition/migration, publication, runner, hosted OS laboratory, integration,
  validation and release review.
- Execution: new execution lab/assessor, original synchronization/schedulers,
  Cortex-M ports and strict runtime audit.
- SMP: separate kernel_smp lab/assessor, real SMP guest and concurrent OS services.
- Curriculum/sources: T49–T56 teaching/project contracts, new taxonomy, inspected
  source additions and search/reflection/search record.

## Evidence discipline

Models, hosted executions, cross-compilation, emulator runs and physical captures
remain separate. Firmware images and compiled stubs do not establish measured
latency or board execution. Record unavailable/not-run explicitly. No generated
source receipt may relabel an inherited v2 success as current validation.

Current status: implementation complete. T49–T56 add 92 projects and 276
objectives; the full curriculum has 56 domains, 429 projects, 1,881 objectives
and 204 reference records. The added route budgets 1,023 provisional hours.
The v2 project bytes, objective identities/text and 182 resource records remain
preserved and checked against the accepted baseline.

Three executable families provide unsolved scaffolds, separate assessor
implementations and bounded stage gates: `os_observatory`, `execution` and
`kernel_smp`. Original images use the strict runtime profile. The SMP kernel
enforces 8 processes, 32 total threads and 4 live threads per process; actual
device interrupts, remote quiescence and acknowledged TLB invalidation are
tested. The H745 baseline builds two images with cold-reset, noncacheable shared
SRAM and HSEM; cache/DMA, clock calibration and independent peer recovery remain
required learner extensions with physical evidence.

Independent reviews cover curriculum consistency, execution, SMP lifetime and
hosted runner behavior. Release acceptance requires current publication, all
nine laboratory families, target matrices, runner checks and the pinned
Linux/FreeRTOS comparison under `reviews/v3`. Packaging revalidates those records,
rebuilds extracted sources, compares HTML/Markdown, and promotes staged archives
only after success. Physical observations and learner mastery remain not-run
and not-assessed respectively; denied privileged Linux experiments remain open.
