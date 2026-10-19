# PR-T37-08: equivalent work, dimensional models and native measurements

This required successor implements the exact T37.10a/b/c ordinary tasks and
their original seven-day variants. It reuses the actual C `algo_lower`, checked
assembly admission `ab_checked` and original T36 `algo_asm_lower`. Preserve
their immutable sorted-input and storage contracts. The ancestor's nine-sample
single-C harness was insufficient for this paired experiment. Historical hours
and receipts remain unchanged; the new workload is additive.

## Prerequisites, execution and sources

Bring completed PR-T36-01/02 lower-bound invariants, original C and assembly,
and ABI admission evidence. `prior_learner.c` is an explicit placeholder for
those learner artifacts. Implement the performance/statistics adapter without
a library that supplies its rational arithmetic or order statistics. Read
`performance.h`, the parent project/fixture, and the incorporated
`labs/algorithm_abi/CONTRACTS.md`. Relevant inspected primary records and the
exact selected prior C/assembly source passages are in
`research/v5/expansion/legacy_architecture_performance/resources.json`.

The pure component is C17/Profile C with `uint64_t` arithmetic and no allocator,
I/O or OS dependency. Audit actual object imports and compiler helpers. The
measurement harness is Linux SysV x86-64/Profile A: libc, GNU `sched_getcpu`,
Linux `CLOCK_MONOTONIC_RAW` and thread CPU clocks are explicitly outside ISO C.
GCC/Clang optimization and sanitizer profiles have separately labeled results.
No C edition makes hand assembly automatically instrumented, validates a
pointer's allocation, or turns elapsed time into a cycle/energy guarantee.
Final published C17/C23 source gaps remain open.

## Required component contracts

Implement checked reduced unsigned rational arithmetic internally. A ratio has
positive denominator; zero reduces to0/1. Cross-cancel before multiplication,
detect remaining overflow and leave the previous output untouched on all
rejections. Reject zero instruction count/CPI/frequency/speed, invalid fraction
outside0..1, null outputs and nonrepresentable intermediates. Such bounded
arithmetic rejection is not a claim that the mathematical value does not exist.

`pe_equation` returns exact baseline nanoseconds, new nanoseconds, whole-task
speedup and the infinite-local-speed limit. The improved fraction denotes
baseline time. Add the supplied nonnegative fixed communication nanoseconds
**after** accelerated local work; the fixed cost also affects the infinite-speed
limit. Represent an unbounded limit explicitly, never by denominator zero.
Derive dimensions before coding. Original inputs count1200000,CPI3/2,2GHz,
fraction3/5,speed3 give900000ns,540000ns,5/3 and5/2. Investigate communication
costs0,359999,360000,360001,500000ns, full/zero improved fractions, fractional
nanoseconds and overflow. Determine when acceleration makes the whole job slower.

`pe_stats` borrows a nonempty1..128-element sample array, preserves it, and
returns minimum, maximum, floor of the median and a Boolean half-unit. Sort a
local bounded copy; for even counts compute the middle mean without overflowing
even when both values are UINT64_MAX. Invalid input preserves output. Do not
discard high samples. Use the parent's explicitly synthetic five-sample lists:
their medians are100 and95ns, ranges96..110 and93..120. These are reporting
fixtures, never measured hardware evidence. Component costs: Euclidean GCD is
bounded by the input bit width; insertion-sort statistics use O(n²) comparisons
and128 uint64 storage slots. State actual stack/code size separately.

`pe_c` and `pe_asm` expose identical checked lower-bound behavior for live
immutable `uint32_t a[n]`, n<=1024. Reject missing output, null nonempty data,
oversized count and descending input without output publication. Both validate
sortedness for every call; the C side calls the actual `algo_lower` with a null
work counter, the assembly side calls actual `ab_checked` with capacity=n and
trusted=0. A trusted raw assembly loop versus checked C is a different contract
and is rejected here. Time the whole matching checked APIs, not an alleged
isolated instruction-loop cost. Caller-provided storage extent, alignment,
distinct output and absence of concurrent mutation remain obligations.

## Native experiment and concrete variants

Use two sizes16 and1024 with repeated sorted keys `a[i]=(i/2)*8388608`; queries
are the64 evenly spaced uint32 keys ending UINT32_MAX. Verify every query
against a separate linear oracle before timing; verify each timed batch's
checksum outside its interval. Also compare every size0..1024 with five boundary
keys, and rejection behavior, before collecting any performance conclusion.
Record validation comparisons and actual C search comparisons separately.
Assembly has the same derived interval invariant; do not relabel that as a
measured assembly hardware-instruction counter.

For each size, run two preparation policies independently: an untimed warm
batch, and an8MiB volatile stride64 displacement pass before each timed batch.
The latter is **not proof of a fully cold cache**: subsequent queries reuse the
data and cache topology is not controlled. Holding size fixed changes only this
preparation intervention; holding preparation fixed changes size. Explain which
limited comparison addresses the proposed causal question and what confounding
remains. Record exact binary/CPU/ABI/compiler/options and deterministic inputs.

Read clock resolution and101 back-to-back timestamp intervals; retain every
overhead observation. Calibrate common batch repetitions for both kernels at
each size/policy by powers of two1..512, targeting at least50us,1000 resolution
units and100 times the largest calibration clock overhead. Keep bounded failure
or insufficient resolution explicit. Do not subtract overhead blindly. Normalize
only by the disclosed number of queries/repetitions, preserving raw totals.

Collect31 paired blocks for each size/policy, alternate C-first/assembly-first,
and retain all248 intervals, order, repetitions, endpoint CPU numbers, thread
CPU time and checksum. Equal endpoint CPUs do not prove no intervening migration.
No affinity, host clock/frequency or interrupt policy is changed. A recorded
wall-clock nanosecond interval is not a CPU cycle counter. Sanitized results
include instrumentation and must not be substituted for optimized unsanitized
timing. There is no required speedup threshold.

For `.10c-V1`, analyze paired normalized differences/ratios and unpaired means
from the actual data. Also construct a labeled synthetic drift counterexample:
C cost100,assembly95, common block drift100i for i0..30. Contrast pairing in
one block with taking all C observations before all assembly observations.
Then add a ten-unit within-block order effect and show where pairing also
fails. Do not present this model as measured drift or infer causality from
averages without its assumptions. For `.10a-V1`, retain the communication-cost
break-even derivation; `.10b-V1` is the independent size/preparation intervention.

## Deliverables, verification and rubric

D1: exact formulas, units, overflow admission proof, ratio/statistics source,
pre/postconditions, caller ownership and costs. D2: actual imported producer
identities, matching checked wrappers, disassembly showing retained calls,
component import/stack/code audits and5125 result comparisons. D3: original and
changed exact-rational/ordinal packets, a separate Python Fraction/statistics
oracle, invalid-output preservation and discriminating flawed variants.
D4: reproducible native harness and raw clock/calibration/248 sample records,
CPU/tool/binary identities, normalized medians/ranges and paired summaries with
every outlier retained. D5: ordinary/V1 conclusions that distinguish algorithmic
work, native time, synthetic models and unobserved hardware properties.

Reject unit scaling, omitted communication cost, dropped maxima, trusted-only
assembly admission, signed assembly ordering and a timed loop with work removed.
Each rejection must identify the first semantic assertion or checksum failure,
not just a successful compiler exit. Verify each report against current source,
binary and raw-output hashes. Unavailable clocks/failed calibration leave the
corresponding observation open; do not fabricate a replacement sample.

Assess .10a for dimensional/rational derivation and break-even counterexample;
.10b for equal contracts, retained real work, independent size/preparation and
31 samples per condition; .10c for all raw data, correct summaries and bounded
interpretation under drift. Scores0/1/2/3 distinguish absent/wrong, memorized
fixture, derived and verified mechanism, and independently completed transfer.
The last requires an actual unaided delayed learner attempt at least seven days
after study. Author tests never confer that score or prove absolute mastery.

Hints progress from (1) write seconds/instruction and time fractions with units,
to (2) separate validation/search/timer costs, to (3) draw the chronological
blocks before taking means. Record hint use. Assessor derivations are separate.

## Containment and evidence limits

Use original synthetic arrays and local native processes only. Flawed kernels
are deliberate teaching controls; no third-party target or host setting change
is part of this task. No physical Cortex-M measurement, WCET, energy, universal
ranking, external review or independent learner claim follows. Source-bound
author execution and source-access status are recorded separately in the
successor report; correct code may be slower and still satisfy the assessment.
