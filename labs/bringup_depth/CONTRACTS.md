# Reset-to-entry contracts (T27 successor)

This required supplement preserves PR-T27-01 through PR-T27-05 and their original
ordinary/DELAYED identifiers. Work in this order: identity and authority ledger;
vector/image validation; memory initialization; clock and hooks; retained state
and handoff; linked-image diagnosis and changed-target assessment. The original
module supplies the prerequisite C, ELF, ABI and interrupt concepts. Submit the
learner implementations of `bringup.c` and `target/reset.S`, an ELF/map/closure
dossier, predictions, a source ledger and the board capture packet. References
and solutions live outside this learner directory.

## Environment and enforcement

C17 is the implementation baseline. Standard headers provide types only.
Host tests may use libc; target code may not import libc, libgcc, compiler-rt,
libatomic, a HAL or a vendor runtime. GNU section/inline-assembly syntax, ELF32,
Arm EABI, Cortex-M4 registers and semihosting are separately declared extensions
and platform contracts. C23 changes do not turn C startup into a hardware reset
contract; no published-C23 equivalence is asserted. N1570 remains an older draft.

The reference uses GCC/Clang on Linux and M4 cross compilation. Host objects are
not firmware. `mps2-an386` is an Arm FPGA model, not an STM32; its code RAM starts
at zero and selected working SRAM at 0x20000000. The F439 link uses flash
0x08000000, a deliberately restricted64KiB SRAM region, and CCM0x10000000 as the
second data bank. This does not assert that the MCU has only64KiB SRAM. F439
images are cross-built only; physical identity, power and peripheral behavior
require the separate capture protocol.

## Numeric addresses and transaction boundaries

`BuRange` is a half-open numeric interval. Reject any overflowing endpoint;
zero-length intervals are permitted at a containing interval's endpoint and
never overlap. `bu_vectors` supports encoding1: little-endian M-profile words.
Reject other encodings explicitly before interpreting bytes. Require at least
eight supplied bytes, a512-aligned vector base, a wholly contained image, an
eight-aligned stack pointer equal to the reserved stack's upper endpoint, and
an odd Thumb entry with two bytes available in code. This deliberately stronger
application contract is not a claim that every architecture uses512 alignment.
Output changes only on success. Validation cannot authenticate executable code.

`bu_memory` operates on distinct caller-owned RAM/ROM arrays. At most four copy
and four zero records use numeric offsets. Validate every range, no-touch
retained interval and pairwise destination overlap before writing any byte.
ROM source ranges may overlap each other. Reject a bad later record without
partial modification. Copy/zero loops touch exactly the union of declared
destinations; gaps, canaries and retained bytes remain unchanged. Null arrays,
out-of-bounds offsets and overlap fail. No allocation. Validation O((c+z)^2),
execution O(bytes copied+zeroed), bounded stack. Host array bounds checking is
not a proof of physical memory accessibility. `bu_f439_dma` accepts only
nonempty ranges within0x20000000..0x20030000; CCM is CPU D-bus-only. A DMA engine
and direction must still be selected before turning reachability into a driver.

## Clock and runtime state

`BuClock` is an explicit synthetic ready-bit model, not an RCC register overlay.
`start(limit,bypass)` requires limit>=1 and bypass0/1. Phase1 enables the selected
oscillator; phase2 represents a PLL configured while disabled, then enabled;
phase3 publishes dependency readiness. Each waiting phase has its own limit;
ready on the last permitted poll succeeds. Timeout enters phase4 (safe fallback,
dependencies invalid). Reconfiguration is permitted only in phase4, with both
controlled sources disabled in this model. A post-start clock failure increments
the timing epoch and withdraws all dependent readiness before recovery. Epoch
exhaustion is rejected. A polling count is not an elapsed-time guarantee.

For the F439 hardware adaptation, distinguish crystal from driven HSE bypass,
wait for documented ready and switch-status fields, and establish voltage,
flash-latency and bus-frequency limits before selecting faster clocks. The cold
cross image keeps the default HSI path. Do not write modeled phase values to
hardware registers. UM1974's default ST-LINK MCO depends on board assembly and
debugger power, so it cannot silently support standalone clock assumptions.

`bu_hooks` admits0..8 hooks. Dependency bit i refers to hook i; services are a
required capability mask. Reject missing functions, unknown dependencies,
unsupported services and cycles before any callback. Use the lowest available
index to break topological ties. A callback's nonzero return stops the chain;
`completed` contains only successful predecessors, and prior effects persist.
Callbacks cannot retain borrowed table storage or reenter the executor. No
implicit constructors, destructors or exception unwinding exist. Failure means
diagnose then halt/reset, not pretend rollback. O(n²), at most8 callbacks and
eight indices of auxiliary storage. A restartable application task needs a
separate supervisor that resets task-owned state; machine startup is not rerun.

## Retention and handoff

Serialized records are24 bytes: magic `BU27` at0..3; version4, length5,
generation6; zero reserved bytes7..10; payload11..18; little-endian FNV-1a32 over
bytes0..18 at19..22; commit0xa5 at23. Version1 has four payload bytes; version2
has eight. FNV detects selected accidental corruption; it is not authentication
and collisions exist. Validate fixed extent and length before exposing payload.
Decode failure leaves output untouched. `store` first invalidates commit, writes
23 body bytes, then commits:25 byte stores total. A cut after zero stores may
leave the old valid record; cuts1..24 cannot publish the replacement. `writes`
records completed stores. Byte atomicity and persistence ordering are assumptions
of this abstract model, not promises about SRAM under brownout or cache.

Two slots require an external single-writer protocol: write the inactive slot,
keep the old slot, then select a valid newer record. An8-bit generation is newer
if it is1..127 forward steps away. Equal or128-apart valid generations are
ambiguous and rejected. Never allow live versions separated by128 updates.
`migrate` accepts only version1/four bytes into a distinct output, preserves the
four bytes and generation, and initializes the new four-byte field to zero.
Persist migration to the other slot; do not destroy the old schema first.

`bu_handoff` requires privileged Thread mode, an eight-aligned stack,512-aligned
vector base, Thumb entry, closed configurable interrupt masks, cleared pending
sources, inactive DMA, known clocks, clean/invalidated caches as applicable, and
quiescent peers. It rejects unknown state instead of normalizing arbitrary
firmware. A DMA shutdown protocol must stop admission, request stop, wait for
the selected engine's documented acknowledgment, drain callbacks and transfer
ownership; timeout retains the old owner and forbids entry. A boolean set by the
caller is a contract assertion, not evidence that a physical DMA engine stopped.

## Target execution and diagnosis

Original reset assembly masks interrupts, establishes SP/CONTROL/masks and VTOR,
enables CP10/11 only for the hard-float variant, seeds BSS with known nonzero
bytes for this teaching test, executes two nonempty plus one empty copy record,
zeros two separated BSS ranges, and fills the reserved stack before the first C
call. Retained stage1/2/3/4/5 marks entry/copy/validated application/completion/
entry return. Semihosting is used only after initialization in the MPS2 harness.
The quiet variant still checks the retained stage without printing. The F439
image has no semihosting dependency and halts after the retained completion.

The2KiB stack has a32-byte no-access MPU guard. A genuine isolated write to the
guard must produce a data-access MemManage fault with valid MMFAR indication;
it is not an estimate of stack exhaustion detection for every access pattern.
The test call chain has48+64 bytes of explicit automatic arrays. An SVC handler
then invokes higher-priority SysTick, producing two nested exception frames.
Compute compiler-frame sums for the actual call chain, add two32-byte basic
frames, up to4 alignment bytes each, and any software saves. The FP variant
adds72 bytes per extended frame and possibly64 bytes for s16..s31 software saves
where the actual handler ABI uses them. Lazy stacking reserves space even before
all values are written; a watermark is a lower bound on the executed writes,
not a static maximum or full-program proof. Inspect `.su`, disassembly and
exception paths together. Disabled interrupts do not mask NMI or HardFault.

Run `python run.py --build PATH` for a learner implementation. The unfinished
starter compiles and fails semantically. An assessor may pass an explicit
external `--source`; solutions must never be copied into a learner export.
`check_image.py` independently reads ELF sections, program load mappings,
vectors, symbols and copy/zero tables. A passing image check is not execution.
Diagnose missing vectors before running, wrong LMA via copy sentinel, omitted
zero via poisoned BSS, and combined defects in causal order. Retain raw evidence
and source hashes; rerun after a compiler, linker script or dependency changes.

## Restart and handoff transfer exercises

Implement `lifecycle.c` after the core mechanisms. Run `python run.py --unit
lifecycle --build PATH`. These are serialized state machines in C17, not an
interrupt-safe DMA driver. Callers own all storage; no allocation or callbacks
occur inside these operations. Every operation is O(1) with bounded storage.

`bu_drain_begin(d,n,limit)` closes admission and records 0..4 outstanding callback
owners and a positive poll limit. The external adapter must already have issued
the selected device's stop request. `step(ack,done)` accepts a Boolean observed
stop acknowledgment and a count of callbacks actually completed. Reject an
excess count without mutation. A stop acknowledgment is sticky. Transfer occurs
only after acknowledgment AND zero remaining callbacks. Each unsuccessful valid
step consumes one poll; readiness on the last poll succeeds. Timeout is terminal
for this transaction: old ownership persists, and later observations cannot
authorize entry. A new transaction requires separately re-established state;
reinitializing the struct is not permission to free live DMA memory. A transferred
transaction rejects further events. The model cannot distinguish fraudulent
caller observations or establish physical cache/device coherence.

`BuTask` starts zero-initialized, with machine-owned retained state outside it.
Phase0 permits start; phase1 runs; phase2 drains task-owned cleanup. Start increments
a generation and clears exactly eight task-owned bytes; generation UINT_MAX
retires the supervisor rather than wrapping. Return requires the current generation
and records the result plus 0..4 pending cleanup owners. Zero owners permits a
new start; otherwise each matching completion releases one owner. Stale/duplicate
completions reject without mutation. Do not restart machine clock/vector/retained
initialization when a task returns. The single serialized supervisor, rather than
ISO C, enforces this lifetime protocol. User callbacks must not bypass it.

Submit a three-run task trace, stale completion counterexample, and a DMA trace
where stop acknowledgment precedes two callbacks. Predict all states before
execution. Then reverse acknowledgment/completion ordering and exhaust the poll
budget. Explain why freeing the buffer on stop request alone is a use-after-owner
defect even when the synthetic bytes appear unchanged. The private assessor
includes independent event enumeration and compiled incorrect implementations.

For target tests, the checked ELF remains the source of image/closure evidence.
The runner converts its ROM load bytes with objcopy and loads that binary into
QEMU, with independent nonzero RAM seeds. This avoids QEMU ELF zero-fill hiding
a missing startup zero loop. It is an explicit emulator setup, not a power-on
RAM guarantee. The public reset starter remains intentionally incomplete.
