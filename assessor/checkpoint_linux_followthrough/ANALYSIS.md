# P5 assessor reasoning and discriminators

Keep this file, reference C and delayed schedules outside learner exports. Preserve
the original criterion IDs and tasks. Each criterion is assessed independently on
four levels: 0 incorrect ownership/semantics; 1 ordinary example passes but proof
or changed condition fails; 2 complete implementation plus explained invariants,
negative evidence and limits; 3 independently reconstructs the changed condition,
finds the misleading explanation and defends transfer without the worked solution.
Author reference executions can support level2 implementation readiness, never
level3 learner credit. A criterion fails for its named misconception even if other
criteria or many tests pass. Require every ordinary and delayed objective, not a
single aggregate score. Graduated public hints stop before the schedules below.

## t2408b

The Linux reader uses a64-bit `(generation << 32)|slot` token and checks it before
access. Its saved fd is a resource, not identity. `fd_reuse.c` requires descriptor7
initially unowned, creates only its own descriptors and uses F_DUPFD_CLOEXEC with
minimum7; it refuses to overwrite an inherited descriptor. Generations1/2 actually
use7; stale g1 leaves queue, cursor, end and parser-ready untouched while g2's frame
remains readable. This differs from merely assigning slot7 in a model.

Retained `linux.c` executes LT and ET: three frames fill a two-entry queue and leave
one parser-held after the kernel pipe drains. After taking one item, the caller
must resume the reader explicitly; epoll_wait alone returns no new event. The
64-frame/two-worker pipeline demonstrates bounded repeated progress, not fairness
under arbitrary scheduling. Mutants29(stale token) and30(drop blocked parser frame)
must compile then fail. A valid alternative can maintain its own ready queue, use
ONESHOT rearm carefully, or poll; it must preserve admitted bytes and expose its
different scheduling/cost contract. A generation counter cannot repair a pointer
already dereferenced before token validation.

## t2502b

Literal pressure trace: offer1,2; consume1; offer3; reject4; consume2; offer5;
reject6; consume3,5. Accepted4, completed4, rejected2, releases4. Reusing a released
slot then finishing its old handle leaves one owned slot intact. The four-slot
pool's retirement test proves a blocking call can return FULL permanently when
no generation can be reused. Parser-held frames are outside these admitted counts.

`ownership.c` blocks the producer through cpa_offer while two workers consume.
Only the queue mutex participates in that wait; the result mutex is held only
while updating result counts. A solution that holds a pool lock needed by workers
across this blocking call can deadlock even if queue locking is correct. Require
a lock-dependency graph and a schedule showing release of every consumer-needed
lock before waiting. Count capacity and transient copies separately.

## t2502c

Reference copied workload64 has worker-local messages and two workers. The producer
zeros its input immediately after successful offer. Shared comparison has one
mutable cell guarded by a single mutex/condition; the worker mutates that cell
under the same mutex. Both yield each sequence once and sum2016. Queue state,
results, shared message, ready and closed have explicit owners/locks. The copied
design's decoder belongs only to ingress; workers never read a borrowed parser.
The mutable cell is not a race merely because it is mutable: synchronization and
ownership are the issue. C does not enforce immutability or move-only transfer;
const through one pointer does not prevent a writable alias. Channel/actor or Rust
ownership comparisons must identify static language restrictions absent from C.
No timing ratio is accepted across these unequal capacities/concurrency contracts.

## t1804c

Retained operation tests allocate128 bytes, larger than inline CP_PAYLOAD32, and
zero the original buffer. The callback validates the surviving owned copy. Failed
allocation leaves generation output99 unchanged and creates no allocation or
release. Two accepted generations cause exactly two allocations/releases. A stale
completion cannot free g2. Replacing the copy with a pointer requires an explicit
external buffer lease and a release callback at the true terminal transition;
stack lifetime is not extended by queueing its address. Reject an answer that
reads freed bytes merely to test whether they look intact. Mutants23/24 discriminate
copy corruption and stale slot release;31/32 discriminate premature/stale operation
completion. Validate both compile and runtime outcomes separately.

## t2411b

The queue records OPEN/CLOSING/DRAINING/STOPPED; public CLOSED is a result, STOPPED
the internal state. Abort releases queued items2,3 while retaining worker-owned1.
An immediate CLOCK_MONOTONIC deadline returns pending and destruction refuses.
Only finish1 settles it. Repeat close and old finish do not change counts. Graceful
close instead permits queued work to be consumed. Closing wakes blocked producers
and empty consumers; reporter failure is counted separately from terminal ownership.

The bridge gate deterministically withholds acknowledgement. A timed wait cannot
license destroy or even queue discard. Releasing the gate permits observation,
then join, then descriptor cleanup. Creation failure has no worker: settle the
attempt locally, preserve the output ticket, retain work for retry or abandon it
on explicit destruction. `failures.c` tests both choices. The late-close wrapper
really closes and reuses the descriptor before injecting EIO/EINTR; one call and
a still-live replacement discriminate the forbidden retry. This is an injected
Linux error outcome, not an observed filesystem fault. Alternate OS close contracts
need a distinct adapter. Error suppression does not fix ownership.

## t2504c

While-loop predicates are essential even without a spontaneous wake. The retained
tests cover signal-before-wait, forced broadcast with false predicate, two consumers
and blocked-producer closure. `waiter.c` additionally forces a competing consumer
to take/finish the sole item during a real unlocked interval before the victim
returns to its predicate. The GNU wrapper first runs the real cond_wait, then
releases/reacquires once more under explicit gates. This is a valid controlled
scheduling seam, not a claim about the internal trace of libc cond_wait. The victim
must enroll again and return CLOSED only after closure. Mutant27 returns PENDING
instead and is rejected. Atomics publish the victim identity and seam stages;
queue fields remain under its mutex. TSan silence corroborates only these runs.
Changing drain to abort changes queued dispositions, not permission to release
worker-owned values. A signal is not a reserved message or durable event credit.

## t2106b

Use actual FAsync/in_shutdown, not a fresh lookalike enum. Public bridge tests keep
queue1,2 and release0 after cancel(false) and immediate deadline. The worker has
actually read ABC; its gate prevents any matching callback observation. Once
acknowledged, queue0/discard2/release1; duplicate observation stays at1. `in_shutdown`
trusts its completed Boolean; the adapter must prove it using mutex-published
quiescence. `f_cancel` alone does not stop poll/read. A wake pipe avoids closing an
in-use descriptor as a purported cancellation mechanism. The second round proves
g2 cannot accept stale g1. Bridge mutants1(early quiescence),2(ignore stale ticket),
3(permission to destroy before join) fail without deliberately freeing live state.
No arbitrary code execution claim follows from those assertion failures.

## t2106c

Retained CPOperation transitions ACTIVE/CANCELLING -> REPORTING while locked,
unlocks for reporter, releases the copied payload, then advances generation or
retires. REPORTING prevents reentry from beginning/cancelling/completing/destroying
the operation. `report_trace.c` observes ARFARF: allocate/report/free for each of
two generations. It checks payload still live at report, REPORTING through free,
reporter error17 recorded, duplicate terminal callback refused and generation
retired. This is cleanup order, not merely a final count. The callback may return
an error but cannot reopen completion or recurse into reporting. Allocator callbacks
run while locked and must be nonreentrant; do not generalize reporter permission
to them. A different protocol may queue reentrant work after return, with a stated
bound and ownership; it must not silently process it inside the reporting transition.

## Validation and limits

Run `tools/run_checkpoint_linux_followthrough.py`, then its independent report
guard, actual isolated CLI exports and package guard. Source and binary hashes,
tool identities, argv, environment and stdout/stderr are retained. GCC/Clang O0/O3
and ASan/UBSan execute seven positive mechanisms and15 compiling negative cases.
Separate GCC/Clang TSan executes six synchronized mechanisms and an original
intentional race sensitivity control. The race has no portable numeric oracle.
No physical observations or external/independent/delayed performance are implied.
Three-second test deadlines and bounded retry loops are laboratory fail-fast bounds,
not real-time guarantees. Source review must still assess all declared premises.
