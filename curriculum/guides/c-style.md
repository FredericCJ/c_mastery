# The C Mastery house style

Every C file in `labs/` and `assessor/` follows one house style. It is built from
four published sources that real teams use: the Barr Group *Embedded C Coding
Standard* (BARR-C:2018), the SEI CERT C Coding Standard, MISRA C:2023 and the
Linux kernel coding style. They disagree with each other in places, and those
disagreements are worth learning: a team adopting a standard has to make the
same decisions and write down why. This guide records our decisions.

The purpose is readable code and a working introduction to industry standards.
Following this guide is **not** a claim of compliance with MISRA C, CERT C or
BARR-C. MISRA compliance in particular needs a documented compliance process
(MISRA Compliance:2020 sections 4–5), which this curriculum does not run.

Sources and exact locators are listed in
`research/v5/expansion/polish_revision/style_sources.json`. Citations below use
the form *BARR-C 1.3.a*, *CERT ERR34-C*, *MISRA Rule 15.6* (C:2023) and
*kernel §3* (section 3 of `Documentation/process/coding-style.rst`, Linux v7.0).
MISRA rule numbers are cited for orientation; the guideline text itself is
licensed and is not reproduced here.

## Three tiers

| Tier | Applies to | Enforced by |
| --- | --- | --- |
| L — layout | all C | pinned `clang-format` 21.1.8 with `.clang-format` |
| N — naming and structure | all C | `tools/check_c_style.py` and `clang-tidy` (`.clang-tidy`) |
| C — correctness by context | where the context needs it | compiler warnings, sanitizers, `clang-tidy`, review |

Tier C exists because a safety rule is only meaningful in its context. MISRA's
ban on dynamic allocation after initialization (Rule 21.3) is right for firmware
that must never fail an allocation in the field; it would make a hosted
allocator laboratory impossible. Each tier-C rule below names its context.

## Tier L: layout

Run `build/author-tools/python/bin/clang-format -i FILE` (or `--dry-run` to see
what would change). Use only the pinned version: another clang-format release
can lay the same code out differently.

**Width, indentation, line endings.** Lines are at most 80 columns
(BARR-C 1.2.a). Indent by four spaces per level, never with tabs (BARR-C 3.4.a,
3.5.a). Lines end with LF only (BARR-C 3.6.a).

**Braces.** Every body of `if`, `else`, `for`, `while`, `do` and `switch` is a
braced block, even a single statement (BARR-C 1.3.a, MISRA Rule 15.6,
CERT EXP19-C). Braces go on their own lines (BARR-C 1.3.b):

```c
if (ring == NULL)
{
    return PF_RING_INVALID;
}
```

*Why mandatory braces:* without them, adding a second statement to a body that
looks indented silently places it outside the condition. clang-format inserts
missing braces automatically (`InsertBraces`).

**One statement per line** (BARR-C 3.3.a). A line such as
`a=x;b=y;if(a)return b;` hides control flow; it becomes three lines and a block.

**Spaces.** One space after `if`, `while`, `for`, `switch` and `return`, and
around binary and assignment operators; none between a unary operator and its
operand (BARR-C 3.1.a–d). The `*` of a pointer declarator binds to the name,
`uint8_t *p_buffer`, because `int *a, b;` declares only one pointer.

**Not adopted from BARR-C layout.** Aligning declaration names in columns
(BARR-C 3.2.a) and an end-of-file comment (3.3.c) are not used: column alignment
churns on every edit, and version control makes the end-of-file marker
redundant.

## Tier N: naming and structure

**Case.** Functions, variables, parameters, members and type tags are lowercase
with underscores (BARR-C 6.1.e, 6.1.g, 7.1.f, 7.1.h; kernel §4 frowns on mixed
case). Macros and enumeration constants are uppercase (BARR-C 6.1.f).

**Module prefixes.** Every public function, type and macro carries its module's
prefix: `pf_pmu_open`, `struct pf_pmu`, `PF_PMU_READY` (BARR-C 5.1.c, 6.1.i).
A file-local helper may drop the prefix if it is `static`.

**Types: tags, not typedefs.** Structures, unions and enumerations are used
through their tags:

```c
struct pf_ring_stats
{
    uint64_t samples;
    uint64_t lost;
};

void pf_report(const struct pf_ring_stats *stats);
```

This is the house rule where the sources conflict most directly. BARR-C 5.1.a
and 5.1.b require a typedef ending in `_t` for every new type. The Linux kernel
calls a typedef for a structure or pointer a mistake (kernel §5), and POSIX
reserves every identifier ending in `_t` for the implementation, which
CERT DCL37-C (do not declare reserved identifiers) and MISRA Rules 21.1/21.2
make relevant for hosted code. We keep the tag. You always see `struct` where an
aggregate is passed, copied or compared, and no `_t` name can collide with a
POSIX header.

Typedefs remain for things that are not aggregates: function-pointer types,
which end in `_fn` (`typedef int (*pf_sample_fn)(void *context, const struct
pf_sample *sample);`), and the few scalar, array or vector types a lesson is
about. Those use lowercase prefixed names without `_t`.

**No Hungarian prefixes.** BARR-C 7.1.j–m prefixes globals with `g_`, pointers
with `p_` and booleans with `b_`. Kernel §4 rejects encoding a type in a name,
and our compilers already know every type. We do not use these prefixes. Short
loop counters such as `i` are allowed, against BARR-C 7.1.e's three-character
minimum, when the loop is short and the counter has no other meaning.

**Units and ownership in names and comments.** A quantity carries its unit:
`deadline_ns`, `capacity_bytes`, `period_cycles`. A public function's comment
states its contract: preconditions, who owns each pointer before and after the
call, what is returned on each failure, and whether the function blocks
(BARR-C 2.2.g asks that assumptions be written down; CERT API00-C that
parameters be validated on one side of each interface).

**Comments explain reasoning.** Comment the invariant, the constraint or the
reason a simpler-looking alternative is wrong (kernel §8: say *what* and *why*,
not *how*). Never leave commented-out code (BARR-C 2.1.c); use `#if 0` while
investigating and delete it before review.

**Visibility and immutability.** File-local functions and objects are `static`
(BARR-C 1.8.a and 6.2.e, MISRA Rule 8.7). Declare variables in the smallest
scope that works and where they are first needed (CERT DCL19-C, MISRA Rule 8.9,
BARR-C 7.2.b). Use `const` for anything a function does not modify (BARR-C
1.8.b, CERT DCL00-C, MISRA Rule 8.13). One declarator per declaration
(BARR-C 8.1.a): `int count; int limit;`, not `int count, limit;`.

**Functions stay short.** Aim for one screen. BARR-C 6.2.a caps a function at
100 lines; kernel §6 asks for one or two 80×24 screens and at most five to ten
locals. `clang-tidy` reports functions over 80 lines or with high cognitive
complexity; treat the report as a reason to look again, not as a verdict.

**Control flow you can follow.** Every `switch` has a `default`
(BARR-C 8.3.b, MISRA Rule 16.4) and every intentional fall-through is
commented (BARR-C 8.3.c). An `if … else if` chain ends in `else` when the cases
are meant to be exhaustive (BARR-C 8.2.d, MISRA Rule 15.7). No assignment inside
an `if` condition (BARR-C 8.2.c). Infinite loops are written `for (;;)`
(BARR-C 8.4.c).

**Early return and `goto` cleanup.** BARR-C 6.2.c prefers one exit at the
bottom of a function and MISRA Rule 15.5 (advisory) asks for a single exit;
BARR-C 1.7.c and MISRA Rule 15.1 (advisory) discourage `goto`. Kernel §7 uses
a forward `goto` to a cleanup label as the clearest way to release resources
acquired in sequence. Our rule: return early on validation failures before any
resource is acquired, and once resources are held, use forward jumps to cleanup
labels named after what they undo (`err_unmap:`, `err_close:`). This satisfies
the required MISRA Rules 15.2 and 15.3 (forward jumps within the same or an
enclosing block) while deviating from the advisory 15.1 and 15.5, and the
reasoning goes in the function's comment.

**Macros.** Prefer a `static inline` function to a function-like macro
(BARR-C 6.3.a, MISRA Dir 4.9, kernel §12). When a macro is necessary,
parenthesize its parameters and body (BARR-C 6.3.b, MISRA Rule 20.7) and wrap
multi-statement bodies in `do { … } while (0)` (CERT PRE10-C, kernel §12).
Headers have include guards (BARR-C 4.2.b, MISRA Dir 4.10).

**Not adopted.** Constants on the left of `==` (BARR-C 8.6.a) are not used:
compiler warnings (`-Wparentheses`) and the `bugprone-assignment-in-if-condition`
check catch the accidental assignment they guard against, and readers find
`if (count == 0)` easier to read.

## Tier C: correctness by context

**Everywhere: checked conversions and checked errors.** Never convert text to
a number with `atoi` or an unchecked `strtoul` (CERT ERR34-C; MISRA Rule 21.7
bans the `ato*` family). Use the course parser in `labs/support/cm_parse.h`,
which rejects empty input, signs, whitespace and trailing text, and checks the
range in the wide type before narrowing (CERT INT31-C). Check and handle the
result of every library and system call that can fail (CERT ERR33-C, POS54-C,
MISRA Rule 17.7).

**Integer arithmetic on untrusted or size values** checks for wrap and overflow
before it happens (CERT INT30-C, INT32-C). MISRA's essential type model
(Rules 10.1–10.8) is the systematic version of this discipline; we teach it in
the representation domains and enable `-Wconversion -Wsign-conversion` in the
freestanding/firmware profile.

**Freestanding and firmware code** (Cortex-M, the educational kernels) also
follows these MISRA rules: no dynamic allocation after initialization
(Rule 21.3), no recursion (Rule 17.2), no `<stdio.h>` I/O (Rule 21.6), fixed
width integer types where width matters (BARR-C 5.2.a, MISRA Dir 4.6), and
`volatile` for memory shared with interrupts and devices (BARR-C 1.8.c).

**Hosted laboratories** may use `malloc`, recursion and `stdio` where the
lesson needs them, but they still release every resource they acquire
(CERT MEM30-C, FIO42-C, MISRA Rule 22.1) and validate external input
(MISRA Dir 4.14).

## Exceptions and deviations

A rule is broken only with a recorded reason, in the spirit of MISRA Compliance
deviation records (MISRA Compliance:2020 §4.2):

- **Deliberate specimens.** A file that teaches a defect carries a
  `CM-SPECIMEN: <defect class>` comment and an entry in
  `tools/c_style_manifest.json`. Its layout follows the house style; its defect
  stays.
- **Layout-dependent lessons.** A region whose layout is the lesson is wrapped
  in `/* clang-format off */ … /* clang-format on */` with a `CM-STYLE:`
  justification on the same line.
- **Linux kernel modules.** The six files built with kbuild as loadable kernel
  modules follow the Linux kernel coding style (tabs, K&R braces) because that
  is the standard their real-world counterparts are reviewed against. They are
  listed under `kernel_style` in the manifest.
- **Externally named identifiers.** Names dictated by another API, such as
  FreeRTOS `configUSE_*` macros or STM32 HAL `HAL_*_MODULE_ENABLED`, keep their
  upstream spelling (`naming_exemptions` in the manifest).

## Checking your work

```sh
build/author-tools/python/bin/clang-format --dry-run --Werror FILE.c
python3 tools/check_c_style.py --family <family>
```

`check_c_style.py` checks layout and the naming rules that need no compiler.
`clang-tidy` (with a compile database) adds identifier case, declaration
isolation, function size, macro parentheses, `switch` defaults, reserved
identifiers and CERT ERR33-C/ERR34-C. None of these tools judges whether a
comment is true, an invariant holds or an ownership contract is honoured: that
remains review work.
