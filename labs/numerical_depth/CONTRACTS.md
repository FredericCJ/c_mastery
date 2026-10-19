# T17 numerical implementation and changed-condition contract

This required successor supplies concrete implementation work for the original
30 T17 leaf criteria and their original seven-day delayed variants. It does not
change historical project IDs, estimates, acceptance receipts or the historical
elective label on PR-T17-07. Euler work is required in this expansion. The exact
criterion and rubric in each parent supplement remain the assessment authority;
this contract supplies representations, operations, inputs and failure rules.

## Environment and dependencies

Use ISO C17 on the selected hosted radix-2 binary64 implementation, round to
nearest/ties even, preserved subnormal values, no reassociation or contraction.
ISO C alone does not guarantee these conditions. Record compiler and runtime,
`FLT_EVAL_METHOD`, rounding mode, all flags and emitted instructions. Strict
component builds use `-ffp-contract=off -fno-fast-math`; the floating-environment
experiment adds `-frounding-math`. Relaxed builds are separate experiments with
changed semantics, never substitutes for strict acceptance. GCC's rounding option
has documented limits; a successful probe is not universal compiler conformance.

Profile B components allocate no storage dynamically. Their selected headers are
the public type header, `<float.h>`, `<limits.h>` and `<math.h>`; finite checks,
absolute value and `floor` for conversion are individually permitted primitives.
No library may perform summation, interpolation, elimination, layout conversion
or filtering. Profile C applies to the FIR/IIR core and `nd_iir`: caller storage,
no external C-library calls; actual object audits are required separately from
the hosted harness. Sanitizers, Python `Fraction`/`Decimal`, printing, pthreads,
timing and `exp`/`fma` comparison calls are host verification dependencies.

The profile-C audit uses the selected native compiler's ordinary floating
intrinsics, with stack-protector insertion disabled for this component boundary.
Compiler-emitted byte helpers are explicitly limited to memcpy/memmove/memset/
memcmp, supplied by the original T11 `bytes.c`/`memory_provider.c`, compiled with
freestanding/no-builtin flags. A combined relocatable component must have no
undefined symbols and is then exercised by the hosted filter harness. This is
not a promise about compiling with every builtin disabled: that different
configuration emitted a `fabs` library dependency and was rejected. The selected
native object/closure evidence is not an unaudited MCU portability claim.

All supplied extents must describe actual live objects; validation cannot prove
that a forged pointer names storage. Controls, scalar outputs and input objects
are disjoint except the specifically allowed data overlap below. No concurrent
mutation of one control or array occurs. Unless stated otherwise, a rejected call
preserves every output value and control value. Padding bytes are not an object
equality oracle. The harness may use object snapshots only for unchanged objects
whose representation was copied before the call. No physical measurement is
implied by hosted observations or synthetic cache models.

## PR-T17-01: summation, model errors and reproducibility

Implement `nd_sum` for 0..4096 finite values with magnitude at most 1e100. Null
input is allowed only for zero length, whose sum is positive zero. Method0 is
left-to-right addition starting at zero; method1 recursively divides the sequence
at floor(n/2), sums each half, then combines them. This exact tree, including odd
lengths, is part of the bitwise contract. Method2 uses Kahan's positive correction
form: save old sum, add `x+correction`, then update `(old-new)+adjusted_input`.
Method3 is Neumaier's magnitude-conditioned correction followed by one final
addition. These are distinct mechanisms, not interchangeable spellings. Establish
their operation counts, correction invariants, assumptions and counterexamples.
The finite bound prevents overflow but does not prevent underflow or cancellation.

Explain and replay every intermediate for [2^53,1,-2^53] and [1e16,1,-1e16], all
six permutations, an exact-zero reference and a long sequence of small addends.
Specify whether an application requires exact input arithmetic, bitwise stability
for one tree, or an absolute/relative error budget. Implement the two-worker
experiment using joined workers and vary the split; compare with serial and the
fixed recursive tree. Revalidate compiler, order and arithmetic assumptions when
changing worker count or instruction set. An unchanged abstract addition formula
does not guarantee the same floating-point result.

Supply a millivolt-to-volt tolerance calculation and a synthetic calibration
error budget separating phenomenon/model, measurement, representation and
algorithm terms. Report exact intended decimal and represented-binary references
separately. The host oracle compares exact rational sums with independently
accumulated Decimal values at two larger precisions. Retain each generated seed,
hexadecimal replay, error quantile, maximum/location and zero-denominator rule.
Changed distribution is a new application acceptance argument. For the original
superaccumulator design variant, derive exponent alignment, bounded limb width,
carry handling, merge rule and one final rounding; distinguish the executed exact
integer design model from a C implementation that was not requested by that
design-only criterion.

## PR-T17-02: fixed-point arithmetic

Implement Q7 and Q11 using `int16_t` storage restricted respectively to
[-128,127] and [-2048,2047]. Their unit is 2^-q. `nd_qmul` widens the product;
`nd_qmac` accepts 0..32 pairs, accumulates unrounded widened products, rounds
once to nearest with ties away from zero, then saturates to the admitted range.
No negative right-shift assumption is permitted. `nd_quantize` converts finite
normalized values with the same tie rule and clips outside [-1,1); NaN/infinity
are rejected. Document clipping separately from interior quantization error.

Derive product and accumulator extrema before selecting types. Test all 65,536
Q7 pairs, the six retained fixture pairs, Q11 boundaries and a reproducible
interior lattice. Compare final rounding with rounding each product; the two
contracts are deliberately different. Translate the quantization error to physical
units after changing full scale or gain. Never infer that clipping still satisfies
a half-unit error bound outside the admitted normalized interval.

## PR-T17-03: views, matrix operations and hot/cold layout

`NDView` contains data, actual element extent, rows, columns and positive element
strides. Rows and columns are 1..8. Reject arithmetic overflow, last-index escape
and two logical elements naming the same index. Two positive strides support a
transposed view; neither padding nor transposition implies contiguous storage.
Inputs to arithmetic contain finite values of magnitude at most 1e100.

Implement matrix multiplication with method0 row traversal, method1 a staged
transposed right operand, and method2 two-by-two output blocking. All methods
accumulate each element in increasing k with separate multiplication/addition,
so they share a strict arithmetic contract. Output logical elements must not
overlap either input's logical elements; preserve padding and all output on
rejection. Temporary output is64 doubles, plus64 for the transposed operand.
Validate and justify the extra storage, traversal work and non-divisible edges.
`nd_scale` explicitly permits in-place scaling. `nd_dot` accepts two one-row
views of equal width and a disjoint scalar output. Admission scans and overlap
checks are part of cost, not hidden by the cubic multiply count.

`NDStore` is a caller-owned, zero-initialized structure of arrays for16 records.
IDs increase within one live owner, never wrap; insertion rejects exhausted
serials/capacity, deletion compacts all associated arrays, and reorder accepts
only a complete permutation. `nd_record` reconstructs one record from its ID.
Indices are unstable; IDs are meaningful only with their live owning store.
There is no cross-owner or dangling-control detection. Labels are24 bytes, not
assumed terminated. Verify identity after deletion and permutation, and strong
failure on invalid permutations. The simple lookup is linear; a proposed sparse
indirection variant must account for owner/generation metadata and retirement.

Compare AoS and SoA for x-only and all-field kernels with identical outputs.
The benchmark uses actual selected compiler layout and monotonic host timing.
A separate64-byte-line model counts useful/touched bytes for an explicitly
assumed48-byte AoS record. Neither model nor timings prove cache misses or a
universal speedup. Record alignment, field offsets, emitted vectorization and
scalar tails; any `restrict` or alignment claim requires actual non-overlap and
alignment, not an optimizer hint pasted onto an incompatible caller.

## PR-T17-04: interpolation

`nd_interp` accepts1..256 finite knots/ordinates and a finite query, all magnitude
at most1e100. Knots must be strictly increasing. One knot defines a constant only
in clamp mode or at its exact coordinate in reject mode. Clamp mode returns the
nearest endpoint outside the interval; reject mode returns `ND_RANGE` unchanged.
Validate the complete knot array before binary interval search. The selected
formula is `(1-t)*y_left + t*y_right` with t formed from the adjacent coordinates.
Return exact stored endpoint ordinates at the upper endpoint; test internal knot
equality and a query just to either side as well as duplicate/unsorted input.

Compare x^2 at uniform and nonuniform knots against an independently evaluated
polynomial on101 declared samples. Derive the continuous quadratic segment-error
bound and its maximizers; the grid observation alone is insufficient. Construct a
narrow feature between grid points and explain what additional derivative or
adaptive-enclosure evidence would be required. State storage, validation/search
cost, arithmetic operations and the changed endpoint policy.

## PR-T17-05: streaming composition

Learners implement the compatible FIR/IIR producer in `filter_learner.c` as their
T17 artifact, or import their own already assessed compatible artifact. Later P6
checkpoint acceptance is not a prerequisite. The author campaign reuses the
existing reference implementation and separately proves that the public producer
stub is rejected. This reuse avoids duplicating assessor code without creating
a backward curriculum dependency or supplying a solved filter to learners.

Use the interface shared with the frozen PR-P6-02 `checkpoint_numeric.h` producer:
`CPNDouble`, `cpn_double`, `cpn_transaction` and `cpn_iir_half`. Its exact source
and receipt are pinned separately. Its FIR coefficients are [1,2,1]/4; zero
history, finite±1e100 inputs, block capacity256 and unchanged-on-admission-error
rules remain intact. `cpn_double` accepts exact in-place and rejects partial
overlap; the staged transactional/IIR functions permit data overlap. State and
committed-count objects are disjoint from data. Keep integer-numerator and
ties-even conversion interfaces separate from this exercise's Q7 ties-away rule.

Check the impulse/step fixtures through all8 partitions of four samples. Inject
failure before every sample in prefix and atomic modes; preserve exact accepted
history, retry only the unaccepted suffix, and compare with an uninterrupted
run. `nd_iir` adds a first-order coefficient/state variant: finite feed with
absolute value<=1, finite feedback with absolute value<1, finite state/inputs
bounded1e100, 0..256 samples and sufficient capacity. It stages256 doubles and
admits arbitrary input/output overlap; caller state is disjoint. Rejection leaves
state/output unchanged. Derive nonzero-initial-state response, quantization and
the first-order pole test, then show why a two-pole recurrence needs its own
characteristic equation. A bounded unstable experiment is labeled as such.

## PR-T17-06: direct and iterative solvers

Implement `nd_solve` for n=2 or3 using the retained two-stride view, finite
coefficients/RHS of magnitude<=1e100 and caller output. Stage a3-by4 augmented
matrix,3 row scales and3 result scalars. Choose each pivot by largest
abs(column)/original-row-max, lowest row on ties. Reject a zero row scale or
abs(pivot)<=tolerance*row-scale, with finite tolerance in[0,1). This relative
screen is a declared diagnostic policy, not proof of good conditioning.
Return `ND_RANGE` on nonfinite arithmetic; publish output and `NDCost` only
after successful back-substitution. Inputs remain unchanged.

Count elimination/back-substitution additions, multiplications and divisions
separately from admission, pivot comparisons and scratch row swaps. Execute the
zero-leading-pivot fixture, dependent rows, a large equation scaling, the rational
near-dependent perturbation and a padded3-by3 transfer. Independently generate
systems with known exact solutions, rather than using the same elimination as
the oracle. Report residual and forward change separately.

`nd_jacobi` starts from zero, with the same n/view/input limits and nonzero
diagonal. Each iteration reads only the old vector. A finite nonnegative
absolute infinity-residual tolerance is the stopping rule; limit<=10000 bounds
work. `ND_LIMIT`, `ND_STAGNANT` (unchanged iterate with excessive residual),
`ND_RANGE`, invalid input and singular diagonal preserve output and iteration
count. These diagnostics say nothing by themselves about the physical problem.

## PR-T17-07: Euler and model boundaries

`nd_euler` implements exactly n updates `y=y+h*(-rate*y)` for y'=-rate*y,
including initial y in n+1 caller outputs. Admit n<=256, positive finite h/rate,
finite y and magnitudes<=1e100; require capacity>=n+1. Stage257 doubles and
reject nonfinite intermediate values without publishing. An unstable finite
trajectory is intentionally permitted and labeled; positive h alone is not a
stability guarantee. Negative or zero h/rate is rejected even for n=0.

Compare the discrete rational recurrence with an independent100-digit exp(-t)
continuous reference. Execute h=.5 and3 at four steps, h=2 bounded oscillation,
rate2 with a newly derived interval, and fixed-T=1 step halving. Changing the
model to include forcing or a conserved two-compartment quantity invalidates
the old model oracle while leaving some implementation arguments reusable.

## Verification, rubric and evidence limits

Build the public tests against learner.c first: compilation must succeed and
the named `ND_ASSERT` must reject the unsolved mechanism. Use the existing
objective-specific four-level rubrics: level0 missing/wrong; level1 fixture-only
with unexplained assumptions; level2 correct bounded mechanism plus independent
boundary/failure evidence; level3 justified changed-condition transfer with
counterexamples to inflated guarantees. Assessor implementations and technical
analysis remain in the separate assessor tree. Public tests are examples, not
an exhaustive solution rubric or the assessor's hidden answer.

Required artifacts are source, exact replay/oracle record, units/ownership and
resource ledger, compiler/binary audit, failure snapshots, and each original
delayed artifact. Start hints: write the arithmetic and storage domains first;
next derive one exact rational case; finally isolate the first differing
operation or index. Do not repair a failed contract by widening tolerances until
the defect disappears. Delayed assessment retains the original minimum seven
days and changed-condition IDs. Author execution is not delayed learner evidence.

All programs use original local synthetic inputs; no external targets, real
credentials or hardware are involved. Bounds failures preserve outputs rather
than demonstrating exploitation of another program. Sanitizer success, finite
generated cases, exact arithmetic models and raw timings establish only their
stated scopes. The exact published C17 text, independent review, independent
learner performance and any physical target validation remain separate.
