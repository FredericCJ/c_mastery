# PR-T04-04: floating representations, error contracts and compiler permission

This required successor supplies implementation and investigation support for the
twelve retained `T04.09.a`–`T04.12.c` criteria. Their stable IDs, historical hours,
original tasks and seven-day variants remain in the project assessment card.
Families A/C/D/F/I/J apply; representation, construction and measurement work
also exercise B/H. A passing reference campaign is bounded author evidence.

## Environment, prerequisites and deliverables

Use hosted profile A, ISO C17, Linux x86-64, GCC or Clang, and the platform math
and floating-environment library. First derive the model from `<float.h>`;
binary64-specific claims require radix2, precision53, exponent1024, eight-bit
bytes and the checked representation of `1.0`. The ULP adapter additionally
requires `uint64_t`. Unsupported models must be reported, not silently coerced.
The campaign's four rounding modes and subnormal behavior are checked platform
capabilities, not universal C promises. GCC `-frounding-math` is an experimental
compiler contract; Clang's supported `FENV_ACCESS` pragma and actual generated
operations are separately evidenced. No claim of complete compiler conformance
follows from these examples. C23 comparisons in other T04 packages remain
bounded; this package's source and numerical claims use C17.

Prerequisite knowledge: T01 builds and separate translation, T02 expressions,
T03 object representations and `memcpy`, T04 integer/bit contracts and the
completed PR-T04-03 fixed-point operations. Bring your scale16 fixed-point
component and its rounding contract. The author oracle imports the unchanged
`bf_mul16` reference, explicitly as assessor work. Future T12/T17 implementations
are not prerequisites. MPFR/GMP and Python rational arithmetic are host oracles,
permitted for verification but forbidden as implementations of your sum, dot,
comparison and environment mechanisms.

Deliver `learner.c`, your own observation and compiler-experiment programs,
an input/expected-result corpus written before execution, a criterion-indexed
reasoning dossier, exact build commands, raw outputs, generated operations,
stack/size observations, and a manifest of source/compiler/OS/ABI/library
identities. Retain failed hypotheses and minimized cases. Run the public smoke
contract with `python3 run.py --build <directory> --source <your-file>`;
`--cc clang` changes the compiler. The default scaffold deliberately fails.
All pointers designate live, suitably aligned objects; inputs and output do
not overlap. No function takes ownership or retains a pointer. No allocation,
recursion, parallel reduction, external input service or hardware is needed.

## Concrete component contracts

`ff_sum(x,n,method,out)` supports naive, Kahan and Neumaier methods. The vector
has at most4096 values. Each input is zero or finite with magnitude in
`[2^-100,2^100]`; require round-to-nearest. Validate the entire vector before
publication. Reject bad count, method, null nonempty input, NaN, infinity or
out-of-range magnitude with `FF_RANGE`; reject a different rounding mode with
`FF_MODEL`. Leave `*out` unchanged on rejection. Empty input returns positive
zero. Successful arithmetic may raise status flags. It does not promise to
restore them. Each method costs O(n) operations and O(1) auxiliary storage.

Derive the running-sum invariant for naive addition. For Kahan, explain the
meaning and sign of the subtracted correction; for Neumaier, explain why the
larger-magnitude branch changes the residual expression. Neither method gives
exact arbitrary summation, permutation invariance or an absolute-error bound
independent of the input scale. Disable reassociation for compensated code.

`ff_dot(a,b,n,out)` has the same input/ownership/publication limits and forms
each product at the semantic `double` precision before adding sequentially.
The admission bounds keep products and sums finite and normal when nonzero
under the declared binary model. A contraction or reduction-tree change is a
new numerical algorithm and requires a new error argument.

`ff_compare(a,b,absolute,relative,preserve_zero_sign,out)` accepts finite operands,
finite nonnegative tolerances and relative tolerance at most1. Its policy is
absolute difference OR scaled relative difference
`abs(a/max(abs(a),abs(b)) - b/max(abs(a),abs(b))) <= relative`.
Handle equality first, except that sign-preserving zero comparison distinguishes
the signs. Avoid overflow while deciding the absolute branch, including opposite
`DBL_MAX` values. This is an operational floating policy: rounding near its
threshold can differ from a mathematical exact-relative inequality. Derive
those limits; use an interval oracle if the application requires a real-valued
threshold decision. Reject nonfinite operands without publishing a result.

`ff_ulps(a,b,out)` counts adjacent representable binary64 steps, identifies both
zero encodings as one point, and rejects NaN/infinity. It must work across zero
and for negative numbers. Copy representations through `memcpy`, never an
aliasing pointer cast. Validate the model before interpreting bits. Numerical
ULP equality and application sign equality are deliberately separate policies.

`ff_round_probe(x,y,out)` records runtime `x+y` under the four available standard
rounding directions; restrict finite `abs(x),abs(y)<=1`. Preserve caller errno,
control modes and existing exception flags. Snapshot before any mutation,
install non-stop handling, stage results, and restore before publishing.
`ff_probe_ops` exposes the same path through documented save/hold/mode/restore
operations for fault injection. A failed snapshot returns `FF_ENV` without a
restore attempt; after a successful snapshot every exit attempts restoration.
An operation failure returns `FF_ENV`; a failed restore returns `FF_RESTORE`.
Neither publishes partial observations. If the platform itself refuses restore,
state cannot be promised unchanged: report it so the caller can contain failure.
The seam is a trusted function-table contract, not an ISO C effect system.

## Required progression and exact investigations

| Criterion | Required construction and explanation | Boundary or changed assumption |
|---|---|---|
| T04.09.a | Emit storage bytes, radix, digits, min/max exponents, evaluation method and subnormal macros for all three floating types. Derive why an evaluation format can differ from storage. | Build the narrow `-mlong-double-64` observation and revise the oracle; do not call ABI-incompatible long-double library functions or `%L` conversions. |
| T04.09.b | Classify ±0, infinity, quiet NaN, `nextafter(0,1)`, normal minimum and largest finite value, preserving the caller environment. State the supported model. | Save/modify/restore process-local SSE FTZ; multiply normal `DBL_MIN` by runtime0.3 and compare gradual and flushed outputs. Do not assume x87 shares that control. |
| T04.09.c | Derive the rational1/10 and its repeated binary expansion; decompose the error of runtime0.1+0.2 into the two input-conversion errors and one addition error. | Derive decimal1/10 and nonterminating decimal1/3; label this a rational radix model, not observed decimal C hardware. |
| T04.10.a | Implement all three sums and evaluate all six orders of `[1e16,1,-1e16]` against exact1. | Before execution derive a bound for up to2048 pairs `[1,-1+2^-50]`; compare exact dyadic and floating results without changing the bound. |
| T04.10.b | Compute `fma(1+2^-27,1-2^-27,-1)` and a stored-product expression. Derive the rounding points. | Repeat all four rounding directions and compare an actual wider x87 intermediate. |
| T04.10.c | Implement absolute, scaled-relative and ULP policies. Assess0 vs1e-6 with absolute2e-6, large finite neighbors and adjacency across zero. | Require sign-preserving zero equality and show why ordinary `==`/ULP policy is insufficient. |
| T04.11.a | Implement the transactional environment probe; test a caller with downward mode and existing divide-by-zero status. | Fail snapshot, hold, each of four mode changes and restore; retain original caller state when restoration succeeds and leave output unchanged on every failure. |
| T04.11.b | Compile separate and potentially contracted expressions with explicit flags and inspect the actual object instructions. | Execute GCC x87 wider evaluation; record stores, return rounding and excess-precision flags rather than treating compiler options as proof. |
| T04.11.c | Build strict and fast kernels separately, link a strict caller, and state admission before collecting five timing trials. | Let NaN mean missing data. Compare a fast-compiled filter with strict caller filtering before the finite kernel; include signed-zero and cancellation controls. |
| T04.12.a | Generate zero/minimum/maximum/power-neighbor and cancellation partitions algorithmically with `nextafter`; classify every partition. | Change actual precision to float and generate radix2/10 rational-format neighbors at two precisions each. |
| T04.12.b | Compare your earlier scale16 fixed-point operations to rational product/rounding/clipping results; configure a separate higher-precision oracle. | Directed interval endpoints may straddle a binary64 rounding boundary: refuse a single answer and increase precision. |
| T04.12.c | Minimize a numerical disagreement with absolute tolerance1/4 frozen, recording the exact reference after every deletion. | Transfer to a sequential dot product and derive its own multiplication-plus-addition bound. Classify a defective implementation, an unsuitable contract and an invalid oracle separately. |

For summation in the admitted binary64 round-to-nearest model, let `u=2^-53`
and `gamma(k)=ku/(1-ku)`. Derive a sequential naive absolute bound
`gamma(n-1)*sum(abs(x_i))` by expanding each rounded addition as a factor
`(1+delta)`, with `abs(delta)<=u`, and bounding products of factors. The dot
bound `gamma(2n)*sum(abs(a_i*b_i))` conservatively includes every product and
addition. These require no overflow/underflow and `2nu<1`; they are not good
relative-error guarantees when cancellation makes the exact answer small.
The supplied dyadic grid and alternating workload have stronger exactness
arguments; explain those separately. Do not apply the naive bound to a changed
algorithm without a derivation.

## Verification, assessment and limits

Public tests distinguish compensation, staged rejection, zero sign, negative ULP
ordering, caller flags and seven environment-failure positions. Add your own
cases before seeing private transfer material. The assessor has separate rational
and directed MPFR oracles, selected defined semantic mutants, GCC/Clang O0/O3
and sanitizer profiles, actual compiler-mode objects and isolated export checks.
Each raw result must name its criterion; a failure to compile a mutant does not
count as a semantic rejection. Record unsupported capabilities honestly.

The NaN/fast-math specimen deliberately violates the compiler's relaxed input
contract. Its diagnostic and local observations are separate from defined
semantic mutants and from the admitted finite timing workload. They have no
portable golden result; compiler differences must remain visible. Warning
severity changes used to observe that original specimen do not repair it.

For each row, level0 is an incorrect result or missing contract; level1 reproduces
the example but cannot derive its cause; level2 supplies the implementation,
reasoning, independent reference and intended rejection evidence; level3 solves
the withheld changed-condition task and explains which guarantees ceased to
apply. A reviewer must score each objective separately. Actual independent
learner attempts, delayed performance, external technical review and physical
observations are pending even when author tests pass.

Hints progress from (1) locate every rounding/publication point, to (2) separate
input conversion from operation error and caller state from temporary state,
to (3) use exact rational inputs and directed endpoints to challenge the proposed
oracle. No exploit or external target is involved; run only these original
programs locally. FTZ changes belong inside the test process, never a host-wide
setting. Source attribution and exact inspected passages are in
`research/v5/expansion/legacy_float_contract/resources.json`: WG14 N1570 (C11
draft, not published C17), GCC15.2 and Clang21.1 floating documentation, MPFR4.2.2 manual,
and Intel2025 FTZ documentation. Math derivations above are supplied analysis,
not a claim that an uninspected numerical textbook was read.
