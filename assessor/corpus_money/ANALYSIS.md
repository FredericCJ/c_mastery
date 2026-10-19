# X-T17-004 assessor analysis and delayed package

Keep this directory out of learner/source exports. The complete reference is
money.c; the independent model is oracle.py. An ordinary passing suite earns
execution evidence, not level 3 or delayed learner evidence.

## Reasoning and discriminators

1. Units: the tuple (currency[3],scale) is the identity, not just the code or
amount. `cm_add` detects mismatch before arithmetic. A struct wrapper in C can
be forged and has no operator overload; constructor checks are not maintained
automatically. Compare a language library value object with private fields
and checked methods, then a currency-indexed generic type: only the latter can
reject some currency combinations during type checking. Runtime FX rates,
deserialization and scale changes still require policy. C diagnostics enforce
declared pointer/function types; they do not prove unit compatibility.

2. Ratios: using `double` fails at integers above 2^53, while unchecked signed
multiplication can have undefined behavior before a range check. The quotient
decomposition is exact. In `product`, q*n overflowing uint64 implies no signed
result fits because the remaining term is nonnegative. FLOOR increments the
unsigned magnitude only for negative nonintegral results. EVEN compares 2*r
with d; exact half increments iff the magnitude is odd. Negative ties use the
same parity rule. `signed_value` handles 2^63 separately so the cast/negation
never asks C to represent +2^63 as int64. ZERO is truncation, not floor.

3. Allocation: rational floors leave an integer deficit L<n. Ranking the
fractional residuals and allocating L units minimizes the local absolute quota
errors under this one-extra-unit constraint. Input order resolves exact ties;
it is an application policy, not an anonymous fairness proof. Negative totals
are sign-reflections. A sorted list of residuals with stable ties is an accepted
O(n log n) alternative; an original heap is also valid. Fixed iteration selection
is acceptable at 32. The finite oracle uses Fraction and Python arbitrary
integers, not the C quotient decomposition or divider. It includes zero weights,
large sums, INT64 extrema and values beyond exact binary64 integer range.

4. Settlement: validate all destination identities first, stage debit and
credits, publish once all checks and cuts have passed. The mathematical sum
is conserved because the allocated shares sum to amount. Recipient overflow
must restore neither by guessed inverse operations nor partial compensation:
the original state was never published. Failure is not repaired by catching
a sanitizer report. Duplicate destinations are rejected even when repeating a
recipient would conserve the total; the API promises one credit per identity.
An alternative undo log is valid only with a proof covering every fault point,
including failure during undo. It must satisfy the same no-visible-change rule.

## Compiled defect controls

All controls compile and run a selected semantic anchor. Mutants 1/2 respectively
ignore currency compatibility and incorrectly reject adding zero; 3/4 use
ties-away rounding and lose negative FLOOR; 5/6 drop remainders and reverse
allocation ties; 7/8 accept duplicate recipients and publish debit before an
abort. Each must fail a `CM_CHECK` assertion, not compilation or an unrelated
crash. The default learner scaffold also compiles and fails each substantive
anchor. Preserve the distinction from a negative translation fixture.

## Graduated hints and assessment

Hint 1: compare units before amounts and draw the negative number line around
-2.5. Hint 2: represent INT64_MIN's magnitude as unsigned and write M=q*d+r.
Hint 3: stage complete value outputs; prove allocation's deficit bound before
selecting extra units. Log which hint was used. Do not disclose the full model
before the learner submits their invariant and an independent expected result.

For R1 require invalid-code, scale mismatch and INT64_MIN-plus-zero traces and
the static/runtime enforcement distinction. R2 requires the quotient proof,
both signs at even/odd ties and a valid large intermediate result. R3 requires
conservation, bounds and tie-policy explanations plus a counterexample to
permutation invariance. R4 requires the actual allocation/add composition,
every cut, last-recipient overflow and unchanged caller state. Level 1 means
running supplied cases without the relevant argument; level 2 requires the
stated implementation and boundary proof; level 3 requires a fresh changed
assumption, an independently derived oracle and a correct limitation claim.

## X-T17-004-DELAYED: settlement after seven days

After at least seven days, provide only the public header and this task:
transfer 11 minor units from a source containing 20 to three accounts with
weights [2,3,5]; one destination already contains INT64_MAX-2. Derive exact
shares and identify whether the complete operation can commit. Then change
the destination order and choose a cut after the second staged credit. Build
the adapter from the learner's own earlier value/ratio/allocation artifacts;
submit an ownership diagram, conservation proof and all failure traces.

Expected shares are [2,3,6] (fractions .2,.3,.5); with the large account as
the third destination the operation overflows and all balances stay unchanged.
If that account receives the first share, the credit reaches INT64_MAX and can
succeed provided all other balances admit their credits. An explicit cut still
leaves every original balance unchanged. Do not replace actual learner
integration by execution of this reference. An assessor can choose a new
permutation and code/scale mismatch without altering constants as the sole
challenge: require the learner to explain why identity and publication matter.

## Further changed assumption already exercised

The ordinary allocation contract is symmetric for signed totals, while FLOOR
rounding of individual negative quotas is not. Derive [-1,-1,0] for -2 across
three equal weights and compare naive [-1,-1,-1]. Investigate the Alabama
paradox with a separately constructed positive-weight example; this analysis
does not change the implemented contract. If proposing a different policy,
state which quota, conservation, monotonicity or tie property is surrendered.
No physical measurement, external technical review or independent learner
attempt is implied by these author reference tests.

For the required monotonicity counterexample, weights [1500,1500,900,500,500,200] allocate total25 as [7,7,4,3,3,1], but total26 as [8,8,5,2,2,1]. The two weight500 recipients each lose one unit. The exact oracle includes both totals and this vector; neither finite evidence nor the quota proof implies population/total monotonicity.
