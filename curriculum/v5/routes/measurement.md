# PMU observations test hypotheses

Begin with `labs/performance_followthrough/PMU_PROTOCOL.md`. First establish
protocol competence using the injectable acquisition interface and deterministic
rejection paths. Then attempt a hardware campaign on the actual identified Linux
machine. Finally use controlled changes to distinguish competing explanations.
A denied event can validate rejection behavior but cannot establish a hardware
measurement or microarchitectural diagnosis.

Before measuring, identify CPU/topology, affinity, kernel/perf versions, PMU
domains, event definitions and permission restrictions. Do not silently change
host security policy or substitute software events for hardware events. A strict
group either retains every requested event or fails as a unit. Keep IDs, raw
counts, enabled time and running time; zero running time is not a measured zero.
Scaling and ratios require their own validity checks.

Use the same recurring components: sort/search the record corpus; compare layouts;
exercise pool placement; run scalar/numerical kernels; vary shared-state placement;
compare callback dispatch; and measure bounded service traces. For each, record
two competing hypotheses and an observation or controlled change that could reject
one. Inspect generated code and consume results. Keep correctness checks and
serialization outside the measured interval where that matches the declared scope.

Sweep bounded sizes through actual working-set regimes; warm up, repeat, vary
order and record interference. Compare supported event measurements with standard
perf under matched scopes, accepting run-to-run variation. No universal cycle,
cache-miss or performance-order golden values belong in the tests.

Sampling adds attribution uncertainty: binary/symbol identity, lost data, skid
and available precision controls. An instruction-address sample does not by itself
prove the sampled instruction caused a cache miss. Target-specific advanced
classification is used only after verifying its event formulas for the detected
processor; unavailable support remains a boundary.

Transfer supplies an observed slowdown with equal visible output. Require a
workload check, two plausible causes, a discriminating experiment and an explanation
of what remains unknown. A rigorous inconclusive result is acceptable. Unsupported
causal certainty fails even when the timing numbers look persuasive.
