# T53.10 — owned partitions, ordered reductions and measured overhead

## 1. Identity and families

This required successor supplies `AX-PR-T53-10-T53.10.a`, `.b`, `.c`, each
original `-DELAYED` variant, and the parent histogram/false-sharing transfer.
Families A/B/C/D/F/G/H/I/J assess separate partition, arithmetic, publication,
failure, cost and changed-condition obligations. Historical IDs/hours/receipts
remain unchanged; finite author tests are not an independent learner assessment.

## 2. Objectives and inspected references

`.a` proves exact interval ownership and fixed order for a noncommutative
operation. `.b` implements checked partials, completion publication and failure
without a misleading total, then distinguishes floating accuracy from order
reproducibility. `.c` verifies each index/CPU identity and measures equivalent
work including launch, completion and combination. The source register is
`research/v5/expansion/legacy_smp_reduction/resources.json`: bounded OpenCilk
work/span concepts, Linux join/clock contracts, C draft synchronization, and
actual retained `parallel_user.c`/`user_v2.c` are inspected. OpenCilk is a
conceptual comparison, not an allowed implementation dependency. No physical
cache result or exact unavailable published C17 passage is implied.

## 3. Prerequisites and artifacts

Provide the original `PR-T53-09` ownership argument, completed `PR-T53-02`
startup and the original CMK/T56 worker substrate. Supply an independently
predicted partition fixture before running it. The guest adapter requires
explicit completed kernel/startup paths. Keep the existing dynamic257-job
cursor producer separate from the new static-range implementation. Explain
the dynamic producer's fetch-add ownership, per-worker partials, completion and
affinity readback; its fixed unsigned input cannot establish arbitrary signed
or floating arithmetic or general partial-launch cleanup.

## 4. Environment and dependencies

Active C17, GCC/Clang debug/release/ASan+UBSan; native pthread runs additionally
use both TSan implementations. The core requires uint64_t/int64_t and size_t
no wider than uint64_t. Host Python provides mathematical integer expectations.
No reduction, partitioning, histogram or solver library implements the mechanism.
Target integer experiments use the original no-network QEMU TCG x86-64 guest,
1/2/4 CPUs, explicit startup and no hosted runtime. Audit ELF undefined symbols,
compiler helpers and real generated atomics. The hosted floating experiment
selects binary64, default round-to-nearest and no fast-math/reassociation; it is
not linked into the general-register-only guest.

## 5. Problem, supplied inputs and outputs

Implement the public core and floating stubs plus the guest adapter. For n
elements and p in1..4, let q=n/p and r=n%p. Worker i owns a consecutive interval
starting at `i*q+min(i,r)` with length `q+(i<r)`. Derive why these expressions
cannot overflow size_t under the stated bounds, including n=SIZE_MAX without
actually allocating that many objects. n=0 permits null input/ledger and empty
workers; otherwise real extents must cover n live elements.

The original1..10, p=3 fixture yields [0,4),[4,7),[7,10), partials10/18/27 and55.
The separate squared1..257 input yields5691265. Modes are unsigned64 checked
sum, signed64 checked sum, and byte values0..15. Byte mode computes sixteen
histogram bins and an ordered polynomial summary: start H=0, P=1; each byte b
updates H=257H+b+1 and P=257P modulo2^64. Join adjacent summaries as
`H_left*P_right+H_right`. This join is associative but not commutative; reversing
worker completion must not reverse range order. Hash equality alone never
replaces the explicit visit ledger or full bins.

`sr_init` receives caller-owned world, input and atomic visit storage. Work slots
are claimed once; duplicate `sr_work` is BUSY. `fail_after` injects terminal
failure after that many local completed elements; SIZE_MAX disables it. Failed
work and overflow never publish an accepted total. `sr_finish` waits logically
for all terminal slots, returning INCOMPLETE otherwise; it preserves output on
every error and combines in increasing worker/range order on success.

## 6. Why this work is necessary

Correct total55 does not prove exact ownership: [1,1] visited at indices[0,0]
has the same sum as [0,1]. Unequal interval lengths are valid; missing a remainder
is not. Mathematical associativity does not define overflowing signed C addition.
Separate safe local sums may overflow during their combination. A completed
counter or CPU count does not prove the intended work ran on distinct CPUs.
Floating order, numerical error, race freedom and speed are separate properties.

## 7. Reasoning, ownership and failure contracts

Inputs remain immutable/live through every worker; world, ledger, inputs and
output do not overlap. Each slot has one writer after an atomic claim. A release
terminal state publishes its private value/error; finish acquire-observes every
terminal slot before reading plain partials. Atomic visit counts make intentional
overlap controls observable without introducing a C race. Reuse/initialization
requires all callers joined/quiescent and no stale worker invocation. Public
representation mutation is prohibited by contract, not enforced by C opacity.

Checked sums refuse any unrepresentable intermediate in the selected local and
fixed combination order, even when an unbounded mathematical rearrangement
would end in range. Signed checks execute before addition; no overflow UB is
used as a diagnostic. Terminal failure retains input ownership until every
caller finishes. Partial thread creation releases the start gate in abort mode,
joins exactly the created threads and leaves the result unchanged. Unexpected
platform failure is a failed observation, not a passing injected failure.

Range construction is O(p), local total work O(n), and combination O(p*B+n)
with B=16 including the explicit O(n) ownership-ledger audit. The ledger costs
O(n) extra storage and can change measured traffic; disclose that stronger
verification contract. Per-worker values cost O(pB). The one-worker comparison
performs the same arithmetic, visit updates, histogram work and final audit.

## 8. Implementation and experiments

Submit core, binary64 variant, real thread lifecycle, guest adapter, interval
proof, exact generated symbols and independent expectations. Hosted tests cover
every n0..260 and p1..4 plus SIZE_MAX range arithmetic, empty work, duplicate
claim, local/combination overflow and failure. Differential runs compare1032
actual cases with mathematical integers, independent byte order and all bins.

Native runs use four CPUs from the process's existing allowed mask; each worker
sets its own affinity and records actual CPU identity. No host scheduling or
security policy is changed. Three repeated trials compare p1/2/4 and packed
versus128-byte-separated rows of four atomic histogram bins. Both layouts use
identical65539 byte inputs and per-index work. Record wall/process/thread CPU
times, start-to-ready time, join tail, combination time and raw outcomes.
Initialization is outside both measured intervals; launch, synchronization,
worker work and combination are inside. Verification/log output follows timing.

Run the original257-job dynamic helper separately in the real guest and compare
its scheduler/job-assignment contract with static intervals. The static guest
records per-worker ranges, CPU/APIC identities and actual completion for lengths
0/1/10/257, a failed worker, and histogram/ordered bytes. Repeated local virtual
TSC intervals are not physical CPU time or throughput.

## 9. Verification and expected results

Require six host profiles, native profiles including both TSan builds, actual
guest1/2/4CPU execution and source-bound dependency audits. Six compiled core
controls expose overlapping ranges, dropped tails, unchecked unsigned wrap,
ignored worker failure, early incomplete publication and reversed join order.
They must compile and fail ordinary assertions; crashes, build failures and
sanitizer reports are not passing control evidence. The actual guest control
runs every worker on BSP: the numerical total can match, but recorded CPU
identity must reject it. Learner stubs compile and fail semantic assertions.

For `[1e16,1,-1e16,1]`, the exact mathematical sum is2. Under the selected
binary64 contract, ordinary fixed-order p1/p4 yields1 and p2 yields0. The
provided Neumaier compensation experiment obtains2 for this fixture. Repeat
each order and report both reproducibility and absolute error; compensation is
not an exact-sum guarantee for arbitrary inputs. A changed rounding mode or
fast-math contract requires a new argument and evidence.

## 10. Objective-specific rubric

`.a`: level0 omits/duplicates ownership; level1 repeats a divisible example;
level2 proves all bounds/empty intervals and verifies every index; level3
derives and tests the ordered noncommutative join independently.
`.b`: level0 uses overflow UB or reads uncompleted partials; level1 passes the
ordinary sum; level2 preserves output through overflow and partial failure with
an explicit publication edge; level3 separates floating accuracy/order/races
and justifies the compensation fixture's limits.
`.c`: level0 claims linear speedup from virtual CPUs; level1 reports a checksum
or one elapsed value; level2 shows CPU/index identities, equal work and complete
clock/overhead boundaries; level3 designs and executes packed/padded layout
comparisons while retaining the physical-evidence gate for coherence claims.
Each criterion requires its own evidence, not a shared PASS marker.

## 11. Hints and private analysis

Hint1: partition by quotient and remainder, not rounded floating division.
Hint2: locate the first acquire that permits reading each plain partial.
Hint3: distinguish an associative join from a commutative one.
Hint4: place the total's publication after every failure check. Private analysis
and implementations are under `assessor/legacy_smp`; learner exports contain
only the public unsolved mechanisms, tests and explicit import adapters.

## 12. Changed conditions and transfer

The original `.a-DELAYED` uses the ordered polynomial summary, `.b-DELAYED`
uses actual binary64 cancellation and compensation, and `.c-DELAYED` changes
packed versus separated histogram rows. The parent histogram transfer is
implemented as private per-worker bins and fixed-order merging. Preserve the
original seven-day unaided delay. Physical cache-line size, topology and PMU
evidence are required before claiming measured physical false sharing; native
WSL timing alone does not close that part. Revisit overflow policy, ledger cost,
rounding and source dependency closure under changed assumptions.

## 13. Scope and containment

All inputs are original synthetic arrays. Guest work uses disposable original
CMK images with no network or host disk passthrough. No third-party target,
credentials, hardware flashing or host security settings are involved. Flawed
variants demonstrate numerical/ownership/publication failures, not exploitation.

## 14. Status and evidence limits

The successor is authored and focused pilots are separate from a final campaign.
Only current passing `reviews/v5/expansion/legacy_smp_reduction/campaign.json`
and its guard establish the declared executed matrix. Physical cache/latency
observations, independent human review and delayed learner attempts remain
unattempted. No finite test collection proves absolute mastery or all C runs.
