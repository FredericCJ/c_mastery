# T12.01–02: declarations, types and runtime closure

Required PR-T12-01 criteria T12.01a/b/c and T12.02a/b/c, families A/B/C/D/F/I/J.
Keep their original delayed IDs and minimum seven days. Prerequisites are C17
translation units, function-pointer types, integer promotions, object lifetime,
linker relocations and the existing freestanding runtime contract. Use the
supplied headers.h, unfinished headers.c, headers_tests.c and source_matrix.json.
Assessor implementations and completed probe notebooks are withheld.

A header supplies declarations, types or macros; it is neither a runtime object
nor a guarantee that a function definition will be linked. A compiler option
selecting C17 does not reinstall a C17-era libc. A header supplied in an older
mode may be an extension. Moreover the C library spans many headers: stdlib.h
does not declare every library facility. Inspect the bounded primary passages
R-LLIB-N1570-HEADERS, R-LLIB-N1256-HEADERS, R-LLIB-N3096-HEADERS and
R-LLIB-N1570-TYPES before interpreting observations.

Create six independent translation units, each taking the address of exactly one
of memcpy, strtol, snprintf, clock, signal and qsort through its owning header.
Use a correctly typed global function pointer so the definition remains needed
even without a call. Compile with warnings as errors and inspect each undefined
symbol. Repeat in freestanding compilation mode, then attempt a no-runtime link.
State which headers your selected implementation supplies and which definitions
remain absent. A linker symbol may differ from the C API name: preserve the
preprocessed declaration and dependency identities when a libc redirects it.
Do not replace a missing declaration with an invented prototype. A separate
negative TU including only stdlib.h must diagnose the undeclared memcpy.

For all six headers stdbool.h, stdint.h, stdatomic.h, threads.h, stdckdint.h and
stdbit.h, construct all thirty C90/C99/C11/C17/C23 cells. Keep two distinct
columns: source entitlement and observed compiler/header acceptance. The supplied
matrix identifies directly inspected C99 TC3, C11 and C23 draft lists. C90 and
published C17 exact text remain gaps; an encrypted retrieved committee draft was
not read. A successful include cannot fill those source gaps. Check definitions,
conditional implementation macros and link capability separately. In the C23
draft stdbool.h remains a header but its role changes when bool becomes a
language keyword. Do not infer definitions merely from the header's continued
existence. No draft is relabeled as a published ISO edition.

Implement lh_c11_threads as the declared backend selector: require a supplied
header, an edition value at least201112L, no advertised absence and a passing
link probe. This decision is a necessary admission policy for this experiment,
not a complete C conformance test. Exercise all sixteen Boolean combinations
including a supplied older-mode header and a no-threads advertisement. Select
the explicit serialized fallback otherwise. That fallback has one caller and
no interleaved callbacks; it does not emulate parallel progress, atomics or
thread cancellation. A different platform backend needs its own contract.

Classify snprintf as ISO since C99, aligned_alloc as ISO since C11, memcpy_s
as optional Annex K, getline/strtok_r as POSIX, and the GNU unused attribute as
a compiler extension. Distinguish the inspected draft basis from the C17
baseline's exact-text gap. Probe declarations and links independently. Requesting
Annex K with __STDC_WANT_LIB_EXT1__ cannot force a library to supply it. A
core-only parser must build without these optional or extension dependencies;
accepting an attribute in a strict compiler mode does not make it ISO syntax.

Complete lh_size: accept a stable counted ASCII span with grammar [0-9]+,
including leading zeros, and publish size_t only after a uintmax_t accumulator
has passed both its own arithmetic bound and SIZE_MAX. Reject signs, spaces,
embedded NUL and trailing bytes. A null span is valid only for zero extent,
which is a syntax failure. Missing output is an argument failure. Every failure
preserves the prior output; valid spans and output are disjoint live objects.
Validate all characters before range accumulation so syntax has defined priority
over range. The implementation allocates nothing, retains no pointer, takes
O(n) time and constant space, and uses no library parser or I/O mechanism.
Explain why comparing a value after a narrowing cast loses the evidence needed
to reject it. Derive quotient/remainder thresholds without overflowing them.

Build a ledger for size_t, ptrdiff_t, optional int32_t/uint32_t, uintmax_t,
CHAR_BIT, SIZE_MAX and PTRDIFF_MAX from their owning headers. sizeof counts C
bytes, whose bit width is CHAR_BIT. Exact-width typedefs are conditional; their
underlying native types are not portable. PRI and SCN macros serve different
argument contracts. A printf integer undergoes promotions; a scanf destination
pointer does not. Use %zu for size_t and %td for ptrdiff_t, and match PRIu32 and
SCNu32 to actual uint32_t objects. Limit numeric scanf experiments to known
representable tokens; it is not the untrusted range-checking path.

Format UINT32_MAX and SIZE_MAX, parse and compare their full round trips, then
reject SIZE_MAX followed by0 without changing output. Run on native LP64 and
the explicit private i386 libc environment documented in
labs/posix_lifecycle_depth/io/TOOLCHAIN.md. The latter executes the decisive
UINTMAX-width-greater-than-size_t case, including SIZE_MAX+1. Separately compile
against the AVR int16 frontend and avr-libc headers: verify uint32_t is unsigned
long, check its PRI format, and require a %u mismatch diagnostic. No AVR runtime
execution is claimed. An explicit cast to unsigned can make a format match while
truncating the value; a clean diagnostic log is insufficient evidence of value
preservation. Do not use an invented typedef to claim a real ABI observation.

Compile a constants-only object using sizeof, offsetof and integer limits, and
another retaining malloc, snprintf and free. Inspect relocations and final maps
with builtins disabled. Then enable cross-translation-unit LTO so an unreachable
library path disappears. Explain the changed reachable program, not a newly
available freestanding malloc. Link the original lh_size with an original entry
under -ffreestanding -nostdlib, inspect the actual final image for every undefined
symbol and compiler helper, and retain disassembly/map. This image is a closure
experiment, not a booted operating system or a hardware observation.

Deliver the source/observation matrix, typed TUs, parser, configure decisions,
range derivation, object/link audits, dependency hashes and counterexamples.
Pass requires each named observation, not only the parser's ordinary tests.
Run GCC/Clang O0/O2 and hosted sanitizers; reject compiled zero-rejection,
header-only capability and premature-narrowing mutants for their intended
assertions. A compiler crash or an unrelated compile failure is not success.

Rubric:0 confuses declaration, type or runtime;1 reproduces native output;
2 derives all contracts and discriminates the incorrect explanations;
3 independently reconstructs the original changed-ABI/backend/LTO variants
after the required delay. Hints: inspect the undefined-symbol list; compare
before casting; separate header presence from edition and implementation
capability. All inputs and artifacts are local and synthetic. Author execution
does not supply independent learner, external review or physical evidence.
