# PR-CAP-MCU-01 capstone composition assessor analysis

Withheld technical analysis. The learner receives the shared capstone contract, project supplement and public fixtures. Assess the specific integration claims below; passing a retained component gate is not complete capstone acceptance.

<a id="t2001b"></a>
## T20.01.b

Mock signatures cannot establish clear-on-read, write-one-to-clear, ordering or physical quiescence. Superloop simplicity does not remove IRQ sharing. Interface compatibility includes completion and lifetime semantics, not just function types.

**Independent expected relation:** The core builds unchanged for host and target. Every MMIO side effect, completion observation and concurrency obligation belongs to an explicit adapter contract; the host model states what it cannot emulate.

**Reject superficial evidence:** A RAM mock and a board adapter have identical semantics because their C signatures match. The learner must show the causal boundary and an independent discriminator; an unrelated compiler/runtime failure is not a semantic negative control.

**Delayed transfer review:** Replace polling acquisition with DMA completion after cancellation. Revise buffer reuse and foreground handoff without exposing registers in the core. Select a new compatible event history and require predictions before execution. Check updated preconditions, resource accounting and retained successful behavior, not recall of the supplied values.

<a id="t3007b"></a>
## T30.07.b

The arithmetic is exact for the stated model, not for the physical F439. Alignment, linker regions, interrupt nesting and lazy FPU context can alter the required bound. An observed watermark only covers exercised paths.

**Independent expected relation:** The model totals1896 bytes, leaves2200 under4096, and grows to2056 with the extra frame, exceeding2048 by8. Unknown library/exception costs stay unknown and cannot be assigned zero.

**Reject superficial evidence:** Observed stack watermark and queue bytes are the whole memory budget. The learner must show the causal boundary and an independent discriminator; an unrelated compiler/runtime failure is not a semantic negative control.

**Delayed transfer review:** Enable floating-point context preservation or change interrupt nesting; identify new ABI/core documentation and stack evidence required before recomputing the bound. Select a new compatible event history and require predictions before execution. Check updated preconditions, resource accounting and retained successful behavior, not recall of the supplied values.

<a id="t3501a"></a>
## T35.01.a

Integrity against accidental corruption, authority, freshness and progress are distinct. Sequence wrap and reset persistence complicate replay policy; the framing parser does not own that application decision.

**Independent expected relation:** Malformed or unauthorized requests never change protected synthetic state; overload has bounded rejection and recovery cannot repeatedly reapply a partially completed command. Hardware output state remains a separate required observation.

**Reject superficial evidence:** A checksum and a watchdog together establish a safe authorized command path. The learner must show the causal boundary and an independent discriminator; an unrelated compiler/runtime failure is not a semantic negative control.

**Delayed transfer review:** The input transport can replay an earlier authorized command. Define sequence/idempotency policy without claiming CRC prevents replay. Select a new compatible event history and require predictions before execution. Check updated preconditions, resource accounting and retained successful behavior, not recall of the supplied values.

