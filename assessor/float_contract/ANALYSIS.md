# Private technical analysis: PR-T04-04

Keep this file, all implementations and transfer material out of learner exports.
The public contract defines ownership, failure, environment and grading. This
analysis gives distinct discriminators for every original criterion. Alternative
algorithms are valid when they satisfy the stated operation/rounding contract;
an exact accumulator may be a useful comparison but cannot replace the requested
naive/Kahan/Neumaier derivation. Review explanation as well as executable output.

## t0409a

Read `<float.h>` properties as implementation metadata, not a binary64 guarantee.
`sizeof(long double)>sizeof(double)` alone does not imply more precision: padding
and ABI storage matter. The actual normal build reports its properties; the
`-mlong-double-64` probe makes both precisions53 and tests a long-double addition.
It uses no long-double library arguments, avoiding the host libc ABI mismatch.
Reject the proposed oracle “repeat in long double” in this changed build. Exact
Fractions or configured MPFR precision remain independent of nominal C type width.
Level2 needs metadata plus storage/evaluation reasoning; level3 needs the changed
reference strategy and ABI boundary, not just another compiler flag.

## t0409b

Signed zeros compare equal numerically but differ under sign-sensitive contracts.
Quiet NaN is unordered; classifying one does not establish signaling-NaN behavior
or payload preservation. `nextafter(0,1)` is the true minimum in the supported
model, not `DBL_MIN`. The FTZ test starts with normal operands and observes an
inexact subnormal result; it saves/restores MXCSR inside the original process.
Classification can itself set the x86 denormal-operand flag: perform it before
the restoration check and restore after output as well. This was a real pilot
defect, retained in the failed receipt. FTZ and DAZ have different result/input
roles; this exercise changes FTZ only and claims neither DAZ nor x87 equivalence.
Level3 must identify which instruction family and environment state were changed.

## t0409c

A reduced fraction terminates in radix b exactly when its denominator divides
some power of b. Ten includes factor5, so binary1/10 repeats; decimal1/3 still
repeats. The binary64 value of0.1 exceeds1/10 by
`1/180143985094819840`. For runtime0.1+0.2, sum the input-conversion errors
separately from addition error `1/36028797018963968`. Their sum is the error
against exact3/10. A decimal literal printed as0.1 is not an exactness proof.
The rational model checks both decomposition and cycle detection; no actual
decimal-radix C execution is claimed. Level3 derives the denominator criterion.

## t0410a

Naive and Kahan return0 on the displayed order; Neumaier returns1. Kahan's
correction can itself be lost when combining a small residual with the later
large negative term. Neumaier branches by magnitude to capture the residual of
the rounded sum, then adds accumulated correction at the end. Its general result
is still rounded. Enumerate six orders; do not infer permutation invariance for
all inputs. The3917-case rational corpus includes3906 short dyadic vectors, six
orders, four long sequences and one dot transfer. On the short grid, sums are
multiples of1/16 with small enough numerators for exact representation. In each
alternating pair the cumulative increment is2^-50; the largest partial sum
still represents this grid exactly. Derive the generic naive bound first, then
explain why these partitions admit the stronger exact result. The omitted-final-
correction mutant is a real implementation defect and must compile and fail.

## t0410b

The exact product is `1-2^-54`, so the exact fused result is `-2^-54`, representable
in binary64. Nearest-even product rounding selects1, leaving zero after subtracting1.
Downward/toward-zero product rounding selects`1-2^-53`, leaving`-2^-53`.
The fused answer stays`-2^-54` in all four modes. Wider evaluation retains the
product's low term and can produce the fused numerical answer without a fused
instruction. Thus equal results do not prove equal instruction sequences or cost.
Level3 must reason about changed rounding points, not “FMA is always more accurate”.

## t0410c

Absolute2e-6 accepts the near-zero pair; a relative-only comparison against zero
does not. Scale division avoids opposite-extreme subtraction overflow in the
relative branch; guard the absolute branch before subtracting. The operational
scaled policy is subject to threshold rounding, so do not present it as an exact
real inequality. For ULPs, ordered unsigned keys map both zeros to the same point,
negative values in increasing numerical order, and positive values above zero.
The distance from `-true_min` to `+true_min` is2. Raw unsigned subtraction of
representations fails this test. A sign-sensitive application adds its separate
zero policy. Level3 must reject the claim that ULP-zero distance preserves sign.

## t0411a

Snapshot first, then hold exceptions, change modes, evaluate volatile operands,
stage observations, restore the original snapshot, restore errno, then publish.
`feupdateenv` would merge temporary flags and violate this routine's stronger
contract; `fesetenv` restores the saved flags. Existing divide-by-zero plus a
downward caller mode discriminate clearing/restoring only the rounding direction.
The injection seam fails snapshot, hold, each mode, and restore. A held-environment
failure may already have changed state, so use the earlier snapshot. A failed
restore has an explicit containment error and no output, not a false guarantee
that the library repaired an unavailable platform operation. No result should
be published before successful restoration. Level3 explains that limitation.

## t0411b

Inspect the separately compiled operation object. The strict profile contains
multiply and add; contraction uses a multiply-add instruction. A volatile product
store prevents their fusion in the second function. The GCC x87 profile uses
wider register arithmetic; spills and return conversion are part of its contract.
The CPU capability program must pass before FMA/AVX2 execution, with no fallback
to another backend. A compiler accepting flags alone is not evidence of either
instruction selection or numerical semantics. Object stack/size outputs are
observations, not a platform-independent cost model. Level3 connects all three.

## t0411c

Compile a fast kernel object but link the driver with strict flags. This avoids
silently installing process-wide fast-math startup modes from the link command.
The raw fast NaN filter may disappear under finite-only assumptions; the strict
caller observes NaN where “missing” should have contributed nothing. Repair by
strict admission/filtering before the finite kernel. Signed-zero addition also
changes under fast assumptions. These exceptional controls violate the relaxed
compiler contract; their observed values are not portable C golden outputs.
GCC's NaN counterexample and both compilers' signed-zero violations discriminate
the contracts; Clang's different NaN observation has no promised value.
Clang21 diagnoses NaN use under the selected flags. Preserve that negative
translation result, then retain the warning while separately observing this
original bounded specimen. Changing warning severity does not repair the
contract. The strict-filter repair and finite timings have admitted inputs. Timings use64
small dyadic values whose sum is exactly-1/32 for every admitted order; five raw
trials of32768 calls are recorded. They cannot justify an exceptional-data or
compensated-algorithm equivalence claim, a universal speedup, or worst-case latency.

## t0412a

Partition zero neighbors, normal/subnormal boundary, three powers, and the largest
finite endpoint; use both signs and directions. Infinity beyond the endpoint is
classified/rejected by the finite ULP contract. Float changes actual precision;
four rational radix/precision models regenerate spacing on each side of powers.
Spacing below a radix power is one radix factor smaller than spacing above it.
A midpoint rounding test checks nearest-even, not just enumerated values. Uniform
random middle values miss these partitions. Level3 regenerates, rather than
reusing binary64 constants under a new format.

## t0412b

Import exact binary64 inputs into512-bit MPFR objects and use directed sums to
bound the exact result, then round endpoints to binary64. Do not import the C
value0.1 when the intended input is mathematical1/10; construct numerator and
denominator instead. The midpoint`1+2^-53` at53-bit oracle precision has endpoints
that round differently, so it is unresolved. At128 bits it is exact and
nearest-even selects1. A single rounded oracle value would conceal this distinction.
The six scale16 cases compare the unchanged earlier provider with rational
product, ties-even integer rounding and clipping, including negative half ties
and saturation. MPFR is a host verification dependency, not a target helper or
a permitted replacement for learner arithmetic. Level3 explains uncertainty.

## t0412c

Keep tolerance1/4 while deleting inputs. The original six-value vector reduces
to `[1,-1e16]`: even without a cancellation triple the rounded sum loses one unit.
Neither single remaining element disagrees, so the result is one-deletion-minimal.
This is not proof of global minimality across all possible values. The naive
method obeys its forward bound but does not meet this absolute tolerance. A
Neumaier implementation omitting its correction is defective; a naive “exact”
reference is invalid. These are distinct diagnoses. For the dot transfer derive
`gamma(2n)*sum(abs(a_i*b_i))` from rounded products and additions; for the displayed
three pairs it permits the observed zero, yet does not satisfy the stricter
absolute1/4 application contract. More epsilon hides the contract mismatch.
Level3 must derive the new bound and retain the original failed criterion.
