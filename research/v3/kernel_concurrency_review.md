# Independent bounded kernel review

Reviewed 2026-10-10 by the curriculum/source author, independently of the implementation author. This document records static findings and their final code dispositions. Actual commands, exit codes, serial transcripts, source hashes and acceptance are recorded separately in `reviews/v3/kernel_independent_runs.json` and `reviews/v3/review_kernel.json`; this narrative is not an execution result.

## Scope

Read `assessor/kernel_smp/kernel.c`, `os.inc`, `async.inc`, `smp.c`, `interrupts.S`, `ap.S`, `boot.S`, `cmk2_model.c` and relevant `model.c` paths. Cross-checked ABI headers, the host contracts, user and spawn regressions, parallel workloads, service/storage adapters, guest runner, CMake configuration, manifest and README. Focus: saved frames and ownership across CPUs, process/thread reclamation, concurrent page faults, channel shutdown, outstanding I/O and truthful execution boundaries. Review source pins identify the accepted snapshot; earlier line numbers were deliberately removed because the implementation changed during review.

## Findings and final code dispositions

1. **Closed channels could not drain — resolved.** Close formerly removed the endpoint from lookup before receive could drain it. A closing identity now rejects sends, wakes blocked senders, preserves FIFO receive and requires a separate drained destroy. The guest fills a queue, blocks a sender, closes it, checks the sender's closed result, drains the values and then destroys it.

2. **A peer-resolved page fault was fatal — resolved.** Two threads could fault before either acquired the shared kernel lock. The second then saw neither ZERO nor COW and was killed. The fault path now revalidates the recorded user/read/write/execute access against the current mapping and invalidates/retries only if that mapping satisfies it. Pinned concurrent first-touch and post-fork write fixtures exercise the surrounding paths. They do not force every possible stale-fault interleaving.

3. **Unconsumed completed I/O survived owner exit — resolved.** Completed state previously occupied the single request slot forever when the owner exited before BLOCK_WAIT. Process exit now discards completed metadata, while pending requests retain their DMA storage and pin process identity until completion. A child with explicitly delegated disk capability observes completion through non-consuming BLOCK_STATUS and exits without BLOCK_WAIT; the parent waits and successfully submits another flush.

4. **The last thread did not terminate its process — resolved.** Final THREAD_EXIT now invokes process cleanup. A spawned sole thread exits with status 7 and its parent receives that status through WAIT. Cleanup remains subject to CPU quiescence and outstanding ownership.

5. **Owner-death channels exhausted endpoint slots — resolved.** Owner death now closes publication, clears the exiting process's grants and reclaims queues with no live grantee. A surviving grantee may drain and destroy an orphan. The guest creates all channel slots in a child, exits it and creates them again in the parent.

6. **WAIT could precede remote sibling termination — resolved.** Exit now requests remote rescheduling. The reaper publishes quiescence and wakes waiters only after no CPU's current thread belongs to that process. The guest child runs a sibling that writes shared memory; after WAIT the parent verifies that the value stays unchanged across three ticks. The scheduler also recognizes remote dead threads still retiring, avoiding a false deadlock during this transition.

7. **SPAWN could panic on TCB exhaustion — resolved.** Loading now preflights a reusable TCB before constructing a process. The bounded profile has eight process slots, 32 TCBs and at most four live threads per process. Failure returns a recoverable capacity result before mapping. This review verified the guard by code inspection; the integrated guest is not a complete allocation-failure campaign.

8. **Idle CPUs retained a reusable process's CR3 — resolved.** Process-slot reuse could clear page tables while an idle CPU still used them for kernel mappings. Scheduling now switches the CPU to the permanent boot page table before dropping current-process ownership and allowing reaping/reuse. Repeated spawn/wait tests check generation changes and stale process-handle rejection.

9. **Parent death stranded child slots — resolved.** A child's dead parent generation could never consume WAIT, making its reaped slot permanently ineligible for reuse. Parent exit now marks matching children wait-consumed, retaining their original parent identity so a nonzero orphan status does not become a boot-root failure. This edge was verified statically; no separate forced orphan-process fixture is claimed.

10. **APIC priority and kernel-lock inversion — resolved.** During review the implementation owner reproduced an intermittent two-CPU storage failure. A ring3 reschedule interrupt waited for the shared lock before EOI, keeping a high-priority APIC vector in service while the lock holder awaited a lower-priority device interrupt. Hardware IRQ accounting and EOI now precede shared-lock acquisition. The independent review checked the synchronous/async shared bounce-buffer serialization, used-ring ownership and separate storage partitions; it found no additional direct buffer overlap in those inspected paths. Actual repeated execution results belong to the run receipts.

11. **Runner evidence and project dependencies overstated or misidentified execution — resolved.** Runtime/integration CTests now propagate the configured CPU count rather than silently defaulting to four. Capability keys match the doctor, target metadata identifies actual SMP user execution, and the MADT project includes a guest gate. README setup names the new assessor family and bounded implementation. PR-T53-08 explicitly depends on the PR-T56-01 process/thread substrate used by user-mode affinity. The workpool uses an atomic cursor and join, so no unsupported process-channel dependency was added.

All findings were discussed with the implementation owner and parent agent. The final acceptance receipt must pin the inspected files and report the actual independent execution result; resolved static findings alone do not close runtime gates.

## Inspected safeguards and limits

The reaper checks every CPU's current process before freeing mappings. Saved user frames are copied into TCB storage under the kernel lock; selection marks a destination RUNNING before return to user mode. The lock wait path admits interrupts, while a dedicated TLB IPI uses a separate IST and avoids that lock. Idle CPUs move to permanent kernel mappings before surrendering process ownership. Pending I/O prevents process-slot reuse, and timeout poisoning retains static DMA storage rather than recycling it.

The earlier cooperative kernel-thread stage publishes READY before saving context, but selects only threads permanently assigned to that CPU. Other cores cannot resume that context in this stage. Adding stealing or migration there requires revisiting this publication boundary; the later user scheduler saves frames under the shared lock first.

The parallel helpers have finite serial oracles and release/acquire publication. Their private workpool requires process exit after partial worker/allocation failure; it is not a transactional general-purpose library. Strict image checks cover original freestanding images and all user ELFs, with explicit runtime/store dependencies. Emulator timing does not establish physical cycle counts, scalability or real-time bounds.

This is a bounded static review plus the separately recorded finite host and 1/2/4-CPU guest campaign. It is not an exhaustive architecture proof, C memory-model proof, hostile-device audit, failure-injection proof or timing analysis. Physical board execution is outside this x86 emulator review. Passing supplied tests does not prove every interleaving or every unsupported extension.
