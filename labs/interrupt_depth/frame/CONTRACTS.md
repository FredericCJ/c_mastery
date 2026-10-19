# Exception frames and priority are different state machines

This required successor implements the six original criteria of **PR-T29-01**,
`AX-PR-T29-01-T29.01.a` through `.01.c` and `.02.a` through `.02.c`, and their
unchanged `-DELAYED` variants (at least seven days after the ordinary attempt).
Use each exact ID in the dossier. The original baseline task, hours and receipts
remain historical. Passing this finite suite is evidence under its premises,
not proof of absolute mastery.

Prerequisites: T08 stack/call ABI, T10 vectors and reset, T11 original memory
helpers, T27 privileged register access, unsigned bit operations and bounded
spans. Bring your startup/vector artifact, original `memcpy`/`memset` and required
ABI helper implementation, an object/import audit, and your T08 stack dossier.
The assessor campaign supplies explicitly named author implementations of those
dependencies; these are not learner artifacts.

The portable decoder/model is C17 with exact-width `uint32_t`/`uint8_t`, live
nonoverlapping input/output objects, no allocation, I/O, libc or implicit runtime
dependency. Hosted tests use libc only for reporting. Target code additionally
uses GNU-compatible inline assembly and the Armv7-M ABI/ISA. MPS2 AN386 Cortex-M4
has its own RAM/vector/device model; it is not an STM32 board. C does not specify
NVIC registers, exception frames, `EXC_RETURN`, interrupt atomicity or FP lazy
stacking. No C23 feature is required; changing C edition does not change these
architectural facts or create a language interrupt-safety guarantee.

Inspected references are **R-XF-DDI** (DDI0403E.e), **R-XF-DUI** (DUI0553B), and
**R-XF-FREERTOS** (Kernel V11.2.0 ARM_CM4F), with exact passages/hashes in
`research/v5/expansion/legacy_advanced_frame/resources.json`. Cite a relevant
passage per argument. The selected FreeRTOS rules are a port contract, not a
universal RTOS rule. Published ISO source gaps remain separately registered.

## Representation and ownership contract

`xf_frame` borrows a captured live array of `available` words. `stack_address`
is a declared 32-bit target address, not a host pointer conversion. The input
must remain stable during the call. The caller proves the capture is readable
and identifies its context. A stacking fault must be rejected before any field
read: this API cannot rescue an unreadable stack by dereferencing it carefully.
Do not pass a smaller actual array with an exaggerated length.

Accept only `FFFFFFF1/F9/FD` and `FFFFFFE1/E9/ED` for this Armv7-M profile. Do not
generalize this whitelist to Armv8-M security extensions. A basic frame has eight
words; an extended frame has 26, including a reserved word. The eight core words
remain at offsets 0–28 in both forms: r0–r3, r12, stacked LR, PC, xPSR. The FP
area follows them. xPSR bit9 describes an extra alignment word **after** the
frame. Frame size and original SP must account for it. PC is the even instruction
address; xPSR.T supplies Thumb state. Thread returns require IPSR zero; handler
returns require a nonzero IPSR. This structural decoder is not a complete
exception-return integrity checker.

With STKALIGN set, or an extended frame, the captured SP must be eight-byte
aligned. Otherwise require four-byte alignment and no alignment padding flag.
Reject address addition overflow. Unsupported return encodings and missing FPU
capability are distinct from a short capture or structurally inconsistent frame.
All failures leave the output unchanged. Success stages a complete `XfFrame`.
`hardware_words` excludes the optional padding word; `original_sp` includes it.
Execution and storage are bounded constants (at most 27 input words, one result).

An extended allocation does **not** prove its FP words were written. The capture
contract separately supplies `fp_materialized`. When false, do not read the FP
area; report zeroed FP fields and a false validity flag. For an actual capture,
correlate FPCCR.LSPACT and FPCAR with the particular context and record the
capture order. A global LSPACT bit alone is insufficient for an arbitrary old
or nested frame. Copying an FP register in the handler can itself materialize
the lazy save; record state before that operation. A basic frame with the
materialized flag set is invalid.

`xf_priority` admits 3–8 implemented high bits and PRIGROUP 0–7. Unimplemented
low bits read as zero. Return the effective encoded byte, implemented logical
priority and compressed group/subpriority values. Subpriority chooses between
pending peers; it cannot cause preemption within one group. Validate arguments
and preserve output on errors. Conversion is constant space/time.

`xf_pick` is a pure bounded selection from at most 16 distinct external exception
numbers 16–511. Records contain separate enabled, pending, active and source
bits; each is Boolean. Include **all active exceptions**, not just the currently
running handler, in execution priority. `current_raw=-1` supplies no additional
active priority. Candidates must be enabled, pending, inactive and unmasked.
PRIMASK masks these configurable IRQs. A nonzero effective BASEPRI masks its
group and less urgent groups. An eligible candidate must have strictly higher
group urgency than every active priority. Among eligible candidates choose the
lowest effective encoded byte, then lowest exception number. No candidate gives
`XF_NONE` with unchanged output. NMI, faults, escalation and priority boosting
are outside this selector and belong to the following T29 exercises. Worst-case
O(n²) includes duplicate-ID validation; selection itself is O(n), storage O(1).

`xf_source(v,1)` models an asserted source event/resampling and sets pending;
`xf_source(v,0)` deasserts only the source. It does not clear NVIC pending.
`xf_pend` changes pending explicitly. These are serialized model transitions,
not MMIO read/modify/write recipes for a level-sensitive device. A physical
source still asserted after ICPR may repend; consult that peripheral's contract.

`xf_urgency` uses a policy where larger logical values mean greater urgency.
Normal Arm encoding reverses the finite range; `opposite_encoding=1` uses the
opposite controller convention. Both produce an aligned encoded byte.
`xf_api_allowed` implements only the selected FreeRTOS threshold admission:
nonzero exactly representable threshold, permitted grouping, and candidate
numerically at or below the port's allowed urgency. With eight priority bits,
PRIGROUP0 still has one subpriority bit, so the threshold's low bit must be zero.
The routine does not authorize an arbitrary API call or prove scheduler safety.

## Six criterion packages

| Exact suffix / families | Concrete work and observable acceptance | Required transfer |
|---|---|---|
| **01.a**, A/C/D/F/I | Implement `xf_frame`. Start with `[1,2,3,4,12,08000201,08000120,01000000]`, address20000000, returnFFFFFFFD. Report PC08000120, thread/PSP and originalSP20000020. Explain why handler LR is not the stacked LR. Reject all shorter lengths, unsupported return patterns, bad Thumb/IPSR/alignment, overflow and inconsistent flags without output mutation. | Add extended FP/padding/lazy cases from the documented layout. Supply the size derivation, not a guessed offset shift. |
| **01.b**, A/B/D/H/I | Supply an entry assembly shim that captures SP and LR before a C prologue. Annotate r0–r3/r12/LR/PC/xPSR hardware storage, shim saves, compiler locals/outgoing arguments, callee saves, and MSP versus PSP. Bind every stack claim to actual ELF/disassembly and static usage records. | Rebuild at O0/O3 with an FP-using callee. Account for lazy materialization, possible nested extended hardware frame, and compiler changes across the complete reachable call graph. |
| **01.c**, A/D/F/J | Create A, B, C on original emulator IRQs0/1/2 (exceptions16/17/18), A/C equal encoded priority60 and B50 with group3. Pend B and C while A runs. Emit active/pending/entry SP/EXC_RETURN/PC records and explain A→B→A→C. Require a trace of the emulator's tail-chain event in addition to equal saved addresses. | Place B's arrival between the beginning of A's stacking and handler entry in an explicit architectural event model. Identify what was interrupted, the active/pending transitions and maximum live frames. Do not pretend an emulator instruction boundary injects physical mid-stack timing. |
| **02.a**, A/B/C/F/G | Implement the selector and independent source/pending transitions. Pend A while disabled, enable while BASEPRI40 masks it, then clear the mask in a controlled target sequence. Explain why no handler ran while masked. Include active+pending, source-clear/pending-retained and source-reassertion cases. | Deassert a source while pending remains. Predict residual delivery, then distinguish clearing the source from acknowledging the interrupt-controller state. |
| **02.b**, A/B/F/I | Derive group/subpriority from implemented-bit readback, not an assumed eight-bit controller. Cover bits3–8/group0–7 and tie-breaking. Target writes all ones to one priority byte, records readback and AIRCR, then derives its effective bit count. | Change grouping3→6. For A60/B50/C70, recompute preemption and pending order. Confirm B now remains pending while A runs; explain why byte values alone do not define nesting. |
| **02.c**, A/B/C/F/J | Implement urgency conversion and selected API admission. Test urgency0, maximum urgency, unimplemented bits, zero/unrepresentable thresholds, forbidden grouping and eight-bit special case. Identify who enforces each property. | Use the opposite encoding controller with the same abstract urgency policy; preserve event order through an explicit conversion. Show a counterexample to copying raw priority bytes across controllers. |

For every row submit source, build recipe, reasoned pre/postconditions, failure
atomicity, ownership/lifetime argument, criterion-indexed evidence and a rejected
plausible implementation. Explain the finite bounds and any unexercised claims.
Use one assertion about each selected misconception; a generic passing test name
does not cover all six objectives.

## Workflow and target protocol

Run `python3 labs/interrupt_depth/frame/run.py --build build/frame` against your
own `frame.c`. The initial scaffold returns `XF_UNIMPLEMENTED` and must fail.
`target_build.py --build build/frame-target --runtime PATH_TO_YOUR_RUNTIME`
compiles the initial target scaffold but its execution must report
`XF_TARGET_UNIMPLEMENTED`. Implement `target.c` and the vector/entry portions of
`target.S`; retain word-copy startup, vector-size checks and linker reserve.
An explicit `--source`, `--driver`, `--boot`, `--fp-callee`, `--cc` or `--opt`
selects an alternative artifact. No implicit reference fallback exists.

Use `qemu-system-arm -M mps2-an386 -cpu cortex-m4 -display none -monitor none
-serial none -semihosting-config enable=on,target=native -d int -D interrupts.log
-kernel build/frame-target/image.elf`. This original teaching image uses only
software-pended IRQs and semihost output/exit. No external target, networking,
host security change or board flashing is part of the exercise.

The target protocol sweeps grouping3/6 × thread FP0/1 × lazy0/1 × padding0/1.
Set CPACR explicitly, configure STKALIGN, own a dedicated 512-word PSP area,
reserve 8KiB MSP and retain IRQ masks during setup/reporting. Seed r0–r3=1..4,
s0=1.0 and s15=15.0 when FP is active. Record entry SP and EXC_RETURN before
compiler instructions; emit sixteen `XF_TRACE` rows containing grouping, FP,
lazy, padding, A/B/C EXC_RETURN, A/B/C SP and A's lazy-before/after flags and captured FPCAR.
Compile integer driver/decoder with soft ABI to avoid accidental FP copies;
compile the optional FP callee separately with softfp and fpv4-sp-d16. Softfp
floating instructions do not mean an FP-argument hard-float ABI. Audit each
object's imports, linked symbols, map, generated assembly and final unresolved
symbols. Use an explicit linker and original runtime objects, not default
library closure. A successful link alone is insufficient.

For a dedicated board continuation, identify part/revision/core, FPU, implemented
priority bits, vector/clock setup, board schematic and compiler. Select unused
teaching IRQs with documented source/acknowledgment semantics. Capture the same
fields plus pin/trace instrumentation, trigger timing, clock configuration and
instrument resolution. For late arrival, supply an independent timing source
and show its uncertainty relative to the documented stacking window. If that
window cannot be established, report the observation as inconclusive. Never
infer a cycle bound or silicon late-arrival recognition point from QEMU logs.

## Rubric, hints and status

Each criterion is assessed independently: **0** missing/wrong mechanism;
**1** ordinary output without premises, ownership or discriminating rejection;
**2** correct representation/contracts, relevant negative tests and source-bound
evidence; **3** independent changed-condition derivation, revised implementation
and justified limits. A level3 claim requires a separately observed learner
attempt and the scheduled delayed variant. Author tests establish support
readiness, not learner scores. An answer that conflates FP reservation with
materialization, hardware with compiler frames, priority with urgency, or source
with pending cannot pass the corresponding criterion.

Graduated hints: (1) write the two state tables—memory layout and interrupt
eligibility—before coding; (2) track the active stack separately from pending
order and annotate which address space each pointer names; (3) compare the
documented core-word offsets in basic/extended diagrams, and compute group
boundaries only after masking unimplemented bits. Stop at the first helpful
hint and record its use. Withheld solutions/transfer traces and mutation labels
remain in the assessor package.

Validation status and exact commands are in
`research/v5/expansion/legacy_advanced_frame/validation.md`. Physical observation,
external technical review, independent learner and delayed evidence remain
separate dimensions. The next T29 projects retain faults, synchronization,
latency and target-specific recovery responsibilities.
