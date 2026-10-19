# CMK2: freestanding SMP and bounded OS

This is the new T53/T56 family. `labs/kernel` remains the ABI-v1 regression family. Default learner sources compile but deliberately do not implement the kernel, AP startup, channels, parallel workload or service. Reference answers live in `assessor/kernel_smp` and require explicit opt-in.

## Fixed target and bounds

Use Linux GCC, GNU binutils, CMake, Ninja, Python3, GRUB, xorriso and QEMU 10.2.1. The target is `x86-smp`: QEMU `pc-i440fx-10.2`, `qemu64`, MTTCG, 128 MiB and 1, 2 or 4 virtual CPUs. There is no physical timing or hardware compatibility claim. No network or host disk passthrough is used; the guest disk is a fresh 4 MiB ordinary file.

The original runtime has 8 process slots, 32 total thread slots with at most 4 live threads per process, 8 channels of 8 integer messages, 4 single-page shared objects, a 2 MiB virtual user window per process and one outstanding virtio request. Process slots are reused only after termination, quiescence, frame reclamation and parent wait (or boot-root exit). Generation-tagged process/thread/channel handles reject stale reuse; pending I/O pins process identity. Orphan children become automatically wait-consumed when their parent exits. No FP/SIMD context is supported. Build with `-mgeneral-regs-only`; the kernel and every user ELF link directly with `ld -nostdlib`. Original memory routines are supplied. No libc, libgcc, compiler-rt or libatomic is linked. `nm -u` and dynamic dependency audits cover every ELF; compiler-supplied C type headers are permitted.

## Build and checks

From a copied lab directory, `cmake -S . -B build -G Ninja` and `cmake --build build` build the unsolved learner. Host contract failures and absent guest markers are expected until implementation. For review only, set `-DCMASTERY_ASSESSOR_ROOT=/absolute/repo/assessor/kernel_smp`. Select CPUs with `-DCMASTERY_CPUS=1`,2 or4. `ctest --test-dir build --output-on-failure` runs available gates.

VM/store integration is explicit. Supply absolute `CMASTERY_RUNTIME_ROOT` (implementation source directory), `CMASTERY_RUNTIME_INCLUDE` (runtime/include), `CMASTERY_RUNTIME_ADAPTERS` (runtime/adapters), `CMASTERY_STORAGE_ROOT` (implementation source directory), and `CMASTERY_STORAGE_INCLUDE` (storage/include). No reference answer is silently substituted for a missing learner dependency.

`family.json` maps project IDs to exact CTest names. Early guest stages stop the emulator after their marker; they do not claim later OS mechanisms. T53-08 onward revisits the T56-01 process/thread substrate. Reduction, reusable pool and DAG have separate execution modes and markers. T53-09 checks an acknowledged IPI/CR3 barrier; later T56 zero/COW tests exercise page-permission transitions. Complete all 1/2/4 CPU runs before claiming SMP completion.

## Architecture and ownership contracts

MADT discovery validates record bounds/checksums and rejects the unsupported x2APIC/address-override profile. INIT/SIPI starts each AP through original 16/32/64-bit assembly, per-CPU stacks, GDT, TSS and IDT. The first workload uses 2 cooperative kernel threads per CPU, native spinlocks and a bounded cross-core channel. Local APIC timers and IPIs must actually fire.

The second phase runs preemptive ring3 threads on every CPU. Process objects own address spaces/capabilities; TCBs own saved execution frames and wait state. A coarse kernel lock protects scheduler/object/page-table transitions. A thread can be READY, RUNNING, blocked or dead, and only one CPU owns a RUNNING thread. Per-CPU kernel stacks save traps into TCB-owned frames before migration. Hardware IRQs are acknowledged before waiting for the shared lock, so a reschedule IPI cannot block a lower-priority device IRQ required by the lock holder. While contending for the lock, CPUs admit the independent TLB IPI handler; its separate IST avoids lock-dependent shootdown deadlock. Explicit affinity excludes disallowed CPUs. This teaching scheduler is bounded round robin, not a production fair scheduler.

ABI-v1 syscall numbers remain unchanged. ABI-v2 starts at 0x200 (`include/cmk2_abi.h`). Thread entry functions must call THREAD_EXIT; join consumes a generation-tagged handle. The last thread exit closes its process. Process EXIT requests remote reschedule; WAIT becomes observable only after all CPUs leave the dead process, then frame references are reclaimed. SPAWN reads a named ELF from the read-only boot archive; its optional flag 1 explicitly grants a disk capability only from a capable parent. Default SPAWN and COW children have no disk capability.

Normal channel close rejects sends, wakes blocked senders, and preserves FIFO drain before ECLOSED. DESTROY requires a closed drained queue and makes handles stale. Owner death closes publication; queues with no live grantees are reclaimed. A surviving grantee may drain/destroy an orphan. Payloads are copied bounded nonnegative 32-bit integers. There is no byte stream, arbitrary pointer message or POSIX descriptor claim.

Demand-zero reservations and restricted single-live-thread COW use original page faults and reference counts. Shared mappings retain references after object close. Concurrent faults recheck the current mapping after acquiring the kernel lock: a peer may already have resolved the recorded fault. COW forbids multithreaded fork. Kernel copyin/copyout also materializes the required mapping. All relevant CPUs acknowledge invalidation before page ownership can be reused.

Modern virtio PCI uses one split queue and MSI-X vector 43. Submit copies metadata/write payload into static DMA storage; wait blocks the calling thread. Completion requires a real device interrupt sequence plus used-ring progress. BLOCK_STATUS observes completion without consuming it. If an owner exits after unconsumed completion, the request slot is released. Pending DMA storage stays owned until completion; a deadline poisons the device and never recycles that storage. Timers may check the deadline, but used-ring polling alone cannot satisfy completion. Sync ABI-v1 operations serialize with this request and must wait before touching the shared bounce buffer.

The VM client is a separate process with no disk capability. Integer request/reply channels reach one store owner. The original VM and store implementations are linked explicitly; the service partition starts at disk page 128. A committed write can receive an indeterminate response, and a fresh boot recovers 42. The old ABI-v1 integration partition remains separate.

## Evidence and exercises

`guest/build.json` records source and ELF hashes, flags and tool versions; logs and execution.json record actual commands/results. Host model passes alone do not prove AP startup, preemption, blocking or device interrupts. The parallel helper checks an integer reduction and 16-node DAG against serial oracles; emulator TSC intervals are observations, not physical cycle complexity or scalability claims. Finite concurrent fault tests do not prove all interleavings.

The workpool helper is private to the test process; partial allocation/worker-creation failure requires process exit. It is not a transactional reusable library under allocation failure. Extend that contract as a separate drill. Likewise test unimplemented policies (priorities, dynamic capacity growth, timeout/cancellation, hotplug, VFS, SMP multithreaded fork, general PCI/IOMMU/DMA) as extensions rather than silently claiming them.

See `HINTS.md` for progressive prompts and the repository source-inspection records in `research/v3/kernel_smp_sources.md` for bounded primary readings.
