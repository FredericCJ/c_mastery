# Assessor analysis: PR-T30-03

The lifted model enumerates candidates now-deadline+k*modulus within the open
half interval. Exactly one determines equal/past/future; exactly half has none.
The C branch computes a residue without a signed conversion. Critically, an
unobserved true separation beyond the admitted horizon is not detectable from
the pair: do not reward a claim that all late observations are rejected. Elapsed
measurement has its different less-than-one-full-wrap premise. The10/266 pair
and the16-bit horizon are independent discriminators.

Rounding uses quotient plus a nonzero-remainder bit; if a remainder exists the
quotient cannot already beUINT64_MAX. The rounded half-range guard follows.
Checked epoch multiplication and nanosecond multiplication are separate. A wide
counter with independently read components can still tear; even atomic fields
do not make a coherent multi-object snapshot.

All shared clock operations are SC. If equal even revisions surround the reader
loads and sequence never wraps, no publication's odd/even pair can be crossed in
the SC order. Writer preflight sees a stable stamp/revision; CAS protects it from
a competing writer. No failing validation publishes fields. The finite3432 event
model validates the mapping for one writer/read attempt and demonstrates350 mixed
snapshots if final validation is skipped. It is not an entire-program proof or
weakest-order analysis. The concrete hook additionally forces the torn state in
actual C. Native stress and TSan are separate scoped evidence, not a proof.

The final sequenceMAX-1 remains readable, while further publications refuse.
Boot change requires increasing external identity. Persistence only preserves
the supplied identifier: CTS1 bytes do not create durable uniqueness, authenticate
the contents or prove reset timing. The separate-process test deliberately uses
new boot8 with a larger lifted count to ensure rejection depends on incarnation,
not merely backwards arithmetic. Physical boot-counter allocation remains outside
this exact criterion's explanation/rejection exercise.

Metric slices form a complete disjoint half-open partition. Summing a subset
cannot overflow because its total is bounded by finish-start; input checks precede
publication. Latency is outside that partition, while response already includes
all internal slices. CPU3/ISR2/queue2 gives7; add DMA1 gives8. Preempted time is not
CPU service. Jitter is signed magnitude from a named ideal, not a synonym for
response. Exact rational mean prevents an unacknowledged truncation policy.

The correlated example has the same multiset, mean and max as the dispersed one,
but different consecutive runs. Actual clock samples may differ due to scheduling,
instrumentation, resolution and system load. Check their arithmetic and workload
identity; do not require a specific empirical maximum or ratio. CPU and wall reads
are distinct observations, so tiny cross-clock differences do not establish a
bug. Explicit sleeps are a controlled workload change, not physical DMA evidence.

Score each of six rubric rows separately. Seven controls isolate the precise
misconceptions; generic failures do not earn credit. Source inspection, reference
execution, human review, physical measurements and unaided learner transfer are
separate dimensions. Preserve all original criterion text and historical status.
