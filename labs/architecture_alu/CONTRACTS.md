# Gate slices, carry conventions and real branch observations: PR-T37-01

This required successor completes the original three `T37.02.a/b/c` criteria
and their unchanged seven-day `AX-PR-T37-01-T37.02.*-V1` tasks. Families
A/B/C/D/F/H/I/J apply. Preserve accepted baseline identities, original fixture
and objective rubrics. Historical hours are descriptive. The architectural
state/commit objectives owned by PR-T37-03 remain separate required work.

## Prerequisites, environment and supplied artifacts

Use the learner's completed Boolean operations, integer-promotion reasoning,
four-bit `algo_alu4` and selected C/assembly interface. Import the actual prior
function for ordinary addition comparison; do not count its existence as the
missing subtraction/admission implementation. The author comparison extracts
that exact unchanged function body with pinned provenance. Learners may import
their own reviewed producer. The public placeholder is link-only, not evidence
of a completed prerequisite. No library implements the assessed arithmetic.

The active C17 Profile C component uses live typed caller output, unsigned
operations and selected32-bit `unsigned`/`uint32_t`. Python integers, hosted
printing and sanitizers are host verification dependencies. Linux x86-64 uses
the System V convention; dedicated Cortex-M4 QEMU uses AAPCS32, original reset
and explicit byte helpers only. Audit objects, generated instructions and final
ELF closure, including implicit compiler helpers. C17 source semantics, ISA
flags, ABI register conventions and emulator behavior are distinct claims.
N1570 is an inspected C11 draft; final C17/C23 text remains a source gap.

## Problem and exact operation contract

Implement `aa_slice` from Boolean gates. Inputs a,b,carry are each0/1. Output
contains sum and carry; invalid bits or null output reject before publication.
Derive all eight rows before running them. Implement `aa_alu` for width4 or5,
operation0 addition or1 subtraction, operands0..2^width−1, and extra input0/1.
For addition, extra means carry-in: a+b+extra. For subtraction, it means
borrow-in: a−b−extra. Ordinary tasks use width4 and extra0. Reject every other
width/operation/input/operand before shifting or writing output.

Return modular value, Z, N, Arm C, x86 CF, signed overflow V and every internal
bit/carry connection. Addition uses the same carry convention for both ISAs;
subtraction exposes Arm no-borrow and x86 borrow separately. Unused trace entries
are zero. Publish the complete result only after validation and construction;
rejection preserves the entire previous output. All outputs are caller-owned
live objects; there is no allocation, callback, I/O, retained pointer, partial
commit or concurrent access. `aa_relations` accepts a valid subtraction result
from this component with borrow-in0; it returns unsigned less/greater, signed
less/greater and equal bits. It is not an arbitrary corrupted-flag validator.

The four retained examples are7+1,15+1,8−1,0−1. Explain why Z and N cannot both
be1 here, and why the modular zero has no separate negative-zero representation.
Invalid operation must not be treated as addition merely because one control
bit happens to match. The deliberately swapped carry/overflow implementation
must fail a flag assertion, not be accepted because its result bits still match.

## Reasoning and meaningful transfer

For slice i prove `x+y+carry[i] = bit[i] + 2*carry[i+1]`; compose the weighted
equations. Subtraction complements each selected b bit and starts with1−borrow.
Track carry into and out of the sign slice and derive overflow independently
from widened signed range. Keep gate construction separate from the arithmetic
oracle. Complexity is O(width) slice operations and fixed trace storage; counting
gates in this C model does not measure transistor delay, power or CPU cycles.

`AX-PR-T37-01-T37.02.a-V1`: generalize to width5. Derive the new signed/unsigned
ranges, operand-pair count and all promoted shift bounds before compiling. The
mask expression shifts unsigned1 by at most5; slice positions are0..4. The
selected ISA comparison adapter shifts a `uint32_t` operand by27 or28. Explain
why unsigned type and an admitted shift count are both necessary; a hardware
shift instruction's behavior does not repair undefined source C.

`AX-PR-T37-01-T37.02.b-V1`: derive carry-in/borrow-in semantics and count formulas
before changing the initial wire. If N=2^width, ordinary carry/borrow count is
N(N−1)/2; extra1 changes it to N(N+1)/2. Signed overflow count is N²/4 in both
cases. Derive these from independent pair ranges; a repeated run of the slice
model is not an independent oracle. At width4 the counts are120/136 and64;
at width5 they are496/528 and256. Verify all internal connections too.

`AX-PR-T37-01-T37.02.c-V1`: compare signed/unsigned branch decisions after the
same subtraction using the newly inspected encoding passages. For four-bit
8−1, N0/V1 makes signed less true although unsigned greater is true. For0−1,
N1/V0 makes signed less true and unsigned less true. Neither result permits an
overflowing signed-C expression. Map high-aligned operands `a<<(32-width)` and
`b<<(32-width)` to an actual32-bit CMP so signed and unsigned finite ordering
both correspond; simply zero-extending the original small negative bit pattern
would give a different signed comparison.

Execute the original x86 and Thumb comparison primitives, capturing native
carry/borrow, overflow, negative and five branch outcomes for all1,280 pairs.
Compare native x86 CF against borrow and emulated Arm C against no-borrow.
The assembly executes actual conditional branches, not C predicates merely
labeled as ISA output. Inspect disassembly and emitted branch bytes. The five
x86 short forms use the inspected JAE/JBE/JGE/JLE/JNE conditions; the selected
Thumb T1 forms use CS/LS/GE/LE/NE, no IT block. Their immediate bases/units are
different. Decode only these emitted forms; unsupported instruction families
remain outside this packet, not silently accepted as a complete decoder.

## Source discrepancy to diagnose

The actual DUI0553B Table3-4, printed3-20, displays LE with an AND between its
two tests. DDI0403E.e A7.3.1, printed179, derives LE as the inverse of GT:
`Z || (N != V)`. Use that authoritative architecture definition and independently
check equality, where the printed AND fails. This successor records the bounded
conflict without rewriting historical inspection evidence. Do not generalize it
into a claim that the entire manual was reviewed or is unusable.

## Deliverables, verification and rubric

- D1: truth table, gate derivation, carry-chain invariant, typed shift proof,
  exact source locators and model/ISA/ABI/physical evidence classification.
- D2: admitted transactional ALU, trace outputs, actual prior-function comparison,
  native/Thumb branch primitive, assembly/disassembly and final-link audit.
- D3: all5,120 model packets, internal connection checks, independent range/count
  oracle,1,280 actual ISA packets per native profile, target observations and
  smallest counterexamples for four intended semantic mutants.
- D4: all three original changed tasks with fresh seven-day transfer reasoning,
  source-table conflict explanation and limits of finite execution evidence.

The four public cases are truth, exhaustive, admission and branches. The author
campaign uses GCC/Clang debug, optimized and ASan/UBSan profiles, two strict
Cortex-M4 emulator builds, raw short-branch encoding checks and learner-stub
rejection. Assembly itself is not sanitizer-instrumented. A compiler error,
timeout or unrelated failure is not a passing mutation control. All emitted
observations and selected binary sources must match the receipt's hashes.

Assess .02a from independently derived gates and width5 shift reasoning; .02b
from every output/connection/count, invalid-output preservation and extra-input
transfer; .02c from carry polarity, signed/unsigned decisions, exact inspected
encodings and the source-language boundary. Level0 has an invalid invariant or
undefined test; level1 reproduces nominal values; level2 derives and verifies
the ordinary contract; level3 independently completes the named changed task
and falsifies a plausible overclaim. A shared test earns multiple objective
credits only when each corresponding argument is assessed explicitly.

Hints: first total the three one-bit inputs; next telescope the weighted slice
equations; then try7+1 versus15+1 before equating carry with overflow. For borrow,
derive the complement identity before choosing the initial wire. For signed
branches use the correction from V, then test equal operands against the disputed
LE table. Solutions and technical analyses remain separate from learner exports.

## Security and validation limits

All targets are original local synthetic programs or dedicated emulator images.
No external service, physical firmware flashing or host security change occurs.
Harmless success conditions are a controlled wrong flag/branch or unchanged
rejected output. They are not arbitrary-code-execution demonstrations. Invalid
C signed overflow is analyzed rather than executed as an expected numeric test.

The source-bound successor supplies author execution only. External review,
independent learner performance, elapsed delay and physical timing/energy remain
unperformed. Exhaustion covers the declared finite inputs, not all toolchains,
ISAs, hidden hardware state or future component changes. Remaining T37 projects
remain required; this ALU does not prove a pipeline/cache/MCU model accurate.
