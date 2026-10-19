# B06 follow-up primary inspection and execution boundary

The register `followup_resources.json` records actual bounded passages read on
2026-10-17. It supplements the original register without rewriting its historical
inspection claims. N1570 was also downloaded to ignored build storage and parsed
locally when web PDF retrieval timed out; the selected integer-conversion and
expression/pointer paragraphs were read from that text. No private book extract
is published. Exact published C17/C23 texts remain an access gap.

Actual Linux source inspection used the extracted Ubuntu7.0.0-38.38 source
package (kernelversion7.0.14), including the IRQ descriptor owner macro, request
and free paths, irq_sim comparison, device registration and hardirq timer API.
This exposed a real self-reference problem in the first threaded-IRQ experiment:
requesting a descriptor owned by the same module pins it until free_irq. The
one-shot implementation now quiesces before initialization returns; no forced
unload or ignored diagnostic was substituted. The private debug kernel and its
config/archive hashes are recorded in `debug_kernel_build.json`.

Arm memory/exception teaching PDFs and selected RISC-V supervisor/ABI passages
support the actual guest experiments. Complete Arm architectural register fields,
all FP/SIMD context rules, complete ISA revisions and STM32 part/board manuals
are not inferred from those passages. No hardware measurement was produced.

The actual CBMC commands distinguish bounded proof, witness reachability,
contradictory-assumption vacuity, insufficient unwinding and mutant failure.
Linux completion observations use original local pipes/rings; EALREADY remains
specified but was not forced in the observed schedule. Clang CFI comparison
demonstrates type rejection and a same-type authorization failure independently.
The firmware verifier is an injected trust-boundary witness, not cryptography.

P6 numerical reuse was inspected in its public numeric/vector headers, actual
vector implementation and numerical contract. The source-bound receipt
`reviews/v5/expansion/legacy_completion/checkpoint_numeric.json` supplies bounded
AVX/OS-state, scalar/auto-vector/explicit-vector and timing evidence; it does not
claim physical missing-ISA hardware or a universal speedup. T64 variants not
implemented by that contract remain explicit in the completion disposition.

Author implementations, actually executed checks, independent reviewer judgments,
learner transfer and physical validation are different dimensions. All original
gap IDs survive. Finite tests and bounded models provide evidence under stated
assumptions; they are not absolute mastery certificates.

## Numerical environment continuation

Inspected N1570 7.6.1 paragraph2 and footnote213 from the actual extracted draft: floating environment access changes assumptions available to optimizers; pragma defaults are implementation-defined. This is C11 draft evidence, not exact C17 text. Inspected Intel 2025 guide Set the FTZ and DAZ Flags, the MXCSR input/result table and runtime-only caveat: FTZ concerns result subnormals, DAZ concerns inputs, and these modes are outside IEEE gradual-underflow behavior. The guide does not establish GCC/Clang defaults or a measured speedup. The original experiment uses process-local saved/restored MXCSR and runtime operands.

## ABI and verification transfer sources

Read N1570 5.1.2.4 paragraphs5-25 in the local extracted draft: relaxed operations, release/acquire reads-from, happens-before and data-race distinction. Read actual AAPCS64 SIMD/FP register section, result return and veneer paragraphs: low64 call preservation differs from full vector context; x8 hidden result leaves scalar arguments in place; IP0/IP1/flags are permitted veneer clobbers. Read RV psABI return-by-reference implicit-first-parameter paragraph and versioned RV32I B/J immediate variants/jump reach. The actual independent assembler comparison supplements the encoding derivation; it does not imply complete ISA inspection.

## Signature and borrow/scan boundaries

Read actual OpenSSL3.5 EVP verify RETURN VALUES and PureEdDSA notes, plus ED25519 DESCRIPTION/NOTES: require exact success1, NULL digest and complete-message one-shot verification. The real host adapter preserves key identity and policy; provider algorithm correctness is external. Read Rust book mutable/immutable overlap and last-use examples, E0502 and dangling-reference discussion; the compiler comparison is safe Rust only. Read CRYPTO_memcmp DESCRIPTION/RETURN VALUES/NOTES: equality without ordering, length-dependent/content-independent timing contract. That library contract is an external benchmark claim, not proof about the original C scan.

## Linux I/O transfer and persistence sources

Read actual writev DESCRIPTION/RETURN VALUE/ERRORS ordered-prefix and short-progress text; epoll ET example plus starvation/application ready-list discussion; eventfd read/reset/nonblocking counter rules; signalfd blocked-signal read/consumption semantics; linked-timeout completion and error rules. These support the actual application queue and callback transitions, not a universal latency bound. Read open O_DIRECT and its NOTES on three alignment dimensions, STATX_DIOALIGN, filesystem-dependent misalignment and separation from synchronization; read fsync file-versus-directory and fdatasync retrieval-required metadata. The first /tmp tmpfs experiment lacked DIOALIGN and remains a failed feature observation; the selected private-home ext4 path is separately recorded. Read ext4 journal introductory commit/replay paragraphs, data modes and missing-commit/checksum replay rule. Guest termination with host caches intact is not physical power loss. QEMU manual was retrieved for navigation, but no selected cache passage has yet been registered as inspected.

## Read-only callback-table source boundary

Read GNU ld -z now and -z relro/norelro passages: eager resolution and a PT_GNU_RELRO segment intended to become read-only after relocation when supported, with a page-size caveat. The actual target checks its own Linux mapping and compares in-range corrupted indices under plain and CFI compilation. This does not equate C const, read-only mappings, type checks or bounds with authorization.

## AP5–AP7 architecture follow-through

Revisited actual AAPCS32 stack constraints6.2.1.1–6.2.1.2: universal and public
alignment rules differ. Inspected official RISC-V source chapter translation
steps, non-leaf reserved fields, Sv39 address/descriptor and maintenance passages
(R-B06-FOLLOWUP-RV-WALK). The versioned rendered URL was unavailable in this
retrieval, so the current official source URL and its bounded locator are explicit;
this is not an assertion of reading the entire versioned manual. Inspected GCC
atomic builtin width/fallback passages (R-B06-FOLLOWUP-GCC-ATOMIC); Clang lowering
is an experiment, not guaranteed by GCC documentation. The complete Arm manual
landing page redirected to support.arm.com and returned no inspectable text;
no new full-manual inspection is claimed. The earlier official Arm introduction
and actual guest source remain the bounded comparison evidence.

## H745 cache/peer continuation source inspection

Read the actual private ST PM0253 revision5 CCSIDR and cache-maintenance passages listed in R-B06-FOLLOWUP-PM0253-CACHE; rendered and visually inspected pages240-241. The line-size encoding1 means8words/32bytes; DCCMVAC and DCIMVAC use PoC, and completion DSB is distinct from ISB. The manual explicitly restricts privileged access and describes asynchronous maintenance faults. Read actual RM0399 revision4 HSEM procedure/register pages and the selected memory-map entry: word-only accesses, COREID3/1, read-take with PROCID0, matching owner release and notification clear. Cross-checked COREID definitions in the already private pinned ST header; this does not broaden its historical inspection record. The latest official PDF web-parser failures were size limits, not proof the manuals were unavailable. These bounded private revisions are not silently treated as current revisions. No exact-silicon errata, board behavior, physical cache/DMA ordering or power-cut result is claimed.

## H745 flash-word correction

Read RM0399 rev4 pp167-170:256-bit flash word plus10-bit ECC, write-buffer/queue completion, nonvirgin-word warning, force-write preventing later missing-bit updates and PSIZE as parallelism. Read the listed error/status/control/key passages and memory-map base. This materially changes the earlier4/8-byte synthetic model: a delayed commit marker needs its own virgin32-byte flash word, and a timeout is not cancellation. The new original backend models complete32-byte programming and compiles the actual privileged MMIO adapter; physical erase, voltage/clock/errata, ECC BusFault recovery and complete dual-image boot integration remain separate requirements.

## Selected erase and F439 programming continuation

Read the exact new bounded RM0399 revision4 and RM0090 revision21 passages registered separately from the earlier flash-word inspection. H745 erase resets both data and ECC to virgin; it uses a two-entry erase queue, may follow queued program work, and bank erase overrides sector erase. Bank1 sector6/7 are128KiB at0x080C0000/0x080E0000. F4392MiB uses sectors10/11 at those addresses; logical bank2 sector12 starts at encodedSNB16, not12. RM0090 status text says EOP depends on EOPIE, unlike an unconditional completion witness. Its selected2.7..3.6V x32 parallelism must match supply, and successive1-to0 programming does not imply H745 ECC-safe rewrite. Actual FLASH_ACR definitions place ICRST/DCRST in ACR and require disabled caches, resolving the earlier sequence prose naming FLASH_CR. These are bounded primary-text observations, not exact-device errata closure or physical execution. The first target link exposed compiler-generated memset from aggregate initialization; its failed receipt is retained, and the implementation now initializes semantic fields explicitly rather than silently linking a hosted runtime.

## Signed target and precise-read boundary

Read the official Monocypher Ed25519 API, release4.0.3 and current bug notes, the pinned check wrapper and BLAKE2b declaration, and the private retained license. Ed25519 success0 differs from the existing OpenSSL success1 boundary; its default EdDSA/BLAKE2b API is not Ed25519. The provider is a separately permitted primitive for admission/boot assessment, never evidence of first-principles crypto internals. Current comparison-result timing and earlier signing timing issues are explicit limits; selected public-verdict use is not a constant-time certification. Read RFC8032 section7.1 TEST1 directly, then corrected a fixture transcription extra hex nibble and added compile-time string-size checks before rerunning the campaign.

Read actual PM0214revision10 bounded MPU/AP/barrier/VTOR/SysTick and precise BusFault passages. An MPU privileged background map does not sandbox privileged code or DMA; updating MSP cannot be followed by an ordinary C epilogue. Exact basic-frame fault recovery admits only the designated load PC/address and precise data-fault flags; imprecise, instruction, stacking and lazy-FP cases are distinct. Read RM0399revision4 section4.7.7: double ECC detection generates a bus error, burst responses can repeat it, the first fault address is retained, and clearing flash flags is not mandatory before distinct reads. Actual MPS2 fault entry demonstrates CPU recovery, not physical H745 ECC routing. No whole-document, complete provider audit or silicon errata inspection is claimed.

## Selected reset and irreversible peer release

Read actual RM0399revision4 boot-address/current-option and hold-boot passages, HSI readiness versus divider-transition flag, reset domain prescalers, SRAM1/3 allocation, HSEM/SYSCFG clocks, and SYSCFG current bank/BCM4 fields. BOOT_C2 may be set but cannot be cleared by software to undo a release. A failed peer must not trigger another generation while the first still executes. BOOT pin sampling is an external provisioning condition, not proved by reading current option fields. Read RM0090revision21 chapter6 F42/F43 HSI/PLL/CFGR passages; chapter7 describes a different device subset and is not the selected F439 authority. The F439 target probe remains single-core and narrower than the H745 boot composition. Actual target ELFs link; no board reset or dual-core observation is inferred.

## Recovery framing and selected UART

Read actual RM0399revision4 bounded UART clock, FIFO-disabled configuration, BRR, receive-error precedence and ICR/RQR/RDR/TDR passages plus GPIO field encodings. Read DS12923revision3 January2026 title, PD8/PD9 pin definitions and table9 AF7 mapping through the web PDF text. Direct private PDF retrieval timed out; no local datasheet hash is invented. MCU pin mapping does not establish a board jumper route. The original receiver admits complete immutable frames through actual Ed25519 and distinguishes TXE from TC. Host register execution and linked target MMIO code do not establish physical waveforms or persistent update commit. Initial passing campaign prose had a fixture count typo672; actual loop used684bytes. The pre-correction receipt is retained and the corrected-source campaign rerun.

## Inactive installation and code placement

Read RM0399revision4 sections4.3.7 and4.3.9 directly: read-while-write is supported across different banks, programming is256-bit withECC, and overwriting nonvirgin words is discouraged because ECC can become inconsistent. The new original writer is placed entirely in bank2 while targeting only inactive bank1 sectors1..4. Its report checks every code/read-only symbol address, not just a filename or linker heading. Initial target links exposed generated memcpy from policy structure assignment; preserve that failed receipt and rerun after explicit semantic field copying. Final passing evidence covers an inactive install, not trial/application-health/commit or physical power loss.

## Complete selected lifecycle source follow-through

Actually read RM0399 revision4 p.355 system-reset scope; inspected bounded ES0445 revision6 passages through the official PDF, with the applicability disposition in `labs/firmware_lifecycle/targets/H745_ERRATA.md`. This is not an entire-sheet inspection or physical silicon identification. Separately read RM0090 revision21 pp.76–77 and84–88 before starting the F439 single-core deployment. The two-MiB map and cross-bank execution are explicit; F439 supply qualification is a precondition, never inferred from a successful readback.

## F439 single-core deployment continuation

Read RM0090 Rev21 flash-cache enable/reset, BFB2, SYSCFG bank/main-memory mapping and USART SR/DR/BRR passages. The F439 port uses SR-before-DR error handling, waits for RXNE before any PE-clearing read, waits for TC after a transmit, and never reuses H7 ICR semantics. Read DS9484 Rev14 pp.58 and75 actual PA3/PA2 functions under AF7. No physical clock, pin, supply, flash retention or whole-datasheet inspection is inferred.

## F439 errata applicability

Actually opened ES0206 Rev24 July2026 and read the bounded revision/core/flash/USART passages registered above; the search snippet had an older revision and was not treated as the document. Selected revision3/4/5/B excludes the earlier PA12 condition; disabled data cache addresses the cross-bank condition. `F439_ERRATA.md` records limits. RM0090 p.1693 supplies the read-only device/revision register.
