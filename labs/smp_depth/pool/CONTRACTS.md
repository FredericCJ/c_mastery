# T53.11: bounded jobs, owned worker startup and honest shutdown

## Identity and assessed objectives

This required successor implements `PR-T53-11` and the original criteria
`AX-PR-T53-11-T53.11.a`, `.b`, `.c` plus each original `-DELAYED` variant.
Families are construction, boundary diagnosis, failure/recovery, verification,
resource measurement and independent transfer. Historical tasks and receipts
remain unchanged. It supplies actual support for their previously unexecuted
queue, partial-startup, failed-join and changed-deque obligations.

## Sources and prerequisites

Read the exact passages in `research/v5/expansion/legacy_smp_pool/resources.json`:
the pthread creation, caller-owned stack and join contracts; the actual retained
CMK ABI and `cmk2_parallel_run` startup paths; the bounded C11 memory-model
passages already inspected for the C17 baseline. No readable exact published
C17 text is claimed. C99 lacks these ISO atomic interfaces; C11 introduced them,
and this exercise does not rely on a new C23 feature.

Bring the completed `PR-T53-05` lock and `PR-T53-10` ownership/reduction artifacts,
and explain the queue/cancellation states from `PR-T51-06`. The original target
prerequisite remains the CMK ring3/thread ABI from `PR-T53-04`. Supply those
implementations explicitly; a learner export never copies a private answer.

## Environment and dependency profile

The pool core uses C17, `<stdint.h>`, `<stddef.h>`, `<stdatomic.h>` and the explicit
`SlLock` producer only. It has no heap, pthread, libc or implicit helper allowance
in the target. Audit the final guest and user ELF with `nm -u`, dynamic-section
inspection and generated assembly. Native verification uses pthreads, private
custom stacks, process/thread/monotonic clocks and existing allowed CPU affinity.
These are host-side tools, not target dependencies. Run GCC and Clang at debug,
release and ASan/UBSan settings; use both TSan builds for actual native threads.
The target compiler comparison selects GCC/Clang for the pool, imported lock and
new user adapter as separate translation units. The retained GNU-assembly/kernel
prerequisite stays GCC-built, as in its existing contract; this is not a claim
that the entire retained kernel has been ported to Clang.

The dedicated QEMU TCG guest is the original curriculum CMK, with1/2/4 CPUs,
synthetic jobs and an ordinary private4MiB disk image. It uses no host device,
network, deployed system, hardware flashing or host security changes. Its ABI
allows at most four threads per process including the coordinator. Consequently
requesting four child workers must fail after a three-child prefix. The native
adapter can create four child workers. Do not silently equate these environments.

## Concrete problem and supplied artifacts

Implement `pool.c` and `pool_guest.c` against `pool.h`; the shipped stubs compile
and deliberately fail the semantic checks. The interface has sixteen reusable
job slots, one shared FIFO of capacity3, or four fixed worker deques of capacity3.
A pool has0–4 workers and a positive externally unique world identity. Job inputs
are copied `uint64_t` payload and cost values; result publication is a copied
`uint64_t`. There is no general borrowed-pointer callback ABI hidden in this task.

Reproduce: put10, put20, claim10, put30, put40, reject put50, complete10,
claim20, complete20, close/drain, complete30 and40. The queue contains30/40 after
the second claim; a claim is not a completion. Rejection preserves output IDs.
Also inspect `tests.c`, `replay.c`, `native.c`, `oracle.py`, and explicit `guest.py`.
The finite model generates every operation word of length0–5 over six operations.
It is an independent state representation, not a second call into the C pool.

## Mechanism and necessary counterexamples

Distinguish accepted, queued, running and terminal jobs. Free queue space does not
authorize reusing a running job's slot or the worker's stack. A queued cancellation
removes the job without calling its work; a running cancellation is a request and
the worker retains authority until it completes. Close/drain rejects future jobs
but leaves accepted work live; close/abort cancels queued work and requests running
cancellation. Neither close operation joins workers or frees their storage.

Explain why all workers blocking on child submission to a full queue creates a
wait cycle. The selected API returns `SP_FULL` without blocking; callers may
propagate refusal or execute a bounded helper outside the pool lock. The supplied
native schedule has three running parents, three queued children and three
nonblocking refusals, then six completions. It does not use a timeout as proof.

## Representation, invariants and ownership

Each accepted job has a `(world, slot, generation)` ID. A slot transitions
FREE→QUEUED→RUNNING→COMPLETED/CANCELLED→FREE, with queued cancellation taking the
direct terminal edge. Only the matching worker claim completes RUNNING. A terminal
job remains queryable until explicitly collected. Duplicate completion is stale;
collecting and reusing a slot advances its generation. `UINT64_MAX` is usable once
and then the slot retires. Accepted-count exhaustion refuses new work but never
blocks cancellation, completion, collection or shutdown. Reinitialize only after
all calls have ended and no old handles survive, using a new external world ID.

All job/queue state is protected by one imported lock. Work executes outside it.
The deque variant has owner LIFO removal and thief FIFO removal, both under that
same lock. Storage never resizes or moves; there is no speculative unlocked
reader, old-array reclamation, lock-free claim or unbounded deque algorithm.
Prove that every queued slot appears in exactly one queue and every running slot
has one owner; cumulative completed+cancelled never exceeds accepted. Operations
scan at most16 slots/4 queues and shift at most2 entries. Derive costs for a
generalized capacity rather than calling these bounded loops magically free.

The actual four-thread deque experiment suspends owner0 before claiming, fills
all four home deques with three jobs, and requires the other workers to finish
all twelve jobs before owner0 resumes. At least its three jobs must be stolen;
each job still has one completed result. This complements the exact sequential
owner-newest/thief-oldest order checks and runs under both thread sanitizers.

## Worker-startup and recovery contract

`SpLaunch` has one controller and a stable callback context. Workers acquire-read
gate0(wait),1(run),2(stop). Allocate distinct stacks, create workers without
publishing RUN, then publish RUN only after all requested creates succeed.
Create failure starts no worker and its handle output is invalid. On any failed
prefix publish STOP, join every started worker and release each corresponding
allocation only after that join succeeds. The failed-create allocation can be
released directly. Callbacks do not reenter the launcher; release is infallible.

A failed join returns `SP_DEGRADED` and retains the exact handle/stack/borrowed
context. No restart is allowed until `sp_shutdown` successfully retries every
remaining join. A permanently failing join means permanent containment, not
permission to free or assume termination. Native injected join failure occurs
before the real pthread join; the later retry actually joins. This is cooperative
shutdown only, without asynchronous cancellation or arbitrary stuck-callback
recovery. Zero workers is a valid empty launch; a zero-worker pool refuses jobs.

Native stacks are1MiB distinct readable/writable page-aligned allocations set
through `pthread_attr_setstack`; no automatic guard-area guarantee is claimed.
The CMK target allocates four16KiB regions once and leases them from a private
arena. Its ABI has no free syscall: release returns a joined stack to that arena,
not to the kernel frame allocator. Compare the unchanged historical helper's
early returns after partial creation with this cleanup; process exit can contain
resources but is a different API result and lifetime policy.

## Implementation and experimental deliverables

Submit core/state and launcher implementations, a table for every startup prefix,
the actual native stack adapter, the original CMK adapter, and exact per-job
submission/claim/completion/cancellation evidence. Include generated-code and
link-dependency audits. Supply the queue and ownership proof, deque-reclamation
argument, finite-ID boundary argument and a justified failure-status matrix.

Compare static cyclic assignment with dynamic queue assignment for the same
twelve disjoint ranges and1,310,000 independent unsigned mix operations. Both
policies must produce the same modular sum. Repeat1/2/4 workers three times.
Then split the long interval into six chunks while preserving the exact total
interval and checksum; record changed queueing and overhead. Record every range,
start/completion, CPU readback, result, sampled occupancy and full refusal count.
The admission-attempt time precedes enqueue linearization and bounds queue
residence; sampled occupancy can miss its true maximum. Worker CPU includes idle
polling; total wall/process clocks include launch/work/join. Initialization and
post-run verification/logging are outside the measured interval. Do not report
virtual/native clock differences as a universal speedup or physical WCET.

## Verification and evidence-based acceptance

All positive profiles must pass the exact state/finite-boundary checks; all nine
semantic controls must fail with `SP_CHECK`, after successful compilation.
The release-before-join and ignored-failed-join controls use a mock allocator that
records erroneous releases without freeing live memory. They never run unsafe
stack release on actual pthread workers. A crash or sanitizer diagnostic is not
an acceptable semantic rejection. The unsolved learner core must also reject.

Compare every C replay row with the independent Python ledger, including refused
and stale transitions. Native tests must actually create/join every successful
prefix, retain a failed join, force queued/claimed cancellation, and execute the
three-parent full-queue schedule. Validate every native work row with independent
wide integer arithmetic. The QEMU target must show real thread calls, actual
CPU placement, exact12 jobs,25 startup cases and degraded/retry ownership.
A deliberate all-workers-on-CPU0 control must fail placement on2/4CPU guests;
the target stub must reject separately. The runner stops at a complete explicit
marker and does not claim later guest stages or normal full-image completion.

## Rubric

For `.a`, level0 invents completion at dequeue or loses an accepted ID; level1
passes only the ordinary trace; level2 proves and tests state/queue/slot invariants
and both cancellation points; level3 independently resolves the full child-submit
wait cycle and explains the progress tradeoff of refusal/helping.

For `.b`, level0 releases an unjoined stack; level1 handles only an empty startup
failure; level2 executes every prefix and retains all failed joins with a real
retry; level3 distinguishes custom pthread stacks, CMK arena recycling, kernel
resource reclamation and process-exit containment under changed failures.

For `.c`, level0 accepts totals without exactly-once IDs or calls a spinlock deque
lock-free; level1 gives one unqualified timing; level2 verifies equivalent work,
placement, cancellation and costs; level3 supplies the changed deque ownership
proof, independent model falsification and a sound interpretation of long-job
chunking without overstated timing or reclamation claims.

## Graduated hints and separate analysis

Hint1: draw slot state and stack ownership as separate state machines.
Hint2: publication of STOP permits workers to exit; only a successful join permits
stack release. Keep successful-allocation and successful-create masks distinct.
Hint3: a coarse-lock deque avoids speculative-reader reclamation but changes
progress and contention. A generation belongs to the live slot lifetime, not
the queue position. Technical solutions remain in the assessor distribution.

## Changed conditions and independent assessment

Keep the original delayed IDs and minimum seven-day policy: `.a-DELAYED` child
submission, `.b-DELAYED` failed join/shutdown, `.c-DELAYED` owner/thief deque.
Use the implemented schedules and oracle as author verification, then require
the learner to derive an unfamiliar schedule and adapter without the solution.
Changing worker count alone is insufficient. The long-interval split also changes
granularity while retaining its mathematical operation. Independent learner
performance and an external human review remain separate from author execution.

## Security scope and validation limits

All targets are original local curriculum programs or the dedicated original
guest. An adversarial submitter can fill queues or present stale IDs; the selected
property is bounded admission and ownership-preserving refusal. Opaque structs
are not an ISO C protection boundary against callers writing arbitrary memory.
Harmless controls observe incorrect state/count/placement or mock release order;
they make no arbitrary-code-execution or privilege-escalation claim. Repair is
the explicit state/generation/join protocol plus regression cases. Residual limits
include scheduler fairness, trusted port callbacks, finite capacities, same-world
discipline and noncooperative worker failure.

Current validation is described only by the source-bound campaign/support record.
Authored tests and stubs are not learner acceptance; finite traces, sanitizers
and native stress do not prove all C executions. Physical measurements, external
review and delayed unaided learner attempts are not fabricated.
