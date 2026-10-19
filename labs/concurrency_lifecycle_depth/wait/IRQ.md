# Saved-mask transfer with an actual Cortex-M4 interrupt

This is the concrete target variant for AX-PR-T51-01-T51.01.a-DELAYED.
Implement `irq.h` in the supplied unsolved `irq.c`, importing your completed
wait wrapper and CMX artifacts. The parent wait contract and original rubric
apply. The target is an original disposable QEMU `mps2-an386` image; no hardware
flashing, host interrupt configuration or external device is involved.

Use privileged ARMv7-M thread/handler execution, soft-float Cortex-M4 ABI and
the supplied vector/linker layout. GCC/Clang inline assembly, MMIO addresses,
exception numbering and memory-helper ABI are target/compiler contracts, not
ISO C guarantees. `cw_irq_save` returns the old PRIMASK then masks configurable
exceptions. `cw_irq_restore` restores that exact value with compiler and
architectural ordering appropriate to this port. Unconditionally enabling IRQs
breaks a caller's outer critical section. PRIMASK excludes neither NMI nor
HardFault; neither handler may mutate this world's state. No second CPU exists
in this exclusion proof. The native two-CPU adapter uses a real mutex instead.

`cw_irq_begin` saves/masks, invokes the test hook, performs the complete
predicate/enrollment operation, then restores the saved state. It returns the
CMX model result: CMX_BLOCKED is later published in the node, not a sleeping C
call returning after a context switch. `cw_irq_post` applies the same saved-mask
discipline and cannot block. This experiment verifies an interrupt producer and
wait enrollment; it does not implement a new context switcher or task runtime.

The harness enables only configurable IRQ0 and sets its NVIC pending bit inside
the masked hook. It verifies no handler entry while masked, then verifies real
exception entry with IPSR=16 after restoration. The IRQ handler posts to the
actual CMX semaphore. Repeat while an outer mask is already1: enrollment must
leave the interrupt pending until the outer owner restores0. Finally publish
before the next enrollment and require immediate predicate consumption. These
are deterministic architectural observations, not measured physical IRQ latency.

Run reference and two wrapper controls (restore zero, omit masking), a stale
predicate control, and each unsolved wrapper/adapter. Success is CW_M4_PASS and
exit0; a semantic control is CW_M4_CHECK and exit1. A fault, unresolved symbol,
timeout or generic process failure is not the expected counterexample. Inspect
mask instructions, vector entry, exception count and invariant failures.

Link no standard startup or library archives. The imported, original course
`support.c` explicitly implements compiler-emitted memory operations, including
the required AEABI aliases. Audit its exact source, map and disassembly and list
the aliases actually linked per image. Do not call this image helper-free.
Reject unresolved symbols, libgcc/compiler-rt/libc/libatomic and undeclared
helpers. Use the learner-owned support implementation as an explicit dependency;
the author campaign's completed support stays in the assessor package.

The author campaign records GCC O0/O3 and Clang O3 builds and18 isolated guest
outcomes. Current results and exact source identities are in the wait successor
ledger. Deliver a saved-mask ownership argument, interrupt timeline, maps and
generated-code evidence. The model's two-field completion is safe here only
because every relevant updater shares this exclusion domain. Native weak-memory
ports, physical boards and independent delayed learners need their own evidence.

Primary inspected support: CMSIS-Core PRIMASK access and configurable-exception
masking, NVIC pending-bit semantics, and core_cm4.h's ISER/ISPR offsets/base.
The original CMX port's saved-mask sequence and course memory helpers were also
read. These passages do not establish a peripheral or STM32 board-specific claim.
