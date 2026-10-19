# T25 shared-state successor, version 1

Required original criteria are `AX-PR-T25-01-T25.02.a`, `.b`, `.c`, with their
exact `-DELAYED` variants. The original criterion tasks, prerequisites, source
gaps, seven-day interval and objective rubrics remain binding. Apply the
[shared assessment contract](../../../curriculum/v5/expansion/legacy_assessments/shared-contract.md).
Families A/B/C/F/H/J are selected because this work distinguishes representation,
ownership, failure and equal-contract costs rather than merely producing a sum.

## Prerequisites and environment

Supply the T25.01 successful creation/join ownership ledger and immutable versus
transferred object classification. Understand signed arithmetic admission and
mutex-protected invariants. This is C17, hosted profile A, original local Linux
targets with POSIX pthreads and GCC/Clang. Allowed primitives are allocation,
thread creation/join, mutexes and ordinary C operations; do not use a work-pool
or histogram library. Verification Python and sanitizers are host dependencies.

All pointed-to objects are live and inputs/outputs do not overlap. Controllers
disable cancellation throughout each operation; no external actor cancels or
joins its workers. Inputs are immutable for a borrowing call. All workers join
before automatic argument cells expire. The selected worker stack request is
256KiB; creation/attribute errors are direct statuses. On partial creation,
join the successful prefix and preserve outputs. An unexpected join or valid
mutex-operation failure terminates the disposable teaching process rather than
returning borrowed storage. Generic recoverable join ownership belongs to the
preceding retained-heap exercise; it is not silently promised by these calls.

## T25.02.a: partition and snapshot

`ts_sum` accepts 0..16 int32 values, 1..4 workers, mode0 private or mode1 shared,
and an int64 output. Null input is allowed only for length0; output is required.
Other invalid controls return EINVAL without output changes. Worker i owns
indices `[n*i/workers, n*(i+1)/workers)`. Prove disjointness and complete union,
including empty partitions. Each private result has one writer and is merged
only after join. The alternative updates a common int64 total under one mutex.
Both compute the exact mathematical sum: even sixteen INT32_MIN values fit
int64. C integer atomicity is not being substituted for mutex synchronization.

The delayed `ts_snapshot_sum` reads one live TSInput under its mutex, copies
its 0..16 values and version, and releases the source lock before processing.
A supplied hook runs exactly after the copy, without that lock. It may start
a source mutator, but may not retain the private copy or reenter this operation.
Subsequent source changes must hold the input mutex; the result and version
refer to the captured snapshot. The caller may reuse the original while the
private copy remains immutable. Publication of both outputs happens only on
success. Version is a label captured with the data, not a lock-free validation
scheme; neither equality after wrap nor ordinary racing version reads is safe.

Test every n0..16, worker count and mode, then capture `[1,2,3,4]` at version7,
start a mutator after the copy, and require sum10/version7 while the original
becomes four99s/version8. The experiment establishes source mutation after
capture during the call; it does not claim a measured simultaneous instruction
overlap. Derive O(n+workers), private-cell/copy storage and mutex update counts.

## T25.02.b: ownership survives rejection and shutdown

TSQueue has capacity2, unique IDs0..7 and one episode per ID. All IDs start
caller-owned. Admission changes caller to queued only on success. Full returns
EAGAIN, closed returns EPIPE, and an already transferred ID returns EALREADY;
failure retains the caller's ownership. Nonblocking dequeue returns EAGAIN
when empty/open and EPIPE when empty/closed, preserving its output. Success
removes the oldest ID and changes it to worker-owned. A successful or failed
worker settles that ID exactly once; other states return EALREADY. Invalid
IDs, null required controls or unsupported settlement outcomes return EINVAL.

Close is terminal and idempotent. Its selected policy cancels every pending
queued ID exactly once and leaves already-dequeued work with its worker.
There are no sentinel messages to consume capacity or confuse multiplicity.
Inspection returns state, terminal outcome and settlement count under the
same mutex. Destruction requires all operation callers quiescent and admission
permanently excluded; it additionally rejects queued/worker-owned IDs with
EBUSY. A counter alone does not implement that exclusion.

The controlled worker takes ID0, waits at a test barrier, then reports a local
failure. Meanwhile close cancels queued ID1 and rejects caller-owned ID2.
Expect ID0 failed once, ID1 canceled once, ID2 caller-owned with no settlement.
This explicitly chooses a failure result for a known local computation failure;
it does not claim certainty after process death or an external side effect.
Derive O(1) admission/dequeue/settlement and O(capacity) close, bounded state and
no fairness guarantee. A retry policy would need idempotence or an attempt
identity and is a changed contract, not a second silent transfer of ID0.

## T25.02.c: four equivalent histogram mechanisms

`ts_histogram` accepts n0..262144 immutable uint32 keys, bins1..65536, mode
shared/private/sharded/batched, a caller budget, bins uint64 output cells and
a resource output. Every key must be below bins. Invalid input returns EINVAL;
budget rejection or allocation failure returns ENOMEM. Failures preserve both
outputs. All modes use four workers and return exact counts: no bin count can
exceed n, so uint64 overflow is excluded by admission.

Shared mode locks one mutex per update. Private mode assigns one complete
histogram per worker and merges four arrays after joins. Sharded mode uses
four mutexes selected by `key % 4`, preserving one common count array. Batched
mode first accumulates at most64 distinct keys per worker, flushing counts
under the same shard locks when full or at the end. Frequent repeated keys
may remain in a batch longer, but its counters remain bounded by n. Each
mode must support empty and singleton partitions; merging only worker0 is
a deliberate failing implementation.

The common **requested numeric-buffer budget is 4MiB**. Admission charges the
caller input and output plus internal arrays: `4*n + 8*bins*(copies+1)` bytes,
where copies is4 for private and1 otherwise. It deliberately does not pretend
to bound the whole process: separately report four256KiB stack requests,
fixed argument/mutex/batch frames, allocator metadata, TLS/guards/kernel state,
and host instrumentation. The resource record describes requested sizes, not
resident memory or an observed stack high-water mark. Do not compare one mode
under weaker arithmetic, different worker count or unaccounted input storage.

Derive work and traffic before measuring: shared/sharded perform n updates and
O(bins) initialization; private adds four-bin initialization and merge traffic;
batching adds up to64 key comparisons per item and fewer updates for repetition.
The delayed experiment uses64 and65536 bins, uniform and eight-hot-key inputs,
one warmup per mode, three samples per combination, rotated mode order and
identical correctness checks. Record the selected libc's reported L1D size and
require the large count array to exceed **that L1 data cache**. This does not
assert exceeding every cache level or directly measuring physical hardware.
Time whole calls including creation, initialization, allocation and joins;
do not describe the result as a steady-state worker-kernel benchmark. Retain
48 raw samples per optimized compiler and make no universal speedup claim.

## Deliverables, verification and assessment

D1 is the original prediction, owner table and each changed-condition proof.
D2 is the learner's three mechanisms in `learner.c`, with explicit admission,
failure and complexity contracts. D3 contains exact commands/raw logs, all
partition results, the controlled queue trace, scalar histogram comparisons,
budget failures and the raw cost comparison. D4 contains each exact delayed
task independently performed after the original minimum delay. Author tests
do not stand in for those learner artifacts or invent a seven-day interval.

Run debug/O3/ASan+UBSan/ThreadSanitizer with GCC and Clang where supported.
An intended semantic negative exits41 with TS_ASSERT; a compile failure, race
report, timeout or signal is not its pass. Mutants omit a partition element,
transfer a rejected ID, and omit three private histograms. Compile the supplied
learner scaffold and require its semantic failure. Scalar expected counts and
mathematical sums are independent of the parallel work decomposition. Clean
instrumented histories do not prove every schedule or all ownership invariants.

For `.02.a`, level2 requires disjoint ownership plus publication/joins; level3
requires the snapshot or an equally justified mutation protocol. For `.02.b`,
level2 distinguishes rejected and accepted IDs with exact terminal outcomes;
level3 requires the post-dequeue failure policy and its limits. For `.02.c`,
level2 preserves exact counts and cost accounting; level3 compares the changed
cache/budget assumptions without claiming timing as correctness. Level0 is an
incorrect contract or unsafe race; level1 is only an ordinary matching result.
The original per-objective rubrics remain canonical.

Hints progress from writing each object's writer/last-reader, to separating
successful admission from attempted admission, to counting the private merge
and batch work. For source mutation, locate the snapshot linearization interval
before adding a version field. For measurements, first verify every count and
explain which costs the timer includes. Assessor implementation and analysis
remain under `assessor/thread_depth/sharing`, outside learner exports.

## Containment and limitations

Use only the original local workers and synthetic arrays; no scheduling,
affinity, host-security or device changes. Inputs with invalid lifetimes and
actual unsynchronized ordinary accesses are analyzed, not run as value oracles.
Source inspection and actual campaign evidence are separately registered under
`research/v5/expansion/legacy_thread_sharing`. Unavailable exact standards,
external review, independent learner attempts and physical validation remain
explicit. Later T25 condition, atomic and performance work stays required.
