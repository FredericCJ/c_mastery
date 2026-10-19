# T64 changed-condition numerical contracts

These required continuations retain PR-T64-01 through PR-T64-08 and all parent
leaf objectives. They incorporate `curriculum/v5/contracts/b06.md` for evidence,
delayed assessment and the four rubric levels; hours remain descriptive. Families
A/B/C/D/F/H/I/J apply. The separate assessor implementation is never a learner
input. Read `numeric_extension.h` and implement the supplied stubs.

## Prerequisites and environment

Bring the parent fixed-point derivation, summation error analysis, scalar matrix
artifact and admitted-domain contract. Use hosted C17 with exact int16/int64 and
binary64 (radix2,53 significant bits, minimum normal exponent -1022), GCC/Clang,
no fast-math, no contraction, and rounding-aware compilation for fenv tests.
The x86 MXCSR experiment is an explicit ISA extension. Restore the calling
environment after every experiment. Python integer arithmetic and external
compilers are independent host verification tools, not taught target mechanisms.
No numeric library may implement the fixed-point, reduction or matrix kernel.

## Problems and contracts

N3 (T64.01.a/b/c) implements Q3.12 multiplication: int16 encodes real raw/4096,
including sign, with range [-8,8). Form the exact int32 product; divide by4096
toward zero before applying wrap, saturation or error policy. Error leaves output
unchanged. Wrap is an explicitly defined low16-bit result interpreted as signed;
do not rely on an out-of-range signed C cast. Derive the -3 times2048 result -1.
Prove the maximum dot product of4096 pairs fits int64 and compare one/four partial
accumulators at every length0..4096. Reject4097 rather than silently extending
the proof. `nd_accumulate` admits larger workloads by checking each int64 partial
sum before addition; a failed addition preserves state.

N4 (T64.02.a/b/c and T64.05.a/b/c) investigates arithmetic rounding versus casts.
Predict `1+2^-53` and `-1-2^-53` for all four available rounding modes from exact
rationals. Integer conversion of1.75 still truncates toward zero. For x86, execute
DBL_MIN times0.5 with gradual underflow, then FTZ, retaining runtime operands and
generated instructions. Expect2^-1023 versus zero. DAZ controls subnormal inputs,
whereas FTZ controls subnormal results. A compile-time folded operation is not a
measurement of hardware mode. Record pragma support and actual flags; observing
one compiler is not a portable FENV_ACCESS guarantee.

N5 (T64.04.a/b/c and T64.08.a/b/c) implements midpoint pairwise reduction for
0..257 finite values with magnitude at most1e100, returning positive zero for
empty input. The tree splits each interval at floor(n/2); it is fixed independently
of scheduling. For [1e16,1,-1e16,1] the exact real sum is2 but this tree yields0;
permuting to [1e16,-1e16,1,1] yields2. Explain why better worst-case depth does not
promise a smaller error on every input. Preserve output on failed admission.
The separate N7/N8 continuation below supplies actual parallel scheduling and
finite-value accuracy policies; this scalar tree alone does not establish them.

N6 (T64.06.a/b/c) implements dense row-major A[m*k] B[k*n] for m,n,k<=64, finite
inputs of magnitude<=7, disjoint live extents and output capacity>=m*n. Zero
dimensions are valid; k=0 yields zero in each existing output cell. Compare
scalar, transposed-B and3x3 blocked traversal with the same left-to-right k order.
All admission precedes publication; failures leave output unchanged. Include
uneven dimensions and tiles. Derive O(mnk), transposed storage4096 doubles and
active working-set costs, including the transpose in end-to-end measurement.

## Deliverables and verification

D1 supplies representation/overflow proofs and implementations; D2 contains
independent expected values, all-tail and partial-tile cases and rejected inputs;
D3 records rounding/FTZ modes, restoration and disassembly; D4 explains tree
error and permutation counterexamples; D5 retains raw timings, tool/CPU flags,
sample order, outputs, code size, stack/working-set estimates and limitations.
For every criterion, assess each `.a` derivation, `.b` implementation/boundary
evidence and `.c` changed-assumption argument explicitly.

Run public tests with learner code first. The opt-in author command is
`python3 tools/run_b06_numeric_extensions.py`; it separately tests three host
profiles, three mechanism mutants and learner-stub rejection, then compares
972 fixed-point cases and3861 integer matrix cases against Python arithmetic.
The timing driver takes216 raw samples per compiler over sizes7,16,31,64, three
layouts, warm and8MiB eviction-sweep conditions. Eviction is a controlled workload,
not proof every target cache was cold. Correctness and output observability are
preconditions for timing. No universal speedup follows from the distributions.

## Rubric, hints and transfer

For N3-N6, level0 lacks an admitted domain or independent oracle; level1 passes
ordinary examples but misses a relevant boundary/invariant; level2 satisfies the
exact contract, explains each negative control, and retains reproducible evidence;
level3 independently solves the delayed change at least seven days later.
Hints, in order: derive a mathematical bound before choosing the storage type;
separate admission from publication; freeze reduction order before comparing
layouts; inspect actual arithmetic instructions before interpreting mode effects.

N3-DELAYED changes scale and maximum n and requires a new bound. N4-DELAYED admits
subnormal operands and separately controls DAZ/FTZ with restoration. N5-DELAYED
uses the identical fixed tree across two worker schedules and compares bits,
latency and resource costs. N6-DELAYED changes matrix shape and cache budget,
retaining an equal-contract comparison with transpose cost included. These are
changed assumptions, not substitutions of test constants.

## Security and validation limits

Only synthetic local arrays are inputs. Extent rejection and sanitizers assess
memory properties, not a claim of exploitability. Source-bound author results
are in `research/v5/expansion/b06/numeric_extension_validation.json`; learner and
external assessment remain separate. Inspected references are
R-B06-FOLLOWUP-C11-TYPES (7.6.1) and R-B06-FOLLOWUP-INTEL-FTZ in the bounded source
register. The P6 explicit-AVX/scalar-dispatch composition remains required under
N1/N2 in README; this extension does not substitute scalar code for SIMD evidence.

## Actual vector partials and parallel tree: N7/N8

N7 completes T64.03.a/b/c with `numeric_transfer.h`: implement exact signed16
dot products using SSE2 eight-lane products and signed64 partial accumulators
on x86-64. Combine low/high16 product halves into signed32 products, then sign
extend before accumulating; signed16 multiply-low alone loses high bits. The
admission bound remains4096 and every tail executes. Test all lengths0..4096
at all eight int16 offsets modulo16 bytes with minimum/maximum signed values.
SSE2 is an x86-64 baseline requirement here; the existing N1/N2 AVX experiment
separately tests CPU/OS feature dispatch. Other architectures must reject this
requested mode unchanged. D6 is the lane/width proof, scalar comparison,
actual `pmullw`/`pmulhw`/`paddq` instruction evidence and resource cost.

N8 completes the stated finite-domain T64.04.a/b/c and T64.08.a/b/c continuation.
Use two POSIX workers to evaluate the fixed tree's two top-level subtrees and
join them in the same order. Each worker has distinct result storage; all joins
precede publication or frame destruction. On creation failure, join the already
created worker and preserve output. Changing which worker yields must not change
the result bits. Compare lengths0..257 and both schedules; retain27 raw timing
samples per compiler profile including thread creation/join, not merely arithmetic.
No arbitrary scheduler or speedup theorem follows from those observations.

Implement finite binary64 ulp distance with monotonically ordered bit encodings;
collapse the two zero signs to distance0 but report their sign distinction.
Reject infinity and NaN rather than inventing payload ordering. The separate
absolute/relative policy admits |values|<=1e100 and finite tolerances0..1, accepting
either threshold. D7 contrasts zero versus the least subnormal (one ulp but a
large relative change), opposite least subnormals (two ulps), adjacent values at1,
and the ill-conditioned sum with exact result2. Derive power-of-two scaling for
the exactly representable [2,4,8] case without using implementation outputs as
expected values. Broad exceptional-value algorithms remain separate contracts.

Run `python3 tools/run_b06_numeric_transfer.py`; its source-bound report is
`numeric_transfer_validation.json`. It checks three host profiles, dropped-tail
and missing-subtree mutants, stubs, actual packed instructions and raw timings.
N7/N8 use the same objective-specific level0-3 rubric: level2 requires all named
invariants and negative controls, level3 requires delayed lane-width or worker-
partition changes with new proofs. Hints: widen before accumulation; preserve
the mathematical tree independently of worker completion order; separate bitwise
identity from numerical closeness. The assessor analysis remains withheld.
