# Private cache analysis

The legacy criteria retain all original tasks. The public contract specifies
the machine; do not award implementation credit for reciting six misses.

For .07a, the first0 must miss even though zero-filled tag storage equals its
tag. Validity is an independent state bit. Four lines/two ways means two sets,
not four sets with twice the capacity. The original four distinct blocks split
as0/4 and1/5 across the two sets; each fits and the two direct-map conflicts
disappear. Invalid late input must leave the prior entire certificate intact.
On the selected sparse/packed field traces, logical values and use count match
while the declared addresses differ; requiring compiler struct layout would be
a different premise. An unfamiliar two-way trace with three same-set blocks
must still evict under the declared LRU rule.

For .07b, 6/9 is an observed trace fraction, while the separate estimate uses
24*(1/4)=6 misses in100 instructions. Equal numeric miss totals do not make
these the same measurement. A31ns total miss includes the1ns hit term; using it
as an additional penalty yields65/3 rather than21ns. The conversion from cycles
at2GHz uses0.5ns/cycle, not division by a unitless2. Conservative rational range
failure is allowed by the explicit contract, but wraparound and partial output
are not. Probe a huge numerator, zero denominator and lower-than-hit latency.

Independent20-cycle intervals with two slots start0,1,20,21 and end20,21,40,41.
With four slots, issue remains one per tick: starts0..3 and last finish23.
With dependencies, finish/start join at20,40,60; slots do not break causality.
The full max chain starts at256 and finishes4352, exercising the last legal
tick. An implementation using `<4352` can fail only after otherwise valid work.
Do not describe these elapsed service-model ticks as actual processor cycles.

For .07c, LRU history after0,1,2,3,0,4 retains0, while FIFO evicts0 when4 is
inserted. The last direct-map0 misses both ways, but the shadow classification
changes. FIFO-relative capacity is not evidence that the LRU reference capacity
was insufficient. Demand a new witness where the target hit sequence is held
fixed and only the diagnostic shadow changes. A repeated block is not always
a conflict miss: the original five-distinct capacity fixture is the counterexample.

The Python oracle uses oldest-first banks, independent distinct-reuse distance
and an event-driven service schedule; the C core uses most-recent-first arrays
and tick iteration. Both are still authored checks of a finite declared model.
Actual original `algo_cache` is extracted unchanged with whole-source and body
hashes. Whole predecessor coverage, new textbook reading, physical caches,
independent review and independent delayed performance are not inferred.
