# T17 assessor technical analysis

Withhold this file and the reference implementation from learner packages. Use
the original objective-specific rubric and original delayed IDs. The completion
envelope maps each ordinary and delayed task to exact executed commands or
explicitly labeled mathematical dossiers. A passing author campaign is neither
independent review nor independent learner performance.

## Error model and source interpretation: T17.01 and T17.10

For [2^53,1,-2^53], the exact represented-input sum is1 and the sum of magnitudes
is18014398509481985. A relative condition measure is therefore huge despite an
exact integer reference. For [1,-1], the nonzero-reference relative convention is
inapplicable. Converting mV to V scales absolute errors/tolerances by1/1000 but
preserves the ratio sum(abs(x))/abs(sum(x)) when defined. Require a separate
controller full-scale/reference contract; unit conversion is not calibration.

`finite_oracles.py` supplies a known linear calibration bias and decomposes
measurement, represented-input and final-addition terms, then changes the
phenomenon to a quadratic response. The old linear algorithm may still implement
its formula correctly while failing the new model. Award transfer credit only
when the learner names which evidence survives and which error budget changes.
The sum of three binary64 representations of0.1 is not the rational3/10.

The oracle generates523 vectors across ordinary, cancellation, extreme and
near-zero groups, evaluates four methods, and retains hex replay/error summaries.
Exact `Fraction` input conversion is checked against Decimal accumulation at900
and1100 digits, including ill-conditioned cancellation. This precision agreement
is bounded reference validation, not a claim that arbitrary decimal precision
always suffices. There are2557 driver cases/profile:2092 sums,202 interpolation
queries,7 Euler trajectories and256 generated systems with independently chosen
solutions. Q7, fixed filters and padded matrices additionally use independent
integer/rational fixtures in their mechanism tests.

Assess numerical accuracy separately from bitwise reproducibility. A distribution
with exact ordinary sums can hide the cancellation failure. Require maxima and
their locations, near-zero rules, a replayable input and a justified application
error budget; an average error or one larger-precision printout is insufficient.

## Summation: T17.02

Strict binary64 results for [2^53,1,-2^53] are0,1,1,1 for naive, the explicitly
specified midpoint tree, Kahan and Neumaier. For [1e16,1,-1e16], Kahan returns0
while Neumaier returns1. A learner who claims that every compensated method
recovers every lost low-order bit has missed the correction-rounding assumptions.
Higham's positive-correction equations were visually checked; changing their
association can remove the mechanism. The public midpoint tree's odd-length
grouping is not silently identified with an adjacent-pair tree.

The increasing-magnitude sequence [1,2^54,2^55,-3*2^54] loses the unit while the
reverse order retains it. Sorting by magnitude is not universally best. The
native worker experiment evaluates two partitions with two actual joined threads
and a separate three-worker reduction; strict results1 versus0 expose the tree.
No data race is needed to lose bitwise reproducibility.

The original superaccumulator variant explicitly requests a design. The model
uses integer units2^-1074; up to4096 finite binary64 inputs require at most2110
magnitude bits,2111 signed bits, hence33 64-bit limbs. A C design must decode
the selected representation, propagate carries, merge exactly, preserve the
input-count bound and round once with guard/sticky information. The24-permutation
Python model demonstrates the design's arithmetic, not an unimplemented C backend.

Naive and compensated sums take O(n) arithmetic and O(1) auxiliary storage.
The midpoint tree has O(n) work and depth at most13 for n<=4096 including the
singleton frame. Input validation adds a complete scan. The magnitude bound
avoids overflow on this finite domain; it does not remove underflow, conditioning
or the need to inspect excess precision and contraction.

## Fixed-point: T17.03

Q7 product range is[-16256,16384],32-term range[-520192,524288]. Q11 gives
[-4192256,4194304] and[-134152192,134217728]. The chosen int64 accumulator is
deliberately wider than required by these stated bounds. A type choice without
the bound is superficial completion. Negate only this bounded magnitude, divide
by2^q, round magnitude ties upward, restore sign and saturate. The INT64_MIN
negation failure cannot arise under the proved admitted input domain.

Two products1*64 in Q7 round separately to1+1=2 but round once to1. Saturating
each term or shifting a negative product changes the specified contract. Interior
normalized quantization error is at most2^(-q-1); endpoint clipping outside the
admitted physical interval is a separate effect. All65,536 Q7 pairs and a Q11
lattice are executed, with exact half ties and extrema covered explicitly.

## Views and layout: T17.04 and T17.09

Compute `(rows-1)*rs+(cols-1)*cs` only after multiplication and addition bounds
are checked. A sufficient extent alone does not rule out overlapping logical
indices, so the bounded validator checks uniqueness. Positive two-stride views
support transpose. Pointer equality compares only actual addressed elements;
the code does not order unrelated pointers to manufacture an overlap result.
Reject forged extents by precondition rather than claiming to validate storage
existence. Matrix outputs must be disjoint from logical inputs; scale is in-place.

All three matrix variants use increasing k per output. Blocking or staging the
right transpose can change memory traffic while keeping this arithmetic order.
The exact2x3 by3x2 result is[[58,64],[139,154]], with untouched padding. Padded3x1
and transposed inputs refute fixed-block and contiguous-only assumptions. Temporary
arrays are64 output doubles and64 transposed-right doubles. Admission includes
bounded uniqueness/overlap checks; excluding those costs would misstate the API.

The hot/cold implementation has owner-local nonwrapping IDs, linear lookup and
bounded16-record storage. Deletion must move labels and IDs with numeric fields;
the missing-label mutation catches a semantically wrong but memory-safe repair.
Reorder validates a full permutation before publication. Stable IDs do not make
raw pointers, indices, arbitrary foreign-owner IDs or destroyed controls safe.

The selected native layout is inspected, not prescribed by ISO C. A48-byte
record model touches96 aligned64-byte lines for128 x fields, versus16 for a
contiguous x stream; useful x bytes are1024 in either case. The all-selected-field
kernel reads x,y and24 label bytes, not the bookkeeping ID. Actual timing has108
raw batches/compiler and preserves calls with a declared compiler barrier. It
includes harness effects; there is no cache-miss-counter or universal-speed claim.
Inspect vectorization diagnostics and disassembly for the actual reduction,
not merely vector instructions in initialization. Reassociation/FMA require a
new arithmetic equivalence argument before accepting faster code.

## Interpolation: T17.05

Validate every knot before selecting an interval; returning an endpoint early
must not hide a malformed interior array. Duplicate/descending/nonfinite input
rejects unchanged. Singleton clamp and singleton reject are different contracts.
For the known quadratic, linear interpolation error on a segment[a,b] is
(x-a)(b-x), with maximum(b-a)^2/4 at the midpoint. Uniform spacing1/2 gives1/16
at1/4 and3/4. Nonuniform spacing7/8 gives49/256 on the larger interval. The exact
known quadratic itself costs one multiplication; that comparison does not generalize
to an unknown function merely because it has sampled values.

A continuous triangular bump centered at1/200 with half-width1/1000 is invisible
to all101 grid points but peaks at1. A finer finite grid is still not a continuous
proof absent smoothness/derivative or enclosure assumptions. Require the analytic
quadratic argument or a new certified validation model for the changed function.

## Filtering: T17.06

The actual checkpoint producer is unchanged. Its zero-history FIR impulse is
[1/4,1/2,1/4,0]; its unit-step half-IIR is[1/2,3/4,7/8,15/16]. Four samples have
three possible internal cuts, hence8 partitions. All execute. Failure ordinal k
in prefix mode commits k-1 samples; atomic mode commits0. Retry must use the
returned state and only the unaccepted suffix. Resetting a block or replaying an
already accepted prefix changes history. Tests retain unpublished sentinels.

Reference reuse is distinct from the learner dependency graph: T17 learners
implement `filter_learner.c` or import their own compatible filter, without
requiring the later P6 checkpoint. Both the new numerical stub and the separate
filter-producer stub compile and fail by intended semantic assertions.

With IIR initial state2, step response is[1.5,1.25,1.125,1.0625]. Fixed Q7 FIR
and half-IIR use the separate ties-away MAC contract; do not import the producer's
ties-even conversion by name alone. The local unstable coefficient1.25 is run
for16 bounded steps with synthetic data, while the admitted first-order API
rejects abs(feedback)>=1. The second-order factors(z-.9)^2 have stable poles
despite a1.8 recurrence coefficient. First-order screening cannot assess that
system; require the characteristic equation and all pole magnitudes.

## Solvers: T17.07

The pivot fixture returns[1,2] after a row swap; dependent rows reject unchanged.
Scaling the equations by1e90 preserves the declared relative screening model.
This does not establish accuracy for all accepted systems. For epsilon2^-30 and
RHS perturbation2^-40, the exact solution becomes[1-2^-10,1+2^-10] despite zero
exact residual. Residual and forward error answer different questions.

Counters count arithmetic in elimination/back-substitution:2x2 requires3 additions,
3 multiplications,3 divisions and1 row swap on the fixture;3x3 no-swap case gives
11/11/6/0. Admission, comparisons, data copies and row-swap scalar traffic are
separate. The3x3 scratch derivation precedes the implementation; generated systems
use independently selected exact solutions rather than another elimination oracle.

Jacobi tests distinguish convergence, finite iteration exhaustion and stagnation.
For A=[[2,1],[1,3]],b=[1,2], zero tolerance can leave the unchanged rounded
iterate[.2,.6] with nonzero residual. An explosive finite-input system can produce
nonfinite residual arithmetic while its next vector is still finite; the final
source review added explicit residual-range rejection and an unchanged-output
regression. A NaN comparison must not masquerade as residual zero.

## Euler: T17.08

The exact recurrence multiplier is1-rate*h. With rate1, h=.5 gives1/16 after
four steps; h=3 gives16. At h=2 the sign oscillates with magnitude1, so boundedness
is not decay. Rate2 changes the admissible decay interval to0<h<1. The three
fixed-T=1 halvings are compared with independent100-digit exp(-1), retaining
errors and ratios. No observed ratio proves convergence for another equation.
The forced equation y'=1-y with y(0)=1 stays at1; the old decay solver can pass
every recurrence test and still model that phenomenon incorrectly. For conserved
two-compartment transfer, assess the conservation law explicitly.

## Compiler changes, failure controls and assessment limits

The strict native experiments observed separate multiply/add0 versus explicit
FMA -2^-54 on the selected counterexample. Relaxed GCC/Clang reassociation removed
the Neumaier correction in the tested triple. The x87 experiment reported
`FLT_EVAL_METHOD=2` and changed intermediate rounding, including the three-worker
combination. These are source-bound observations, not predictions for arbitrary
versions or targets. Compiler manuals and ISO obligations remain distinct.

The eight deliberate mutations omit compensation, mishandle negative rounding,
ignore output stride, admit duplicate knots, suppress pivoting, replace the Euler
endpoint, detach labels and reset IIR initial state. Each compiles and is rejected
by its intended assertion in every profile. Every learner stub also compiles and
is rejected semantically. A build failure is not counted as a killed mutant.

The first dependency experiment rejected an emitted fabs call. The final selected
profile uses native floating intrinsics and actual T11 byte-helper closure, with
no unresolved symbols in the combined component and a fresh executed harness.
It does not claim every freestanding/no-builtin or MCU build has this closure.
Profile B explicitly permits primitive floor; no numerical mechanism is delegated
to a mature algorithm library. Exact source pins, output hashes and the dedicated
guard prevent a changed executable, dropped delayed variant or invented learner
result from inheriting the current author receipt.
