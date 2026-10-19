# X-T43-001 exact paired variants

Keep the original s_paired lower-median and signed64 difference contract. The
benchmark imports that completed producer and two separately compiled equivalent
sum kernels. Each timed operation performs64 calls over4096 initialized words,
checks the independent expected sum536739840 and consumes the result. No LTO
is used. Sixteen warmup pairs precede32 measured pairs. A recorded32-bit LCG
shuffles16 AB and16 BA choices. Durations use CLOCK_MONOTONIC in nanoseconds;
record its actual resolution. Zero/equal times and either performance ordering
are valid observations. Preserve every pair, order, seed and compiler profile.

Implement the host-only percentile bootstrap over the paired B-A differences.
Each replicate draws n paired indices with replacement and computes the lower
median; sort the replicate medians and select floor((R-1)*0.025) and
floor((R-1)*0.975). R40..4096, n1..32, seed0..2^32-1. Use the specified LCG for
reproducibility, not cryptography; index=floor(state*n/2^32) is slightly
biased finite generator unless n divides2^32. Preserve the replicate list.

The interval is a declared resampling summary. Nominal95% terminology does not
establish95% coverage for these observations: serial dependence, warmup drift,
nonstationarity, quantization and selection bias can invalidate an IID paired
sampling premise. Randomized AB/BA balances systematic order placement but
does not remove every trend or prove causality. Compare order-specific differences
without treating that post-hoc contrast as a universal confidence theorem.
Bootstrap resamples pairs, not separate A/B arrays. A sampled maximum is not WCET.
