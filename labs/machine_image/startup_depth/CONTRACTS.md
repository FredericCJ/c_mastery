# T10: entry is a contract boundary

This additive package supplies executable support for the thirty existing
`T10.01.a`–`T10.10.c` criteria and their distinct changed conditions in
`curriculum/v5/expansion/legacy_assessments/PR-T10-*.json`. Keep those stable
assessment IDs, project prerequisites, hints, rubrics and seven-day independent
attempts. This contract adds concrete mechanisms and acceptance evidence; it
does not replace the accepted project, its historical hours or its receipts.
Sources and exact inspection bounds are in [SOURCES.md](SOURCES.md).

## Environment and ownership of initialization

Use the active C17 baseline. Core algorithms use fixed-width unsigned arithmetic,
caller storage and explicit lengths. GNU assembly, ELF, the SysV AMD64 ABI,
Linux syscalls, GNU constructor attributes and CRT internals are separate contracts.
The primary language text inspected here is the C11 draft N1570, not the published
C17 standard. No C23-only facility is required; changing language mode does not
turn process entry, a syscall or a CRT filename into an ISO C interface.

For this Linux experiment the kernel maps load segments, handles the file/memory
size difference, supplies the initial table and may enter an ELF interpreter.
`AT_ENTRY` identifies the application entry even when it is not the first PC.
Relocation, TLS and language-runtime initialization have additional owners.
Removing the interpreter makes those obligations explicit; it does not satisfy
them. These claims are bounded by the inspected Linux6.18 loader functions and
the separately observed WSL kernel, not an assertion that their builds match.

Derive a responsibility table before execution: operation, producer, consumer,
required state, failure, evidence. Include mappings, BSS, relocation slots,
TLS, guards, init hooks, main and exit. Inspect glibc2.43 and musl1.2.5 bodies
and the actual installed object origins; filename equality proves no role.
`crtbegin*`/`crtend*` belong to compiler support; the selected entry and fragment
objects must be identified from the actual link, symbols and disassembly.

## Dependency-respecting construction

First complete T07 build/provenance work, T08 register/stack contracts and T09
ELF/relocation/initialization work. Retain the learner's own byte readers and
checked arithmetic. Then complete the bounded core below, construct a raw
assembly entry and adapter, audit their linked image, and compare hosted CRTs.
The later diagnosis/transfer phase must reuse the same core and target contract.
No libc parser, formatter, allocation routine or retry library may implement the
mechanism under assessment. Host Python, `snprintf` oracles, GDB and binutils
are verification tools and must not become target dependencies.

`startup.h` defines the core API. `startup.c` is intentionally unfinished.
Run `python3 run_core.py --implementation ./startup.c --cc gcc`, then Clang;
the supplied scaffold is expected to compile and fail a named `STARTUP-CHECK`.
Do not treat its failure as an authoring defect or copy an assessor implementation
into it. Write your own assembly entry, `sd_raw6`, descriptor adapter and target
driver in your project. `linux_raw.h` is the explicit target interface.

## Bounded core: table, strings, decimal and transfers

The original LE64 fixture words are
`[2,8192,8200,0,12288,0,6,4096,9,4198400,0,0]`. Derive argc, argv,
envp and auxv offsets and total extent without casting bytes to native pointers.
Input may be unaligned. Decode 4- or 8-byte words with the header's limits;
require every terminator, checked extent, nonzero argument label and bounded
auxiliary-pair count. Publish the result only after complete validation. A
synthetic address is a label resolved through a separate immutable, disjoint
mapping table. A missing terminator, missing mapping, ambiguous overlapping
mapping or label overflow rejects without publishing output. A real kernel table
is a distinct platform precondition; never use its native-pointer scan on fuzz bytes.

Derive the unsigned decimal precondition before multiplying: the accumulated
value must not exceed `(UINT32_MAX-digit)/10`. Accept ASCII spans of1–10 digits;
reject signs, whitespace, Unicode digits, separators, embedded NUL and overflow.
For signed input allow a leading minus and magnitude2147483648 only for the
negative limit. Do not negate `INT32_MIN`. Format through caller-owned storage:
ten unsigned digits plus optional NUL, eleven signed characters plus optional NUL.
On inadequate capacity or invalid input, both output bytes and result metadata
remain unchanged. Span mode needs no terminator. Compare repeated division with
bounded powers-of-ten subtraction under this entire contract, not only value42.

The transfer state owns no buffer: its caller supplies live, exclusively borrowed
storage until completion, cancellation or abandonment after a documented return.
Invariant: `0 <= offset <= length`; every backend receives exactly the remaining
suffix. Success advances only by the accepted count; an error accepts no bytes.
Counts above the request, inconsistent status fields and invalid error numbers
are backend contract violations. EINTR consumes one budget unit without progress;
EAGAIN returns resumable `SD_BLOCKED`; an exhausted budget yields. Read zero means
EOF, write zero means stalled. Cancellation never retracts an already accepted
prefix. Terminal states make no further backend calls.

For `ABCDEFG` and `[2,EINTR,3,2]`, retain submitted slices and accepted prefixes.
Also exercise `[0]`, `[2,0]`, `[2,EAGAIN]`, `[EINTR,EINTR,3]`, overreported counts,
zero budget, terminal resume and cancellation after2 bytes. Compose one owned
frame containing a four-byte big-endian payload length12288 and12288 payload
bytes. Hold it across a real nonblocking local pipe with4096-byte capacity,
first forcing EAGAIN, then draining/resuming. Decode the resulting frame
independently and prove no header/payload byte was duplicated or lost.

## Process and syscall boundary

Capture original `%rsp` before adjustment. Establish16-byte alignment before a
SysV call; the callee entry includes the pushed8-byte return address. Preserve
the original pointer for argument decoding. The process itself has no ordinary
caller return address: its bridge translates a helper result to explicit exit,
followed by a terminal instruction if the supposedly nonreturning operation returns.
For the changed aggregate, allocate a32-byte-aligned four-word argument at the
call stack boundary; preserve the original table independently of this allocation.

For Linux x86-64, derive the C-to-kernel translation for the number and six arguments.
Account for the seventh C parameter's stack location, `r10` versus `rcx`, and the
`rcx`/`r11` syscall effects. A separate assembly function is one valid implementation;
inline assembly additionally needs correct operands and memory clobber reasoning.
Only the fixed local operations in this lab are admitted: output, mapping/protection,
unmapping and exit. Use a synthetic temporary file unlinked immediately after opening
to make the sixth mmap argument4096 observable. Never fuzz arbitrary syscall numbers.

Translate the Linux64 reserved error range to `{failed,value,error}` without TLS.
Check0,7,-9,-4,-4095,-4096 and an unsigned word with its high bit set; the last
two are classification inputs, not permission to dereference an address. Cross-assemble
the documented AArch64 adapter and label that evidence compile-only. Derive the
ARM EABI even-register pair and padding for a64-bit offset; do not equate it with
an AArch64 or x86-64 convention.

## Image, runtime, termination and failure experiments

Build GCC/Clang O0/O2 fixed-address raw images with `-ffreestanding -fno-builtin
-fno-stack-protector -fno-pie -no-pie -nostdlib`. Emit exactly4096 bytes
`'A'+(i%26)`, exit42, and optionally append the ten digits of UINT32_MAX. Use
`python3 run_target.py --image ./image` (plus `--decimal` for that extension).
Retain object/final symbols, map origins, relocation/program/dynamic headers,
disassembly, section sizes and compiler stack-usage files. Audit every increment:
base, parser, formatter, division-free formatter and restricted private-state hook.
The target service table admits these bounded services and explicitly refuses
locale, Unicode digits, heap growth, stdio, TLS/errno and implicit constructors.
Compile a forbidden-call negative control and inspect a statically resolved
forbidden provider under ordinary and LTO links; `nm -u` alone is insufficient.

For dynamic, PIE, static and static-PIE hosted originals, retain driver traces,
startup-object hashes, metadata and a first-instruction GDB capture with ASLR left
enabled. Compare initial PC with auxiliary-vector entry, stop at application entry,
then main; stop separately at static-PIE self-relocation. Run equivalent TLS/init/
errno probes with the private musl toolchain. Remove one actual CRT fragment from
a copied original link. Diagnose what disappears; a link failure is not mandatory.

Compare hosted return, `exit` and `_Exit` with a static fully buffered stream and
two separately registered handlers writing distinct raw-descriptor markers. Predict
order and bytes before running separate children. N1570 makes immediate-exit stream
cleanup implementation-defined; the Linux observation is not a portable golden
output. A blocked original worker distinguishes raw thread exit from exit_group.
No process other than the original child is signalled or terminated.

For custom-entry library proposals, list initialization assumptions and inspect
the actual archive/provider before execution. The proposed musl errno service uses
the TLS register, so this raw runtime rejects it. Replace it with caller storage;
finding a resolvable symbol does not initialize TLS. The restricted hook may only
write supplied/private static state, may not register cleanup or call unsupported
services, and must appear in the closure audit.

Create controlled wrong-stack-pointer and wrong-alignment specimens that reject
before dereferencing the invalid input. Separately inspect an original static PIE
with a retained pointer relocation and no relocation consumer: reject the mismatched
slot before dereference, then repair the one slot explicitly. This is not a general
loader. A ROM-to-RAM copy model tests missing initialized-data setup separately
from Linux ELF loading. Combine alignment and relocation defects; fixing one must
expose the other. Retain failing artifacts before minimization and compare two
compilers. Debug a stripped image only with its identity-matched companion.

## Deliverables and assessment

For each existing criterion submit its prediction, implementation or explicit
derivation, exact input identities/commands, ordinary positive and intended
negative result, resource/failure argument and changed-condition package. The
mapping is objective → original criterion → these named artifacts → private
technical analysis → current source-bound evidence. Shared runs count for a leaf
only when their observation and rubric explicitly assess it.

Level0 includes unsupported startup assumptions, accidental runtime dependencies,
wrong ownership or invalid “portable” outputs. Level1 merely reproduces the ordinary
case. Level2 explains the causal boundary and rejects the specified counterexample
with independently checked evidence. Level3 derives and implements the changed
condition after at least seven days without the worked analysis. Passing the
author reference suite does not award any learner level or prove absolute mastery.

Graduated hints: (1) locate the earliest consumer of the disputed state; (2) compare
the incoming ABI state with the first contract the code assumes; (3) record exact
bytes/addresses or accepted prefixes before changing code. Original criterion hints
and delayed tasks remain binding; assessor answers are excluded from learner exports.

All executable targets here are original local programs with synthetic data and
bounded timeouts. Harmless success conditions are exact bytes, exit codes or a
specific precondition rejection. No crash, UB or sanitizer finding establishes
code execution or privilege escalation. Compiler/profile variation is required;
board execution, non-author review and independent delayed performance remain
separate evidence. Current authored/executed states are recorded in the canonical
successor support, never inferred from this document alone.
