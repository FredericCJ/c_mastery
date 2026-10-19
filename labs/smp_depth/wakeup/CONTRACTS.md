# Durable work, coalesced notifications and sleeping CPUs

## Identity, objectives and families

Required successor for PR-T53-07 and AX-PR-T53-07-T53.07.a/b/c, including their unchanged `-DELAYED` variants. Families: semantic reasoning, construction, boundary diagnosis, falsification, failure recovery, measurement and transfer. Objective .a distinguishes a doorbell from a work counter and derives clear/recheck order; .b binds identity, queue ownership, publication and actual idle entry; .c separates measured intervals from lost-wakeup evidence and compares batching with identical delivery semantics.

## Prerequisite knowledge and artifacts

Complete PR-T53-05 locks, PR-T53-06 queues and PR-T53-02 startup. Bring their completed `locks.c`, `queues.c` and startup files explicitly; preserve their contracts and generation/world lifetimes. Understand C release/acquire, the queue's saved-context completion boundary and the distinction between IF masking on one CPU and mutual exclusion across CPUs. The new wrapper calls the actual completed queue mechanism. Replacing it with a synthetic ready counter does not satisfy composition.

## Environment and source distinctions

The core is fixed-storage ISO C17, without allocation or target library calls. C11 introduced the atomics used; N1570 is the inspected draft, while exact published C17 remains unavailable. No unsupported C23 equivalence is inferred. The target adapter is original x86-64 GNU C/assembly in a disposable CMK QEMU TCG guest, with private stacks and fixed physical-destination xAPIC interrupts. The specific C/assembler/compiler/link dependencies are recorded by `guest.py`. Host pthread condition variables, Python models and sanitizers are verifier dependencies only. Physical interrupt timing, other ISAs, user address spaces, FP/SIMD, TLS and preemptive task switching are outside this selected adapter.

Inspected Intel2016 Volume2B STI description/footnote requires IF=0 for the guaranteed deferral through the next instruction. Volume2A HLT describes interrupt resume and the saved next instruction address. Volume3A's idle example starts with interrupts disabled and uses adjacent STI/HLT. Its APIC IRR/ISR/EOI passages describe a different state machine from the C queue. These sources are bounded passages, not a complete current CPU/errata audit.

## Problem, supplied inputs and outputs

Implement `wakeup.c`, `wakeup_guest.c` and its assembly adapter against `wakeup.h`, using the explicit completed queue/lock inputs. The world contains up to four CPUs and the queue's fixed task slots/capacity. A stable `SwPort` supplies `ring(context,cpu)` and `ack(context,cpu)`. The core serializes queue operations plus doorbell state under one additional control lock; queue storage remains per-CPU. This intentionally simple publication boundary is not a scalable kernel scheduler claim.

Initial host fixture: four generation1 tasks at CPU1 are dispatched and saved BLOCKED. Publish all four while CPU1 has acknowledged an empty queue. Require four READY entries but only one initial rising-doorbell notification. Repeated wake of READY or RUNNING returns DUPLICATE without another insertion. A stale generation/home is rejected. During TRANSFER, wake returns MOVING, an explicit unaccepted request; after commit the caller resolves the current home and a retry observes the one READY insertion. Notification failure leaves admitted work queued and is separately retried.

## Necessary mechanism and misconceptions

One notification need not mean one item: the destination drains durable queue state. An IPI does not validate lifetime, publish ordinary C data or make a check-then-sleep sequence atomic. Conversely, an early notification is not automatically a demonstrated lost wake if another lock/recheck still orders the publication. The negative model and actual idle control must state the exact failing schedule. A low average latency cannot exclude a rare stranded item.

## Representation, ownership and reasoning

`SwWorld.queues` retains all PR-T53-06 invariants. After initialization and creation, **all** queue operations use the wrapper; bypassing it invalidates the protocol. One caller owns each actor CPU; ISR code never calls a wrapper or acquires its control lock. The port and its context remain live and unchanged. Callbacks are nonreentrant and cannot call queue operations. The selected target callback is a bounded APIC operation; the hosted condition-variable adapter has a separate blocking-host contract and never calls a wrapper while holding its predicate mutex.

The atomic `work[cpu]` mirrors READY count after each admitted queue transition, with release publication. It permits the controlled ISR/port to observe publication without touching ordinary protected queue fields. It is not an independently writable queue. `bell[cpu]` is a coalescing Boolean under the control lock, not a monotonically accumulating item counter. Queue publication precedes an SC fence and the selected port's ordering before ringing. ISO C alone does not specify MMIO ordering; the actual adapter uses the declared x86 fence/APIC operation separately.

The whole `sw_init` lifetime is externally distinct: invalidate every old handle/ticket/context pointer before reinitializing. Quiescent slot creation preserves queue generation history. A moving task reserves source rollback and destination commit capacity through the queue contract. Refusal during movement does not cancel that ownership. Notification after transfer commit may fail, but commit is already final; replaying the ticket is stale, and the required recovery is `sw_retry`, not rollback or duplicate completion.

## Clear, recheck and idle-entry contract

The destination must actually mask local interrupts before `sw_prepare_idle(...,1,...)`; the argument is an assertion by a trusted port, not an implementation of masking. Under the control lock, acknowledge/deassert the level, clear the software bell, then inspect durable work and current execution ownership. If READY work remains, reassert the level and return BUSY; if notification fails, return SIGNAL with that work retained. A current running task also prevents IDLE. Only the empty/no-current result authorizes the selected idle sequence. A producer after the lock release reasserts and rings; a pending fixed interrupt remains pending while IF=0.

The x86 adapter executes **adjacent** `sti; hlt; cli` with IF initially0. The interrupt deferral covers HLT; an interrupt already pending or arriving during halt resumes it. After wake, drain/recheck queue state. A fixed IPI's EOI acknowledges interrupt servicing; it does not dequeue work or acknowledge the software level. The hosted level adapter uses a mutex-protected Boolean predicate plus condition wait. It rechecks that predicate even if a signal preceded the wait. Do not present the fixed-IPI guest as an actual physical level-triggered I/O APIC device: the delayed level variant is implemented by the original level port and its executable finite schedule model.

## Failure semantics and finite resources

INVALID/STALE/MOVING and other queue admission refusals preserve caller outputs and do not insert work. DUPLICATE reports existing READY/RUNNING ownership, not a new entry. SIGNAL is deliberately different: admitted queue work or completed transfer remains published, but the ring failed. Keep its task/queue lifetime and retry notification. The port may have partially delivered a failed notification, so retry can be spurious; draining the durable queue preserves exactly one insertion. There is no unbounded retry deadline or hard real-time completion promise. FULL remains the underlying fixed-capacity refusal. Generation/revision reserve and mandatory completion at limits are imported unchanged from the queue contract.

Control serialization plus at most two queue locks makes the core O(K) per queue operation for capacityK, with O(C*K+T) fixed storage. Ring/ack cost belongs to the chosen port. The original TAS lock has no fairness bound. Batching lowers possible ring frequency while delaying publication groups or service opportunities; report the actual workload and retained delivery contract. Do not substitute a throughput result for lifetime or idle correctness.

## Required implementation and experimental artifacts

Submit source, transition/failure tables, publication and lock-order arguments, a port-specific idle proof, exact link/disassembly audit, and an independent schedule/observation checker. Host tests cover four-item coalescing, acknowledgement with remaining work, duplicate RUNNING/READY wake, movement refusal, stale destination/generation, failed ring after wake and after committed transfer, and retry without duplicate insertion. `native.c` composes the real queue wrapper with a sleeping level-predicate destination for500 rounds and2,000 deliveries; the expected synthetic checksum is5,000. Run both GCC/Clang TSan profiles and ordinary/debug/release/ASan+UBSan conditions.

The target creates four actual saved blocked task contexts on CPU1. Twenty-four rounds exercise publication before check, between masked check and sleep, after entry toward sleep, and while task0 is executing. Batches are one or four under the same four-entry capacity; total deliveries are69. Two-CPU runs have no extra workers; four-CPU runs add explicit background workers on CPUs2/3. A busy task is not preempted by the wake wrapper: it cooperatively returns and the scheduler drains all newly READY tasks. Preserve that progress assumption.

## Verification, negative controls and expected observations

Require successful translation and real execution, including guest exit33 and a record for every round. Modes1/2 must have an interrupt frame whose saved RIP is the label after HLT; this validates the executed halt/resume path, not a wall-clock duration spent asleep. Require all69 dispatches, no residual queue work and an empty acknowledged doorbell. The independent transition model enumerates its declared finite platform schedules; it is not C model checking or an all-interleaving proof.

The no-recheck host mutant returns IDLE with queued work and is rejected. The IPI-before-publication host/guest control observes notification before the READY mirror; it violates the stated order, but the retained control lock/recheck can still prevent stranded work. The distinct actual missed-wakeup control separates STI from HLT: it consumes the first notification, halts with READY work, and resumes only after a deliberately later rescue IPI. Accept its specific diagnostic only when the rescue interrupt's saved RIP proves it interrupted that HLT and durable work is still present. A crash, generic timeout or failed build is not either demonstrated mechanism.

Record publication, notification, dispatch and observed-completion markers. To avoid an unsupported cross-CPU clock assumption, compute sender-local publication→observed-full-batch intervals and receiver-local IRQ→first-dispatch intervals separately. The former includes notification, dispatch, task return and sender polling delay; it is an observation envelope, not an exact isolated dispatch latency. Virtual TSC ticks are not calibrated physical nanoseconds, WCET or a physical interrupt guarantee. Report batching/interrupt counts alongside equivalent delivery results.

## Rubric, hints and separate assessor material

For .a, level0 counts IPIs as work; level1 drains only an ordinary single item; level2 derives durable-state/doorbell and acknowledge/recheck invariants with coalescing; level3 transfers to the level port and constructs the late-clear counterexample. For .b, level0 wakes stale or moving identities; level1 relies on polling; level2 enforces generation/home/movement and actual IF0 idle entry with a saved-RIP witness; level3 diagnoses the controlled lost wake and explains SIGNAL after already committed work. For .c, level0 equates latency/crash with correctness; level1 reports a mean without conditions; level2 checks all schedules and markers with clock scope; level3 compares batching under unchanged storage/delivery semantics and distinguishes order violation from actual stranding. Tests alone cannot establish absolute mastery.

Hint1: draw work storage, doorbell state, interrupt pending state and CPU IF separately. Hint2: decide which state survives a failed ring and what retry may duplicate. Hint3: put the clear and durable-state recheck under the publication exclusion, then prove the selected ISA's enable/sleep boundary. The reference implementation and detailed solution are separate assessor material.

## Changed conditions, security scope and validation limits

Retain all original seven-day delayed variants: level-triggered doorbell with acknowledge/recheck, actually sleeping destination with a missed-wakeup control, and batched work under fixed storage with latency/interrupt-rate tradeoffs. The author implementations/tests make these assessable; they do not fabricate elapsed learner time, unaided work or independent review.

All controls affect only original synthetic objects, the private host process or disposable QEMU guest with no network. The adversarial capability is stale/duplicate teaching handles or deliberately altered original publication/idle code. Success is a bounded assertion or controlled stranded-work/rescue observation. Repair lifetime validation, durable publication, acknowledge/recheck and ISA idle entry, then rerun the corresponding regressions. No arbitrary code execution, third-party kernel attack, physical board operation, production service, external reviewer approval or physical timing result is claimed. Exact current author execution is in the source-bound successor report, not implied by this document.

## Inspected reference mapping

See `research/v5/expansion/legacy_smp_wakeup/resources.json`: exact Intel2016 STI/HLT/idle/APIC passages, selected N1570 synchronization clauses and the actual frozen queue/lock/startup contracts. Original level/queue composition and finite schedule derivations are course analysis, not attributed to unseen sources. Current physical errata, other ISA ports and physical observations remain separate requirements for any expanded platform claim.
