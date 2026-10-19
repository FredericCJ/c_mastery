# Numeric interval contract 1

This is required X-T17-003 work, mapped to PR-T17-01. Historical project hours do not limit this addition. Implement the mechanism in `src/interval.c`; the reference implementation and technical analysis are withheld under assessor. The C17 baseline and profile C apply. Host tests and the Python rational oracle are profile A tools, not target dependencies. Exact-width int32_t/int64_t availability is a selected platform premise. No arithmetic library may implement the component. Compiler-generated copying or integer division support must be named and audited; this package does not claim a linked freestanding image or physical execution.

## Representation and ownership

`NiInterval {lo,hi,bits}` denotes every REAL number in the closed interval `[lo/2^bits,hi/2^bits]`, including values between representable grid points. Valid values have `0<=bits<=16` and `lo<=hi`. Empty sets are returned as a status, never as a reversed interval. The type contains no pointer or hidden allocation. Inputs are passed by value; `out` may therefore name a caller input object. A nonnull output must point to one live writable properly aligned object. Arbitrary dangling pointers cannot be validated in portable C. Calls have no shared mutable state.

Every successful operation publishes one valid interval. Every error preserves all output members; no partial endpoint is published. Output padding bytes carry no contract. Statuses are OK0, INVALID1, SCALE2, ZERO3, RANGE4, EMPTY5. Admission precedence is null output, invalid input representation or unsupported operation; then incompatible scales; then division by an interval containing zero; then result range. EMPTY is used only for a disjoint intersection.

## Operations

`ni_ratio(n,d,bits,out)` encloses the exact rational n/d, requiring positive d and admitted bits; otherwise INVALID. `ni_binary` accepts ADD, SUB, MUL, DIV, HULL and INTERSECT. Both inputs must use the same bits. Its arithmetic result is the smallest interval on that grid enclosing the operation on all independent pairs of real input values. Division rejects any divisor interval containing zero, including [0,0] and an endpoint zero; disconnected/unbounded extended-real results are outside this API. HULL spans both operands. INTERSECT admits touching singleton endpoints.

`ni_square(a,out)` uses the same underlying value twice and returns the smallest enclosure of `{x*x:x in a}`. It must differ from independent multiplication on an interval crossing zero. `ni_rescale(a,new_bits,out)` changes the grid while preserving real containment; refinement is exact where representable, coarsening rounds outward. Out-of-range endpoints return RANGE instead of saturating or wrapping. Saturation would lose containment.

No floating-point operation or floating environment is used. The bound proof must account for int32 endpoint products (at most 2^62), ratio numerators times 2^16 (at most 2^47), differences and rescale factors. C17 integer division truncates toward zero; negative remainders need a separate downward correction. Do not right-shift a negative signed endpoint to implement a portable rounding rule. Outward lower/upper rounding is a semantic requirement, not ordinary nearest rounding.

## Concrete work and expected cases

At bits8, 1/3 encloses as [85,86]/256 and -1/3 as [-86,-85]/256. For a=[-1,1] at bits0, a*a=[-1,1] but square(a)=[0,1]; a-a=[-2,2], whereas the algebraic expression x-x for one shared x is exactly0. Explain the independence assumption that causes the difference. Coarsening [-257,257]/65536 to bits8 returns [-2,2]/256. Intersection [1,2] with [2,3] is the singleton2. Dividing by [-1,1] is ZERO without writing the output. Adding [INT32_MIN,INT32_MAX] to itself at bits16 is RANGE; claiming a clamped result encloses the true sum is false.

Use a synthetic sensor whose stated possible voltage is [1/3,2/3] volts; square it and compute a difference with a second independent reading. Keep the asserted sensor bounds separate from enclosure arithmetic: a miscalibrated sensor can violate input assumptions even if the implementation is correct. This exercise does not infer a probability distribution or a confidence level from interval width.

## Required evidence and limits

Derive extrema, signed floor/ceiling, representation bounds and atomic rejection before testing. Account for fixed stack/struct size, number of widened multiplies/divisions and the selected compiler helper closure. All operations here have constant bounded work and no heap allocation. Compare generated code for GCC and Clang; no speedup claim follows from smaller endpoints. Run public tests, an independently represented exact-rational oracle, both semantic mutants, sanitizer builds and the delayed precision change. A test failure is a counterexample; a finite passing grid is not a proof over all int32 values.

The changed precision is required implementation, not an optional appendix. After at least seven days, independently derive `ni_rescale`, compare bits16 and bits8 on the same physical intervals, and show that coarse-to-fine cannot recover lost information. A binary64/fenv implementation is a separate transfer analysis: state the required rounding/contraction/compiler assumptions without claiming this integer implementation validates them.

## Isolation

All values are synthetic, all executables local. No real sensor, external target, credentials, host setting change or hardware flashing is needed. Deliberate mutants only alter these original local arithmetic functions. Their harmless success condition is a demonstrated lost enclosure or false lower bound, not an arbitrary-code-execution claim.
