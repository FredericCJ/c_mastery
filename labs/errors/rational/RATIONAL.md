# Canonical fractions, checked consumers and signed cancellation

Required X-T65-009/R1-R4 and X-T65-009-DELAYED after X-T65-008. Reuse actual
er_refine and prior checked-arithmetic reasoning. The rational core is hosted
C17 with no allocation; the separate opaque comparison permits an explicit raw
storage callback and a host malloc/free test adapter. It does not silently use
the differently typed T66 cell allocator. No library implements the rational
mechanism; Python Fraction is an independent host oracle only.

## Representation and trusted boundaries

er_fraction is a public pair of uint32_t. Valid values have denominator>0 and
gcd(numerator,denominator)=1; zero is0/1. Original er_refine rejects denominator0
unchanged and reduces all admitted pairs. Prove common-divisor preservation under
remainder and termination by a decreasing positive remainder. efr_gcd exposes
the same first-principles recurrence and optional remainder-step count, including
gcd(0,0)=0. Consecutive Fibonacci values2971215073/1836311903 take45steps with
the larger value first. Derive the Fibonacci lower bounds on long Euclid runs;
observing one pair alone is not a worst-case proof.

efr_consume takes a value snapshot, validates canonical form using actual er_refine,
then publishes numerator/denominator truncated to uint32_t. Fabricated denominator0
and unreduced2/4 both reject unchanged. A once-constructed public struct can be
modified through an alias; constructor success is not a permanent static refinement.
The snapshot itself must be acquired without a concurrent data race. Outputs are
live, initialized and disjoint; by-value operands permit writing the result into
the caller variable from which an input was copied.

## Unsigned addition: compare two exact contracts

Both add APIs require canonical inputs and preserve both output fields on any
refusal. efr_add_naive requires each cross numerator, their sum and denominator
product to fit uint32_t before calling actual er_refine. Intermediates are
evaluated in uint64_t with short-circuit admission: sum is considered only after
both cross products fit32bits. This policy deliberately rejects some representable
final fractions. For1/UINT32_MAX plus itself, it rejects unchanged because the
unreduced denominator product is too large.

efr_add first sets g=gcd(d1,d2), left=d1/g, right=d2/g, and
t=n1*right+n2*left. Each product fits uint64_t; check the sum before evaluating it.
Let h=gcd(t,g). The canonical result is (t/h)/(left*(d2/h)); admit only if each
final field fits uint32_t. The improved function returns2/UINT32_MAX for the
example, and1/2 for1/6+1/3. UINT32_MAX/1+1/1 rejects unchanged. No floating point,
arbitrary-width library or division before checking denominator is permitted.

Derive why reducing only by h is sufficient for canonical inputs: the unmatched
denominator factors are coprime and cannot divide the corresponding numerator
sum. If g>=2, the sum bound2*UINT32_MAX*floor(UINT32_MAX/g) fits uint64_t. If g=1
and the cross sum exceeds uint64_t, the unreducible denominator product necessarily
exceeds UINT32_MAX (a product within32bits implies d1+d2<=UINT32_MAX+1). Therefore
that guarded refusal cannot hide a representable canonical32bit result. Defend
this premise; do not generalize the formula to unvalidated unreduced inputs.

## Signed numerator and minimum value

EfrSigned has int32_t numerator and positive uint32_t denominator, with the same
coprimality and zero rule. efr_signed_make computes a negative magnitude using
unsigned conversion followed by unsigned subtraction from0; it never evaluates
-INT32_MIN or abs(INT32_MIN). After reduction, conversion to int32_t occurs only
inside range, with an explicit INT32_MIN case for negative magnitude2147483648.
The denominator remains unsigned; this task does not add a signed-denominator
representation. Constructing INT32_MIN/2147483648 gives-1/1; INT32_MIN/1 stays exact.

Signed addition uses the same denominator cancellation but keeps scaled numerator
magnitudes in uint64_t. Equal signs add magnitudes; different signs compare and
subtract, retaining the sign of the larger. Each term is at most2^31*(2^32-1),
so even the sum fits uint64_t. Reduce by gcd(magnitude,g), check positive result
<=INT32_MAX or negative magnitude<=2147483648 and final denominator<=UINT32_MAX,
then publish. INT32_MIN/MAX plus INT32_MAX/MAX becomes-1/MAX; INT32_MIN/1-1/1
refuses unchanged. Exact cancellation is0/1, not a signed zero or unreduced pair.
efr_signed_consume validates and truncates toward zero using magnitudes; -7/2
produces-3. It does not accidentally perform mixed-sign unsigned division.

## Opaque hosted comparison

opaque.h declares an incomplete EfrOpaque and a raw-storage EfrMemory callback
pair. Ordinary clients cannot name fields; the negative translation specimen
must be rejected by both compilers. The implementation privately stores the
canonical fraction and the matching memory callbacks. make requires a live
empty owner variable, complete callbacks and valid denominator; validation occurs
before allocation. The callback returns distinct suitably aligned raw storage
of at least the requested size or NULL. NULL produces IO with the empty owner
unchanged. Successful construction publishes only after initialization.

consume accepts only an authentic live object produced by make and may rely on
its private invariant; it cannot validate arbitrary forged nonnull pointers.
drop caches callbacks, empties the unique owner, then releases exactly once.
Dropping NULL is idempotent. Callback/context lives through drop; no reentry,
nonlocal exit or retained control access is permitted, except the test release
observer may read the owner variable to verify it is already empty. No copy of
an owning pointer becomes a second owner. The actual heap test injects allocation
failure and verifies no allocation for invalid input, one release and empty
publication before that release. No automatic C destructor is implied.

Private layout prevents ordinary field access across a header boundary, not
memory corruption, forged pointers, casts into unknown storage or expired aliases.
Compare runtime validation of public snapshots with authenticated opaque lifetime
discipline; neither is a language-level dependent/refinement type proof.

## Verification, rubrics and transfer

D1 supplies Euclid/value/canonicality proofs, intermediate bounds, signed-minimum
derivation and the public-versus-private enforcement matrix. D2 implements all
exact headers and publication/refusal contracts. D3 runs the independent Fraction
oracle:65585unsigned normalizations,1024unsigned consumers,5204unsigned additions,
65578signed normalizations,8651signed additions,70signed consumers and47Fibonacci
pairs per profile; add your own changed cases. D4 seals the signed representation,
range proof and consumer contract before independently implementing the delayed case.

Six anchors cover retained normalization, consumer admission, cancellation, signed
arithmetic, actual opaque lifetime and invalid arguments. Eight compiled controls
skip canonical validation, publish before refusal, omit denominator cancellation,
omit final gcd reduction, reject minimum signed input, combine opposite signs
incorrectly, or compile actual ER_MUTANT17/18. Require ER_CHECK semantic rejection,
not a sanitizer error or divide-by-zero. Learner stubs compile and fail each anchor.

R1 assesses proof and enforcement boundaries; R2 exact construction/consumer
ownership; R3 extrema/Fibonacci/oracle/compiled falsification; R4 bounded addition
and delayed signed transfer. Nominal reproduction earns1, complete justified
bounded evidence2, unaided changed-condition analysis and counterexample3. Undefined
signed arithmetic, unchecked denominator, noncanonical output or leaked ownership
fails regardless of aggregate score. Hints: identify canonical input assumptions,
cancel common denominator factors first, keep sign separate from magnitude, and
publish a complete pair last. Separate assessor analysis is not learner material.
Seven-day independent transfer remains unperformed by author execution.

## Cost review (thirteen dimensions)

1. Representation: record unsigned/signed pairs and private object size separately;
   layout/padding is not a serialization format.
2. Work: Euclid is logarithmic in the smaller nonzero operand; addition runs a
   bounded number of gcd/product/division steps, not constant-time cryptography.
3. Space: fixed scalar temporaries, no recursive stack or arbitrary precision in C.
4. Allocation: none in the core; exactly one raw allocation per successful opaque
   construction and one matching release, with explicit failure injection.
5. Traffic: by-value snapshots and one final pair publication; private object stores
   allocator context/function pointers as real overhead.
6. Dispatch: actual er_refine calls and opaque allocator callbacks; measure separately.
7. Code size: retain object/disassembly observations and native compiler flags.
8. Stack: preserve emitted stack-use records; division helpers are target-specific.
9. Timing: seven10000-run1/6+1/3cancellation samples, checksum210000; no comparison
   with naive rejection as if they implement the same admitted-input contract.
10. Failure: invalid versus unrepresentable versus allocation failure; unchanged
    output differs from an empty-before-release destructor transition.
11. Scheduling: serialized live owners and snapshots; no atomic publication claim.
12. Builds: clean/incremental, negative opaque translation, actual source/include
    closure and producer imports retained.
13. Portability: exact-width types and uint64 intermediates are required; changed
    widths need fresh bounds. C17 remains active, drafts stay accurately labeled.

Original local numeric inputs and teaching allocations only; no external target,
credential or exploit claim. Physical evidence, independent human assessment and
learner mastery are not inferred from finite author campaigns.
