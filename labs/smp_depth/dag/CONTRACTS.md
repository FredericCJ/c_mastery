# T53.12: arbitrary DAGs, runtime admission and failed dependency ownership

## Identity, families and objectives

This required successor implements `PR-T53-12`, all three original
`AX-PR-T53-12-T53.12.a/b/c` criteria and their unchanged `-DELAYED` identities.
It adds derivation, construction, boundary diagnosis, model falsification,
failure/recovery, resource measurement and independent transfer. A fixed graph
heading or an old sixteen-node test is not credited as arbitrary graph support.

## Inspected sources and prerequisite artifacts

Read the exact OpenCilk work/span passages and actual retained CMK fixed-DAG
helper listed in `research/v5/expansion/legacy_smp_dag/resources.json`. The work
and span concepts compare a language-level fork/join system with this explicit C
representation; this task does not import OpenCilk or claim its runtime guarantees.
The bounded N1570 synchronization passages support the publication discussion.
The implementation baseline remains C17; readable exact published C17 remains a
reference gap. C99 lacks these ISO atomic operations; C11 introduced them. No C23
feature or compiler extension silently replaces a dependency/ownership argument.

Supply completed `PR-T53-05` lock and `PR-T53-11` launcher implementations and
the CMK ring3/thread substrate. The actual target imports `SpLaunch` for startup,
stop and joined stack ownership. It does not label a new synthetic launcher as
the prerequisite. Learner exports supply interfaces/stubs and explicit paths;
private implementations remain in the assessor distribution.

## Environment, language and dependency profile

The C17 core uses fixed arrays, checked integer operations and the imported lock.
It has no heap, libc or OS dependency. Strict target builds must have no undefined
symbols or dynamic dependencies, including implicit compiler helpers. Native
pthread scheduling, affinity readback, clocks and Python arbitrary integers are
host verification tools. Run GCC/Clang debug, release, ASan/UBSan and both TSan
native profiles. The target compiler comparison selects the new DAG, imported
pool/lock and user adapter units; the unchanged GNU-assembly/kernel prerequisite
remains GCC-built. Audit that actual boundary instead of claiming a whole-kernel
Clang port.

The explicit `target_runtime.c` supplies byte-loop `memcpy` and `memset` for
compiler-lowered structure copies and zero initialization. Its volatile stores
prevent the bootstrap loops from being optimized into recursive helper calls.
Their ordinary live-region and nonoverlap preconditions still apply; they are
not synchronization or a general memory-safety mechanism. Test lengths0–257,
guard bytes, returned destination and unsigned-byte conversion, then inspect the
target link and generated code. No host libc is linked into the guest. The first
Clang release link exposed these missing imports; its failed campaign is retained
and is not credited as a successful target execution.

Target experiments use only the original CMK in disposable QEMU TCG1/2/4CPU
guests, synthetic integer data and a private ordinary disk-image file. No network,
host device, production target, host security change or flashing is authorized.
The coordinator participates as one worker and launches at most three children,
respecting the original four-thread process bound. Stacks are caller-owned arena
leases returned after joins; CMK has no free syscall to infer.

## Concrete input, output and implementation task

Implement `dag.c` and `dag_guest.c` against `dag.h`. `DgSpec` contains0–16 nodes,
0–32 directed edges, unsigned64-bit node costs and payloads. Copy all admitted
input; do not retain graph-description pointers. Node IDs are their original
indices. Reject out-of-range endpoints, every duplicate edge and every cycle,
including self-loops. An admission error leaves the previous graph unchanged.
Initialize only exclusively, with no old caller/claim and a positive externally
unique world identity. Graphs do not move or reset while any worker is alive.

The ordinary analysis fixture is A2→B3/C5→D1, payloads1/2/3/4. Derive work11,
span8 and lower bound max(11/P,8). Two ideal CPUs attain8: A[0,2), B[2,5),
C[2,7), D[7,8). Four CPUs cannot beat the span. Adding B→C leaves work11 and
makes span11. These are positive integral task costs with zero scheduling and
communication overhead, not measured time guarantees. The DAG is determined by
input/dependencies, not processor count.

The assessment's ordinary task returns `claim.input`, defined as payload plus
all successful predecessor results. It yields1,3,4,11 for the diamond; adding
B→C yields1,3,7,14. A callback may compute another declared pure result, but the
same node program and input must be used in each comparison. Task cost describes
work, not the numeric value of its result. Work happens outside the graph lock.

## Representation and last-dependency publication

Each node has predecessor/successor masks, remaining count, input accumulator,
result, cost, span, owner and state. States are WAITING, READY, RUNNING, DONE,
FAILED, SKIPPED and CANCELLED. Initial topological validation uses a separate
degree array; zero indegree in the running graph does not itself prove acyclicity.
The sum of all costs and each longest-path extension use checked64-bit addition.

All graph transitions use one imported lock. A successful predecessor stores its
result before distributing it and reducing each waiting successor's remaining
count. The transition to READY happens exactly once at the last dependency.
A claim changes READY→RUNNING and binds world/node/worker. Only that owner can
finish; duplicate or old-world completion is STALE. Graph masks and node slots
are not capabilities against arbitrary writes through public C representations.
Callers may inspect but may not forge or concurrently mutate them.

Prove: remaining counts equal outstanding successful-dependency obligations for
waiting nodes; ready nodes have all required values; each node has at most one
claim and ready event; terminal count never exceeds admitted nodes. Derive the
actual fixed sixteen-node storage, including two16-bit adjacency masks per node.
Generalizing that dense-mask representation requires O(V²) adjacency bits plus
O(V) node values; do not call it a sparse O(V+E) adjacency list. Validation and
critical-path work take O(V²+E), scanning claim O(V), and total terminal propagation
O(V²) because each newly terminal node scans all possible successors, apart from
repeated empty polling and lock contention. Capacities remain
explicit; being bounded does not make traffic or synchronization free.

## Failure, cancellation and cleanup

Worker failure terminalizes its node as FAILED with no result publication.
Every waiting transitive successor becomes SKIPPED; independent branches may
continue. A checked predecessor-value sum overflow similarly produces FAILED
before execution and skips its dependents. A node skipped by an earlier failure
ignores later predecessor notifications, preventing counter underflow or repeated
release. The finite propagation queue holds each newly terminal node at most once.

Cancel WAITING/READY immediately terminalizes CANCELLED and propagates skips.
Cancel RUNNING only sets a request and returns REQUESTED: that worker still owns
its claim and storage until finish publishes CANCELLED. Neither cancellation nor
a zero ready queue authorizes graph destruction. `dg_drained` succeeds only when
admission is sealed and every admitted node is terminal; actual caller quiescence
and joins are additionally required before storage reuse. Repeated terminal
cancellation is STALE. There is no asynchronous thread termination, leaked heap
cleanup hidden behind a status, or promise to recover a permanently stuck worker.

## Runtime creation variant

`dg_append(payload,cost,parents,count,out)` admits one new node while workers may
run. Parents must be distinct existing indices; adding edges to old nodes or
referring to a future index is refused. Under the lock, validate all endpoints,
capacity, cost/span arithmetic and already-completed input sums before changing
graph state or output. Copy parent identities and publish once. Completed parents
contribute their actual values immediately; running/waiting parents remain
obligations; any failed/skipped/cancelled parent makes the new node SKIPPED.

This is a meaningful bounded runtime-creation rule, not arbitrary back-edge graph
mutation. Since each new node points only from older nodes, append order provides
an incremental acyclicity proof. Static validation of the initial graph alone
does not establish safe new endpoints, completed-parent accounting, capacity or
publication. `dg_seal` permanently closes admission for this world. An empty,
unsealed graph is not drained. Competing append and finish operations serialize,
so the predecessor is accounted for exactly once on either side of completion.
Test both actual thread schedules and failed-parent/cancelled-owner cases.

## Deliverables and independent expected results

Supply the copied graph representation, validation and checked-work/span proof,
last-dependency trace, fixed-storage cleanup proof, runtime append protocol and
all changed-condition implementations. Include exact source/dependency/toolchain
identities, generated assembly, ELF link audits and per-node owner/result evidence.
Use `tests.c` and `replay.c`; the independent `oracle.py` enumerates every one of
4096 directed simple four-node graphs, computes topology and values without
calling the C implementation, and expects543 DAGs. Self-loop and duplicate-edge
inputs are tested separately because the simple-graph enumeration excludes them.

The native diamond pauses its two predecessor owners before both complete, then
requires one release/execution of D. Additional actual-thread schedules append
before and after parent completion, append to a failing branch and retain a
cancelled running claim. Compare original, B→C and decomposed graphs at1/2/4
workers for three trials, recording visits, ready events, worker, actual CPU,
task cost and start/completion intervals. There must be one executed result per
successful node and no invocation for a skipped node.

## Work-preserving split and measurement interpretation

For the performance task, A returns0 after two units of work; B returns the
bounded mix sum over indices50000..79999 after three units; C returns the same
mix over0..49999 after five units; D returns its accumulated input after one
unit. A unit is10000 iterations; each mixed item contributes only its low10 bits,
so the aggregate fits the checked result contract. All inputs/outputs are fixed
and independently recomputed; the final result is40843194.

Split C into independent2-unit and3-unit disjoint ranges, both after A, and make
D depend on B and both ranges. Its result is unchanged, total work remains11,
and the critical path becomes6. This extra parallelism requires an actual valid
decomposition of the node program. Simply deleting dependencies or splitting a
sequential recurrence would not justify it. The ideal two-CPU optimum remains8;
four ideal CPUs attain6. Actual launch/poll/lock/extra-node overhead may dominate.
The target uses the same construction at1000 iterations/unit with result4174073.

Native wall and process CPU clocks include graph initialization, creation,
execution and joins; fixture construction and final verification/logging are
outside the interval. Ready time is reported honestly as an interval: take the
maximum before-finish and maximum after-finish timestamps of its predecessors.
The actual ready publication lies inside that interval. A successor may start
before a predecessor records its after timestamp; do not invent an exact clock
at the lock transition. Root readiness is bounded by the initialization interval.
Target per-task TSC durations are local to their pinned CPU, without cross-CPU
calibration or physical WCET/scaling claims.

## Verification and pass/fail evidence

All positive profiles, exhaustive finite graph comparisons, actual native
schedules and dedicated target runs must pass. Nine compiled controls must fail
ordinary semantic assertions: duplicate-edge acceptance, cycle acceptance,
premature readiness, duplicate finish, ignored failure, early append output,
premature running cancellation, unchecked arithmetic and distributing a value
before storing the predecessor result. The public core and target stubs must
also fail. Compilation failure, crash, timeout or sanitizer report is not an
intended semantic rejection. A target all-workers-on-CPU0 control must fail actual
placement on2/4CPU guests. Require exact FAILED/SKIPPED/CANCELLED state evidence,
not just a final zero or a marker naming a topic.

## Rubric and graduated hints

For `.a`, level0 confuses processor count with DAG work/span; level1 repeats11/8;
level2 derives bounds and independently predicts the B→C change; level3 justifies
the decomposed node program and explains its ideal versus observed overhead.
For `.b`, level0 accepts an invalid graph or double-releases; level1 handles the
fixed diamond only; level2 validates arbitrary bounded inputs and demonstrates
the simultaneous-last-predecessor case; level3 derives runtime append authority
for both completion orders and rejects unjustified general back-edge mutation.
For `.c`, level0 equates total output with correctness or frees a running claim;
level1 gives one ordinary trace; level2 supplies independent state/result/timing
evidence and failure propagation; level3 handles an unfamiliar failed/cancelled
runtime graph and qualifies the changed storage/progress argument unaided.

Hints: separate validation indegrees from running dependency counts; store values
before releasing dependents; distinguish graph drained from workers joined;
restrict runtime mutation deliberately and prove the restriction rather than
pretending the initial topological check covers later input. Full technical
analysis and implementations remain assessor-only.

## Delayed transfer, security scope and validation status

Keep all original minimum-seven-day delayed identities: add B→C, implement runtime
creation, and propagate failure/cancellation. The long-node split remains an
additional required transfer with concrete executable support. Author execution
prepares these tasks; it does not satisfy a learner's unaided attempt or external
human assessment. No independent review or physical measurement is fabricated.

Adversarial inputs are only synthetic local graph descriptions and deliberately
flawed original implementations. The observed property is invalid-admission
refusal, exactly-once ownership or preserved lifetime; no crash is called arbitrary
code execution. Capacity/overflow/cycle controls preserve state under the repaired
contract. Native object validity, a trusted lock/launcher, cooperative workers and
external world-lifetime discipline remain limitations. Exact validation status is
the source-bound campaign and support record; finite passing tests do not prove
absolute mastery, all interleavings or unrestricted runtime graph algorithms.
