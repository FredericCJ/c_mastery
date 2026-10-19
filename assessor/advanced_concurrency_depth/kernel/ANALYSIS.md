# T26.09/.10 assessor analysis

This is assessor-only author evidence. Assess the original six IDs independently; do not substitute one guest success for every criterion.

## Context and partial copy

The copy residual is an unsigned count, not errno and not bytes successfully copied. All560 n/residual pairs are tested. The implementation stages a32-byte packet and commits only at residual0. A full residual leaves the output unchanged; even the normal zero-padding copy_from_user wrapper must not make its zeroed tail trustworthy. The actual original T57 probe crosses into PROT_NONE, receives EFAULT and verifies unchanged writes/payload. The kernel owner copies into a36-byte stack staging extent, validates exact header length/flags, commits owned bytes under its lock and schedules work; no borrowed user pointer reaches work. Its allocation and mutex calls occur in the ordinary worker. The separate IRQ source executes an actual hard-interrupt primary handler and a threaded handler. These do not establish behavior on PREEMPT_RT, where ordinary spinlock_t semantics change; raw_spinlock_t stays strict spinning.

## Release and model boundaries

The four-event C example with atomic data and flag has data store sequenced before flag release, the successful acquire reads that release, and data read is sequenced after acquire. Therefore the data store happens before data read and atomic write/read coherence excludes the initial data value. Weak flag operations omit synchronizes-with; with plain non-atomic data, conflicting accesses without hb are a data race, not an allowed output experiment.

For the pinned LKMM marked MP example, the bad candidate reads flag1 and data0. Release contributes wx→wf; acquire contributes rf→rx; reads-from contributes wf→rf; from-read contributes rx→wx. The inspected `prop` expression combines external overwrite, cumulative release and reads-from, yielding rx→rf within the consumer. Together with rf→rx this gives the forbidden hb cycle. The finite checker implements only this no-lock/no-RMW/no-dependency fragment and matches the two inspected litmus expectations. Its relations named hb are not ISO C's definition. It neither executes herd7 nor certifies arbitrary Linux code.

The descriptor/doorbell graph adds a third contract. Coherent mapping still does not eliminate store-order barriers. dma_wmb orders shared coherent-memory writes before device ownership but supplies no MMIO ordering by itself; writel/readl ordering applies under the documented mapping/device assumptions. Cache synchronization, streaming-map ownership, addressability and posted-write completion cannot be inferred from an ISO atomic or a Python ledger.

## Work and unload

Original fixture unload answers are false then true; event2's claimed synchronous-cancel postcondition is rejected because producers can still enqueue. smp_mb leaves every lifetime edge untouched. Adding one timer makes the final ordinary-work-only verdict insufficient. The new module performs16 actual work callbacks: the first15 request a distinct producer to enqueue another instance. Both scheduling and closure use one gate; close gate before waking/joining producer, then call cancel_work_sync from sleepable module-init context without that mutex. The timer is a separate reference and is synchronously canceled. Storage is static until module exit, and no deliberately unsafe use-after-free control executes. The false-drain control deliberately rejects its claim after safe cleanup.

The producer remains alive until kthread_stop requests exit, including the gate-closed path; otherwise a stopped thread's task pointer lifetime would itself require another reference argument. Allocation failure before completing16 callbacks yields bounded timeout and cleanup, not fabricated success. Kernel configuration determines diagnostic coverage. Both selected module verifications must match their kernel's release and symbol ABI; the mismatched packaged module is rejected in the debug guest without force-loading.

## Queue/backend composition

The actual ASQueue supplies its own single-producer/single-consumer and release/acquire contract. The serialized service adds a held value, so total capacity is four queued plus one held. Map failure leaves the held value for retry. Submit failure promises that the device never acquired it, allowing one unmap; success forbids unmap or CPU completion until quiescence. The synthetic backend's stage checks independently detect omitted device synchronization and skipped quiescence. A negative result from the backend is not proof of physical stop; this interface defines positive acknowledgement as the prerequisite. Real hardware must establish that condition from its own spec and observations.

Queue FIFO, mapping ownership, worker references and physical visibility are four distinct claims. None establishes the others. Require changed-condition solutions to retain each original claim at its proper boundary, expose unsupported board assumptions and report equal-correctness resource comparisons. External reviewer, independent delayed learner and physical target dimensions remain open.
