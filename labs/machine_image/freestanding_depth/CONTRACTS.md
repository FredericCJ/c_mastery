# T11: constructing and proving a declared runtime boundary

This required successor serves all thirty T11 leaf criteria and their original
seven-day changed-condition assessments. Keep the parent PR-T11 projects and
accepted `mi_move`, `mi_compare`, `mi_divide` and dependency-audit artifacts.
The new APIs complement them; they do not replace their historical contracts.
Stable assessment IDs remain `AX-PR-T11-…-T11.nn.x` and the corresponding
`-DELAYED` IDs. See the parent supplement for each exact project/criterion link.

Prerequisites are T07 command/build provenance, T08 ABI/assembly, T09 object and
linker maps, T10 startup/syscalls, C17 integer/pointer/lifetime rules and the
learner's existing machine-image artifacts. First record predictions, construct
the primitive and its invariants, inspect actual lowering, then compose an image.
Finish with a fresh changed-condition dossier. A supplied test is not that dossier.

Use C17. GNU assembly, `unsigned __int128`, ELF, Linux syscalls and MPS2 MMIO
are explicitly named implementation/target facilities. The strict reference
profile permits original source, compiler headers, caller-owned storage and the
declared target interface. It excludes libc, libgcc, compiler-rt, libatomic and
instrumentation runtimes. Host oracles, Python, binutils and QEMU are verification
tools. The compiler-runtime, hosted diagnostic and newlib comparisons have their
own explicit dependency profiles; none passes as the strict image.

Inspected references and editions are in [SOURCES.md](SOURCES.md). The available
C11 N1570 draft requires a limited header set for freestanding conformance and
leaves startup/termination details to the implementation. N3096, an April2023
working draft, expands the minimum with `stdbit.h`, most `string.h` functions and
selected facilities. Its `memalignment` wording is a draft observation, not a
claim about the published C23 standard. Exact published C17/C23 access remains
a source gap. Compiler `-std=c23` acceptance alone establishes neither library
conformance nor a forbidden-dependency exemption.

## Environment classification and dependencies: T11.01

Classify these five concrete specimens along five separate axes: hosted,
freestanding compilation, libc-free, allocation-free and custom startup.

| Specimen | Actual boundary to inspect | Changed condition |
| --- | --- | --- |
| T10 decimal parser inside a hosted harness | Parser object versus whole process | Add an allocating diagnostic callback |
| `fr_*` byte archive | Object imports, caller storage and address model | Move it into the original raw image |
| Linux `_start` calling only raw exit | Object/final symbols, segments, syscall | Add a raw write and count its service dependency |
| Custom entry with declared arithmetic runtime | Exact helper ABI and archive member origin | Replace the helper with original limbs |
| A component accepting an acquisition callback | Callback contract, lifetime and actual producer | Substitute the original malloc/free host adapter |

Explain every yes/no/conditional answer. A component with no unresolved allocator
symbol may still allocate through its callback. Supply source/header hashes,
effective compile/link commands, object imports, final object/archive origins,
dynamic dependencies, system calls, callback ownership and storage acquisition.
Bind the callback producer's contract separately; unknown behavior stays unknown.
Do not credit host `memmove` as a target primitive implementation.

## Lowering and tool controls: T11.02–03

Construct original aggregate copy/zero probes of1,3,16,64,128 and257 bytes and a
runtime-selected copy limited to64 bytes. Compare equal fixed-size copies, then
change the first byte to128 versus127 and check both ordering directions. Fill
input byte i with i modulo256;
the independent expected output is that pattern over the requested prefix and
zero elsewhere. For the aligned64-byte variant, use actually32-aligned objects;
never claim an alignment precondition merely to encourage vectorization.

Compile GCC and Clang under hosted, freestanding and no-builtin modes at O0/O2/Os
and LTO. Record `__STDC_HOSTED__`, object relocations, final disassembly and map.
Treat the provider's compilation policy separately from its consumer. Then vary
`-nostartfiles`, `-nodefaultlibs`, `-nostdlib` and `-nostdinc` independently.
Predict which CRT/library/declaration remains before observing driver output.
Use a local `stdint.h` defining32/64-bit unsigned types and reject a deliberately
leaked LP64 header that spells `uint32_t` as `unsigned long`.

Implement exact compiler-facing `memcpy`, `memmove`, `memset`, `memcmp` signatures
using your byte primitives. Copy requires nonoverlap. Memmove preserves a source
snapshot. Compare promises sign, with unsigned-byte ordering. Standard-symbol
signatures do not turn a local null/zero extension into an ISO library guarantee.
Intentionally link duplicate providers and require the linker to reject them.
Inspect a recursive provider without executing its unbounded recursion.

Two concrete diagnosis cases belong in the dossier. A driver may request an
interpreter even with `-nostdlib`; inspect `PT_INTERP` before calling an image
independent. A hosted optimizer may recognize the provider's own byte loop and
reintroduce that provider. Repair the provider's build contract and re-audit its
final body. A renamed routine or successful link is insufficient evidence.

## Arithmetic contracts and ABI: T11.04

Implement `fr_divmod128` using four little-significance32-bit limbs. The operands
are values; quotient/remainder destinations are distinct live objects. For a
nonzero divisor require the mathematical identity n=q*d+r and0<=r<d. On divisor0
return `FR_DIVZERO` with both outputs unchanged. Compare the unchanged accepted
`mi_divide`, whose local divisor0 result is0. Never execute C `n/0` as a defined
equivalent. Likewise exclude signed-minimum/-1 and out-of-range floating-to-integer
conversion before making an ISO-semantic claim.

**Versioned correction LSYS-T11-CARRY-2026-01:** in exactly128 iterations starting
from zero remainder, the processed prefix bounds the remainder below2^127 before
every shift. The high carry is unreachable. Its removal is an equivalent mutation
for this domain. The active supplement applies the correction recorded in
`research/v5/expansion/legacy_systems_freestanding/corrections.json`; the original
snapshot and accepted algorithm remain inspectable.

Now implement `fr_divstep128(r,d,bit)` with arbitrary r<d and bit in{0,1}.
Here2r+bit can require129 bits. Derive the carry/subtraction argument or an equally
valid representation. Invalid preconditions must not commit outputs. Test
r=2^127+3,d=2^127+7,bit1; use an independent unbounded-integer oracle, not another
copy of the limb algorithm, and distinguish streaming mutants from equivalent
ordinary-division changes.

Inspect variable unsigned/signed division, wide multiplication and conversions
on x86-64 and Cortex-M4. Match observed helper declarations, width and ABI.
For Arm, quotient r0/r1 and remainder r2/r3 require an assembly adapter; an ordinary
C struct return is not automatically the same convention. Direct helper div0
testing is a separate ABI exercise with its declared `__aeabi_ldiv0` policy.
Keep original helper code free of recursive compiler-helper imports.

Compare three policies: an explicitly admitted compiler runtime, original limbs,
and visible `UNSUPPORTED` without output writes. Measure linked sections, retained
object imports, compiler stack-usage reports and paired host timing batches over
the same256 nonzero cases. Record one warmup and seven measured batches of16384
operations per compiler/optimization/policy. Check a mathematically independent
checksum before using any timing. Refusal is not an equivalent faster divider.
Elapsed host time is not a target-cycle or worst-case bound.

## Atomics and instrumentation: T11.05

Use correctly aligned atomic objects of1/4/8 bytes and a supported16-byte type.
Record lock-free queries, macros, generated operations and external helpers.
Reject an ISR proposal if its actual helper lacks a bounded interrupt-safe
contract; do not infer lock freedom from the type qualifier. A deliberately
misaligned atomic is not a valid performance experiment.

Apply stack protection, ASan, UBSan, profiling and coverage to the same original
component, with and without LTO. Require actual final markers and execution
evidence: retained memory access for ASan, a signed-overflow control for UBSan,
new coverage counters and newly generated profiling data. An optimizer-eliminated
invalid access does not establish that the sanitizer checked the proposed access.
Keep these intentionally faulty controls in local original processes with
synthetic bytes; the observable outcome is the named diagnostic, not code execution.

Build an uninstrumented restricted archive with a sanitized host harness and,
separately, an instrumented component. Label their evidence distinctly. A narrow
original fail-stop add-overflow handler may close a declared diagnostic image;
it is not a general sanitizer runtime. Inspect provider retention under LTO,
where compiler-inserted references may arise after ordinary dead-code analysis.

## Byte construction and verification: T11.06 and10

Complete `free.c` from byte operations; the supplied file is intentionally
unfinished. Enumerate every valid src,dst,length in a16-byte arena with guards.
There are1785 valid triples. Compare each result with a pre-operation snapshot;
include identity, both overlap directions, empty and exact-end cases. Test all
65536 one-byte unsigned order pairs,153 split-copy cases and compare antisymmetry.
State the nonoverlap and live-extent preconditions for each metamorphic law.

The raw move adapter uses `uintptr_t` ordering under a declared flat-address
model. The portable offset API validates both ranges before writes and uses
offset ordering within one live arena. Invalid offset/length/overflow cases leave
the arena unchanged. Separately allocated objects test the flat-address adapter;
they do not establish ISO relational ordering of unrelated pointers.

The host executable may use snapshot/oracle utilities. The target must implement
the mechanism independently. Run the fixed corpus in an original no-library
image, then the supplied MPS2 reset/linker environment with original helpers.
Capture full framed output through a two-byte quota adapter. A UART-only image
must contain no semihosting breakpoint; `run_m4.py` validates its exact frame and
then stops the disposable VM. That is observation of a frame, not target exit.

Example host verification, with an explicit learner source:

```text
cc -std=c17 -O2 -ffreestanding -fno-builtin -I. tests.c free.c -o byte-tests
cc -std=c17 -O2 -ffreestanding -fno-builtin -I. service_tests.c services.c -o service-tests
```

Run both compilers and diagnostic profiles after the ordinary build. Preserve a
failure before repair. The assessor can invoke the separate reference campaign;
learner packaging contains no automatic fallback to those implementations.

## Services, startup and lifetime: T11.07–08

Implement `services.c` against `services.h`. The owner copies at most24 bytes,
offers only the unaccepted suffix and advances by a valid accepted count even
when the transport reports failure. `FR_OK` with zero progress is a contract
error. A blocked operation retains its copied data; a terminal I/O error stops
it. The callback borrows bytes only for its synchronous call and cannot reenter.
Use a quota2 callback, zero-progress blocking, short progress and refusal. Check
that exactly the intended24-byte message is accepted without replay.

Heap extension always returns `UNSUPPORTED` without modifying state/output.
The changed implementation has64 aligned bytes and a monotone offset: require
positive size, power-of-two alignment no greater than `max_align_t`, checked
padding and extent before commit. Exhaust every used offset0..64, valid alignment
and size1..65, plus SIZE_MAX and invalid alignment. No free/reuse is promised.

Compare pinned newlib-nano4.6.0.20260123 separately: a9-byte integer printf with a
caller-owned24-byte stdio buffer, two-byte `_write` progress, and an error during
flush. Inspect archive members and seven concrete missing environment hooks.
Refuse absent heap/read/seek/close/stat services honestly; verify no fake success
or state writes. The library's formatting/FILE state is outside the strict
first-principles runtime. A raw-byte service removes those dependencies while
retaining transport availability and progress obligations.

For startup, use128 RAM bytes,32 ROM bytes, data[8,24), BSS[24,40), retained[40,48),
stack[64,128), ROM data offset4, stack alignment8. Validate all extents and
pairwise overlaps before writes. Inject a reset after every write count0..32;
restarting initialization must restore data/BSS while preserving retained and
unrelated bytes. ROM and RAM are separate supplied objects. This host model
does not establish exception/reset behavior of a physical board.

For task state, provide four slots with explicit active/generation/value/hook
fields. A ready service must precede the once-per-instance hook. Reclamation
invalidates old handles; reuse increments generation; maximum generation retires
the slot rather than wrapping. Test all four independent values, early and
duplicate hooks, stale reuse and exhaustion. This explicit model is not ELF TLS
or automatic thread-local enforcement. The owner must schedule initialization,
access and destruction; ISO C does not enforce these lifetimes.

Keep host logging, selected semihosting and actual emulated UART as different
backends. Debugger-disabled startup must return permanent `UNSUPPORTED` before
issuing a trap. A temporarily unready service may become ready explicitly; only
that status authorizes a client retry. No host security settings or hardware
flashing are part of this package.

## Evidence audit and delayed assessment: T11.09

Complete `audit_image.py` as a host-side verifier or supply an equivalent C tool
with the same contract. The bundle binds exact binary and policy hashes; each
report includes exit status, complete text and its hash. Require symbols,
undefineds, dynamic tags, program headers, sections, disassembly and origin map.
Reject missing/truncated/NUL-containing reports, nonzero tool exit, lines over8192
bytes and reports over8MiB. Support the stated GNU bfd/lld map syntax explicitly;
unknown formats reject. Require a symbol table and actual program-header evidence.

Test four different negatives: an interpreter/dynamic dependency, an unresolved
helper, an already linked forbidden helper, and a recursive provider never run.
Also strip symbols, change the allowlist, alter a report or binary, and introduce
an inlined forbidden input under LTO. Correlate retained inputs/generated objects,
maps and final code. A trusted build/tool capture is an explicit assumption;
hashes establish identity, not authenticity or proof of arbitrary machine code.
Indirect calls and inlining require declared provenance/contract analysis. A
name-only symbol search cannot prove complete semantic dependency exclusion.

For each original leaf, retain prediction, derivation, implementation/trace,
independent expected result, rejection control and exact limitations. Level0
fails a contract or explanation; level1 reproduces an ordinary result; level2
explains the mechanism and discriminates the relevant fault; level3 independently
derives and validates the changed condition after the original minimum7 days.
Hints progress from identifying the boundary, to checking a concrete transition,
to constructing a small counterexample. Withheld analysis stays assessor-only.

Changed conditions are the original30 criterion variants, with concrete artifacts
above: callback substitution, raw-image composition, changed compilation/LTO,
header leak, actual alignment, runtime extent, duplicate provider, signedness,
Arm multi-register ABI, remainder/streaming extension, ISR rejection analysis,
instrumentation retention, local diagnostic runtime, offsets/separate objects,
changed compiler, raw service, arena, absent debugger, reset cuts, task instances,
retry/refusal, stripping, inlining, changed allowlist, metamorphic laws, partial
transport and the support matrix. None is a constant-only substitution.

Current status is author implementation with source-bound execution records in
`reviews/v5/expansion/legacy_systems_freestanding/`. Only a passing full seven-module
campaign and its independent rejection guard establish the declared author gate.
External technical review, actual learner attempts/delayed transfer, physical
measurements and published-standard source gaps remain separate dimensions.
