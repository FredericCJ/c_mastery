# P4 acquisition, ownership and measured endpoints — successor contract

This required increment implements the eight ordinary criteria and their eight
changed-condition assessments in PR-P4-01/02. Their IDs, prerequisites, rubrics
and seven-day separation remain normative. It adds missing executable variants,
an original board capture adapter, and a reproducible observation protocol.
References and precise inspection limits are registered in
`research/v5/expansion/legacy_checkpoint_firmware/resources.json`.

Use the full PR-P3-02 streaming frame contract, PR-T27-04 original startup,
PR-T28-02 UART, PR-T29-03 SPSC, PR-T29-05 clocks, PR-T30-03 budgets and PR-T31-05
recovery components. Record source hashes, contract revisions and dependency
profiles for each import. A reference-provider run validates the author adapter;
it is not evidence that these prerequisite learner artifacts exist or work.

## Representations and progression

First derive the reset diagram: flash LMA supplies initialized RAM VMA; BSS is
zeroed; the vector begins with an aligned stack pointer and Thumb reset address;
VTOR selects the table before interrupts run. A linker success cannot establish
that startup copied the data. The author emulator poisons VMA data and BSS before
executing the actual reset routine. Relocation changes addresses, not the copy
contract. DMA1 cannot access the selected F439 CCM region: a deliberately linked
CCM placement must be rejected before any peripheral is enabled.

Next connect the existing `CPFRing`, `CPFSample`, P3 frame encoder and `CPFTx`.
The ring owns four declared sample objects; physical N reserves one slot. For
N=4, five offers100..104 while paused preserve100,101,102 and count two drops.
The producer owns head and dropped; the consumer owns tail. Release/acquire on
both indices publishes payload and licenses slot reuse. `volatile` alone does
neither. Read the plain dropped count only after producer quiescence or under
the selected interrupt exclusion rule. No allocation, waiting or encoding is
allowed in the ISR. Operations are bounded O(1); encoding is O(payload length),
bounded by the existing P3 capacity. Initialization must reject non-lock-free
atomics. This is one producer and one consumer, not a generic MPSC guarantee.

Each acquisition has a sequence, value and timestamp. P3 puts the low16 sequence
bits in its header, all32 in its eight-byte payload, then value16 and tick16,
all little-endian, followed by the original CRC. The selected experiment has at
most64 events, so sequence aliasing is excluded by its bound. A missing sequence
is evidence to investigate, not permission to silently interpolate a sample.

Build polling first. TXE licenses one write to the UART data register; it does
not mean the stop bit has left the pin. DMA EN=0/NDTR=0 marks the selected transfer
quiescent; TC is separately sampled. Keep the encoded buffer immutable and owned
until the promised completion or documented abort acknowledgement. A timeout
enters ABORT and preserves the first fault; a late completion cannot erase it.
Reinitialization requires both DMA and UART quiescence. Stale generations cannot
complete a newer operation. A finite generation limit retires instead of wrapping.

The half-duplex port asserts direction before writing. The existing transfer
component waits for TC and a half-range-bounded tick interval. The F439 hardware
adapter additionally holds PC0 for16000 DWT cycles after its TC marker before
releasing it. That is nominally1ms at16MHz, not an unconditional physical1ms:
qualify the measured upper clock frequency and instrument uncertainty. The
single serialized half-duplex port reinitializes its private generation on each
operation; it has no asynchronous callback source and must not be presented as
a globally unique generation API.

## Executable workflow and delivery

The baseline is C17. Use profile C for original M4 images: no libc, libgcc,
compiler-rt, libatomic or HAL. The explicit original byte/AEABI helpers are part
of the audited image. Inline assembly, linker script, MMIO addresses, interrupt
entry and weak hardware ordering are target/compiler contracts, not ISO C.
Host Python, zlib, sanitizers and QEMU are verification tools, not dependencies
of the target. C23 does not make volatile an atomic synchronization mechanism
or make a 64-bit access atomic on this selected32-bit target.

Run public checks with explicit providers; no repository reference fallback:

```sh
python3 labs/checkpoint_firmware_followthrough/run.py \
  --source YOUR/checkpoint_firmware.c --parser YOUR/checkpoint_depth.c \
  --include labs/checkpoint_depth/include \
  --tests labs/checkpoint_depth/tests/firmware.c --build build/my-p4 --cc gcc
```

For a cross-build, `target_build.py` requires explicit driver, core, parser,
halfduplex, runtime and include paths. It supports GCC/Clang, O0/O3, polling/DMA/
half-duplex, tracing0/1, physical ring2/4 and RAM relocation0/0x4000. It only
builds: it never flashes. Submit every object, image, map, disassembly, undefined
symbol list, compiler dependency file and stack-usage record. Identify all helper
providers, even if fully resolved. Explain the all-frame stack upper accounting,
interrupt nesting assumptions and why it is not a measured stack watermark.

Deliver a reset/ownership diagram, implementation, independent FIFO oracle,
stalled-TXE/DMA/TC and abort traces, first-fault/generation evidence, clock proof,
budget derivation, raw model output and a capture dossier using BOARD_CAPTURE.md.
Include exact rejected-specimen diagnostics. A mutant must compile before a
semantic rejection counts. Physical results remain unobserved until the separate
protocol is actually performed. Supply the complete host model in that case.

## Clocks, budgets and falsification

Unsigned16 subtraction maps65530→6 to12 only under an elapsed interval smaller
than32768. It cannot distinguish an additional wrap. A split64 counter read on
32-bit hardware can combine an old high word with a new low word. A sequence
retry protocol uses atomic sequence/halves in this C17 implementation: plain
racy words plus a retry loop would still be undefined in an ISO thread model.
An odd sequence or changed sequence rejects the snapshot, preserves the output
and consumes a bounded attempt. Four retries do not guarantee success against a
writer that continually interrupts; report unavailable or use a separately
bounded interrupt exclusion design. Retire before sequence wrap to avoid ABA.

Derive `R=C+B+sum(ceil((R+J_i)/T_i)*C_i)` for the declared workload. With C40,
B10, and one higher-priority job C12/T50/J5 microseconds, iterations50,74,74
give74. State the scheduling and jitter model before using this expression.
Compare with an independent unit-step schedule. UART18bytes×10bits/115200baud
requires1562.5us even with zero framing cost. A1ms source offers180000bit/s, so
no finite ring makes the infinite stream lossless. Count admitted, completed,
dropped, failed, missing and outstanding separately. Add ISR/frame costs and
interference to an end-to-end claim; never transplant a UART-only model into a
physical bound. The selected F439 BRR139 at nominal16MHz gives about115108baud,
so even the synthetic115200 model needs revision for that exact image.

## Assessment and graduated hints

The retained objective-specific rubric applies. Superficial completion is a
passing nominal trace with no ownership or source argument. Competent completion
explains each state transition, rejects the discriminating mutants, accounts
for resource/failure boundaries and records source-bound evidence. Independent
mastery additionally reconstructs the argument under withheld changes, identifies
invalidated evidence and separates portable reasoning from observed behavior.
A finite pass is not proof of absolute mastery.

Hint1: write the owner and observable event next to each transition before coding.
Hint2: separate acceptance into a UART register, DMA quiescence, sampled TC and
the external final stop bit. Hint3: for a failed acknowledgement, test that a
second begin is refused and the retained flag remains set for every retry.
For clocks, pause the writer after each individual word store. For budgets,
count queued records before calculating latency; a dropped record has no finite
completion latency. Private analyses and delayed variants are assessor-only.
