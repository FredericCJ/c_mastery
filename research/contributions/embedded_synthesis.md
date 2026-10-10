# Embedded, numerical, verification and security research synthesis

Research date: 2026-10-04. Scope: T17 and T27–T35. Role: metadata_provenance with embedded-domain review. This contribution records selected-passage evidence for authoring; it is not a curriculum acceptance result or a claim of full-domain coverage.

The current resource register has **30 distinct sources: 28 with bounded assignable passages and 2 explicit research leads**. An assignable record licenses selection only within its recorded inspection scope. A section heading in a table of contents, catalog record, search snippet or SWE node does not establish inspection of that section. The search log contains a broad pass, an explicit reflection, and focused passage verification. Private captures, local publication identities and hashes support audit without publishing held book text.

## Discovery and provenance

The pinned active Knowledge generation is `20260927T121039Z-humble-cpp-intake-candidate02`, ruleset 1.3.0. Read-only metadata discovery retrieved held White, Grenning and Higham editions. Their title/front matter and actual selected pages were inspected. The catalog uses metadata AND token-prefix retrieval rather than full-text search; unsuccessful proof/book queries are retrieval gaps, not absence findings. SWE returned 229 broad title/element matches and useful cross-domain leads, but a Yiu node mixed date ranges. Original edition and passage verification supersede this discovery metadata.

Historical path pointers were rebased read-only to the user-selected active Knowledge root. No pointer, registry, library payload, generated database or historical archive was changed. Opaque documentation-package contents and archive descendants were excluded. Current source/database manifests align; a historical pending-state field is not the current database status.

| Pinned artifact | SHA-256 |
|---|---|
| Sorting rules | `f5cb3ab194aceba992f1ed1d5c721d0022ad1b18b2dea0c515b77ae16b3033e9` |
| Agent roster | `d7ef2cd30ee4599fbd270cbc6adfcd1c360067ca07edb0013b9eb45a69111d0e` |
| Shared agent contract | `873cd0d0bf1e02aa490306522a884c427072cae3624f0c6a14bbcd82cda4f5fc` |
| Metadata provenance role | `cb18ccadcdcec5e6e1c9964bab6c1a59e5a5005a15868e4aa8243fcd4c6c39b6` |
| Chief librarian role | `68e0f368b0b6ac08246e22315abed23c955a7736b13a953c55168cd8bb37878a` |
| Source current pointer | `3e6dcad3dbb2ea4e8823e797d64e06e011c06637408827df0222cba4da69ea01` |
| Source generation manifest | `bab31593699a39129e615bc1415bb83a6320d0b5c0f4742e28267ae247271fd5` |
| Source catalog | `350a8ebb411a1c2c78e7c8195b0b1e84df872ecb6cf36a4f1a23d9d3d5749761` |
| Publication index | `1f05f5498f80e49c3d2756de24907abde9eb9a941c40747d021db1ffebe4dba6` |
| Database current pointer | `a81770f9799f39156c27846da808be527c2cd838e8871858a530977cb6fa9ace` |
| Database manifest | `546116494233317d200f435357ecc9b43fef295250ed59c95501236e5b9af1a6` |
| Database file | `d505e41f66818fec5c5e10a1cccb77aec3dbaa07dd8879ea5c9db3d5bc4925e7` |
| Read-only retrieval executable | `b373f0e3e8a78aa1ac6ec4d8a32e47229aa5d687d3abfc14aa8d4e478992a930` |
| Retrieval search profile | `e99d97c054bbdaee452af7445a82b67b687bee0239834ca3556b1a4528ed95d4` |

## Hardware and startup authorities

Use Arm DUI0553B for generic Cortex-M4 memory ordering and fault escalation, AAPCS32 2025Q4 for calling/stack conventions, ST RM0090 for peripheral contracts, DS9484 for device/package and electrical qualifications, UM1974 for the Nucleo board, and ES0206 for conditional silicon limitations. These documents answer different questions. White is a readable interrupt supplement; its STM32F103 examples and generalized fault narration do not override the selected-device manuals. CMSIS gives startup structure, but its generic template contains adaptation placeholders and Armv8-M constructs that cannot be copied unchanged onto Cortex-M4. [Arm ABI source](https://github.com/ARM-software/abi-aa/blob/2025Q4/aapcs32/aapcs32.rst), [CMSIS startup reference](https://arm-software.github.io/CMSIS_5/5.8.0/Core/html/startup_c_pg.html).

The inspected reference manual is **RM0090 Rev 21, June 2024**, vendor-hosted English PDF, SHA-256 `e34807b876642b67a6208e6e4071694f2892b5df088ef89f49cdad5cb254165f`. A newer international catalog Rev 22 exists, but its delta was not inspected. An important internal locator trap is resolved: **F439 RCC is chapter 6; chapter 7 covers the F405/F407/F415/F417 group**. Actual Rev 21 passages establish CPU-only CCM reachability, atomic GPIO BSRR semantics, DMA transactions/channels, HSE/HSI/PLL configuration, RCC_CR, clock gating/sleep entry, PWM preload/update behavior, ADC enable/clock/channel selection, initial I²C master configuration, SPI functional structure, USART TXE versus TC and receive/error clearing, and independent watchdog key/clock behavior. These are bounded selections, not complete peripheral chapter inspections. [Inspected RM0090 Rev 21](https://www.stmcu.jp/wp/wp-content/uploads/2017/08/RM0090_Rev 21.pdf).

The inspected board manual is **UM1974 Rev 8, August 2020**, SHA-256 `c6ce3f2f3bf583210c2281a7da5077cc7fc29e661895da57adb45af30ca58435`. It includes NUCLEO-F439ZI applicability and actual clock/USART3 configuration passages. The chief librarian independently identified current Rev 11 in the official document portal; this contribution does not substitute that identity check for Rev 11 passage inspection. Learner hardware must still record board revision, fitted clock source and solder-bridge state. [Inspected UM1974 Rev 8](https://www.stmcu.jp/wp/wp-content/uploads/2017/08/UM1974_Rev 8.pdf).

QEMU `mps2-an386` models an Arm Cortex-M4 FPGA platform. Its documented remapping/device differences reinforce a limited inference: it supports selected core/startup exercises, but gives no STM32F439 peripheral, pin, silicon-erratum or calibrated physical timing evidence. The inspected moving master page must be reconciled with the installed emulator version before executable instructions are frozen. [QEMU MPS2 documentation](https://www.qemu.org/docs/master/system/arm/mps2.html).

## Testing, timing and faults

Grenning provides an explicit progression from host TDD to compiler checks, evaluation-board runs, target unit tests and acceptance tests. This is stronger evidence for the learning/testing workflow than treating a passing host test as hardware validation. Its historical tooling needs current reproducible commands. White helps explain interrupt priority/nesting; Arm/ST establish the actual exception and peripheral contracts. [Grenning publisher sample](https://media.pragprog.com/titles/jgade/strategy.pdf).

Liu–Layland supplies foundational fixed-priority reasoning under stated task assumptions. The inspected original environment is periodic, independent, single-processor work with period-related deadlines and bounded costs. Blocking, jitter, arbitrary deadlines and DMA contention need separate analysis. The tagged FreeRTOS V11.2.0 GCC CM4F port connects priority assertions, BASEPRI critical sections, PendSV and FPU context to actual implementation. Its assertions and selected port configuration are part of the contract; no timing certificate follows from source inspection. [Original scheduling paper](https://bears.ece.ucsb.edu/class/ece253/papers/laylandliu77.pdf), [Tagged FreeRTOS port](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.2.0/portable/GCC/ARM_CM4F/port.c).

## Proof and numeric reasoning

Hoare's original assignment reasoning is historical motivation; the first two scan pages were visually inspected after empty text extraction. Modern C overflow, aliasing, lifetimes, memory safety and termination need current semantics and tools.

CBMC training gives concrete unwinding/guard-count and completeness checks. Frama-C WP supports contract-driven deductive proof with explicit runtime-error obligations and selected memory/arithmetic models; its Real model cannot establish C/IEEE floating-point behavior. Eva supplies value analysis and approximation limits; its current manual does not directly model asynchronous interrupts. ACSL mathematical integers must be connected explicitly to bounded C integers. No tool was executed, so the research establishes readable contracts and caveats, not successful proofs. [CBMC unwinding tutorial](https://model-checking.github.io/cbmc-training/faq/loop-unwinding.html), [WP manual](https://www.frama-c.com/download/frama-c-wp-manual.pdf), [Eva manual](https://www.frama-c.com/download/frama-c-eva-manual.pdf), [ACSL 1.24](https://www.frama-c.com/download/acsl-1.24.pdf).

Higham's second-edition summation passages and Goldberg's rounding/cancellation discussion complement one another: practical summation strategies require a conditioning/error explanation, and compensation does not guarantee a small relative error under severe cancellation. Damaged extracted formulas require original-page visual inspection before exact bounds are taught. Fixed GCC 15.2 documentation records optimization contracts, including aliasing and fast-math assumptions. Roofline is useful for measured bottleneck reasoning with an explicit memory boundary; its HPC DRAM model does not predict Cortex-M4 WCET, energy or interrupt latency. [Higham edition identity](https://nhigham.com/accuracy-and-stability-of-numerical-algorithms/), [Goldberg historical reprint](https://docs.oracle.com/cd/E19957-01/806-3568/ncg_goldberg.html), [GCC 15.2 optimization contracts](https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/Optimize-Options.html), [Original Roofline paper](https://aiichironakano.github.io/cs596/Williams-Roofline-CACM09.pdf).

## Security, rejected evidence and remaining work

Bernstein's inspected April 2005 preliminary cache-timing attack supplies a concrete historical threat model. Its Pentium III/OpenSSL setting does not establish a Cortex-M4 leakage mechanism or production cryptographic assurance. Educational experiments need their own controlled target and observable metric. [Author-hosted attack paper](https://cr.yp.to/antiforgery/cachetiming-20050414.pdf).

Yiu third-edition publisher metadata/contents identify a promising explanatory book, but no actual chapter passage was inspected. CERT's older PDF route returned an HTML registration form and rule-page verification failed with gateway errors. Both remain `assignable:false`. Neither SWE's mixed dates, a catalog title, nor an indexed rule snippet is promoted to an assignment. The broader curriculum's inspected language/arithmetic, testing, Linux and hardening sources must complete T32/T35 coverage; this contribution alone does not cover every subtopic.

| Domain | Strong inspected selection in this contribution | Integration or evidence still needed |
|---|---|---|
| T17 | Higham, Goldberg, ACSL arithmetic, GCC contracts | Data layout/vectorization and exact quantitative bounds |
| T27 | Arm core/ABI, ST boot/memory/board, CMSIS structure | Exact target startup/linker/runtime source and installed tool identities |
| T28 | ST GPIO/DMA, PWM, ADC, serial functional/contracts; Grenning workflow | Selected package pins, complete transfer sequences, fixture/device contracts |
| T29 | Arm ordering/faults, ST DMA/GPIO/errata, FreeRTOS port | Device-specific ordering details, races and real interrupt/DMA experiments |
| T30 | F439 clocks/sleep/watchdog, Liu–Layland, CM4F port | Measured clocks/costs, blocking/jitter, full Stop/Standby procedures |
| T31 | Core fault status/escalation, board connection, silicon conditions | Debugger/flash/deployment commands, physical revision and recovery trials |
| T32 | Grenning strategy, CBMC unwinding, Eva runtime analysis | Broader sanitizer/fuzzing/debugging sources and executed fixtures |
| T33 | Hoare, CBMC, WP, Eva, ACSL | Reproducible tool/solver versions, obligations and environment models |
| T34 | GCC contract, Roofline, numeric error/performance tradeoff | Measurement harnesses, target counters and architecture-specific vectorization |
| T35 | CBMC completeness and Bernstein historical threat model | Complete input/memory/process/hardening/security rule coverage from integrated sources |

No full-book reading, full-hardware validation, WCET proof, verifier success or learner mastery is claimed. Authoring should remain within the recorded locators/scopes, expose assumptions in each eventual assignment, and introduce a new inspected-source record when selecting passages outside these bounds.

## Authoring addendum, 2026-10-04

Bounded further RM0090 inspection now supports UART character reception/baud calculation, ADC single conversion and sampling/trigger selection, SPI master/full-duplex steps, and seven-bit I2C transmit/single-byte receive closing. The register records disclose the precise additional pages; optimized multibyte I2C, analog precision and full chapter claims remain excluded. Three new primary records establish selected GCC15.2 instrumentation/hardening/CPU-detection contracts and three LLVM20.1.8 SSE2 intrinsic definitions. The register now has 27 sources, 25 with assignable selected passages; the search log has 37 events. Newly authored assignments may use these exact addendum locators. No installed-tool execution or hardware observation is inferred from this research.
Further original-obligation inspection adds Valgrind's illegal-access/uninitialized-value contracts, Linux's ASLR policy and GNU ld's selected hardening definitions, plus Arm exception-priority/frame/return passages. The register now contains 30 sources, 28 assignable; the log has 39 events. These contracts support a concrete Memcheck comparison, targeted MC/DC and actual image/runtime hardening audits without implying any observed tool result.
The final compiler addendum also inspects LTO compile/link consistency and version/plugin constraints, profile-generation compile/link use, and profile-use mismatch diagnostics. Exact new option locators are recorded before T34 authoring; installed behavior and held-out workload results remain learner evidence.
