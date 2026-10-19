# Required signed rational-unit successor: X-T67-002

T67-02-INVESTIGATION and T67-02-TRANSFER/X-T67-002-DELAYED require this successor.
Keep the actual original ADSeconds/ADMillis provider and its overflow contract.
Prerequisites are X-T04-001 checked arithmetic, X-T67-001 wrapper/sum reasoning
and the original X-T67-002 seconds conversion. The new implementation does not
use a bigint, duration, units or rational library. Python arbitrary-precision
arithmetic is an independent host oracle; the C++ duration experiment is a
comparison program, never the C implementation or an overflow oracle.

Use hosted ISO C17 with int64_t/uint64_t and uint32_t, GCC/Clang debug/release
and ASan+UBSan. The core uses no dynamic storage, floating point, compiler
integer wider than64 bits, C++ code, or arithmetic-library callback. Inspect
generated division helpers before adopting a strict freestanding profile.
Current hosted compilation is not a target link/execution claim.

## Exact interface and failure semantics

Implement rational_units.h and its compiling unsolved source. AdqMicroseconds
and AdqTicks are distinct one-field nominal structures; AdqPeriod(p,q) means
one tick is exactly p/q microseconds. Both p and q are positive uint32 values,
and they need not be relatively prime. adq_to_ticks computes trunc(us*q/p);
adq_to_microseconds computes trunc(ticks*p/q), with truncation toward zero.
adq_scale(v,n,d) exposes the same checked arithmetic primitive for comparison.

Success returns ADQ_OK and publishes {value,inexact}; inexact is exactly1 if
the mathematical rational had a nonzero fractional part, otherwise0. It is not
an error: the caller must decide whether the lost fraction is acceptable.
ADQ_RANGE means the truncated mathematical result is outside int64. ADQ_INVALID
means a zero ratio part or null output. Both failures leave output unchanged.
The output must otherwise be one live writable AdqResult. Inputs are by value,
so no input storage or callback survives; calls are independent provided callers
do not race on output. Pointer validity is a premise, not a numeric check.

For period3/2,5microseconds gives3ticks/inexact1 and-5 gives-3/inexact1;
3ticks gives4microseconds/inexact1. A round trip need not recover the input.
scale(-1,1,3) gives0/inexact1, not-1. scale(INT64_MIN,1,1) succeeds exactly.
scale(INT64_MAX,UINT32_MAX,UINT32_MAX) succeeds exactly despite the mathematical
intermediate product exceeding64 bits. Multiplication by2 at either signed
extreme refuses unchanged. Zero input with a zero ratio part is still invalid.

## Derivation and misconceptions

Compute the magnitude in uint64 without negating signed INT64_MIN. Divide it
as a = whole*d + remainder, where remainder<d<=UINT32_MAX. Then
floor(a*n/d) = whole*n + floor(remainder*n/d). The remainder product fits
uint64 because both factors are at most UINT32_MAX and remainder is strictly
smaller than d. Check whole*n and the final sum against a sign-specific limit
before computing them. The negative limit is INT64_MAX+1; converting that
magnitude directly to signed is unnecessary because it denotes INT64_MIN.
The remaining fractional part is (remainder*n)%d. Restore sign only after
range admission. Derive each bound; dividing first and discarding remainder
is incorrect, while multiplying first can overflow despite a valid result.

The inspected N1570 C11 draft6.3.1.3 permits the specified modular conversion
to unsigned; unrepresentable conversion back to signed has different rules.
This is bounded draft evidence, not an inspected published C17/C23 clause.
The inspected rolling C++ duration_cast draft chooses a ratio and common
representation before its casts. It does not add this API's explicit range or
precision result. Only representable C++ comparison inputs are executed. Do
not execute signed-overflow examples and label the result a language oracle.

## Required artifacts, verification and assessment

D1 derives nominal versus typedef identity, dimensional direction, magnitude
conversion and the exact mathematical rounding contract. D2 supplies the
implementation, admission/unchanged-output proof and no-wider-integer source
audit. D3 records the independent integer corpus, all boundary tests, semantic
controls, diagnostic controls and debug/release helper/branch observations.
D4 changes period to an unreduced fractional ratio and includes negative
extrema and round-trip precision loss; compare equal-contract raw primitive
and nominal wrappers, retaining predictions before measurements.

Three anchors separate fractional remainder, signed/admission boundaries and
the actual retained seconds conversion. Six compiled controls discard the
remainder quotient, lose the precision flag, exclude INT64_MIN, reverse the
period ratio, omit the original range check or change the original scale.
Every control must compile and reach its intended normal ADO_CHECK rejection;
compiler failures and sanitizer findings do not substitute for that evidence.
The independent Python model forms the full mathematical product and truncates
its quotient, then checks int64 range; it does not repeat the C decomposition.
The supplied13900 cases include741 invalid-ratio,2981 range and10178 successful
results. Match exact outputs and unchanged99/77 sentinels, not just status.

Both C compilers must reject passing AdqTicks to the AdqMicroseconds parameter,
accept a valid call, and accept explicit field reconstruction. A separate
typedef-only specimen accepts accidental substitution. Both C++ compilers run
the declared positive duration examples. All negative cases have a successful
positive control, exact command and diagnostic marker.

Apply original R1 to the nominal/typedef distinction, R2 to bounds and source
comparison, R3 to actual old and new diagnostics/boundaries, R4 to the complete
signed fractional implementation. Public successes alone earn at most level1.
Level2 requires all derivations and refusal semantics; level3 requires an
unaided delayed changed-condition result and a correct counterexample to the
old proof. Neither source analysis nor a finite oracle establishes mastery.

Hints: first avoid signed negation; next retain the remainder when dividing;
finally separate the positive and negative destination limits. The private
ANALYSIS.md contains the complete argument and remains outside learner export.

## Cost, transfer and limits

Thirteen dimensions: wrappers contain one signed value, period two unsigned
values, result a value+precision flag; no heap-reachable storage; outputs have
caller-owned lifetime; inputs are copied values; allocation/destruction are
nonapplicable to this pure primitive; copies are fixed-size; dispatch is direct;
concurrency depends only on separate outputs; rejection preserves result;
compiler work includes real nominal probes and primitive/wrapper objects;
stack/binary and helper reports are ABI-specific; and the boundary distinguishes
ISO types, runtime representability and caller dimensional intent. Public fields
remain forgeable, and a type name does not certify measurement units.

Seven paired100000-operation samples execute the same values and exact3/2
period through raw scale and nominal wrapper, verify equal checksums and report
both times. Include function-call/optimization/cache/timing noise; do not claim
the wrapper is universally zero-cost from one build. Repeat across the declared
compiler profiles and inspect release assembly alongside timing.

Security scope is synthetic values in this original local conversion program.
An attacker-controlled magnitude or ratio may expose bad admission, wrong
scheduling units or partial output; the harmless success condition is a wrong
numeric result/sentinel. The repair is checked exact derivation and explicit
precision policy. No external target, real timing service or exploitability
beyond this property is claimed. Physical clocks, non-author review and delayed
learner performance remain unobserved; published-edition access gaps remain.
