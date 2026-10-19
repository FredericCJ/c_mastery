# H745 cache-line handoff and joint-generation health

This required continuation addresses G-B06-T59-04-TRANSFER and
G-B06-T59-07-TRANSFER. It extends their original T59.04.a/b/c and T59.07.a/b/c
rubrics under `curriculum/v5/contracts/b06.md`; it does not change historical
acceptance. Families B/C/D/F/G/I/J apply. Prerequisites: the T59 journal and
image-generation artifacts, ARM barrier reasoning, and the existing H745
noncacheable shared-SRAM port. Keep host verification distinct from target code.

## Environment and representation

Use C17, GCC/Clang host tests and GNU Arm Embedded Cortex-M7/M4 freestanding
cross-builds. `h745_cache.h` defines a32-byte peer record. The supplied original
`h745_port.c` uses HSEM semaphore0, process0, M7 COREID3/M4 COREID1, and shared
SRAM3 address0x30040000. Reserve that entire32-byte line in **both** linker maps;
no allocator, ordinary `.bss` clear, unrelated variable, DMA, debugger write or
second cacheable alias may touch it while the protocol runs. The M7 region is
normal cacheable memory; the M4 has no data cache. The preceding noncacheable
route remains a distinct comparison. ISO C alone specifies neither MMIO nor
cross-core cache/HSEM visibility. Compiler barriers and the inspected target
architecture provide those parts of this selected port contract.

Before either core enters the port, trusted cold-start code must hold both users
quiescent, enable the HSEM/SRAM access clocks, configure the MPU/cache, initialize
the line with a nonzero trusted boot generation and zero remaining words, clean
it if necessary, and then release the peers. This continuation does not change
cache enable, clocks, option bytes or reset controls. It does not supply a claim
that an independently resetting peer is held or that a powered DMA is stopped.

## Problem and necessary mechanism

Implement `fw_cache_range` and `fw_peer_step` in the learner stubs. Cache requests
are clean-to-PoC or invalidate-to-PoC. Nonempty ranges must start on32-byte
boundaries, contain whole lines, be at most4096bytes and fit a32-bit address
space. Emit DSB, one address operation per ascending line, DSB, ISB. Zero length
emits nothing. Invalid ranges emit nothing; a valid final line may end exactly
at2^32 without computing a wrapped one-past address. Callback/operation must be
valid even for zero length. The callback is non-reentrant and cannot fail.

The target wrapper masks local ordinary interrupts, attempts the reserved HSEM
once, invalidates before reading, applies the state operation, cleans before
release, completes writes and restores the saved mask. A failed lock returns
`H7_BUSY` without touching shared storage. One serialized local user is a
precondition: same-core recursive read-take cannot distinguish the old owner.
NMI and fault handlers must not enter it. This bounded critical section and
polling are deliberately separate from interrupt-driven HSEM notification.

Generation mismatch rejects unchanged. BEGIN reserves one operation, clears that
core's health and rejects overflow, peer failure or confirmed readiness. END
returns exactly one reservation, including after failure. HEALTHY requires the
caller's outstanding count zero. FAIL_PEER is a **trusted supervisor report**,
not an unauthenticated image command: it marks the other core failed, clears its
health and readiness, but never clears outstanding ownership. CONFIRM requires
both current-generation health bits, no failure and zero outstanding operations.
The `confirmed` word is a volatile readiness decision, **not a durable flash
commit**. A later failure clears readiness. The actual persistent joint-image
commit must separately bind this generation and the journal's commit protocol.
This port has no unsafe runtime force-unlock, generation wrap or restart method.

## Reasoning and deliverables

D1: derive line size from PM0253 CCSIDR encoding and give the exact register/
barrier sequence, cost O(length/32), fixed stack, no heap and helper closure.
D2: code and invariants: one line/one HSEM owner; clean completion before unlock;
fresh invalidation after lock; only END reduces outstanding ownership; rejection
preserves state. Explain why `volatile`, a release C atomic, or a doorbell alone
does not implement this cache contract.
D3: produce range traces, peer-failure histories, stale-generation/overflow
controls, and the dirty-neighbour counterexample. Explain why a cache-line
invalidate can lose valid unrelated data and why alignment must describe the
owned allocation rather than only a pointer value.
D4: cross-compiled disassembly, symbol/link/stack reports, and a target integration
plan identifying exact cold-start/MPU/linker/errata/physical obligations. Show a
failed peer after the first core reports healthy and retain an outstanding DMA
reservation until actual hardware quiescence is established.

## Verification, rubric and hints

Run `python3 tools/run_b06_h745_cache.py`. The independently specified trace
oracle checks all128 admitted line counts for both cache operations and edge
addresses. Host tests check both first-peer orders and reject stale health,
unreturned reservations and count overflow. Three distinct mutants omit the
completion DSB, ignore generation or ignore outstanding counts; each must fail
at `B06_H745_CHECK`. A learner stub must fail semantically. Cross-builds inspect
M7/M4 code at O0/O3, saved interrupt mask, privileged barriers, link closure and
stack reports. Those are **compile/disassembly observations**, not H745 execution.

For each mapped leaf, level0 confuses readiness with durability/coherence;
level1 passes ordinary health or aligned ranges only; level2 supplies D1–D4 and
each boundary/counterexample with the stated ownership argument; level3 derives
the changed-condition protocol independently after at least seven days. The
original transfer IDs above remain mandatory, including the physical traces in
the original cacheable-SRAM variant. Hints: first draw the dirty line and RAM;
then place lock/unlock around ownership; finally distinguish failure detection,
quiescence, readiness and a durable commit.

## Variants, security and limitations

Change to two independently owned lines and interrupt-driven coalescing HSEM
notifications; derive clear/drain/recheck before sleep. Change to a DMA payload
and account for late DMA after CPU timeout. Repeat the original dedicated-board
variant with both image hashes, boot epochs, cache/MPU settings, silicon revision
and a common external timebase. A timeout does not authorize reuse or clearing
another owner's semaphore. All target data are synthetic; no flashing, reset,
option-byte change or host security modification is performed by this runner.

Author traces and generated code live in `h745_cache_validation.json` in the B06
research folder. Actual H745 execution, exact-board errata qualification,
independent learner assessment and external review remain separate. The model
cannot establish bus-fault behavior or real cache/DMA ordering. PM0253 warns that
maintenance faults can be asynchronous; the adapter's integer return is not a
hardware-fault catcher. The remaining boot/flash/persistent joint-commit adapter
is explicitly retained as required work rather than inferred from this layer.

Inspected source records: R-B06-FOLLOWUP-PM0253-CACHE and
R-B06-FOLLOWUP-RM0399-HSEM, exact locator bounds in
`research/v5/expansion/b06/followup_resources.json`. Separate assessor analysis:
`assessor/firmware_lifecycle/targets/H745_CACHE_ANALYSIS.md`.
