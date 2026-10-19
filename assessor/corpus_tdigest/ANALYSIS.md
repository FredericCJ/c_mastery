# Assessor reasoning and limits

The original component implements normalized k2, bounded batch/centroid merge,
stable sorting, singleton extremes, an explicit fitted distribution and
coarser recompression. It retains all weights and means, not raw observations.
The public scaffold remains incomplete. No external t-digest implementation
or math library executes the studied mechanism.

The first and last clusters are singletons because k2 diverges at endpoints;
the implementation handles this by refusing nonsingleton boundary groups,
not evaluating log(0). The real-arithmetic preservation argument adds mass
to the left or right of an existing centroid: its log-ratio span cannot
increase, and the positive normalizer is nondecreasing. The inspected proof
establishes a size invariant, not a distribution-free quantile error bound.
Lowering delta also lowers the effective scale; increasing it cannot split
lost observations and is refused. The numerical implementation uses a
conservative combination threshold and validates its staged result, with
Decimal checks providing separate finite evidence.

Each admitted combination preserves integer weight exactly and updates a
convex weighted mean. Because samples are bounded, intermediate differences
are bounded by2e9 and weighted totals by65536; no accepted arithmetic needs
an unbounded floating sum. The per-combination clamp prevents a rounded mean
from leaving its two endpoints. Stable sorting and convex combination keep
means ordered. Stage-before-publish gives complete refusal behavior, including
failure during final validation. The total limit is checked before any sum
of two input digest counts. These facts do not validate arbitrary pointer
provenance or synthesize a raw-data history for a hand-written state.

The fitted distribution can be expressed independently as a finite mixture
of point atoms and uniform interval densities. The oracle constructs that
mixture, sums CDF contributions, and inverts its ordered breakpoints. The C
code instead traverses centroids and local gaps. This catches interpolation
errors independently of exact raw-data error. The empirical baseline uses
strict-less/less-or-equal ranks at the returned value; its interval handles
ties. Error is distance from q to that interval. The raw nearest-rank sample
is also retained for value-error measurement.

The construction oracle selects maximal admissible prefixes of sorted items
then computes means with fsum. It agrees on every fresh single-batch probe.
An attempted stronger equality test across all histories found four pilot
disagreements on repeated/shuffled values: tiny differences in previous
floating means changed subsequent sorting and therefore centroid weights.
Those observations are retained as reference_arithmetic_example/divergences,
not discarded or declared exact matches. All such histories still undergo
the independent scale, weight, endpoint, mixture and empirical-rank checks.
No claim of bitwise determinism across other arithmetic implementations is
made. The contract specifies the C recurrence and stable tie policy exactly.

There is a direct merge-order witness even without changing arithmetic:
four ascending ramp partitions merged forward and backward produce different
low-quantile estimates. The campaign retains the raw512-value stream, batch,
compression, tree choices and both query vectors. Count conservation does
not imply associative grouping or equal error. The delayed two-level route
executes both leaf32→target8 and leaf32→target32, alongside leaf8→target8;
it is actual recompression and merging, not an unexecuted prose suggestion.

The repeated-zero/sparse-tail case is deliberately unfavorable. A valid large
cluster can contain many zeros and a few distant values; its positive mean
and interpolation can misplace a requested rank badly. The observed maximum
rank error.625 is preserved, so claims of1/delta worst-case accuracy, unbiased
estimates, or automatic improvement for every input are unsupported. This
package teaches how to inspect and bound the actual mechanism while reporting
its failure of an overstrong quality claim. It does not certify a useful
production accuracy target.

Mutant1 removes the scale admission condition, after which final validation
refuses a batch expected to succeed; mutant2 returns the maximum for q=0;
mutant3 discards weights in a mean combination. Public assertions distinguish
these causes. A memory fault or compiler failure does not count as rejection.
The private oracle exercises570 distinct workload/order/compression/batch/tree
cases and30 fresh construction probes within that set, with exact source and
command receipts. This finite author evidence is separate from reviewer and
learner independence, delayed human work and physical execution.
