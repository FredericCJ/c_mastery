# Machine Image labs

Twelve bounded implementation units within existing project budgets. Start with learner src; the default build compiles but semantic gates reject unfinished functions. Assessor code is selected only through an absolute CMASTERY_ASSESSOR_ROOT. Passing these visible tests is finite mechanism evidence, never whole-project mastery.

Supported execution: Linux x86-64 (WSL Ubuntu included), C17, GCC or Clang, CMake and Ninja. Machine-image gates additionally require GNU binutils and the x86-64 SysV/Linux ABI. Hosted synchronization uses POSIX/Linux APIs. No physical MCU claim follows from these runs.

```sh
cmake -S labs/machine_image -B build/machine_image -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/machine_image
ctest --test-dir build/machine_image --output-on-failure
```

Select one CTest name with `ctest -R "^family.case$"`; use the actual names below. Work in a copied learner workspace when editing. Do not consult assessor implementations before an independent attempt.

## LAB-MI-01: Eight-argument ABI boundary
Parent: `PR-T08-02`. New work: 2 hours within its existing budget. Tests: `machine_image.abi`.

Unsigned weighted sum across register and stack argument locations.

1. Implement abi.c mi_mix with modulo arithmetic.
2. Inspect caller and callee disassembly and annotate the seventh/eighth argument addresses.
3. Change the last stack-passed argument and predict the result.

Rejecting boundaries: Zero, UINT64_MAX and independent seventh/eighth arguments.

Still required: Aggregate returns, red-zone rules, other data models and written ABI analysis remain.

Assigned source passages:
- `R-SYS-AMD64-ABI-0-99-6` — §3.2.3 Parameter Passing; §3.2.2 The Stack Frame

## LAB-MI-02: Assembly under a hostile caller
Parent: `PR-T08-03`. New work: 2 hours within its existing budget. Tests: `machine_image.hostile`.

x86-64 SysV assembly uint32 array sum with callee-saved-register sentinels.

1. Implement sum.S without touching inaccessible memory for zero count.
2. Annotate register ownership and stack alignment at the call.
3. Deliberately clobber one preserved register and retain its rejection trace.

Rejecting boundaries: Zero-length null input; unsigned high-bit element; poisoned preserved registers.

Still required: Unwind annotations, floating-point ABI comparison and independent adversarial callers remain.

Assigned source passages:
- `R-SYS-AMD64-ABI-0-99-6` — §3.2.1 Registers and the Stack Frame; §3.2.2 The Stack Frame

## LAB-MI-03: Archive extraction and symbol ownership
Parent: `PR-T09-01`. New work: 2 hours within its existing budget. Tests: `machine_image.archive`.

Create real ELF objects/archive; demonstrate demand-driven extraction and rejected reverse order.

1. Implement archive.c export.
2. Inspect nm symbols and the archive index.
3. Predict both link orders before comparing diagnostics.

Rejecting boundaries: Correct link order executes; reverse archive order rejects; export is globally defined.

Still required: Weak/duplicate symbol experiments and archive grouping decisions remain.

Assigned source passages:
- `R-SYS-CSAPP-GLOBAL-3E` — §7.6 Symbol Resolution; §7.6.2 Linking with Static Libraries

## LAB-MI-04: Relocation to a shared object
Parent: `PR-T09-02`. New work: 2 hours within its existing budget. Tests: `machine_image.relocation`.

A pointer-bearing PIC object is relocated and observed through a real shared object.

1. Initialize the exported pointer to its owned message.
2. Locate the relocation entry before linking.
3. Explain the resolved address and absence of TEXTREL.

Rejecting boundaries: Pointer is non-null and points to the actual exported object; message bytes match; no text relocation.

Still required: Relocation overflow, PIE comparison and architecture-specific relocation calculations remain.

Assigned source passages:
- `R-SYS-CSAPP-GLOBAL-3E` — §7.7 Relocation

## LAB-MI-05: TLS isolation and versioned exports
Parent: `PR-T09-03`. New work: 2 hours within its existing budget. Tests: `machine_image.tls`.

Three real threads independently advance TLS through a versioned shared-library export.

1. Implement tls.c with thread-local state initially nine.
2. Inspect the dynamic export and TLS access sequence.
3. Explain loader and initialization responsibilities.

Rejecting boundaries: Each thread sees 10 then 11; parent does not observe child increments; MI_1 export version exists.

Still required: dlopen lifecycle, initialization arrays, interposition and incompatible-consumer exercises remain.

Assigned source passages:
- `R-SYS-GCC-TLS-15-2` — §6.6 Thread-Local Storage — declaration and address-lifetime rules
- `R-SYS-LD-VERSION-2-45` — §3.9 VERSION Command — global/local visibility and version nodes

## LAB-MI-06: Linker-enforced storage budget
Parent: `PR-T09-04`. New work: 2 hours within its existing budget. Tests: `machine_image.budget`.

GNU ld output layout accepts 32 bytes of BSS and rejects 128 against a 64-byte ceiling.

1. Write the BSS ASSERT in layout.ld.
2. Compare section file size and runtime extent.
3. Change the ceiling and justify the resulting admission decision.

Rejecting boundaries: In-budget object links; over-budget object rejects with the specified assertion diagnostic.

Still required: Multiple memory regions, load/run addresses and table retention remain.

Assigned source passages:
- `R-SYS-LD-2-45` — Linker Scripts — SECTIONS command, output sections and ASSERT expression

## LAB-MI-07: Entry stack to explicit exit
Parent: `PR-T10-01`. New work: 2 hours within its existing budget. Tests: `machine_image.entry`.

A real no-libc x86-64 Linux entry captures the original stack and terminates by syscall.

1. Decode argc and argv from entry.c without main or CRT.
2. Draw initial and aligned stack addresses.
3. Run with changed arguments and explain the nonzero rejection exit.

Rejecting boundaries: argc and two strings must agree; exit status is exactly 42; no interpreter, NEEDED or undefined symbols.

Still required: Environment/auxv decoding, CRT inventory and startup debugger evidence remain.

Assigned source passages:
- `R-SYS-AMD64-ABI-0-99-6` — §3.4.1 Initial Stack and Register State — argc, argv and initial stack

## LAB-MI-08: Raw syscall byte transfer
Parent: `PR-T10-03`. New work: 2 hours within its existing budget. Tests: `machine_image.syscall`.

Actual write syscall wrapper loops to exact byte completion and exposes raw kernel errors.

1. Implement syscall.c operand and clobber constraints.
2. Handle interrupted writes and preserve exact accepted progress.
3. Capture raw output and compare the complete 4096-byte sequence.

Rejecting boundaries: EBADF is -9; zero length performs no transfer; full output is exact; final ELF has no external runtime.

Still required: Forced EINTR/short-write campaigns, syscall/C ABI defense and higher-level cancellation policy remain.

Assigned source passages:
- `R-SYS-LINUX-MAN-6-19` — syscall(2) — NOTES: architecture calling conventions; write(2) — RETURN VALUE and ERRORS

## LAB-MI-09: Local overlap-safe bytes
Parent: `PR-T11-03`. New work: 2 hours within its existing budget. Tests: `machine_image.bytes`, `machine_image.byte_closure`.

Caller-provided storage is moved both directions and compared as unsigned bytes.

1. Implement mi_move and mi_compare in bytes.c.
2. Predict overlapping ranges before running.
3. Inspect undefined symbols with freestanding compiler options.

Rejecting boundaries: Forward and backward overlap; identity; zero null extent; high unsigned byte; no emitted external helper.

Still required: Complete memcpy/memset family, all alignment partitions and independent differential generators remain.

Assigned source passages:
- `R-wg14-n3096` — 7.26.2.3 The memmove function; 7.26.4.1 The memcmp function
- `R-SYS-CLANG-USER-21-1` — Language and Target-Independent Features — Freestanding Builds and required byte-support routines

## LAB-MI-10: Compiler helper census and replacement
Parent: `PR-T11-02`. New work: 2 hours within its existing budget. Tests: `machine_image.wide`, `machine_image.helpers`.

Observe emitted __udivti3, then execute local unsigned-128 division with no runtime closure.

1. Compile the naive division probe and identify its unresolved helper.
2. Implement bitwise division in wide.c with an explicit zero-divisor policy.
3. Prove the high carry case before auditing the complete image.

Rejecting boundaries: High-bit dividend and divisor; maximal value divided by three; zero divisor; no undefined runtime support.

Still required: Other arithmetic/atomic helpers, stack instrumentation and actual libgcc contract comparison remain.

Assigned source passages:
- `R-SYS-CLANG-TOOLCHAIN-21-1` — Runtime libraries — Compiler runtime and compiler-emitted support calls
- `R-SYS-GCC-15-2` — Options for Linking — -nostdlib and compiler-generated support dependencies

## LAB-MI-11: Fail-closed binary dependency audit
Parent: `PR-T11-05`. New work: 2 hours within its existing budget. Tests: `machine_image.closure`.

A learner C audit consumes actual nm/readelf reports for a no-libc ELF, dynamic ELF and unresolved object.

1. Implement audit.c with bounded report parsing and explicit error statuses.
2. Reject every unresolved symbol plus INTERP and NEEDED tags.
3. Retain reports for the positive and both negative artifacts.

Rejecting boundaries: Freestanding image accepts; hosted dependency and hidden helper reject; missing/truncated reports reject.

Still required: Link-map origin tracking, disassembly audits, LTO/instrumentation and toolchain-matrix defense remain.

Assigned source passages:
- `R-SYS-GCC-15-2` — §3.16 Options for Linking — -nostdlib, -nodefaultlibs and retained compiler helpers
- `R-SYS-GABI-2001` — Chapter 5 — Program Header: PT_INTERP; Dynamic Section: DT_NEEDED

## LAB-MI-12: Raw image and matching debug companion
Parent: `PR-T09-05`. New work: 2 hours within its existing budget. Tests: `machine_image.image`.

Real objcopy artifacts correlate an eight-byte payload at its ELF symbol address.

1. Encode the declared image manifest bytes in image.c.
2. Extract and strip debug data while preserving the debuglink.
3. Calculate raw offset from the linked symbol and compare exact bytes.

Rejecting boundaries: Payload bytes and offset match; debug_info lives only in the companion; debuglink names the companion.

Still required: Intel HEX, target loading, debugger correlation and checksum patching remain.

Assigned source passages:
- `R-PD-GDB-20261009` — Debugging Information in Separate Files — objcopy --only-keep-debug and --add-gnu-debuglink
- `R-SYS-GABI-2001` — Chapter 4 — Sections and Symbol Table: section addresses and symbol values

## Component exports

Each reusable contract has a separate static library target and declared public symbols in family.json. Observation/ELF reports are claims, not library exports. Artifact imports are intentionally empty for these introduction units: the later integration consumers must explicitly link the resulting libraries and check the required producer evidence. Do not treat pedagogical ordering as a code dependency.
