# FW-START: selected cold reset through two-core boot admission

Required PR-T59-03/04 target continuation; retain the original objective and
seven-day changed-condition assessment identities. Families A/B/C/D/E/F/G/I/J
and curriculum/v5/contracts/b06.md apply. Prerequisites are the actual guarded
slot reader, signed envelope, paired record, HSEM peer contract and terminal
handoff. Use C17 freestanding with compiler-specific Arm assembly, no hosted
runtime, fixed storage and the explicitly permitted FW-BOOT signature/hash
provider. The cryptographic internals remain outside this assessment.

Implement selected_startup.h and selected_boot.c. The selected H745xI map is
unswapped2MiB flash: immutable M7 bootstrap08000000..0801FFFF, immutable M4
bootstrap08100000..0811FFFF, two M7 candidates08020200/08040200 and two M4
candidates08060200/08080200, each with the exact FW-STORAGE envelope layout;
journals080C0000/080E0000. M7 private stack/data use DTCM20000000..2001FFFF;
M4 uses SRAM130000000..3001FFFF. The shared peer line30040000 and publication
line30040020 belong exclusively to this boot protocol. These exact regions
are not user-provided addresses. Initial BOOT pin0, BCM4=0 and fixed boot
addresses must be independently witnessed; this task never writes option bytes.

The profile starts at a cold system reset with no earlier application, cache,
MPU, DMA, pending external IRQ or active handler. Check HSI64-ready/divide1,
all PLLs disabled, HSI system clock, reset domain prescalers, caches/MPU off
and CPU2 not already released. HSIDIVF resets0; requiring it before any divider
change would reject the valid reset profile. Enable the SYSCFG access clock,
verify unswapped bank state, BCM4 held and exact current boot addresses; then
allocate SRAM1/3 to CPU1 and enable HSEM. Changed assumptions reject before
release. Register enable side effects before later rejection are documented;
rejection does not mean every hardware register is unchanged.

Both cores independently read and authenticate actual stored bytes under the
same compiled public teaching-key/floor policy. The master publishes the actual
FWPeer generation and a sequence/generation/M4-vector/complement mailbox, clears
the release acknowledgment, executes a completion barrier, and only then sets
BOOT_C2. That bit cannot be cleared to re-hold a running core. The M4 bootstrap
must compare its independent selection to the mailbox. Both acknowledge boot
self-checks through the actual HSEM-serialized peer implementation. The master
waits for both and zero outstanding operations, publishes a generation-specific
go word, and both use the actual terminal MSP/VTOR/MPU handoff. Poll iteration
limits bound work but are not time units or measured deadlines. Failure after
release is terminal until system reset; it never tries to boot an older pair
while the other core may still execute the new generation. No persistent journal
is written here. Boot self-check acknowledgment is not application health.

selected_reset.S initializes only its core's linker-owned data/BSS, installs its
own aligned vector table, and preserves the precise BusFault handler. The fixed
linkers assert bootstrap sector and stack reservations. selected_f439_probe.c
is a separate single-core cold-HSI16 startup diagnostic with an observable RAM
sentinel; it is not the H745 pair loader or a completed F439 update deployment.

D1: derive the reset/provisioning preconditions and every register access from
the registered RM0399/RM0090 passages. D2: implement publication-before-release,
independent mailbox matching and one-way release failure handling. D3: test each
wrong clock/divider/cache/MPU/boot address/bank/hold assumption; corrupt each
mailbox field and prove no wrong-generation admission. D4: run six host profiles
and reject the skipped-bootstrap-validation and early-release mutants plus
learner stub. D5: link M7/M4 O0/O2 complete selected-address images with actual
signature, guarded storage, HSEM and handoff dependencies; audit symbols,
sections, map assertions, barriers and bounded stack records. D6: distinguish
the F439 diagnostic, host register model, earlier MPS2 actual CPU execution and
still-unobserved STM32 two-core hardware behavior in the evidence ledger.

Level0 sets BOOT_C2 first or assumes it can be undone. Level1 runs only a happy
register model. Level2 satisfies D1–D6, preserves exact side effects/failure
states and rejects both controls. Level3 independently implements the original
delayed peer-failure variant after seven days and derives why reset/quiescence
is needed before another generation. Hints: inspect current boot addresses,
not programming registers; no changed divider means no new HSIDIVF event;
initialize the shared line before releasing the only other writer. A variant
starts with caches enabled and requires a separately justified clean/invalidate
and MPU transition; the current cold profile must reject it. Another changes
BOOT pin/provisioning and must revise the mapping rather than use the same ELF.
Assessor reasoning remains in SELECTED_STARTUP_ANALYSIS.md.

Security scope is original synthetic signed slots, local register models and
cross-linked teaching firmware only. A harmless counterexample is an early-
release mutant observing uninitialized generation or a mismatched fixed M4
bootstrap. No option bytes, host settings or physical firmware are changed by
the runner. A compiled ELF proves neither electrical startup nor actual dual-
core scheduling. Exact silicon errata, board supplies/reset straps, UART recovery
transport, physical power loss and application health remain explicit separate
obligations. Source-bound commands are in selected_startup_validation.json;
independent review and learner performance are not inferred.
