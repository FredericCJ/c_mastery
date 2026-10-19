# Required T55 construction and changed-condition followthrough

Complete the original ten T55 projects, all thirty criteria and their exact
`-V1` tasks. The unit cards retain those tasks verbatim. Historical hours,
baseline contracts and seven-day independent assessment intervals remain
authoritative. Author execution is evidence that these exercises can be built
and checked; it is not a learner attempt or independent review.

Bring the accepted C memory model, integer, lifetime, T25 synchronization,
T26 fixed pool/queue and T53 execution artifacts. The selected baseline is C17,
32-bit unsigned and exact-width uint32_t. Hosted adapters use POSIX threads,
monotonic clocks, thread affinity and explicit libc verification facilities.
Core ring, pool and counter functions have no allocator, logger or OS call.
The selected Cortex-M4 object/link experiment is a compiler/dependency audit;
it does not authorize ISR use, execute a board, or prove all atomic widths.

## Preserve and compose the actual producers

`providers.json` names the incomplete public producers explicitly. Complete
those prerequisites or select your accepted implementations. There is no
automatic assessor fallback. The private manifest selects the unchanged AP
hazard queue, AR cancellation queue, AG version/epoch tracker, CMX original
algorithms and ApiRegistry. Read their exported contracts. AP's32 physical
nodes, AG's17 versions and CMX's8-call complete-history limit remain unchanged.
The newer AH7-call FIFO/SET checker is a separate comparison, not a replacement
for the original CMX baseline. The new CH checker has its own ten-call bound.

`retained/{advanced,concurrent}.c` are additive warning-clean copies of the
original incomplete CMX skeletons. Their headers record the original byte
identities. Only explicit unused-parameter uses were added. All algorithms
remain incomplete; the original files themselves are untouched. Other public
scaffolds are also intentionally incomplete and must fail semantic assertions.

From the exported root, for example:

```
python labs/concurrency_followthrough/run.py --part core --case ring --build build/ring --cc gcc
python labs/concurrency_followthrough/run.py --part protocol --case pause --build build/queue --cc clang
python labs/concurrency_followthrough/run.py --part epoch --case retention --build build/epochs
```

Use a new build directory. Compile with warnings as errors before interpreting
any run. Reference solutions, independent history oracle and full premise
derivations remain in a separate assessor tree. Do not copy them into learner
exports or label their successful runs as learner completion.

## Ring publication, wrap and diagnostics

`CfRing` has32 caller-owned typed slots, storage a power of two1..32 and a
logical limit1..storage. Select8 or32 counter bits; indices reside in C atomic
unsigned objects but the8-bit profile masks every publication and distance.
Capacity must remain strictly below half the selected modulus. The additional
small capacity bound makes this true for both supported profiles. A full wrap
of an unbounded producer distance would be indistinguishable from empty, so
the bounded occupancy and coherent observation premises are indispensable.

One producer and one consumer own their complete operations. Neither domain
is reentrant. Each exclusively writes its own index. A batch of1..limit copies
all payloads before a single release publication; insufficient free storage
returns FULL without copying. A pop batch requires the whole requested count,
copies all records, then release-publishes reuse; insufficient records returns
EMPTY with every output unchanged. The producer acquires the consumer tail
before any reuse, and the consumer acquires the producer head before reads.
Own-index relaxed loads do not replace either cross-party edge. Output/input
spans are live, adequately sized and disjoint from the queue and each other.
Do not copy an initialized live atomic queue or reset it with users outstanding.

The pause hook runs after the last copy and before publication/reuse. It is
test instrumentation, returns normally, retains nothing and does not mutate
or reenter the queue. Progress claims require its own bound; the host condition
variable gate intentionally blocks. The two faulty controls publish early
under a forced, synchronized schedule. They fail a boundary assertion without
using an ordinary-payload data race as a value oracle.

`CF_PAD=0` changes only index layout; `CF_PAD=1` gives selected64-byte member
alignment. This is not a universal hardware cache-line fact. Compare equal
payloads, capacities, thread counts, shutdown and affinity. The same ring can
be a single-producer diagnostic buffer: full means drop the new diagnostic,
with a bounded owner-local drop count. Retrying forever to force a log entry
would abandon the bounded failure-path contract. Multiple logging producers
require a different protocol or separate per-producer rings.

## CAS, identity and bounded prepublication cancellation

`cf_increment` is a checked unsigned counter. It never publishes wrap and
preserves the value output at UINT_MAX. A failed real weak CAS refreshes its
expected operand. The separate injected-spuriosity branch models permitted
weak failure without claiming that a compiler actually failed spuriously on
that run. Relaxed order suffices for this numeric counter alone; it publishes
no ordinary payload. A successful competitor's changes may establish system
progress without the losing caller's completion. A purely spurious loss does
not establish another caller's success. Signed overflow is not a permitted
alternative to the checked policy.

`CfPool` is an explicit reservation/lifetime exercise, not a second
Michael–Scott queue. It owns32 atomic state words; slot0 is a permanent dummy.
Each other word includes a nonwrapping generation and FREE, RESERVED, LINKED,
RETIRED or permanently retired state. A take scans at most31 candidate slots
with compare-and-exchange. Worker0 owns slots1..7 in the partitioned policy;
workers1..3 own8 slots each. Optional stealing scans the other partitions.
The shared comparison starts every scan at worker0 with stealing enabled.
Failure can be conservative under contention; it is not an instantaneous
global count. All policies retain the same total storage.

A successful take returns a ticket scoped to this pool lifetime. One operation
owns it and initializes any associated payload before a real link CAS. A failed
link CAS leaves the ticket RESERVED. The owner can retry or cancel in bounded
work only before successful publication. After the real link succeeds it marks
the ticket LINKED before relinquishing control. No asynchronous cancel, hook,
reentrant call or second owner is allowed in that interval. `cf_cancel`
compares only the full RESERVED incarnation; it cannot free LINKED storage.
This cancellation boundary is deliberately distinct from AR's concurrently
requested cancellation and producer acknowledgment protocol.

Retirement follows removal. Reclaim is called only by the exclusive retirement
owner after a separate proof that no access can resume. This API does not
discover hazards or replace AP's scan. A release increments the generation or
permanently retires the slot at its maximum. Reinitialization requires all
tickets/references to be gone. Equal address or generation bits alone do not
pin lifetime or establish effective type. The different-type and check/use
counterexamples are typed synthetic ledgers, never invalid native accesses.

## Protection, actual helping and finite histories

AP operations use fixed exclusive registration leases, two hazards per
participant and full-incarnation collector CAS. New tests compose its exact
source. A producer actually pauses after link CAS; another enqueue and dequeue
complete before it resumes. Another test pauses a dequeuer with its old dummy
protected; the competing dequeuer returns the first value and the resumed one
returns the second. Actual atomic event ordinals produce histories checked
independently against FIFO. The40000-item run checks multiplicity and payload
consistency for one finite execution. It proves neither all FIFO orders nor
all weak-memory schedules, fairness or an unbounded progress property.

Four paused registered owners with two distinct protected retired nodes each
retain8 slots. With one dummy and no other ownership,23 slots remain as a
storage bound. If all four owners are paused there is no active authorized
caller to use them. Do not invent a fifth actor without recording that changed
experiment. The existing actual four-participant test instead uses three
holders and one actor, retaining6 and admitting25. Duplicate hazards can retain
fewer distinct slots. Count reserved, queued, dummy, retired and free separately.

The justified AP weakening changes only the candidate allocation-state load:
the full-state successful SC CAS still supplies the acquisition authority.
Weakening hazard publication/scan to release/acquire is a different proposal;
loads of initial values need not synchronize, so the former SC cycle argument
cannot be reused. Keep that optimization rejected without a new valid proof.

## Epochs, nesting, departure and grace

`CfEpoch` composes the actual ordinary-field AGDomain with one C17 atomic-flag
gate. Every metadata access acquires that gate or returns BUSY unchanged.
The owner releases after all ordinary writes; the next successful acquire
orders its reads. A caller may retry but successful service is not lock-free
if a gate owner stops. Initialization is exclusive and fresh. Read-section
lifetime, object removal and collection must still follow one declared
protocol: a snapshot of eligibility alone does not authorize arbitrary reuse.

Registration identities and section counters never wrap into old authority.
AG retires a registration at UINT_MAX; active departure returns BUSY. Nested
mode counts depth and only the outermost exit publishes quiescence. The native
two-reader experiment initializes registrations before starting threads, then
uses only the serialized API. Raw representation injection is allowed only in
explicit quiescent boundary fixtures, never as a concurrent recovery protocol.

The equal12-retirement workload compares quiescence every3,6 or12 updates
against one continuously held hazard per interval. It reports exact retirement
epochs, collection points and peak retained objects. The logical workload is
equal; AP/AG payload and metadata representations differ and are reported.
Another experiment retires version1, enters newer readers of2, and then keeps
at least one new reader active through1000 handoffs. Epoch eligibility can
collect the old version while the coarse CMX global-zero predicate remains
false. The repeated nonzero state supplies the indefinite-extension argument;
1000 iterations alone do not prove an infinite schedule.

The reader-writer-lock comparison reserves equal17×32-byte payload capacity
and supplies the same copy-consistent read/update requests and joined shutdown.
AG uses immutable version slots plus its metadata and a host serialization
mutex; the RW adapter updates one slot while holding a writer lock, leaving
the other reserved slots unused. A paused AG reader can cause retained-version
FULL; a paused RW reader can block the writer. Those algorithmic differences
are the subject of the comparison, not hidden equality assumptions. Separate
reader and writer distributions, retries and retained storage. Neither adapter
is Linux RCU; the frozen guest observation remains a separate provider result.

## History extension and event resolution

CH accepts0..10 calls, capacity1..8 and FIFO or stack semantics. A complete
invocation must precede its response; same-participant overlapping calls reject.
Coarse timestamps establish only strict response<invocation precedence.
Optional distinct event ordinals add a known edge at equal timestamps, and
must themselves be consistent with timestamp order. They are observed events,
not IDs invented later to force a preferred result.

Pending mode chooses a subset of pending calls to complete after the prefix
and removes the rest; every completed original call remains mandatory.
Selected pending results are derived from the sequential specification.
The witness records operation indices, selected pending mask and resulting
values. The independent permutation/list oracle checks both verdict and
witness over67500 finite cases. Original CMX complete-only limits remain
unchanged; passing this extension never silently grants it pending semantics.

The caller supplies a search-node budget1..1000000. LIMIT is undecided, not
NONLINEAR. Recursion depth is at most10 and each copied object holds8 values;
subset/permutation growth is exponential. Witness output changes only on
VALID; the separate visited counter reports completed search work on ordinary
negative/limit outcomes. Invalid admission preserves both outputs. This is a
bounded offline checker, not a real-time or unrestricted-history service.

## Measurement and evidence

The four host queue adapters compare SPSC, unchanged AP, mutex/condition
blocking service and mutex with try admission. Completed transfer requests
use equal distinct identities, an8-byte index/check message, caller-owned
immutable bodies of8 or256 bytes, logical upper capacity31 and exact joined
shutdown. SPSC is compared only at one producer/consumer. AP's physical pool,
conservative FULL and reclamation scans differ from the mutex's logical
capacity check; the outer retry-to-service loop includes these costs. Payload
size changes the immutable-body read workload, not AP's queue struct layout.

Native optimized runs retain every enqueue and completion latency, per-worker
completion/retry counts, five repeats, CPU affinity, exact source/compiler
identity and empirical distributions. Individual thread affinity uses only
CPUs already permitted to the process. No host setting changes, performance
counter access or physical CPU/cache qualification is part of this package.
The eight-worker one/two-CPU cases are oversubscribed. Both faster and slower
locking outcomes are reported; no universal performance ranking is inferred.
Paired resampling intervals describe these five local observations, not a
guarantee about future workloads or a certified population confidence bound.

Sanitizer and race-detector cases execute separately and never enter timing
summaries. Measurements include monotonic clock probes and shared-host noise.
Main-loop I/O is excluded; raw CSV writing occurs after elapsed-time capture
and joins. Full/source-bound checks are necessary before reusing any receipt.
Independent external review, original delayed learner administration and
physical observations remain open. Inspected provider source passages and
the C11 draft must not be relabeled as fresh access to final C17/C23 standards.
