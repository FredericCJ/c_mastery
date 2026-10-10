# Staged hints

Commit a prediction and failing case before revealing the next stage. These
hints name mechanisms without providing the assessor implementation.

| Milestone | First hint: invariant | Second hint: discriminating observation |
|---|---|---|
| 01 boot | Preserve the bootloader arguments before CPUID/MSR work changes registers | Compare the ELF entry, first header bytes, CR3 and far-jump target in GDB |
| 02 boot data | Advance by aligned tag size only after validating the unaligned extent | A 21-byte tag starting at offset 8 leads to offset 32 |
| 03 traps | Hardware and software save different parts of one normalized frame | Draw all pushes in memory order and compare saved CS/RIP/error before return |
| 04 frames | Reserved, free and allocated are different states | Double release must fail even when its address is aligned |
| 05 isolation | User permission must survive every level of the page-table walk | A supervisor page must fail despite a user-capable upper table |
| 06 scheduling | Each suspended process owns its saved stack and CR3 | Run two non-yielding workloads and retain timer-driven switch evidence |
| 07 images | Validate file spans before copying; memory span includes zero-fill | Filesz 1 / memsz 4096 needs one copied byte and 4095 zeros |
| 08 calls | A numerically plausible pointer or handle is insufficient | Check every crossed page and a stale handle after close |
| 09 queues | Descriptor/buffer publication precedes availability, completion precedes reuse | Compare queue index progression and the actual used descriptor identity |
| 10 blocks | Request completion and persistent backend state are separate events | Keep one disk image across a new guest boot after a successful flush |
| 11 shell | A boot archive supplies named spans, not trusted pointers or filenames | Reject duplicate names, overlaps and an overlong command |
| 12 VM | The original core sees a bounded host callback, never arbitrary kernel pointers | Exhaust host budget independently of instruction fuel |
| 13 store | Error after a committed callback is not proof of absence | Restart and query the committed key rather than blindly repeating an action |
| 14 diagnosis | The first broken boundary determines which layer owns the repair | Change one PTE, image extent or completion event and preserve the reduced cause |

After a second failed attempt, compare only the relevant assessor function and
write down the disclosed mechanism. Do not label this a blind assessment. A
reviewer supplies a fresh symptom-only perturbation for the delayed gate.
