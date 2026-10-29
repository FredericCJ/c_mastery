# Telemetry: the query defines acceptable information loss

Start with the existing bounded record stream. A circular history keeps an exact
recent window; a monotonic deque answers a moving extremum while discarding values
that cannot become candidates. Prove why each item enters and leaves at most once
before claiming amortized linear processing. A single update can still remove
many items, which matters to a bounded-latency consumer.

Selection, histograms and probabilistic sketches serve different queries. Preserve
exact small independent oracles. Derive rank and partition obligations for exact
selection; state hash assumptions and adversarial limits for Count-Min, HLL and
MinHash. Quantile approximation error observed on one distribution is not a
universal error guarantee. Merge associativity and storage reduction need explicit
contracts and counterexamples when floating arithmetic changes them.

Numerical kernels add rounding, saturation, overflow and conditioning. PR-T17 and
T64 preserve a scalar mathematical expectation before vectorization. Inspect the
actual generated operations; fast reassociation or saturation at a different step
can change semantics. Check results outside the measured interval, consume the
computed output and avoid making the entire workload volatile merely to retain it.

Reuse telemetry in the service and firmware gateway, where dropped samples,
clock wrap and lost PMU records change interpretation. Record those losses with
the summary instead of presenting incomplete data as a complete population.
Transfer changes the requested statistic, precision and byte limit. Require the
learner to select what to retain and what to discard and show which old queries
can no longer be answered honestly.
