# B06 common exercise and evidence contract

This contract applies to every PR-T57–PR-T64 project and its LAB unit. Each
project supplies the mechanism-specific inputs, API, invariant, boundary and
changed-condition assessment; these obligations supplement its public tests.

## Prerequisites and permitted dependencies

Use the accepted C17 implementation baseline. C11 draft clauses supply precisely
identified semantic reasoning; a draft is not a substitute claim that published
C17 was inspected. C23 comparison work must identify the actual changed clause
and compiler support; unchanged ABI or OS behavior is not an ISO C guarantee.
Before entry demonstrate checked extent arithmetic, object lifetime, byte
decoding, resource ownership and reproducible build capture with the declared
prerequisite artifacts. Reading a module does not accept an artifact.

The supplied portable cores use only C17 types and explicitly declared host
math operations. Implement the mechanism from these primitives: no allocator,
compiler, container, verification engine or protocol library may replace the
mechanism being taught. Host-side Python/CMake/CTest are verification tools.
Kernel headers, system calls and ISA assembly belong to separately named
adapters. A hosted state model does not demonstrate kernel scheduling, device
ordering, cache coherency or flash persistence. For each freestanding adapter
compile with -ffreestanding, inspect relocation and undefined-symbol tables,
linker maps and disassembly for compiler helpers; report every dependency and
provide or remove it. Merely disabling builtins does not prove runtime closure.

## Execution and containment

Run host tests in a disposable working directory using synthetic fixtures.
Kernel modules run only in a disposable VM with a snapshot, no host shared
directories, no bridged networking and no production data; do not insert them
into the workstation kernel. Firmware work uses the supplied model/emulator
first. Physical transfer is restricted to dedicated NUCLEO-F439ZI or
NUCLEO-H745ZI-Q development boards and requires a separate authorized session;
this authoring task does not flash devices or change security settings.
Security cases target only the original teaching arena/dispatcher. The attacker
controls the explicitly supplied bytes, handles or event schedule, not real
credentials. Harmless success is the named sentinel/property transition.
Crash, undefined behavior and sanitizer detection are different observations;
none alone demonstrates code execution or privilege escalation.

## Deliverables and verification

Retain (D1) a prediction before execution, pre/postconditions, representation,
invariants, ownership transitions, failure atomicity and cost derivation;
(D2) learner source and build identity; (D3) complete commands, raw outputs,
tool versions and independently derived expected results; (D4) a minimized
counterexample, repair and regression; (D5) a changed-condition implementation
and comparison; (D6) an evidence-limit statement. Use supplied fixture data,
then independently construct at least one input that distinguishes your repair
from the named plausible wrong implementation. Public finite checks cannot
establish absence of all defects. Sanitizers observe executions; bounded proof
depends on assumptions/unwinding; measurements depend on workload and machine.

## Assessment rubric and hints

Every objective has its own criterion in the project and authored unit. Score
0 for no assessable artifact or a contradiction of its contract; 1 for a
passing demonstration without a causal explanation; 2 for an original
implementation/analysis with a correct invariant, boundary argument and
reproducible evidence; 3 for an unaided delayed changed-condition solution
that identifies which previous proof or measurements no longer apply.
Require all objectives at level2 before technical completion and level3 on the
specified transfer task after at least seven days before independent-transfer
completion. Record whether the attempt was self-reported or externally
reviewed. No fabricated reviewer independence, mastery certificate or absolute
mastery claim is permitted. Hints are graduated in each brief; consulting the
assessor solution changes the attempt to assisted practice.

## Validation states

Canonical writing means authored; a compiling scaffold means executable; a
source-bound observed run means executed. These do not imply learner acceptance,
external assessment, VM execution or physical validation. Each family records
remaining target evidence and each campaign reports exact commands. A model
pass cannot satisfy a missing mandatory adapter investigation. New descriptive
hours are separate from accepted baseline hours and impose no learner-time cap.


## 1: Fixed-point scale, rounding and saturation

Implement ns_q15(a,b) for signed Q1.15 operands: multiply exactly in int32_t, round the scaled result to nearest with ties away from zero, then saturate into [-32768,32767]. In particular -32768 times -32768 saturates to32767. Derive the range of every intermediate and distinguish quantization error from saturation.

Derive the product scale and prove int32_t multiplication and negation bounds before implementing absolute value. Separate saturation policy from the rounding rule.

## 2: Exact rounding model and ties to even

Implement ns_round_even(numerator,denominator,out) for numerator in[-1000000,1000000] and denominator in[1,1000000]. Return the nearest integer, breaking exact ties toward an even integer; leave out unchanged on invalid input. Use integer quotient/remainder, with no floating conversion or rounding library.

Derive nearest-neighbor intervals and parity at exact ties. Prove remainder doubling is avoided by comparing r with denominator-r, and explain the negative sign symmetry.

## 3: Exact accumulation bounds

Implement ns_dot(a,b,n,out) for int16_t arrays and0<=n<=65535. Accumulate in int64_t and publish the exact sum; n==0 returns0 without reading either array. Reject n>65535 without changing out. No vector or BLAS library is permitted.

Derive the maximum positive and negative product and n-times-product bound. Explain which intermediate width is required before multiplication and before addition.

## 4: Compensated reduction and cancellation

Implement ns_sum(a,n,out) using Neumaier compensated summation in the supplied order. Accept n<=65535 and finite inputs; reject any nonfinite intermediate or final result without modifying out. Empty input returns0. Assume binary64 and ordinary non-reassociated operations, and record these assumptions explicitly.

Derive the correction for each magnitude ordering and identify every rounding point. Compare naive, compensated and exact-rational reference results without claiming a universal relative-error bound near a zero sum.

## 5: Scaled norm and intermediate overflow

Implement ns_norm(a,n,out) for at most65535 finite doubles using scaled sum of squares: retain scale and sumsq so intermediate squares are of ratios<=1. Empty/all-zero vectors return0. Reject nonfinite input or unrepresentable final norm without publishing output. fabs and sqrt are permitted primitives.

State the scale/sumsq invariant in real arithmetic and discuss floating error separately. Prove why the ratio is bounded and identify final overflow as distinct from avoidable intermediate overflow.

## 6: Matrix layout, bounds and transactional output

Implement ns_matmul(A,B,rows,inner,cols,C) for row-major int32_t arrays with each dimension1..8 and every input element in[-1024,1024]. Produce int64_t C. Reject invalid dimensions or elements before any C write; all arrays are live and disjoint. No matrix library is permitted.

Derive the three indexing expressions and accumulator bound; prove rejection is transactional. Count arithmetic and memory traffic under the stated row-major traversal before proposing a cache model.

## 7: Saturating lanes and scalar tails

Implement ns_add(a,b,n,out) saturating int16_t sums for n<=65535 with disjoint arrays. Widen each addition, clamp then narrow; handle every element including tails. Empty input succeeds without dereferencing arrays, invalid length fails before writes. Implement an explicit-vector adapter separately and compare it against this scalar oracle.

Specify lane width, widening, alignment and disjointness. Prove the vector body and scalar tail partition [0,n), and distinguish GCC vector operators from ISO C scalar promotions.

## 8: Accuracy admission and exceptional values

Implement ns_close(actual,expected,atol,rtol) for finite values and finite tolerances with0<=atol and0<=rtol<=1. Use the mathematical target |actual-expected|<=atol+rtol*|expected| and define actual admission by the supplied rounded scaled comparison (boundary rounding is part of this contract), scaling to avoid overflow in difference or threshold. NaN and infinities are rejected, including equal infinities. This is an admission predicate, not proof of an algorithm error bound.

Derive the asymmetric expected-relative policy, near-zero behavior and exceptional-value handling. Explain rounding at the acceptance boundary and why a tolerance must come from a prior requirement or error analysis.