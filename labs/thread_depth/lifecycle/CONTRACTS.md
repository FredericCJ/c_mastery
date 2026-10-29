# T25 lifecycle successor, version 1

This is additive required work for `AX-PR-T25-01-T25.01.a`, `.b`, `.c` and
their exact `-DELAYED` variants. The original PR-T25-01 tasks, seven-day delay,
objective rubrics and [shared assessment contract](../../../curriculum/v5/expansion/legacy_assessments/shared-contract.md)
remain binding. Families are B/C/F/G/J for the two lifetime mechanisms and
A/D/I for the interface comparison. This package does not replace the rest
of T25 or certify learner mastery from an author test run.

## Prerequisites and environment

Read T25.01 and the existing `pl_pool` contract; supply its successful-prefix
ownership diagram, source implementation or original diagnostic dossier.
Prerequisites are C lifetime, disjoint result cells, direct POSIX error status,
mutex predicates and the difference between a join handle and the objects a
worker uses. Target is an original unprivileged Linux process, C17, hosted
profile A, GCC/Clang, `-pthread`. ISO C alone does not supply pthread operations.
The allocation library supplies storage; it must not implement the ownership
protocol. Host Python is an oracle/runner, never a target dependency.

Use live valid objects and disjoint outputs. No API checks arbitrary dangling
pointers. Only the declared owner may operate a pool or create a job on a
monitor; no concurrent controller, external joiner or handle casts are allowed.
Disable cancellation around all pool operations. The creator experiment uses
deferred cancellation only; the worker explicitly disables cancellation. No
signals, process termination, asynchronous cancellation, scheduler privileges
or artificial sleeps are an ownership mechanism. Initialization and destruction
are outside all concurrent admissions.

## T25.01.a: retain the join obligation

Implement the original `pl_pool` successful-prefix task first. The successor
`tl_pool_start` admits 0..4 workers, creation failure ordinal 0..workers (0 means
actual calls), allocation fault 0 or 1, and a nonnull error output. A failed
allocation returns null/ENOMEM; invalid values return null/EINVAL. Successful
allocation returns an opaque heap pool and error0 even if creation later fails.
Worker i owns input i+1 and computes its square in its own result cell.
Creation stops on the first direct pthread error, retaining earlier handles.

`tl_pool_collect(pool, join, context, out)` joins the unjoined successful prefix
in order. Null `join` uses actual pthread_join. A supplied join adapter either
performs exactly that join or reports an error **before consuming** the handle;
it must never lie about consumption, reenter or cancel the controller. A
nonzero result is returned directly, preserving the output and heap pool,
including all earlier successful joins. Retrying resumes at the first pending
handle. A complete collection publishes created/joined/completed, square sum
and the saved creation error. Repeating a completed collection does no join.
`tl_pool_dispose` returns EBUSY without mutation while any join is pending;
otherwise it frees the pool and clears the caller's pointer. Null controls
return EINVAL. An unexpected real join error is not permission to free storage
or blindly retry an invalid handle: quarantine and establish its actual
ownership. The controlled recoverable-error experiment preserves a valid
exclusive join obligation by construction.

The invariant is `0 <= joined <= created <= requested <= 4`; cells before
created remain allocated until joined equals created. Sum30 is full four-worker
success; failure at creation3 leaves sum5. Derive O(workers) work and fixed heap
cell count plus OS stacks; join latency is unbounded without scheduler and
worker termination assumptions. Heap lifetime solves a storage-return defect,
not scheduler progress or arbitrary handle corruption.

## T25.01.b: detached completion survives creator cancellation

Implement the joinable borrowing baseline and draw the forbidden trace
`publish stack address; caller returns; worker reads`. Analyze that trace;
do not execute use-after-scope. The executable repair copies 0..16 bytes into
one job before publication and creates its worker with detached attributes.
`struct tl_creator` and its input stay live until the creator exits; the resulting
job owns its copy. `struct tl_monitor` is an independently retained test-controller
object, initialized once and used for exactly one job. Its mutex protects
all monitor fields. Two references are established before successful worker
creation: creator and worker. Creation failure publishes an error and frees
unpublished storage; the monitor can then be destroyed when quiescent.

The controller holds the worker at `release_worker==0`. `creator_waiting`
means the creator has installed cleanup and entered its monitored region;
normal early return can already have occurred by the time it is observed.
With `wait_for_completion==0`, the creator drops its reference and returns.
With value1, it waits on `completed` in a while loop. It may be canceled at
that condition wait. Cleanup must account for the reacquired mutex, unlock it,
and release exactly the creator reference. Cancellation setup has no uncovered
interval between acquiring the reference and installing its cleanup.

Join the **creator**, observing `PTHREAD_CANCELED` for the canceled case. This
does not join or cancel the detached worker. Before releasing the worker,
require one retained reference, no completion and no free; overwrite the old
input only after the creator has joined. The worker then sums its copy, records
completion, and drops its reference. The last reference frees the job exactly
once. The monitor's free publication occurs under its mutex and is its last
user-side worker access; acquire that mutex and observe free before destroying
it. This is user-object quiescence, not a portable observation that every
detached thread runtime destructor has returned. There are no user TLS
destructors or callbacks after the release point in this teaching target.

Trace both possible last releasers for normal completion. Prove the monitor
is owned by neither the canceled creator nor the job it observes. Storage is
one copied job, one monitor and the actual thread resources; computation is
O(length). No fairness, timeout, or asynchronous-cancellation safety follows.

## T25.01.c: API and edition boundary

Build `probe_c.c` and `probe_posix.c` as separate clean translation units in
C17 mode. They check exact function pointer types and link availability,
without creating a second C-thread worker implementation. Record optional
macro, header, link, start-result and join-output support separately; the
runtime reporter only reports this selected implementation. Compile with
`TL_BAD_SIGNATURE` and require the named static-assert diagnostic, not an
unrelated missing header. A cast suppressing the diagnostic is not a repair.
Even equal handle size or compatible implementation typedefs do not authorize
using one API family's handle in the other's operations.

For the exact delayed task, maintain an edition table. Freshly inspected
N1570 7.26.1p2 and N3096 7.28.1p2 retain an optional threads-header clause;
N3096 6.10.9.3 names its macro. These are C11 and C23 **draft** passages.
The exact published C17/C23 wording remains inaccessible in this wave: leave
that table cell and the full exact-edition delayed obligation open. A C23
compiler-mode probe demonstrates selected tool support, not normative text.

## Deliverables and verification

- D1: original task prediction, per-cell/pool/job/monitor owner table, complete
  creation/collection/cleanup transitions, invalid-handle versus recoverable
  backend distinction, and the edition/API comparison with access gaps.
- D2: original learner implementations under this directory, with no assessor
  source in the learner workspace. Include both cleanup paths and retained
  heap state; return values alone are insufficient.
- D3: exact build commands, compiler/libc/kernel identity, all original 0..4
  prefix cases, join-error ordinal1..4, unchanged output, failed early disposal,
  repeated collection, normal/canceled/normal-wait creator schedules and
  zero/full copied inputs. Keep raw logs and intended negative diagnostics.
- D4: each exact delayed task, independently derived after at least seven days;
  author executions here do not manufacture that interval or learner work.

The public driver exits41 with `TL_ASSERT` for a failed semantic assertion.
It never treats timeout, sanitizer termination, compile error or any crash as
an expected semantic rejection. Mutants independently suppress the result sum,
overwrite an output on join failure, and publish completion when only the
creator has finished. The latter is observed while the worker is deliberately
gated; cleanup completes before reporting the failed assertion. Run GCC/Clang debug, optimized and ASan/UBSan; run supported
ThreadSanitizer separately and preserve unsupported runtime results. Passing
tests support only the executed histories. The ownership proof and the
counterexample to stack-borrowing remain independently assessed.

## Rubric, hints and variants

For `.01.a`, level2 requires the exact acquired prefix and no double join or
premature free; level3 requires the heap successor and justified error-retention
policy. For `.01.b`, level2 distinguishes thread resources from arbitrary
borrowed objects; level3 establishes cancellation-safe reference transfer and
the final monitor access. For `.01.c`, level2 requires typed probes and separate
support dimensions; level3 requires the original later-edition comparison with
actual inspected authority. Missing published text is an explicit remaining
requirement, not permission to substitute a draft silently. Level0 is an
incorrect invariant or unsafe experiment; level1 is matching ordinary output
without the corresponding ownership/authority argument. Use the original
criterion-specific rubric and stable delayed IDs for acceptance.

Hints, in order: count only successful creations; keep a separate joined
prefix; distinguish a consumed handle from an operation that did not occur.
For the detached variant, place a separate owner outside the creator, then
draw the two reference drops, then place cleanup before any cancellation point.
Finally check which mutex the cleanup handler owns. Assessor solutions and
mutants are separately stored under `assessor/thread_depth/lifecycle`.

## Containment and validation limits

All targets are original synthetic local processes/threads. No borrowed stack
pointer is dereferenced after its lifetime; no invalid pthread handle is passed
to the OS as a negative control. A harmless logical invariant failure is not
arbitrary code execution. Source references and their exact inspection scopes
are in `research/v5/expansion/legacy_thread_lifecycle/resources.json`. The
source-bound validation report, when present, distinguishes author execution
from external review, learner performance and physical observations. The
remaining T25 sharing, queue, atomic and performance obligations stay required.
