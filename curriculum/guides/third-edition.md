# C Mastery 3: operating systems and concurrent execution

This edition extends the accepted second edition through T49–T56. Follow the
[project route](project-route.md) and retain the earlier implementations. The
second-edition Git commit is `40bf1b2`; the immutable baseline and explicit third-
edition migration preserve its 1,605 objective and 337 project identities.

## What grows across the projects

Begin by observing a process, thread, mapping and descriptor on Linux. Write a
small scheduler model with a hand-derived trace. Next construct task contexts,
stacks, timers and wait queues, then the synchronization objects built upon them.
First make a blocked task wake correctly; only then add priorities, deadlines and
another CPU. A correct queue on one CPU is reusable evidence only after checking
the new ordering, ownership and reclamation assumptions.

The MCU and x86 branches can proceed independently after the common task and
synchronization mechanisms. The H745 uses an original runtime per core and
explicit messages between them. The x86 branch develops coherent SMP, process
address spaces and shared-kernel services. T56 brings completed components into
applications; VM/store completion does not gate an early context-switch exercise.

| Stage | Build on | New implementation |
|---|---|---|
| T49 | Earlier hosted C and architecture work | OS observations and finite scheduling models |
| T50 | Task/context observations | Original stacks, context switching and preemption |
| T51 | Original task runtime | Wait/wake, synchronization and IPC |
| T52 | Task and synchronization mechanisms | Real-time policies and analysis |
| T53 | Shared task and synchronization foundation | x86 SMP |
| T54 | Shared foundation, on a separate branch | Physical M7/M4 AMP |
| T55 | Atomics and shared-memory execution | Lock-free structures and reclamation |
| T56 | Completed mechanisms required by each project | Concurrent services and applications |

This table shows the conceptual route. Exact prerequisite project IDs in the
project route govern entry and allow the MCU and x86 branches to proceed separately.

## Required targets and boundaries

| Target | Actual responsibility | Completion evidence |
|---|---|---|
| Linux x86-64 | Observe hosted processes, scheduling, IPC, virtual memory and resource control | Commands, source, actual observations and controlled changes |
| STM32F439 | Original single-core preemptive tasks and peripheral/interrupt handoff | Strict image audit plus independent board timing and fault observations |
| x86-64 QEMU, 1/2/4 CPUs | AP startup, per-CPU state, IPIs, shared-kernel scheduling and OS services | Real guest traces and fault campaigns at each supported CPU count |
| NUCLEO-H745ZI-Q | Original M7/M4 runtimes, static ownership, shared SRAM, HSEM notification and cache/DMA visibility | Both image hashes, standalone boot and two-core physical trace |
| Pinned FreeRTOS | A separate comparison workload | Configuration, API/port inspection and actual run; does not replace the original runtime |

The H745 branch does not promise general cross-core task migration or cross-core
priority inheritance. The strict profile does not assume undocumented global
exclusive-monitor support; CAS-based MPMC/reclamation work executes on x86 SMP.
No observed overlap on a host or emulator replaces physical MCU observations.

## Work with target-specific evidence

```sh
python3 -m cmastery init PR-T49-03 ../my-os-observatory
python3 -m cmastery check ../my-os-observatory --target host
python3 -m cmastery report ../my-os-observatory
# Select a target and supported CPU count declared by a family:
python3 -m cmastery check ../my-smp-lab --target x86-smp --cpus 4
```

Use a family's manifest and executable-evidence directory to find exact tests.
Default tests are stage anchors. An assessor suite does not accept a learner's
explanation, independent implementation or physical claims. `--profile tsan`
selects a separate hosted diagnostic build; its runtime support must actually be
available. It neither instruments firmware nor proves all possible schedules.

`record-physical WORKSPACE MANIFEST` registers image/capture files with source
hashes, board/silicon revisions, clocks, cache/MPU state, instrument settings,
overhead and trace-overrun records. It preserves submitted evidence with status
`recorded`, requiring assessment of the raw observations. It never manufactures
an automatic physical pass from filenames or a successful firmware build.

## Deep diagnosis and performance

For every synchronization mechanism, deliberately construct a counterexample:
signal before wait, timeout racing grant, owner exit, event coalescing, queue full,
stale identity, premature reclaim, false sharing, interrupt arrival or peer stall.
Find the first incorrect state transition, repair its invariant and replay a
changed schedule. Reuse the T43–T45 reduction, disassembly and measurement methods.

Record ready-selection work versus task count, timer/wait-queue work, critical-
section duration, context-switch and wakeup latency, CPU time, wall time, cycles,
contention, throughput, stack/storage peaks and logical/device I/O separately.
For parallel speedup, retain the serial baseline, workload size, CPU placement and
instrumentation cost. An observed maximum is not a universal worst-case bound.

Complete the [concurrent-system contracts](concurrent-system-contracts.md), exact
project dossier and delayed transfer before accepting each capability. Hardware
waiting time and uncalibrated learner estimates remain visible in the workload.
