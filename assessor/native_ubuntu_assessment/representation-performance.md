# Withheld packet: lookup service with uncertain bottleneck

## Prerequisite check

Require accepted ordering/search, comparator/ownership, layout and PMU protocol
artifacts. Have the learner classify a zero count with positive running time,
a zero-running group and a denied event before any hardware experiment. Record
the actual hardware-access state; lack of access does not waive the protocol gate.

## Initial task to release

A service holds immutable records with a 64-bit key, stable 32-bit ID and 48 bytes
of payload. Queries request either the first matching ID or all IDs in a closed
key range, preserving increasing key then ID order. Keys may repeat. Updates
arrive in batches while existing readers retain their old snapshot. Workload A
is 90 percent unsuccessful point lookup, 10 percent short range queries; workload B
is 20 percent point lookup, 80 percent long ranges. Storage is bounded to 128 MiB
including any copied snapshots and indexes.

Select a representation and define its ownership/snapshot boundary. State two
competing bottleneck hypotheses, predict a discriminating size or workload change,
then inspect generated code and measure. Preserve identical query results and
consumer-visible semantics. An index build may be amortized only over an explicitly
stated number of queries; report construction separately. Do not assume any named
representation or hardware event is available.

## Assessor cases, not released initially

Use tagged duplicates at minimum and maximum keys, missing keys adjacent to the
bounds and an empty range. Retain a reader while publishing a replacement whose
allocation fails halfway. Old readers must keep valid data and no partial new
snapshot may publish. Sweep record counts 1, 16, 256, 4096, 65536 and 262144 if the
measured storage bound admits them, plus actual cache-boundary regimes chosen by
the learner. Keep randomized query seeds and an independent sorted-sequence oracle.

Introduce an intentionally misleading run where one candidate omits payload
consumption, and a second run where the hardware group never schedules. The
learner must reject these observations before interpreting a performance winner.
Observe whether correctness checks pollute the declared measured interval.

## Changed condition to withhold

After the delay, records become mutable under a single writer, stable ID must
survive relocation, and most queries return one very large equal-key group.
The byte budget is reduced to 16 MiB. Require a new cost/ownership argument and
an alternative representation, or a defended decision to retain the old one.
A changed contract that simply discards duplicates is not an optimization.

## Scoring discriminators

Mechanism 2 connects the representation to every query and update; 3 provides a
lower-bound or amortized argument under explicit assumptions. Correctness 2 uses
an independent oracle and keeps equal semantics; 3 rejects symmetric oracle defects.
Ownership 2 preserves readers and failed publication; 3 explains identity through
relocation. Resources 2 counts payload/index/snapshot storage; 3 treats capacity and
failure atomicity together. Diagnosis 2 retains raw enabled/running metadata and
qualified interpretations; 3 distinguishes competing causes with a controlled
experiment. Transfer 2 adapts independently; 3 explains a rigorous inconclusive
result without inventing universal performance rankings.
