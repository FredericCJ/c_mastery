# Fractional credit, paired admission and clock migration

This contract supplies required executable extensions for X-T75-004 R1-R4 and
X-T75-004-DELAYED. Implement `src/fractional_follow.c` using the public
`followthrough/fractional.h`; retain the original integer-bucket exercise and
its inspected source passages. The core is allocation-free ISO C17 using exact
64-bit unsigned arithmetic. All calls on the same limiter are externally serialized.
There is no implicit wait, queue, operating-system traffic control or network
configuration. Test inputs are synthetic ticks and integer costs.

## Representation and exact transition

A bucket has capacity C, refill numerator p, denominator q, integer tokens T,
remainder r and local monotonic `last`. Units are tokens and ticks; exact credit is
`T+r/q`. Require `1 <= C <= UINT32_MAX`, `1 <= p,q <= UINT16_MAX`, `0 <= T <= C`,
`0 <= r < q`, and `r=0` when T=C. Initialization starts full. Time never wraps.
The representation is inspectable for proofs, not permission to modify live fields.

For elapsed e, refill is exactly `min(C, T+r/q + e*p/q)`. No floating arithmetic or
premature per-call rounding is permitted. Prove `(C-T)*q-r < 2^48`, derive the
ceiling time to fill, and compare e with that threshold before multiplication.
When below the threshold, the product and accumulated numerator are bounded by
the missing credit. Saturation discards excess fraction and sets r to zero.
Repeated idle updates therefore agree with one update over their total interval.

`rzf_bucket_consume` rejects invalid representation as `RZ_INVALID`. Backward time
or cost greater than capacity is `RZ_RANGE`, preserving every field. Otherwise
refill and the new `last` persist even when credit is insufficient (`RZ_DENIED`).
Successful admission subtracts an integer cost; zero cost is a valid refill query.
No rejected spend obtains tokens merely by repeating at the same timestamp.
All output destinations must be live writable objects, may not overlap unrelated
live state, and own no active admission authority. An initializer/import rejection
preserves the output. The language does not enforce any of these protocol rules.

## Two limits form one admission decision

`rzf_bucket_pair(average, peak, now, cost)` requires distinct bucket objects. It
validates both inputs, monotonic timestamps and cost bounds before any update.
It computes both refills, then either debits both or neither. A credit denial
commits both refills and their new timestamps; it does **not** roll time backward.
Admission errors preserve both objects. Thus consuming the average allowance
before discovering peak denial is a defect, even though an ordinary integer
bucket call is correct on its own. Both rates and bursts must be specified; labels
"average" and "peak" alone do not imply an ordering between arbitrary parameters.
This is a policer. It neither shapes queued departure times nor caps concurrently
active work.

Counterexample: average C4,p1,q3 is full; peak C2,p1,q4 was drained at0. A cost1
request at1 is denied, leaving average4 and peak1/4. At4 it succeeds, leaving
average3 and peak0. The one-sided-debit control loses average credit on the denial.

## A migration transfers credit, not a timestamp

Export at source time s computes a staged refill and returns
`struct rzf_credit{C,p,q,T,r}`. It does not mutate the source. Before activating an imported
limiter the owner must quiesce and retire the source; a snapshot is not a second
independent admission authority. Source and destination clocks may have unrelated
origins, but use the same tick rate. Import sets `last` to destination time d.
The explicit downtime policy either freezes credit or applies a trusted elapsed
downtime duration before rebasing. Backdating a new process's absolute `last` to
the source clock is never valid. With no trusted duration bound, freeze is the
conservative policy. Neither this snapshot nor its fields authenticate authority.

Concrete input: C2,p1,q3 is drained at source1000, exported at1002 as credit2/3.
Import at destination40 with downtime2 yields 2/3 under freeze and 4/3 under refill;
both have `last=40`. One tick later the frozen limiter admits cost1. Import validates
the complete snapshot before publishing, including the full-bucket remainder rule.
Handoff fencing, durable transfer and Byzantine participants are outside this
in-process arithmetic adapter and remain explicit integration obligations.

## Verification and interpretation

Deliver D1 the saturation and cumulative-admission derivations, D2 the core and
transaction/migration adapters, D3 raw model/control results and D4 the original
delayed attempt plus all 13 cost dimensions. The public fractional test checks
all capacity/p/q1..8 with idle splits1..20, zero cost, huge gaps, malformed migration,
backward time and both admission atomicity directions. The independent Python
model uses `fractions.Fraction`, not the C quotient/remainder algorithm. It checks
51,188 histories, including every word of length0..6 over same-time spend,
one-tick spend, backward time and impossible cost for eight parameter profiles;
5,824 paired histories; and 864 migration cases. The retained integer model remains
separate. Controls discard fractional remainder or debit one budget on denial.

Compare policies using explicit arrival traces and declared units. A fixed-window
quota2 per4 ticks can admit two at3 and two at4; a sliding quota over the preceding
four ticks and a token bucket C2/rate1/2 reject the second pair. These are different
burst contracts, not evidence that one implementation is faster or incorrect.
For equal token-bucket contracts, require the cumulative bound over every interval:
admitted cost is at most initial available credit plus elapsed*p/q, with saturation
only reducing availability. A concurrency cap is a separate mechanism.

Level1 produces approximate nominal refill. Level2 proves the multiplication
bound, preserves exact remainder and refusal state, passes independent models and
rejects both semantic controls. Level3 transfers live credit under changed clock
origins and an explicitly justified downtime/authority policy, independently of
the reference. Hints: write credit as a rational number; next bound time-to-full;
finally stage both refill results before deciding whether either may spend.

Use GCC/Clang debug, optimized, ASan/UBSan and separate TSan builds through the
family runner. TSan does not turn this externally serialized core into a concurrent
API. Allocation, payload destruction and hardware validation are non-applicable
to this arithmetic core; measurement and integration costs still need separately
recorded evidence. Author runs are not a learner's independent delayed assessment.
