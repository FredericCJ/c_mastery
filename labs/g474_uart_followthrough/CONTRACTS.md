# G474 USART2 initialization and register-revision transfer

**G-PL-G474-FULL-PORT-IMPLEMENTATION**, required successor of the recorded
`G-PL-G474-FULL-PORT` gap and `AX-PR-T28-06-T28.11.c`. Families B, C, D, F, G,
H, I and J. This adds implementation depth to the existing objective; it neither
replaces its HAL/direct/generated comparison nor changes accepted units/hours.
The primary-source register is
`research/v5/expansion/legacy_advanced_g474_uart/resources.json`.

Prerequisites: T27 reset/linker/clock ownership, T28 MMIO/USART side effects,
the existing `PlBytePort` completion contract, and the learner's earlier
generated F439/G474 register adapter. Supply the learner's `g4_initialize`,
the byte-transfer provider, a register-order ledger, a failed-initialization
analysis, and a source-bound build/capture dossier. The target is an exclusively
owned, dedicated STM32G474RE development board. Hosted fixtures use ordinary
arrays and callbacks; no host process may dereference the physical adapter.

The implementation baseline is C17. C does not define peripheral addresses,
alternate functions, clock-ready flags, or GPIO electrical behavior. Those
belong to the pinned ST device/LL definitions and selected board documents.
`volatile` preserves the selected compiler's accesses; it neither serializes
another RCC writer nor makes a read-modify-write atomic. C23 adds no automatic
MMIO ownership or asynchronous lifetime enforcement. No C23-only mechanism is
needed here. Host Python, sanitizers and primary-header comparisons are
verification tools, not target dependencies.

## Construction and operational contract

Implement `port.h` from original primitives. Inputs are explicit read/write
callbacks, integer baud, a poll bound, and a result. The selected configuration
is USART2, PA2 AF7, asynchronous TX-only 8N1, oversampling16, prescaler1,
FIFO disabled, HSI16 kernel clock. The caller owns the USART, PA2, and serialized
updates to shared RCC registers. There is no live transfer at entry. Callback
accesses terminate and obey the declared register semantics; they do not signal
errors through this interface. Bus faults/reset recovery require the separate
fault dossier, not a fabricated callback return value.

Reject null operations/result, baud0 or above1000000, bound0 or above1000000,
and divisors outside16..65535 before any callback or result modification.
Compute the nearest integer divisor of nominal16000000/baud, ties upward.
Derive the arithmetic bounds: the addition is at most16500000, representable
in uint32_t; output division is defined after admitting a nonzero denominator.
The nominal clock is an input assumption, not a measurement. At115200 the
required divisor is139; its exact nominal error is -12800/16012800.

The causal sequence is HSI enable and bounded HSIRDY observation; GPIOA and
USART2 clock enable with readback; USART2-only reset assertion/release; HSI16
kernel selection; PA2 output type/speed/pull/AF/mode; disabled USART mode and
prescaler/divisor setup; UE|TE enable; bounded TEACK observation. Preserve
unrelated RCC/GPIO bits. Reset affects the exclusively owned USART. Never
reuse the old F439 SR/DR layout for G474 ISR/TDR.

Result stages1..5 identify waiting for clock, enabling/resetting clocks,
configuring, waiting for transmitter acknowledgement, and successful readiness.
`polls` counts awaited-flag reads only, up to twice the admitted bound; it is not
an elapsed time. On clock timeout, do not access unclocked GPIO/USART. On TEACK
timeout disable UE/TE. Shared enabled clocks remain enabled: blindly undoing
them could break another owner. The output reports actual partial progress.
Initialization costs O(bound) reads, constant writes/storage, no allocation.
Inspect object code: a source-level divide may require a helper on another ISA.
Only explicitly linked original helper objects are permitted in strict images.

The supplied clock-observation adapter is a separate operation, requiring prior
successful initialization and exclusive PA8/MCO ownership. It exposes HSI16/16
on PA8 AF0 for an independent external timebase measurement. Its four model
checks cover missing readiness, exact selector/pin preservation and null
operations. The byte-provider tests distinguish absent TXE from TXE without TC;
only the latter writes a byte before timing out. Neither timeout promises an
idle physical line. The capture protocol includes the clock measurement's
uncertainty in the receiver budget.

## Work and evidence

1. Derive a dependency graph for the sequence and a mask/preserved-bit table.
   Predict each invalid-admission and timeout result before running code.
2. Implement the initializer in a separate learner source. Run
   `python labs/g474_uart_followthrough/run.py --source YOUR_SOURCE --cc gcc
   --output build/your-g4-tests` and repeat under Clang and changed optimization.
   The supplied `learner.c` deliberately rejects work; do not count it as a
   reference solution. Public tests are an anchor, not the complete rubric.
3. Add register-order and independently reasoned boundary tests. Include
   delayed ready/ack, no ready/ack, preserved neighboring bits, divisor limits,
   repeated invalid requests, and recovery after an enable timeout. Identify
   which modeled behavior has not been established for silicon.
4. Compose the initialized contract with the actual earlier byte mechanism.
   A transfer completes at TC, not merely TXE. Test stale F439 mapping and
   FIFO-enabled assumptions as distinct defects. Compare the exact edits and
   object/stack costs with the existing vendor/generated/direct experiments.
5. Cross-build with `target_build.py` using explicit initializer, driver,
   transfer source/include and original runtime source arguments. Retain
   dependency files, map, symbol list, disassembly and stack-usage files.
   Audit both individual objects and linked image for implicit calls and
   actual runtime closure. A link alone is insufficient.
6. Complete `BOARD_CAPTURE.md` and its acceptance analysis. Actual board
   observations remain pending until the dedicated experiment is performed.

Pass requires correct operations/invariants and the causal rejection of each
defect, including a compiling incorrect implementation. A translation-negative
old-map assertion is classified separately. A nominal byte match without
clock/mode/lifetime justification earns only superficial completion. The
assessor's full model, reference, changed conditions and analysis are withheld.

Hints: first write down which fields survive each mask and which accesses
require a clock. Next separate resource ownership from rollback: an enabled
shared oscillator is not an allocation owned solely by this initializer.
Finally distinguish abstract callbacks, generated machine accesses, and a
physical pin transition; evidence for one does not establish all three.

No attack target is introduced. Faults are injected only into original models
and dedicated teaching firmware with synthetic bytes. No host security settings,
external systems, deployed firmware or production data are involved. This
authoring package does not authorize or perform flashing.

Validation states are recorded in the current successor support. Finite models,
sanitizers and image audits do not prove all interleavings, silicon behavior,
physical timing, or independent learner mastery. RM0440 access and the original
parent comparator/manual obligation remain explicitly open.
