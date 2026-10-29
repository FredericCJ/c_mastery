# F439 signed update, trial and single-core health

This required successor implements the selected F439 deployment obligations of PR-T59-02 through PR-T59-05 and PR-T59-07. The original project/unit identifiers and accepted records remain unchanged. Use FW-BOOT in `BOOT_PAIR.md`, `boot_pair.h`, `recovery_transport.h` and `f439_update.h` together with this contract. The external Ed25519/hash provider is explicitly permitted at that boundary; it does not satisfy a first-principles cryptographic-internals exercise.

## Prerequisites

Supply the authenticated header/parser, checked extent, two-record journal reasoning and reset/MPU/handoff artifacts from PR-T59-02/03/05 and PR-T27-01/03. Contrast the H745 dual-core state machine with this actual single-core owner. Prior selection and `struct fb_image` arguments must come from current successful admission; structs are not forge-proof capabilities. Caller owns exclusive flash, UART and RAM, with live nonreentrant callbacks and distinct buffers.

## Environment and limits

C17; selected revision3/4/5/B two-MiB F439 with bank1 at 0x08000000 and bank2 at 0x08100000, no remapping, BFB2 clear, main-flash reset and HSI16 with reset bus divisors. Runtime verifies device419/revision2001or2003 plus the listed clock, mapping, MPU and disabled flash-cache state; actual part identity, BOOT pins and qualified 2.7–3.6 V supply are laboratory preconditions. No option bytes are written. Target code is in bank2 0x08120000–0x0815FFFF while sector7–11 in bank1 are updated. Trust is the original teaching firmware and RFC8032 public test key, never a deployed product key. Only host fixture generation has the public test private seed.

The service owns lower64KiB SRAM; the original application uses upper64KiB with MSP 0x20020000. Its payload has no runtime-initialized data/BSS; the linker rejects either. No DMA, IRQ, NMI, lazy floating-point context, second-core or concurrent flash owner is introduced. Fail-stop and manual laboratory reset retain pending ownership; a poll budget is not cancellation. The static trusted policy floor is an admission boundary, not a new implementation of tamper-resistant monotonic storage.

## Problem

Implement `f439_update.c`, `f439_port.c` and `f439_boot.c` against the supplied headers, tests, assembly and linker recipe. Slots start at 0x08080200 and 0x080A0200, each signed payload514..2048bytes with its32byte header immediately before it and64byte signature at payload+2048. Sector7 at0x08060000 holds trial intent; sectors10/11 at0x080C0000/0x080E0000 hold journals. Each bank1 sector is128KiB. Never erase the active image, committed journal or bank2 service.

Cold boot selects the highest valid committed record whose image is currently admissible, rejects conflicting equal sequences and ignores all trial records. Exact byte `M` within the bounded polling window admits one maintenance frame; silence boots the committed image. Other data/errors fail-stop. Receive one core0 frame using original parser and signature admission, erase/verify/program the inactive sector, then freshly re-admit the stored image. Acknowledge byte5 only after stored verification and wait for USART transmission complete. Erase/program trial intent and its separate attempt word before launch; reset never automatically retries that attempt.

The application computes the specified eight-word recurrence from `trial_application.c`, then calls the fixed bank2 Thumb report entry0x08120401 with result and compiled generation. Only result671377293 and the selected generation allow commit. Commit writes the other journal, retaining the old one, freshly verifies it, then requests a keyed system reset. An authenticated image can still be faulty or malicious privileged code; the workload result is one finite health predicate.

The80byte little-endian journal contains magic, sequence, generation, version, vector, zero,32byte digest, zero parent, zero, repeated sequence, complement, CRC32 of first64bytes and final marker0xC04D17ED. Trial uses different magic and parent sequence, plus attempt word0x41543446 at byte80. See header and assessor-independent public fixture construction for exact offsets. CRC detects modeled corruption, not hostile forgery; signature authenticates image bytes and policy controls admitted generations.

## Necessity and reasoning

Derive why a partially programmed final word can already equal the desired marker: programming bits that were already1 changes nothing. Decide validity from physical bytes, not whether the modeled write instruction completed. Explain why retaining the old record allows interrupted erase of the next destination; why a successful readback under an invalid programming voltage does not establish retention; and why `FH_PENDING` cannot release the controller.

State invariants for exclusive ownership, live slot, durable parent/attempt binding, serial number exhaustion, readback/signature agreement and unchanged output on selection failure. Explain all three layers: ISO C defined unsigned arithmetic/object lifetimes; selected Arm calling/exception rules; STM32 register and voltage requirements. No F439 hidden-ECC mechanism is inferred from the H745 controller.

## Deliverables

D1: three implementations and a map from each controller write to the inspected manual passage. D2: independent byte-level oracle for every journal prefix tear plus active-image preservation. D3: reset and serial-error traces including SR-before-DR clearing, parity-error-without-RXNE, TXE versus TC, late/extra bytes and deadline exhaustion. D4: complete linked service, two original application images, host-only signatures, disassembly, undefined-symbol and whole-call-path stack audit. D5: changed-condition argument and the original required delayed variants; keep physical observations separate.

## Verification

Run `python tools/run_b06_f439_update.py` in the recorded WSL toolchain. The author runner compiles six host profiles, actual provider dependencies, three mechanism mutants, two port mutants and learner stubs. Host tests cover536 interrupted inactive writes,21 trial writes,73 journal tear schedules, three sector erase cuts, readback/controller errors, conflicting equal sequences and health refusal. The oracle compares actual record bytes before invoking selection. Inspect all assertions and fault locations; finite schedules are not all possible corruption patterns.

Link complete Cortex-M4 service at O0/O2 without runtime libraries, audit every code/rodata symbol, report entry, undefined symbols, signer exclusion and static call paths. Sign and re-admit actual original application payloads on the host. Execute good/bad original application code under MPS2 with the actual single-core commit gate; this establishes Cortex-M4 instruction behavior, not F439 USART/flash/reset hardware behavior. No target is flashed by the runner.

## Rubric

For T59.02 admission: superficial completion parses a happy frame; sufficient work independently rejects altered payload/signature/current-policy mismatch without mutation; transfer diagnoses a newly valid signature under a revoked key. For T59.03 launch/health: sufficient work distinguishes admission, live attempt and application health and traces the exact reset continuation; transfer explains privileged application trust and changed ABI/stack ownership. For T59.05 storage: sufficient work derives prefix validity and preserves a recoverable record through each supplied cut; transfer constructs a non-prefix corruption that CRC/signature gates must reject. For T59.07 execution: sufficient work identifies actual target versus model evidence, proves dependency/placement/stack constraints, and measures only real observations; transfer revisits enabled-cache maintenance and physical voltage/timing assumptions. Passing one executable or repeating an assessor conclusion does not demonstrate independent mastery.

## Hints

1. Treat image installation, trial authorization and committed selection as three separate durable states. 2. Begin with an80byte independent encoding table and a single32bit store cut. 3. Check the high bits of the final marker before asserting that only a full word completion can activate a record. 4. The full corrected implementation and detailed fault analysis are separately withheld under `assessor/firmware_lifecycle/targets/`.

## Variants

Required PR-T59-05-transfer retains the original marker-tear task; apply it to x32 programming and derive exactly which prefix masks yield a valid marker. PR-T59-04-transfer remains the two-core H745 recovery task, not replaced by this single-core implementation. PR-T59-07-transfer still requires the original cacheable-SRAM changed condition and physical traces; this disabled-cache target contributes a baseline only. After at least seven days, change from two valid journals to one corrupted record plus a current policy revocation, and reconstruct recovery without the worked oracle. Further transfer: change supply qualification to x16 and redesign store cuts; do not merely change a PSIZE constant while retaining x32 writes.

## Security

Only the original teaching firmware, synthetic images and dedicated emulator/board are permitted. Attacker controls UART bytes, timing and reset cuts; cannot change the trusted key or execute an already-admitted malicious image within this bounded input-authentication demonstration. Harmless success is refusal of unauthorized activation, or a controlled synthetic old/new selected generation under interruption. Compare the intentionally flawed active-slot, readback and health mutants with repaired code. A crash/BusFault is fail-stop evidence, not arbitrary-code execution. Signed privileged code, physical tampering, key provisioning and real flash retention remain outside the demonstrated guarantee.

## Validation

Authored executable sources; actual command/results and hashes are in `research/v5/expansion/b06/f439_update_validation.json` once produced. External review, independent learner assessment and physical F439 UART/flash/reset/power-cut observations are separate and unclaimed. The required `F439_ERRATA.md` disposition qualifies supported revisions and peripheral settings. Source inspection: RM0090 Rev21 §§3.4,3.6–3.6.6,3.9.2/10,9.3.1,30.6.1–3; DS9484 Rev14 table12 PA2/PA3 AF7. The source register records exact bounded passages and access limits.

The trusted bank2 report explicitly disables the application read-only flash MPU mapping and completes DSB/ISB before taking flash programming ownership. A correct signature or a passing host flash model does not make MPU permissions irrelevant.

The shared `boot_program_mode.h` contract and withheld corrected `boot_program_mode.c` implement this transition. `tools/run_b06_program_mode.py` actually executes Cortex-M4 and Cortex-M7 MPU refusal followed by the same compiled phase helper permitting a write to a32byte SRAM cell. Its omit-transition mutant and learner stub must fail. This is architecture evidence, not execution of either STM32 flash controller.
