# T53.04 assessor analysis (withheld)

The a trace must produce masks 0,1,1,3. Generation comparison happens before
setting a bit, and destination identity is an independent index. Accepting the
duplicate CPU0 event as a second participant is wrong even if the total count
reaches two. `struct si_round` observes trusted matching mailbox completion; it does not
validate arbitrary external acknowledgements. Its state is serialized.

The box uses two publication edges: producer payload writes precede release of
`published`, acquired by the consumer; callback and last payload reads precede
release of `acknowledged`, acquired by the producer before overwrite. The
consumer has no access after acknowledgement. This last-access rule is necessary:
putting an acknowledgement at the top permits the producer to overwrite a live
batch. A timeout is only a producer observation and retains the same lifetime
obligation. UINT64_MAX can finish; it can never wrap into a fresh authority.

One batch carries four ordered requests, so a single notification can cover all
four. Notifications do not encode the work count. A duplicate handler observes
the already acknowledged generation and performs no callback. No lock-free MPMC
queue, consumer migration or independent callback cancellation is implemented.

The guest genuinely changes IA32_APIC_BASE for x2APIC. It then uses MSRs for ID,
LVT timer/divider/count, ISR/IRR, EOI and ICR. The 64-bit ICR includes the full
destination field. The MMIO ICR status loop is used only for xAPIC. An MFENCE
precedes notification because x2APIC WRMSR does not itself publish earlier normal
memory stores globally. C release/acquire and the hardware fence solve different
parts of the composition. Unchanged startup runs before the mode change.

One-shot remains at one delivery; periodic automatically reloads and is stopped
by initial-count0. A final already queued event may still arrive. ARAT describes
clock continuity under specified power-state changes, not a frequency or response
deadline. The successful observed matrix concerns QEMU's selected model; neither
TSC ticks nor timer count are presented as physical elapsed microseconds.

The missing-EOI control first completes generation1, then leaves vector241 in
service. Generation2 is pending in IRR; the target owner reads both bits before
the BSP accepts the intended bounded diagnosis. This is much stronger than a
generic timeout, but remains an emulator observation. Early-ack is rejected by
the callback before it consumes the first work item. It does not require a
native data race to expose the violation.

Per-CPU handler ticks include selected C dispatch and EOI; work ticks cover the
drain and callback. Their difference is instrumentation-dependent nonwork handler
cost and excludes assembly prologue/epilogue. Do not call it complete interrupt
latency or worst-case cost. Sparse CPU identities, sequence outcomes, retained
late work and distinct EOI counts must all match, not merely a success banner.

The independently written transition oracle explores all words of length6 over
publish1/publish4/drain/timeout. Its finite alphabet excludes arbitrary weak-memory
interleavings. The actual pthread/TSan run exercises the C publication chain;
the actual guest exercises the ISA/ABI adapter. Neither implies the other's
coverage or reviewer/learner independence. Keep all three evidence classes.
