# Exact values, allocation and publication

This required refinement uses original arithmetic and synthetic accounts. The
Money catalog entry by Fowler motivates two distinct defects: mixed currencies
and lost minor units. The operations and proofs below are this curriculum's
contract, not a claim that one rounding or allocation policy suits every market.
The accounting draft motivates a transaction containing related entries; this
exercise implements an in-memory transfer, not an accounting standard or ledger.

## Representation and admitted environment

`struct cm_money` contains signed 64-bit minor units, three uppercase ASCII currency
bytes and a decimal scale 0..9. Codes are labels, not a currency database: USD/2
and USD/3 are incompatible. There is no exchange-rate inference, silent rescale,
saturation or floating conversion. `cm_make` checks representation; public C
structs remain forgeable, so every operation checks tags again. Integer types
are exact-width on the stated 8-bit-byte Linux and Cortex-M targets. Array spans
must be live, properly aligned and large enough. Apart from scalar output alias
with an input value, output arrays and input arrays must be disjoint. No API can
portably diagnose an arbitrary dangling pointer. Calls on shared accounts must
be externally serialized. Ordinary `const` is neither ownership nor isolation.

The core allocates no heap and calls no host service. Host tests use stdio,
Python `Fraction`, clocks and sanitizers. Profile C target audits permit only
explicit original `assessor/execution/support.c` memory helpers. They inspect
actual objects, linked symbols, assembly, size and stack reports; no libc,
libgcc, compiler-rt or libatomic archive is permitted. A linked image is not a
board observation. C17 is the baseline; the optional C23 checked-arithmetic
comparison must preserve the same exact contracts and does not make currency
labels static types or a sequence of writes transactional.

## Operations and failure behavior

All successful value operations preserve the currency and scale. On any
non-OK return, every caller-visible output or account is unchanged, including
earlier recipients. The caller owns every input and output. `CM_INVALID`
denotes malformed representation, zero denominator, mismatched unit, invalid
count/index or duplicate recipient. `CM_OVERFLOW` denotes nonrepresentable
result or a weight sum above UINT32_MAX. `CM_FUNDS` denotes insufficient source
balance; `CM_ABORTED` denotes an explicit test cut before publication.

`cm_add` checks a signed addition before evaluating it. Adding zero to INT64_MIN
must succeed. Do not negate INT64_MIN to obtain its magnitude: unsigned
conversion followed by modular subtraction gives 2^63 without signed overflow.

`cm_ratio(a,n,d,mode)` computes the exact rational minor-unit value a*n/d for
uint32 numerator and nonzero uint32 denominator. Modes are toward zero, toward
negative infinity, and nearest with ties to an even integer. Reject an out-of-
range final result. Intermediate a*n need not fit 64 bits: INT64_MIN multiplied
by UINT32_MAX/UINT32_MAX still produces INT64_MIN. A wrapped intermediate or
rejection of that valid result is a defect. Examples: 5/2 rounds to 2 with EVEN,
7/2 to 4, -5/2 to -3 with FLOOR and to -2 with ZERO.

`cm_allocate(a,weights,n,shares)` admits 1..32 nonnegative uint32 weights whose
sum is nonzero and at most UINT32_MAX. Take exact proportional shares of |a|,
floor each, then give each remaining minor unit to the largest fractional
remainder, breaking ties by earlier input index. Restore the sign afterwards.
Zero-weight recipients receive zero. Thus allocating 2 to [1,1,1] produces
[1,1,0]; allocating -2 produces [-1,-1,0]. This signed symmetry differs from
flooring negative shares independently. Sum of shares equals a exactly, every
share differs from its real quota by less than one, and no minor unit vanishes.
Do not promise invariance under permutation when equal remainders tie: index
order is a declared allocation policy. This policy also does not promise
monotonic shares when the total changes (investigate the Alabama paradox).

`cm_settle(accounts,count,from,destinations,weights,recipients,amount,abort_after)`
is the implemented changed-condition composition. Count and recipients are
1..32, all destination indices are distinct and different from `from`, all
accounts have valid representations, and involved accounts match the amount's
unit. Amount is nonnegative and source balance must cover it; uninvolved valid
accounts may have different units. Use actual `cm_allocate` for recipient
shares and checked `cm_add` for credits. A recipient may already be INT64_MAX.
Validate destination identities before applying any credit. An abort value of
0 fails after staging the debit; i in 1..recipients fails after the ith staged
credit. SIZE_MAX disables cuts. No intermediate stage becomes public. There is
no callback, allocation, journal, atomic instruction or crash durability. The
sum of all involved mathematical integer balances is conserved after success,
even when that sum itself would not fit int64. Do not sum in int64 as a proof.

## Derivation and cost model

For nonnegative magnitude M and divisor d, write M=q*d+r with 0<=r<d. Then
floor(M*n/d)=q*n+floor(r*n/d), with remainder (r*n) mod d. Since r<d<=2^32-1,
r*n fits uint64; check q*n and the sum before evaluating them. The original
divider maintains processed_prefix=q*d+r, r<d, consuming one bit per iteration.
Its shifted remainder needs at most 33 bits. Convert the final magnitude back
only after comparing with INT64_MAX or 2^63 as appropriate. This derivation
avoids both floating rounding and reliance on nonstandard `__int128`.

For allocation, sum of exact quotas is M. The integer deficit is nonnegative
and less than the number of positive weights; one extra unit per selected
remainder suffices. Prove that the accumulation of floor quotas cannot exceed
M, then justify its uint64 storage. For settlement, publication occurs only
after all staged operations succeed. Its proof assumes no concurrent observer
and valid disjoint storage; it does not follow from the word "transaction".

The reference uses O(n) stack and O(n^2) remainder selection for n<=32. The
divider uses exactly 64 bit iterations, independent of the numeric value;
this alone is not a constant-time security claim because other branches vary.
Measure 1,2,4,8,16,32 recipients at equal policies and input values. Explain
which speed comparisons would be invalid if an alternative silently drops
remainders or uses a narrower range. Inspect call-chain stack, not just one
function's local frame. A fixed array consumes its maximum frame even when
only one recipient is active.

## Progression and laboratory boundaries

First derive the rational and signed boundary rules on paper. Construct value
validation and checked addition, then ratio rounding, then allocation, and
finally staged settlement reusing those exact operations. Public scaffolds
deliberately return refusal. Private implementations and the oracle are never
copied into learner workspaces. An explicitly selected assessor directory is
required to run the private comparison; absent providers must fail configuration.

The attacker controls synthetic amount, scale, code, weights, recipient indices
and the chosen cut. A bounded demonstration succeeds by showing one of: mixed
unit acceptance, a dropped minor unit, duplicate credit, or a debit retained
after an aborted/overflowing transfer. None is arbitrary code execution. Keep
the experiments in this original process with no financial APIs, credentials
or external services. Repair admission, arithmetic or publication and retain
the exact regression. Bounds and rejection checks mitigate these demonstrated
defects; they do not authenticate an account owner or authorize a transaction.
