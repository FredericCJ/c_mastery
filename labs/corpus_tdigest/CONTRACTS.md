# Bounded k2 merging t-digest

Implement the original API in tdigest.h using C17 and the unsolved src/tdigest.c.
This is a merging t-digest with the paper's normalized k2 scale, singleton
extremes and an explicitly specified fitted distribution. It is not an exact
quantile container, an implementation of every Java reference policy, or a
distribution-free accuracy guarantee. The source paper's broad accuracy
language must not be substituted for a proved bound on this finite component.

## Domain, representation and ownership

The caller supplies live complete objects, serializes access and preserves
pointer validity. Output pointers and input spans are disjoint from the state.
Exact self-merge is allowed; partially overlapping objects are not. No pointer
is retained and no allocation or callback occurs. A raw sample is a finite
double in [-1e9,1e9]. A batch has0..64 samples; NULL is accepted only for an
empty batch. Compression is an integer8..32. Total admitted weight is bounded
by65536 and storage by128 centroids. Returns are TD_OK, TD_INVALID and
TD_CAPACITY. Every refused operation preserves destination and output bytes.

Each centroid stores a double mean and positive uint32 weight; their means
are nondecreasing. Weight sums equal count. Empty state has zero count/used.
Every nonempty endpoint centroid is a singleton. The mean represents a group,
not its raw members or a guaranteed median. Input-cluster ranges can overlap
after repeated processing; sorting means does not establish strong ordering
of all original samples. State is not a portable serialization format.

td_valid checks numeric configuration, finite means, sortedness, exact bounded
weights and the numerical k-size condition. It cannot certify that a caller
invented state summarizes real observations. Caller pointer/lifetime/race
premises are not validated. Double has at least53 significand bits and the
declared exponent range; compilation checks this. Fast-math assumptions are
not part of the contract. Hosted floating support is used; no target runtime
closure or board observation is inferred.

## Construction and scale

For total n, define Z=4*ln(n/delta)+24 and
k(q)=delta/Z*ln(q/(1-q)). For a nonsingleton whose cumulative left weight is L
and own weight W, k-size is
delta/Z * [ln((L+W)/L)+ln((n-L)/(n-L-W))]. A group touching rank0 or rank1 cannot
be a nonsingleton. Singleton groups are admitted separately. This distinction
avoids evaluating logarithms at endpoints.

td_add stages existing centroids followed by new unit-weight samples, stably
sorts by mean, then greedily combines adjacent items from left to right. Equal
means retain their previous sequence order. A combination is admitted only
when its computed k-size is at most1-1e-10. Weighted mean updates use the
documented floating recurrence a+(b-a)*wb/(wa+wb), clamped to[a,b] against
rounding escape. The staged result must pass td_valid, whose numerical
tolerance is1+1e-10, before publication. A buffer-limit or total-limit failure
is refusal, never silent sample loss. Empty add is a no-op.

The logarithm uses range reduction to[1,2) and24 atanh-series terms. Its
real-arithmetic truncation bound and accumulated floating rounding are
separate. The paper's invariant proof is about real arithmetic; this
implementation's conservative admission and numerical validation are finite
policies, not a correctly rounded proof implementation. The oracle separately
checks emitted k-sizes with50-digit Decimal logarithms. No math-library
implementation supplies the component's scale function.

## Quantile and CDF semantics

Define a fitted distribution rather than an unspecified interpolation rule.
A singleton contributes one unit point mass at its mean. Between adjacent
means a,b, spread mass (wa>1 ? wa/2 :0)+(wb>1 ? wb/2 :0) uniformly across[a,b].
If a=b, that entire gap mass is a point mass there. All masses sum to count
because endpoints are singletons and each interior nonsingleton contributes
its two halves. This definition handles repeated means and singleton steps.

td_cdf returns the right-continuous fitted mass at or below x divided by count.
It accepts any finite double x. td_quantile is the generalized inverse for
q in[0,1], with q=0 and q=1 returning the exact minimum/maximum. At a jump, all
covered ranks return that point. Empty state and NaN/invalid query refuse
without changing the caller result. For samples0,10, q=.5 returns0 and CDF(5)
is.5; this is not the linear type7 sample-quantile convention.

The fitted distribution is an approximation to the empirical distribution.
Do not check accuracy by comparing only a quantile with this component's own
CDF. Keep the exact raw-data sorted baseline, state a tie-aware empirical
rank interval and measure its distance from the requested q. Value error and
rank error are different, especially on sparse tails. Larger compression
does not establish a universal error bound.

## Merging and required delayed transfer

td_merge stages all destination centroids before source centroids and uses the
same stable greedy construction. Destination compression must be no greater
than source compression. The source is unchanged. Self-merge represents the
multiset duplicated, subject to the total bound; it is not idempotent.
td_recompress admits an equal or smaller compression, preserving count and
all represented weight. No operation increases precision or reconstructs raw
observations from a centroid.

The required delayed task changes from one direct stream to independently
processed partitions, both left/reverse folds and a balanced merge tree.
Implement td_recompress and the two-level route: build leaves at32, merge
compatible digests and compress to8. Compare it with leaves built at8 and
with a retained32 result. Vary batches1,17,64 and ascending, descending and
shuffled orders. Neither associativity of resulting centroids nor equality
of quantile answers across merge trees is promised. Weight preservation is
required in every route; error and resource observations must accompany it.

Floating arithmetic is another ordering condition. An independent model that
forms each mean with fsum can round a nearly tied centroid differently from
the stated recurrence, changing a later stable-sort order and grouping. The
campaign retains such disagreements explicitly. Full independent greedy
partition equality is asserted for30 fresh single-batch probes, while all
570 stream/merge experiments undergo independent centroid constraints,
weighted-sum checks, fitted-distribution checks and raw-data error analysis.
This is a documented boundary of cross-arithmetic equality, not a claim that
every alternative model gave identical histories.

## Verification, costs and assessment

Use GCC and Clang debug/release/ASan+UBSan. Public tests cover singleton tails,
NaN/Infinity admission, empty queries, weighted means, equal-value CDF,
compression mismatch, recompression, self-merge and total exhaustion with
unchanged snapshots. The private compiling controls remove the k-size rule,
return a wrong endpoint and use an unweighted mean. Each must fail TD_CHECK,
without a sanitizer failure. Default learner sources must compile and fail
the same intended assertion. Explicit completed providers are supplied through
TD_PROVIDER or CMASTERY_ASSESSOR_ROOT; private code never ships to learners.

An insertion-sort stage of at most256 items gives a checked finite O(m^2)
sort bound. Scan, validation and scale costs must be included; no constant-time
update claim is made. Merge staging is256 centroids plus a128-centroid result;
add staging is192 plus128; caller states and compiler stack overhead are
additional. Measure actual sizeof and emitted frame usage before a target
port. Fixed arrays do not prove a small stack on every ABI.

Report the large-error examples, not only favorable smooth data. The pilot's
low-compression repeated-zero/sparse-tail case reached rank error.625 under
the defined empirical interval metric despite valid k-size. A smaller error
on another workload is not a worst-case bound. Separate source inspection,
author experiments, outside review, unaided learner performance, seven-day
delay and physical evidence. None of the latter is supplied by these tests.
