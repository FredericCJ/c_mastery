# Explicit CMK2 assessor

Original bounded reference for labs/kernel_smp. This directory is never selected by the default learner build. It extends the copied v1 teaching kernel and preserves its numbered ABI; original labs/kernel and assessor/kernel are untouched.

Build with the lab CMake and an absolute CMASTERY_ASSESSOR_ROOT pointing here. README/HINTS in the lab define the fixed 1/2/4 CPU target and ownership contracts. The reference covers actual INIT/SIPI, APIC/IPI, cooperative kernel threads, SMP ring3 preemption/migration, process/thread split, blocking IPC, demand-zero/COW/shared objects, real MSI-X block completion and explicit VM/store service integration.

The kernel lock deliberately serializes OS transitions. It is not a claim of scalable fine-grained kernel locking. No FP/SIMD, hotplug, general PCI, POSIX, multithreaded fork or physical WCET support is claimed. The private parallel helper assumes process exit on partial construction failure.

Use tools/guest_mutations.py only in a disposable build directory; it copies sources and verifies absent TLB acknowledgements / masked device interrupts are detected. Source receipts must refer to the final source hashes and actual 1/2/4 CPU logs.
