# Copied messages, ownership transfer, and quiescent teardown

This additive contract supplies reference support for the six original criteria
`AX-PR-T51-06-T51.06.a` through `.c` and `AX-PR-T51-14-T51.14.a` through `.c`.
The canonical criteria retain their prerequisites, seven-day learner delay,
rubrics and historical execution descriptions. This contract adds concrete
representations and executable changed conditions. It does not replace the
retained `cmx_queue`, task scheduler, or their target acceptance records.

## Objective and dependency boundary

The queue criteria distinguish a copied byte message, an integer value, a
borrowed pointer, and an ownership-bearing buffer handle. The teardown criteria
distinguish close, completed access, drained work, joined callbacks, actual owner
failure, and reusable identity. An integer queue cannot establish the byte-copy
claim; a balanced reference count cannot establish that every accessor stopped.

Prerequisites: completed `PR-T51-05` condition-wait work for queue composition,
`PR-T51-13` reader/writer and shutdown work for teardown, the original `cmx`
completion protocol, C object lifetime, checked integer extents, and the
`PR-T25-01` pthread create/join ownership artifact for hosted comparison. Inspect
the actual completed prerequisite implementation before using it. The core has
no hidden import of any completed assessor component. The optional hosted
adapter uses POSIX mutexes and conditions for exclusion and sleeping; these do
not implement the queue, buffer allocator, receipt table or lifetime state
machine being assessed. Implementing the underlying scheduler/mutex remains
part of the retained first-principles curriculum.

`include/cl.h` and `src/core.c` are the learner interface and unsolved core.
`include/cl_posix.h` and `src/posix.c` are the separate hosted adapter interface
and unsolved implementation. `tests/` contains public verification programs and
an intentionally incorrect, bounded live-pointer specimen. Assessor solutions
are supplied only through an explicit assessor-root build outside learner export.

## Environment and authorities

The core is ISO C17, caller-serialized, caller-owned fixed storage, without
allocation or library calls. The caller must provide real live, initialized,
nonoverlapping objects and arrays of the declared extents. Inputs and outputs do
not overlap a control object or its backing storage. No C type prevents copying
an owning handle or fabricating its bits. Bounds, generation and ownership
checks detect specified misuse while the registry remains live; they cannot
make access through a freed registry pointer valid. Object representations are
private in intent but visible to make the laboratory inspectable; direct field
mutation is reserved for specified counter-limit fault injection.

Generated native and Cortex-M4 objects must be inspected for implicit helpers.
A successful cross-compile/link audit is not target execution. The separately
supplied `targets/m4.c` and linker layout execute the serialized core in an
isolated QEMU `mps2-an386` Cortex-M4 guest, with semihosting only for reporting.
The target campaign builds GCC at O0/O3 and Clang at O3, rejects every undeclared
runtime helper, and executes reference, ten semantic controls and the learner
stub for each profile. This is actual emulated core execution, not integration
with CMX scheduling, IRQ wakeups or a physical development board. The actual hosted
adapter is C17 plus Linux/POSIX pthread semantics, a monotonic condition clock,
and deferred cancellation only. No asynchronous cancellation, signal-handler
entry, process-shared queue, interrupt timing or fairness guarantee is claimed.
The retained `cmx` operations require port exclusion; `CMX_BLOCKED` completes in
the task result, so repeating the original call is not its completion protocol.
The hosted blocking wrapper has a different, explicitly synchronous interface.

Inspected authorities are registered in
`research/v5/expansion/legacy_concurrency_lifecycle/resources.json`: actual
`cmx_queue_send/recv/close/abort`, the original six criteria, POSIX.1-2017
condition-wait and mutex-lock passages, and Linux man-pages 6.19 robust-mutex
behavior. The POSIX edition is identified rather than silently called POSIX.1-2024.
ISO C does not provide POSIX cancellation or robust owner-death recovery.

## Copied queue: representation, operations and invariants

`struct cl_queue` borrows exactly two caller-owned `struct cl_message` slots until all use ends.
Each accepted message contains `(producer, sequence, length, bytes[4])`.
Length zero is valid; a nonzero length needs a live input array. `cl_send`
copies at acceptance, then increments `accepted`; mutating the caller's array
after success must not change the stored message. Oversize, null nonempty
input, full, closed and exhausted acceptance counter return an error without
changing the queue. The head and count define a two-element FIFO. Receive
checks capacity before changing either queue or output; error preserves all
outputs. Zero-length receive accepts a null byte output. A successful receive
publishes the message and advances exactly once. `delivered` counts removal
from the queue, not application processing or acknowledgement.

`cl_close` rejects later sends but preserves accepted messages for draining.
`cl_abort` closes and counts all remaining messages as discarded; repeating it
discards zero. At quiescent observation, `accepted = delivered + discarded +
count`. A failed/full/timed-out send is not included in accepted. Sequence IDs
are application data, not automatic deduplication. Queue operations cost O(1)
metadata and O(length) copying, with at most four bytes copied per operation.
Explain why a copied integer proves neither byte copying nor pointer lifetime.

Supplied inputs include `{3,0,9,7}`, a sender mutation to 88, an undersized
three-byte receive, zero-length message, full two-slot state, 1,000 ring turns,
and counter exhaustion. The independent Python model enumerates all 7,776
five-operation traces from two sends, short/full receives, close and abort:
38,880 observable transitions. Its finite alphabet and length are explicit
limits, not a proof over arbitrary executions or integer states.

## Blocking, cancellation and actual multi-threaded execution

`struct clp_queue` encloses the same core with one mutex and separate readable/writable
conditions. Calls enroll in the relevant wait predicate while holding the
mutex, recheck after wakeup, and use an optional absolute `CLOCK_MONOTONIC`
deadline. This adapter chooses an unaccepted timeout if the timed wait returns
`ETIMEDOUT`, even if a slot becomes available at that boundary. It does not
claim that the condition predicate must be false on timeout. A borrowed input
remains stable until the blocking send returns or cancellation cleanup finishes.
Deferred cancellation cleanup runs with the mutex reacquired, removes the
waiter and active-operation obligation, then unlocks. Cancellation requested
while another owner holds the mutex cannot establish cleanup completion.
The automatic cleanup record is volatile because the observed C pthread cleanup
macro uses a setjmp-based unwind frame; preserving a modified nonvolatile local
across that unwind failed in the actual Clang release build. Explain why volatile
here preserves unwind state and does not provide inter-thread synchronization.

The public test actually starts three producers and two consumers. Each
producer sends 500 messages; each successful receive obtains a monotonically
assigned delivery ticket under the queue lock. Consumers write distinct trace
cells, and joins precede the independent per-producer ordering/uniqueness check.
The test does not infer removal order from the order consumers later acquire a
separate logging lock. Additional tests enroll a blocked third sender, inject a
broadcast without changing the predicate, exercise timeout/cancellation/close,
and wake an empty receiver on close. Creation failures at each of the four
mutex/condition initialization positions must unwind the initialized prefix.

`clp_destroy` rejects nonempty/open/active objects. External admission must
already have stopped and every accessor must have been joined before calling
it. A count check inside the same object cannot protect a simultaneous new
caller from a destroyed pthread mutex. The lease protocol below supplies the
separate reasoning obligation. Do not use pthread cancellation to imitate MCU
fault containment or change host scheduling/security policy for these tests.

## Owned-buffer and variable-backing changed conditions

`struct cl_pool` has sixteen byte cells and four allocation descriptors. Each handle
contains a descriptor index and nonwrapping generation. Allocation of 0..8
bytes uses the first contiguous run; zero length still consumes a descriptor.
An unavailable descriptor or insufficient total free bytes is `CL_FULL`.
Enough total bytes without a sufficiently long contiguous run is
`CL_FRAGMENTED`. Neither failure alters the output handle or allocator state.
The pool is deliberately small; derive O(descriptors + cells) bit-mask search cost and
contrast external fragmentation with a full queue and descriptor exhaustion.

The owner transitions are FREE → CALLER → QUEUED → CONSUMER → FREE.
Successful `cl_owned_send` transfers ownership; failed/full/closed send leaves
it with the caller. Canceling an unaccepted send therefore releases the caller's
buffer, never a queue-owned buffer. Reads of queued storage and writes/releases
without caller ownership are rejected. Abort releases each queued buffer once;
a buffer already received still belongs to its consumer and must be released
separately. Releasing increments generation; a descriptor at `UINT64_MAX` is
retired rather than reused. The tests free alternating four-byte allocations,
request six bytes with eight total free, and require fragmentation, then free an
adjacent allocation and require successful contiguous allocation. Report every
ownership transition and reconcile allocations/releases, including rejected and
zero-length transfers. A copied handle is not a second ownership grant.

## Application acknowledgement changed condition

`struct cl_ack` is a four-receipt, single-client, serialized **volatile** service. Its
request identity is `(epoch, positive increasing id)` and the payload is an
unsigned delta. Admission records PENDING or reports FULL without advancing
highwater. Repeating the same pending request does not apply it. `cl_apply`
commits one addition and an APPLIED result together under caller exclusion;
overflow produces a retained terminal RANGE result without modifying value.
Retrying an applied request returns its recorded result; mismatched payload is
rejected. Acknowledging a terminal receipt frees its slot but retains highwater,
so an evicted old ID is STALE. A new epoch must not reuse a live service identity;
constructor reuse is not a crash-recovery algorithm. The supplied lost-reply
trace applies +7, discards its first reply, and retries to observe value 7.
The blind retry mutant must be rejected. Explain precisely why this does not
provide durable exactly-once effects, multi-client ordering, or permission to
retry an arbitrary external side effect after losing process state.

## Lifetime leases and restart

`struct cl_life` remains in stable registry storage through OPEN → CLOSING → SHUT →
DESTROYED and subsequent checked restart. Eight lease slots carry separate
generations and ACTIVE, SEND_WAIT, RECV_WAIT, COND_RELOCK or TIMER obligations.
Close rejects acquisition and wakes queue waiters into WAKE_CLOSED; it does
not complete condition-mutex reacquisition or join a timer callback. A lease
can be dropped only from ACTIVE or WAKE_CLOSED, once. Transitioning a condition
or timer lease back to ACTIVE is a trusted adapter assertion that reacquisition
or cancellation/join completed; the C state machine cannot observe another
runtime's scheduler by itself. `cl_life_drained` similarly records the owner's
verified drain/abort of associated work. Do not call it merely because close
was requested. Destruction requires SHUT and no outstanding lease. Generations
and lease slots refuse exhaustion rather than wrapping.

Tests keep two active references, a blocked sender/receiver, a pending condition
reacquisition and a timer lease; early destruction and duplicate drop must fail.
The hosted timer test holds an actual callback thread until a stop handshake,
checks that stop request alone is insufficient, joins, and only then releases
its lease. The restart variant uses three objects with consumer→worker→timer
dependencies, destroys consumers before providers, and restarts providers first.
`cl_restart_group` validates all at most three members before changing any; an
exhausted member leaves the entire cohort destroyed. Old handles must fail
against the new generation. No stale pointer is dereferenced to test this.

## Owner failure, poisoning and repair

`struct cl_record` protects `(value, complement checksum, committed shadow)`. An owner
begins, changes value, and commits checksum/shadow. A trusted death observation
between those steps makes the record unavailable to ordinary readers. Exactly
one supervisor can acquire repair ownership. Its policy rolls back to the
committed shadow, then publishes healthy; it may instead abandon the record.
A checksum alone cannot select the intended committed value. A repairer dying
leaves the record poisoned; after two failed repair acquisitions the next
attempt makes it permanently unrecoverable. This finite fail-stop policy is an
original course design, not a POSIX default. A successful ordinary transaction
resets the repair-attempt budget.

The separate Linux comparison uses actual robust pthread mutexes. A joined
owner dies while holding the mutex with value 13 and old checksum for 7.
The next owner receives `EOWNERDEAD` **while owning the lock**. Repair restores
7 and calls `pthread_mutex_consistent` before unlock; abandoning without that
call makes subsequent locking return `ENOTRECOVERABLE`. A repairer also exits
without making the mutex consistent, and a later repairer observes owner-death
again. Two actual supervisors serialize repair. This observed Linux behavior
is distinct from the course model's two-attempt limit and the retained `cmx`
policy that refuses planned exit while owning synchronization objects.

## Evidence, rubric, hints and limits

Required deliverables supplement the original D1–D4 files: representation and
transition diagrams; acceptance/ownership/reference ledgers for every supplied
trace; implementation with explicit adapters; generated-code/link audit; exact
compiler/kernel/library versions; reference/learner/mutation outputs; finite
model comparison; and a changed-condition report explaining which old arguments
fail. Preserve the distinction between delivered, processed and acknowledged.
Use `cmake -S . -B build` and `ctest --test-dir build --output-on-failure` in an
initialized learner workspace. Defaults are unsolved and must fail semantic
checks after compilation. Never pass by changing the tests or linking the
assessor solution into a learner submission.

Each original rubric still applies. Level 0: incorrect result, missing evidence,
or unexplained error suppression. Level 1: ordinary success only, no ownership
or failure argument. Level 2: every named criterion observation and its error
preservation/cleanup invariant, both semantic mutants, actual adapter tests and
changed-condition author reference agree; evidence limits are accurate. Level
3 requires the separate delayed unaided learner variant, causal diagnosis of a
new injected defect, and transfer without copying the assessor. An author
campaign cannot award level 3 or independent learner mastery.

Hints are graduated: first label the instant ownership changes; then track a
single reference through close, wake, reacquire and join; finally construct a
schedule where a notification, timeout or stop request occurs before the old
owner has finished. For fragmentation, mark sixteen cells before choosing a
run. For repair, compare the checksum with a trusted committed shadow rather
than treating either EOWNERDEAD or a valid checksum as proof of recovery.
Separate assessor derivations and exact mutant explanations are in
`assessor/concurrency_lifecycle_depth/ANALYSIS.md`, not the learner package.

Security demonstrations target only these original in-process records and
live synthetic byte arrays. The attacker controls submitted lengths/identities
or modeled lifecycle schedules; no arbitrary host addresses, credentials,
devices or other processes are used. Success is a changed byte, duplicate local
effect, or rejected early-release transition. These are property violations,
not claims of arbitrary code execution. Repair uses copy/ownership validation,
generation checks, quiescence and explicit recovery; residual limits are the
trusted live registry, correct exclusion, nonreentrant callers and finite tests.
ASan/UBSan/TSan report only exercised defects; they do not prove a scheduler,
lock-free progress, fairness, all C executions, or physical MCU timing.
