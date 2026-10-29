# Compiler extension milestones: PR-T60-01–08

This is required follow-through for `G-B06-COMPILER` and the T60 changed-condition
obligations, not a replacement for the accepted expression compiler. Stable
milestones CE-M1 through CE-M8 belong to the original parent projects below.
The [B06 common contract](../../../../curriculum/v5/contracts/b06.md) supplies the
six deliverables, C17 baseline, evidence states and level 0–3 assessment scale.
The supplied `extension.c` remains a learner rejection scaffold; the author
implementation and technical analysis stay under `assessor/subset_compiler/extension`.

## Prerequisites and execution profile

Retain the learner `cc_token`, `cc_parse`, `cc_validate`, `cc_run`, `cc_x86` and
`cc_thumb` artifacts and their rejection tests. Demonstrate left associativity,
unsigned arithmetic, pointer formation versus dereference, stack-frame lifetime,
and conditional control flow before replacing straight-line stack verification.
The translator is hosted C17 and may use byte copying, string comparison and
bounded formatting; no parser, compiler, bytecode engine or code-generation
library may implement the taught mechanism. Generated targets are freestanding
SysV x86-64, AAPCS32 Thumb-2/Cortex-M4 or Thumb-1/Cortex-M0. Python, GCC/Clang, CBMC if selected,
assembler, linker and QEMU are verification tools, not generated dependencies.

## Problem and exact language

Compile a nonterminated source span into `struct ce_program` using the public header.
Four earlier-defined, nonrecursive unary functions are permitted; the last is
`main`. `fn name(x) { ... }` is teaching notation, not an ISO C translation unit.
Each invocation owns an initially zeroed eight-element unsigned32 array `a`.
`let name = expression;` declares an unsigned32 local and `ptr name = &a[index];`
declares an array-derived pointer. Declarations occur only at the start of a
function body, initialize before publication, cannot shadow `x` or `a`, and
cannot repeat names. There are at most16 locals including `x`,512 instructions,
32 live operands and32 nested parser constructs. Identifiers have1–15 ASCII
letters/digits/underscores, beginning with a letter or underscore. Keywords are
reserved by their grammatical position; do not generate ambiguous keyword names.

Expressions include decimal or hexadecimal unsigned32 literals, `x`, locals,
`a[index]`, `&a[index]`, unary `*`, parentheses, earlier function calls, `*`, `+`,
`-`, `<`, and `==`. The precedence levels are unary/call, multiplication,
addition/subtraction, then comparisons. Operators within a level associate left;
the comparison level is deliberately one level, unlike ISO C. Arithmetic wraps
modulo2^32 for unsigned operands. Conditions are unsigned values with zero false. Array pointers are
represented as an index plus the implicit owning frame, permit formation of
index8 and reject dereference there. Only pointer-plus-unsigned is supported;
the mathematical sum must remain0..8 before addition. Pointer values cannot be
arguments, return values, integers, comparisons or cross-array references.
Statements are local assignment, array/pointer store, `if (...) {...} else {...}`,
`while (...) {...}`, and `return expression;`. Calls cannot modify caller locals
or its array. The built-in `tick(u32)` shares a per-entry unsigned counter across
calls: it returns operand plus the previous counter modulo2^32, then increments
the counter modulo2^32. `main` resets the counter to zero once. Expression operands
and call arguments evaluate left to right; this is an explicit source-language
sequencing rule, stronger than ISO C's unspecified ordering in many expressions.
Every reachable path must end in `return` or a loop; falling off a function is
rejected. A trailing return is required when structured branch patching otherwise
leaves a branch at the function end. Comments are `//` through newline and
`/* ... */`, with no nesting. Full malformed numeric tokens such as `0x12g` and
`4294967296` are rejected with their source spans, never accepted as a prefix.

`ce_compile` publishes the entire program only on success; diagnostic spans are
half-open byte offsets. Failed compilation may update diagnostics but leaves the
previous program unchanged. `ce_verify` structurally checks all instructions;
its exact type-stack analysis checks reachable joins. `UINT_MAX` heights identify
unreachable instructions, which are not emitted. Locals start zeroed in the IR
machine; source declarations still require initializers. `ce_run` validates first,
consumes one shared fuel unit per executed instruction across every call and
preserves the output on rejection. Fuel exhaustion is a bounded-execution policy,
not a theorem about source termination. `ce_emit` stages the entire assembly and
leaves the output buffer unchanged on capacity or validation failure.

The generated entry is `uint32_t cm_ext(uint32_t x, unsigned *fuel, unsigned
*status)`. The caller supplies distinct live pointers and initializes status0.
Success returns the value with status0; failure sets status1 and the return value
is unspecified. Compare the interpreter's success/output with this status/value
pair; failure has no published language result. This hidden ABI is a teaching
interface, distinct from a normal C function's error behavior.

## Required mechanisms and objective criteria

| Milestone / families | Parent objectives | Required evidence and acceptance |
|---|---|---|
| CE-M1 lexical/type boundary (A,B,C,D,I) | T60.01.a–c, T60.02.a–c, T60.03.a–c | Exact malformed-token spans; associativity counterexample `x-3-2`; distinguish explicit u32/i32 notation from complete C promotions and signed overflow; reject pointer misuse before execution. |
| CE-M2 joins and loops (B,C,D,F,G) | T60.04.a–c, T60.05.a–c | Annotate every instruction's stack/type effect; derive equal join states and bound analysis work; reject a pointer/u32 join even with equal height; demonstrate zero iterations, both branches, exhaustion and output preservation. |
| CE-M3 actual nested calls (B,D,F,H,I) | T60.06.a–c, T60.07.a–c | Assemble and execute nested calls on all three targets; retain live operands and `x` across a call; derive x86 16-byte and Thumb 8-byte public alignment; prove frame disjointness and maximum acyclic depth; inspect every undefined helper. |
| CE-M4 addressable array/lvalue semantics (A,B,C,E,F) | T60.03.a–c, T60.04.a–c, T60.08.a–c | Store through one pointer and load through an alias; form one-past without dereference; reject one-past read/write and overflowing offset before access; explain why implicit frame provenance cannot model escaping/general C pointers. |
| CE-M5 independent translation investigation (F,H,I,J) | T60.08.a–c plus each exercised backend objective | Independently derive all case outputs, compare interpreter/native/Thumb/C under equivalent defined behavior, minimize a semantic mismatch, kill operation/join/bounds mutants, audit code size and peak frame use. |

For every listed leaf `.a`, assess the mechanism derivation; for `.b`, the
implementation and independent boundary evidence; for `.c`, counterexample,
changed-assumption argument and evidence limits. A passing return-value table
without the relevant invariant earns at most level1. Level2 requires all listed
mechanisms and negative cases; level3 requires an unaided delayed variant at
least seven days later with the changed proof and independently constructed
oracle. Original learner artifacts, tool versions, source hashes, diagnostics,
IR, emitted assembly, maps and raw execution outputs form D1–D6.

## Verification and independent expected results

`cases.json` supplies original source programs with separately written ISO C
counterparts. The oracle runner additionally derives arithmetic with Python
integers reduced modulo2^32; none of these results is copied from the translator.
For `locals-calls-loop`, x2 returns69. `pointer-alias` returns `8*x+20` modulo2^32.
Nested calls return `2*x*x+x-1` modulo2^32. The fixed Fibonacci program returns34
for zero, otherwise21+x modulo2^32. Supply an additional zero-iteration loop
program; the fixed Fibonacci loop itself always iterates eight times.

Run the public C tests with your implementation, then the executable driver and
an independently constructed comparison harness. The author command
`python3 tools/run_b06_extension.py` opts into withheld reference code and labels
its report as author evidence. It executes three host profiles, three compiled
semantic mutants, learner rejection controls, x86 native output, Thumb QEMU
output, generated helper audits and20,000 seeded fuzz inputs. Parser fuzzing is
bounded exploration; provide a separate malformed-IR campaign before claiming
IR deserialization safety. No successful execution of undefined C is an oracle.

## Hints

1. Give each expression a type and each emitted instruction an explicit stack effect.
2. Use a worklist indexed by instruction and reject unequal type stacks at joins.
3. Store live operands in fixed frame slots; then nested calls need not preserve scratch registers.
4. Separate pointer formation, pointer addition and dereference checks. Reject before computing an invalid host address.

## Changed conditions and remaining boundaries

CE-M2-DELAYED changes the loop to an input-dependent bound and requires a fuel
cost derivation that includes called instructions. CE-M3-DELAYED changes the
calling convention to two arguments and requires a hostile caller plus both
backends. CE-M4-DELAYED adds two arrays, so pointer identity must name an allocation
rather than only an index. CE-M5-DELAYED changes comparison precedence to ISO C's
separate relational/equality levels and requires a distinguishing source example.
The shared `tick` effect makes call order observable while each caller array stays
private. Signed32, canonical IR and Thumb-1 now
have explicit implemented continuations below. General ISO C parsing, preprocessing,
recursion, allocation, escaping pointers and arbitrary external calls are excluded
from this stated subset and cannot be inferred from its tests.

## Canonical serialized IR: CE-M6

Implement `ce_pack` and `ce_unpack` without casting bytes to a C structure. The
eight-byte header is ASCII CEIR, version1, one-byte function count, LE16
instruction count. Each37-byte function record contains LE16 begin/end, one-byte
local count,16one-byte type entries and16name bytes. Names are nonempty ASCII
identifiers, NUL terminated and zero padded; unused types are zero. The final
function is main; names are unique. Each8-byte instruction is opcode, three zero
reserved bytes and LE32 argument. Only constants, local indices, branches and
calls have nonzero arguments. Extent must match exactly. Maximum size4252bytes.

D6 is the codec, byte-layout derivation, transactional rejection tests for every
truncation, unknown version/reserved bits, invalid CFG and inconsistent join, plus
decode/encode identity for accepted bytes. Fuzz both text and byte entrances.
CE-M6 assesses T60.05.a/b/c: level0 copies native structs; level1 round-trips only
trusted input; level2 rejects malformed extents and semantic IR while preserving
outputs; level3 independently derives canonicality and a verifier-bypass
counterexample under a changed opcode set. Hint1: separate byte decoding from
semantic verification; hint2: decode into a zeroed temporary; hint3: require
exact total length before reading variable records. The delayed variant adds a
versioned optional section whose unknown critical tags reject and unknown
noncritical tags have an explicit canonicalization policy. Byte fuzzing does not
prove all malformed IR rejected. Assessor codec remains outside learner files.

## Security and validation limits

The only executable target is generated original curriculum code with synthetic
inputs in a disposable process/emulator. Invalid language indices must produce a
status failure, not a host out-of-bounds access. Runtime status rejection, a
sanitizer report and arbitrary code execution are different claims. The repair
must preserve the check before address formation and retain the boundary mutant
as a regression. The current source-bound author report is
`research/v5/expansion/b06/extension_validation.json`; no learner, external-review,
physical or absolute-mastery claim follows from it.

## Signed arithmetic and Thumb-1: CE-M7 / CE-M8

`int n = expression;` declares an i32 local; literals and x remain u32.
`i32(e)` accepts only a representable unsigned value and otherwise rejects at
runtime; `u32(e)` converts signed values modulo2^32. A signed value can be built
as `i32(0)-i32(1)`. Both-signed arithmetic preserves i32 and rejects mathematical
overflow before publishing a result. Mixed i32/u32 arithmetic converts signed
operands to u32, corresponding to equal-rank unsigned conversion for these exact
widths; it is not a complete implementation of all ISO C integer ranks. Relational
results are u32. Compare `n<1` versus `n<i32(1)` when n=-1. Signed comparisons use
signed ordering. Return values and call arguments remain u32; explicit conversion
is required for signed returns. This checked language rejects signed overflow
where ISO C has undefined behavior, and rejects an unrepresentable unsigned-to-
signed cast where the inspected C11 draft specifies implementation-defined result
or signal. No C17/C23 exact-text equivalence is claimed from the draft alone.

D7 includes type/conversion derivations, minimum/maximum/cast boundaries, signed
add/subtract/multiply rejection, mixed comparison witnesses, and comparison with
separately defined C calculations only where C has a defined result. CE-M7 maps
T60.03.a/b/c and T60.08.a/b/c. Level2 requires output preservation and correct
backend rejection; level3 independently extends a signed operation or width and
supplies a new oracle. Hint: represent signed bits in uint32 storage but perform
interpreter arithmetic in a provably wide mathematical intermediate.

Target2 emits actual Thumb-1 instructions. Stack-relative loads replace unsupported
large-offset low-register forms; constants are assembled from bytes; conditional
long transfers invert a nearby short branch around a local literal island and BX.
The literal explicitly selects Thumb state. An original bounded unsigned-magnitude
multiply helper checks the signed limit before addition/doubling, avoiding hidden
compiler support calls. D8 is Cortex-M0 assembly/disassembly, no-undefined-helper
closure, hostile preserved-register evidence, status tests and code-size/frame
cost comparison with Thumb-2. CE-M8 maps T60.07.a/b/c and T60.08.a/b/c. Level2
requires Cortex-M0 execution, not Cortex-M4 acceptance; level3 changes branch
reach or frame layout and proves literal reach/public alignment again. Hint:
first inventory instructions/offsets unavailable in Thumb-1. Delayed signed/Thumb
variants require new independent expected results after seven days. The fixed
helper's bounded extra work must be added to machine cost, although language fuel
still charges one IR operation.

Inspected reference bindings: R-B06-FOLLOWUP-C11-TYPES, R-B06-FOLLOWUP-AAPCS32.
Exact passages and inspection limits are in
`research/v5/expansion/b06/followup_resources.json`; source titles alone are not
inspection evidence. Retain those locator bounds when changing assumptions.

## Observable call sequencing: CE-M9

For `fn bump(x){return tick(x);}` followed by
`fn main(x){return bump(x)-bump(x)+bump(x);}`, derive the result x+1 modulo2^32
from three ordered effects. D9 contains the effect trace, shared-state ownership,
repeated-main reset checks and interpreter/x86/Thumb-2/Thumb-1 results. The hidden
fourth internal argument points to a main-owned counter; its lifetime encloses
all acyclic calls. Do not reset it in each callee or keep a process-global counter.
CE-M9 explicitly assesses T60.02.a/b/c and T60.06.a/b/c: level0 infers sequencing
from precedence; level1 matches one result; level2 derives effect order, shared
ownership and reset and rejects a reversed-order counterexample; level3 supplies
an unaided delayed nested-call case with an independent event trace. Hints: write
the counter value before each call; then distinguish effect lifetime from each
function's private array. The delayed variant changes the sequencing policy to
reject two effects in one expression and requires a diagnostic and an explicit
sequenced rewrite. This exercise does not claim ISO C evaluates that original
expression in a specified order.
