# Requests after cancellation, lost observation and reset

This is the required executable successor for X-T56-001 V1/V2 and PR-T56-07.
Complete the original B5Request exercise first. Import the learner's checked
kernel/process/user-copy/virtio components through the explicit guest builder;
the author campaign deliberately imports assessor references and labels them.
C17 remains the language baseline. The hosted transition model requires only
stdint; the guest additionally depends on the retained x86-64 ABI, original
byte routines, PCI/MMIO and QEMU virtio profile. Host Python/QEMU/GRUB are tools,
not target dependencies. Neither a libc nor an implicit helper library is allowed.

## V1: derive the transition and ownership contract

Implement request.c against request.h. A zero-initialized Rq begins FREE at
generation0. SUBMIT admits the current generation and clears its terminal count.
CANCEL records intent. TIMEOUT records UNKNOWN; neither gives ownership back.
Only a trusted COMPLETE event ends a pending/cancelled/unknown request with a
known result. RESET_BEGIN moves any owned request to RESETTING; a completion
observed during reset does not release it. RESET_ACK ends it with UNCERTAIN.
REAP is valid only after DONE; increment the generation, or retire permanently
at UINT32_MAX. All rejected transitions leave every field unchanged.

Derive the full operation/state table before reading tests.c. Explain why reset
acknowledgement establishes memory ownership while leaving the write outcome
unknown. Input generations identify this logical request, not a raw pointer.
The model cannot verify that a caller's COMPLETE/RESET_ACK is truthful; the
adapter must establish that premise. A caller that invents trusted events breaks
the contract even if its code is strictly conforming C. ISO C supplies no linear
ownership or typestate enforcement. Rust ownership can restrict application
aliases but a device/reset premise still belongs to the unsafe driver boundary.

Deliver the implementation, conservation argument (one terminal transition per
admitted request), rejected-state snapshots, UINT32_MAX retirement trace and a
counterexample to “cancel means the DMA buffer is free.” The196 table cases
cover seven states, seven operations, current/stale generations and ordinary/
maximum generations. They are a bounded state test, not a proof of all adapters.

## V2: actual two-request virtio adapter

Implement async.inc, using the state machine rather than copying the retained
single-request adapter. Supply a two-entry request pool: each entry has its own
header, status byte,4096-byte DMA buffer, owner process incarnation, waiter and
generation. The existing eight-descriptor split queue admits chains0–2 and3–5.
Pool exhaustion returns EAGAIN without publishing any descriptor. Copy writes
into the owned bounce buffer before publication. Read completion validates the
owner incarnation and complete user extent before publishing bytes. A process
exit closes user publication but retains pending ownership until completion/reset.

Interrupts prompt inspection; they do not count requests. Consume all observed
used-ring entries, validate each chain identity, and advance the16-bit index
modulo65536. Preserve distinct payloads and results when two entries complete
before one observation. The two-waiter test enrolls two actual user threads
before releasing the observer and checks that both matching results wake once.
Waiting on a known completion consumes its result and
reaps once. Waiting on UNKNOWN/RESETTING reports uncertainty without reaping.
The successor changes the original one-request profile; its public syscall
numbers and two-slot capacity are explicit in abi.h and this contract.

For a whole-device reset, stop new submissions and request device reset. Keep
all DMA memory owned until the transport reset acknowledgement is observed.
Only then clear/reinitialize ring storage and renegotiate features/queue state.
Keep request generations across reset. Do not report a cancelled or rolled-back
write merely because reset completed. A fresh request after reset must execute
against the same synthetic image and return unrelated previously flushed data.

Primary authority inspected: [Virtio1.2 §2.4 and §4.1.4.3](https://docs.oasis-open.org/virtio/virtio/v1.2/virtio-v1.2.html).
The reset-status observation is the device contract supporting queue ownership;
the C abstract machine alone provides no such device guarantee. This exercise
uses whole-device reset, not unnegotiated per-queue reset.

## Concrete verification and diagnosis

Public user.c writes different deterministic pages8/9 through two outstanding
requests, holds the observer until two real used entries exist, requests cancel,
and verifies neither slot can be reused. It then releases observation, consumes
both completions, flushes and reads both patterns. An injected timeout enters
UNKNOWN. Reset begins, an explicitly withheld acknowledgement must retain the
request, then an actual status-zero observation permits reuse with an uncertain
result. The test performs131,074 further real reads with exactly one outstanding
request, crossing the16-bit ring wrap twice. A spawned disk-owning child submits
a write, changes its source buffer and exits. The test confirms a dead owner
still has a pending request before releasing the observer, then reads back the
original copied bytes. Completion releases I/O ownership before process reaping.

The separate device-error scenario uses QEMU10.2.1's documented blkdebug rule:
one write overlapping sector96 fails with Linux EIO5. The guest must receive an
error completion, release ownership once, observe no effect from that injected
write, and successfully reuse the queue for a later write/flush/read. This is an
actual emulated device status path, separate from a synthetic C return code.
The rule and errno interpretation are recorded in the receipt. A mutation that
ignores device status must fail the intended error assertion.

HOLD and the withheld-ack option delay the trusted observer. They do not claim
that QEMU delayed device completion/reset, and cannot measure physical reset
latency. The actual device still writes its used ring and receives an actual
reset. Compare these events explicitly in the trace. The normal deadline is
100 guest ticks; a host120-second harness deadline is a failed run, not a
successful target timeout. The explicit TIMEOUT injection isolates the
unknown-outcome state from unreliable wall-clock timing.

Build the portable model with GCC/Clang debug, optimized and ASan/UBSan flags:
compile tests.c plus your request.c, with this directory on the include path.
For the guest, run:

```sh
python3 labs/b05_depth/request_adapters/guest.py \
  --kernel-source /path/to/your/kernel --kernel-lab labs/kernel_smp \
  --adapter-source /path/to/your/request-adapter \
  --build-root build/my-request-adapter --report build/my-request-adapter.json
```

Repeat GCC/Clang, O0/O2, two/four CPUs. Record exact import hashes, commands,
images, exit statuses and raw output; audit every linked ELF for undefined
symbols and dynamic dependencies. Inspect cancellation-as-completion,
reset-intent-as-acknowledgement, omitted-ack-check and ignored-device-status
controls. They must compile
and reach the corresponding violated-property assertion; compilation errors,
crashes and unrelated failures do not demonstrate semantic discrimination.

## Reasoning, costs and assessment

The global kernel lock serializes metadata; user threads execute concurrently.
Explain publication barriers, queue ownership, user-copy validation and the
absence of a claimed general DMA/cache-portability result. Two4096-byte buffers
reserve8192 bytes independent of the actual transfer. Scans are bounded by two
pool entries; copy cost is O(page size), reset includes device negotiation and
waiter handling. Compile-time frame/object measurements are distinct from
whole-call-chain stack bounds or physical latency.

H1: separate request outcome, ownership and reusable identity. H2: identify the
trusted event that may change each. H3: compare a late completion with a fresh
request after generation advancement. Preserve unaided reasoning before hints.
Level1 reproduces markers; level2 derives the ownership/terminal invariants,
implements both variants and explains the controls and evidence limits.
Level3 requires an independently assessed changed-condition attempt after at
least seven days, with a fresh counterexample and explicit new premises.
The assessor package is separate; author success never awards learner mastery.

Only this original disposable guest, original defective controls and synthetic
4MiB files are targets. The adversary controls cancellation/stale ticket/timing
inputs, not arbitrary host memory. A harmless success is premature slot reuse
or false terminal state. No host disk passthrough, network, security-setting
change or physical flashing is permitted or required. A state violation or crash
does not establish arbitrary code execution. Physical device observations and
non-author review remain separate outstanding evidence.
