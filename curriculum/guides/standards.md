# Standards, dialects and measured support

Use explicit `-std=c17` for the portable baseline and keep C23 experiments in a separate build configuration. A compiler's default dialect can change. A GNU dialect permits extensions beyond the corresponding ISO mode. `-pedantic-errors` is useful diagnostic evidence, but acceptance is not a proof of conformance.

## Edition comparison

| Edition | Teaching comparison | Curriculum use |
|---|---|---|
| C89 / C90 | ANSI 1989 and ISO 1990 origins; prototypes, portable declarations and hosted/freestanding distinction | Read historical code; repair implicit assumptions and old declaration style |
| C95 | Amendment to C90, including wider character-library support and alternative tokens | Recognize legacy interfaces and internationalization limits |
| C99 | Mixed declarations, designated initializers, compound literals, `restrict`, integer-width facilities and revised inline rules | Explain which familiar facilities are later than C90 |
| C11 | Atomics, thread storage, alignment, generic selection and static assertions; some facilities are conditional | Separate language memory rules from runtime/library support |
| C17 | Technical corrections to the C11 line; ISO publication is 2018 | Default portable implementation contracts |
| C23 | ISO publication is 2024; type inference, `typeof`, `nullptr`, bit-precise integers, attributes and checked arithmetic among major teaching comparisons | Parallel feature probes and carefully scoped migration |

Publication identities and the committee's draft links are recorded by [WG14](https://www.open-std.org/jtc1/sc22/wg14/www/projects). Its N3220 link is labeled an early C2y draft and N2310 an early C23 draft. The inspected N3096 is a pre-publication C23 draft. Use its recorded locators with that label; do not cite it as an exact copy of ISO/IEC 9899:2024. Defect reports, proposed corrections and later working drafts can describe different wording.

## Compiler and library matrix

Checked 2026-10-04. These documentation entries are bounded support reports, separate from local experiments. [GCC status](https://gcc.gnu.org/projects/c-status.html) and [Clang status](https://clang.llvm.org/c_status.html) are live pages; revisit them when selecting an installed release.

| Facility | GCC documented introduction | Clang documented introduction | Runtime/library question |
|---|---|---|---|
| Explicit C17 mode | `-std=c17`; default GNU17 in GCC 8–14 | C17 mode from Clang 6; status page retains partial qualifications | Hosted libc and optional features are separate |
| Explicit C23 mode | C23/GNU23 modes; default GNU23 from GCC 15 | `-std=c23` from Clang 18; partial support | Do not equate a dialect flag with whole-edition support |
| `nullptr` | 13 | 17 | Pointer use is a compiler feature |
| `typeof` / `typeof_unqual` | 13 | 16 | Distinguish older extensions from ISO semantics |
| Object `auto` inference | 13 | 18 | Not C++ object deduction semantics |
| Object `constexpr` | 13 | 19 | Does not provide general C++ constexpr functions |
| `_BitInt` | 14, target-qualified | Check the selected target/release | Arithmetic lowering, ABI and maximum width need probes |
| `#embed` | 15 | 19 | Inputs and build dependencies must be explicit |
| `<stdckdint.h>` | 14 | Verify selected headers | Header implementation and overflow behavior need tests |
| `<stdbit.h>` | Verify compiler/libc pair | Verify compiler/libc pair | Availability cannot be inferred from language parsing |
| Non-native atomics | Target-dependent | Target-dependent | May require an atomic runtime; lock-free is a separate property |

GCC's floating-point qualifications and Clang's partial/unknown entries prohibit claiming full conformance from this table. C++ library availability says nothing about the C library selected by the C driver.

## Executed local probes

The recorded environment is WSL Ubuntu Linux x86-64, GCC 15.2.0 (Ubuntu 15.2.0-16ubuntu1), glibc 2.43. Seven exact snippets compiled with warnings, `-Werror` and `-pedantic-errors`, then exited successfully: a C17 width assertion; C23 `nullptr`; `auto` plus `typeof`; object `constexpr`; unsigned `_BitInt(17)`; `ckd_add`; and `stdc_count_ones`. Sources and results are preserved in `reviews/compiler_probes.json`.

This establishes only those snippets in that installation. Clang was not installed for execution, the embedded compiler was not exercised, and no complete standard/library test suite was run. Learners repeat feature probes for their environment before relying on a facility. Record language acceptance, header availability, successful linking and runtime behavior in separate columns. Include a negative or overflow case where relevant; a nominal call alone is a small capability probe.

## Claim classification

Label each reasoning statement as ISO guarantee, implementation-defined choice, unspecified outcome, undefined behavior, extension, ABI convention, libc behavior, POSIX/Linux contract or hardware constraint. Save the authority and configuration. For example, a C pointer does not gain a valid lifetime from an integer address that looks plausible; x86 behavior is not a portable object-model rule. A `volatile` access does not establish an ISO interthread happens-before edge. A device barrier is not a mutex.

Old-source porting is an investigation rather than a global C89 restriction. Keep the baseline modern and portable, compare edition-sensitive mechanisms in {ref}`T05`, and document deliberate extensions at the interface boundary.
