# T25.03–T25.05: transactions, predicate waits and primitive selection

This required successor belongs to **PR-T25-02** and its nine unchanged
`AX-PR-T25-02-T25.{03,04,05}.{a,b,c}` criteria. The exact corresponding
`-DELAYED` identifiers still require a separate learner attempt after seven
days. The old brief, objective rubric and
`curriculum/v5/expansion/legacy_assessments/shared-contract.md` remain binding.
This contract supplies concrete executable inputs and constraints; reference
execution is neither independent assessment nor evidence that a learner has
waited seven days.

## Environment and prerequisites

Use hosted C17, profile A, the selected Linux/glibc POSIX backend and GCC/Clang
recorded by the runner. Pthreads supply mutex, condition and thread primitives;
implement the queue, retirement protocol, phase barrier and permit ownership
yourself. No channel, thread-pool, semaphore or barrier library may replace the
mechanism being constructed. The comparison experiment may call `pthread_once`
and the native read/write lock to examine those mechanisms' boundaries.
Host Python, disassembly and sanitizers are verification dependencies.

Prerequisites: the retained T25.01 thread acquisition/join and T25.02 ownership
contracts, integer range admission, C object lifetime and the parent queue
artifact. Submit the previous ownership ledger and a prediction of the old
capacity-three ring fixture before opening its expected values. Read the
registered POSIX.1-2017 reprint passages and Linux implementation notes listed
in `research/v5/expansion/legacy_thread_waiting/resources.json`. These are not
exact published C17/C23 or POSIX.1-2024 text.

All storage is stationary, correctly aligned, live, noncopyable and exclusively
initialized before publication. Inputs, outputs and controls are distinct live
objects. The caller retains every thread argument and object until all users
have joined. Use default nonrobust mutexes, no signal handlers, no process-shared
objects and no scheduling-policy changes. Unexpected synchronization failures
after successful initialization violate this backend's operational assumption
and terminate the original laboratory process; they are not reported as
recoverable arbitrary mutex corruption.
At most16 simultaneous queue calls, including observation helpers, are admitted
by the laboratory's external participant bound. The object is not a general
unbounded admission server.

## Transactions and lock graphs — T25.03.a/b

`TWBank` starts at `(50,50)`. `tw_transfer` admits `0 <= amount <= a`, debits
`a`, credits `b`, and preserves total100 under one mutex. Reject before mutation.
The test hook executes between debit and credit while that mutex is held. It
may synchronize the test controller but must not reenter the bank, cancel the
worker, retain its storage or acquire another application lock. Construct a
separate intentionally split-lock transfer with a controller enrolled between
the two changes. Observe90 there and100 after completion. In the repaired case
the controller's nonblocking lock attempt must report busy at the same point.
Do not run a native data race to obtain the inconsistent state.

For the unlocked-reader variant, implement `TWSnapshot` using atomic sequence
and atomic balances, all sequentially consistent. There is exactly one writer;
reject sequence exhaustion rather than wrap. A reader accepts two values only
between equal even sequence observations. Ordinary non-atomic payload reads
remain invalid even if a later version check would reject them. Reader retry
has no starvation bound. Test10,000 writes, repeated reads, invalid values and
the last even sequence. A finite run does not establish lock-free progress.

Construct `A -> B -> A` with two actual workers holding opposite first locks.
Use nonblocking attempts and enrollment barriers to record the cycle without
letting the test deadlock. Include a recursive callback edge `A -> A`. Repair
by acquiring distinct locks in ascending, unique logical-key order, including
callback edges in the proof. Keys start at40 in the reuse example and never
wrap; `UINT64_MAX` is exhausted. Destroy/reinitialize the same mutex storage
only after all users join, allocate a new key and invalidate every stale borrow.
Do not order unrelated pointers using C relational comparison or assume an
address remains a unique lifetime identity.

## Queue representation and lifetime — T25.03.c/T25.04

`TWQueue` contains a capacity-three ring, one core mutex, nonempty/nonfull
conditions, a test-observer condition, and a separate admission-root mutex.
Its state is OPEN(0), CLOSING(1), RETIRED(2). Under the core mutex:
`count <= 3`, head/tail are in0..2, tail equals `(head+count)%3`, and exactly
count cells contain queued values. Integers are copied; successful push
transfers the queued copy and successful pop consumes exactly its oldest copy.
Failed push transfers nothing; failed pop preserves the caller's output.

Initialization has five injected failure ordinals: root mutex, core mutex,
nonempty, nonfull, observer. Fail immediately before the selected operation;
destroy the actual successful prefix in reverse order and report counts.
Condition attributes are initialized, set to `CLOCK_MONOTONIC`, and destroyed
with actual error handling. They are temporary configuration objects, separate
from the five synchronization-object fault ordinals.

The admission root protects state, persistent references and active operation
count. State changes affecting core users also hold the core mutex. Acquire
root before core; release core before dropping an operation reference under
root. OPEN begins with one owner reference. `tw_ref` rejects CLOSING/RETIRED
and zero references, limits persistent references to16, and never resurrects
zero. `tw_unref` closes admission atomically with the final decrement. That
decrement rejects with `EBUSY` while queued copies remain, preserving the last
reference for draining. After zero references, all new operations reject.
`tw_close` is idempotent, rejects future producers and references, wakes waiters,
and **drains** existing queued copies through consumers. It does not discard
or silently transfer those copies. A consumer may enter CLOSING to drain.

`tw_retire` requires CLOSING, zero persistent references, zero active operations
and empty contents, checked while excluding new admissions. It destroys only
core synchronization objects and keeps the root alive. Calls through that live
root then reject without touching destroyed core objects. Final
`tw_root_destroy` additionally requires external caller quiescence: no future
call, including an attempted admission, is permitted. Reference counting cannot
make its own admission lock's freed storage safe for an unregistered stranger.
The public observation helpers are test controls, not production API examples.

## Predicate, deadline and cancellation experiments

`tw_push` waits while full and OPEN; `tw_pop` waits while empty and OPEN. Both
test predicates under the same core mutex used by their condition wait. Close
rejects pushes with `EPIPE`; pop drains an item before reporting empty-close as
`EPIPE`. With no item and no close, expiration gives `ETIMEDOUT`. Invalid
nanoseconds give `EINVAL` before acquisition. A past deadline does not prevent
an immediately available item from being consumed. This is a waiting-deadline
contract, not a promise to complete the whole operation before a wall-clock
instant. If expiration and close compete, the under-lock decision returns one
named outcome; no relative timeout is restarted after a wakeup.

All timed conditions use `CLOCK_MONOTONIC`. Pass the identical absolute
`timespec` to every wait in one operation. The test counts actual wait entries
and records the supplied deadline; observation helpers do not change the
predicate. Force notification before any wait, an empty broadcast, two waiting
consumers for one item, and an awakened consumer whose item is taken by another
consumer. The wake hook temporarily releases the core mutex with cancellation
disabled and then reacquires it; the controller removes the item during that
gap. It may not reenter from the same thread, retain storage or cancel itself.
This deliberate extra gap exposes a permitted competing transition without
pretending a notification reserved the item. Record the incorrect `if` path as
an attempted empty transition; never read an uninitialized ring cell.

Queue operations require deferred cancellation or cancellation disabled.
Disable cancellation before admission and install cleanup while the core mutex
is owned. At every cancellation-enabled point that cleanup owns the core
mutex; a canceled condition wait restores that ownership. Cleanup disables
cancellation, unlocks core, then releases the active admission. Normal return
also restores the caller's previous cancellation state. Do not keep a mutable
automatic “lock held” flag across the platform's cleanup/setjmp machinery.
The queue does not support asynchronous cancellation in its library operations.
The controllers' reference/close/retire/observer calls must not be canceled.

## Primitive construction and changed assumptions — T25.05

Implement `TWBarrier` from a mutex, condition, expected participants, arrival
count and generation. Support1..4 participants. In the three-worker case,
each worker writes its own cell1,2,3, rendezvous, worker0 calculates6, and a
second phase publishes that result to all. Generation exhaustion breaks the
barrier instead of wrapping. A participant that leaves before completing a
phase invokes an explicit supervisor break; every current/future waiter then
gets `ECANCELED` (the exhaustion initiator gets `EOVERFLOW`). Native thread
cancellation is disabled for this barrier; departure is a protocol event.
Destroy only after every caller joins. Lost participants are not automatically
detected by this bounded protocol.

Construct the following selection table and a concrete rejected-alternative
trace for each row; the enum returned by `tw_select(0..3)` names the selected
row in order, and an unknown requirement returns-1.

|Requirement|State and selected mechanism|Failure/progress boundary|
|---|---|---|
|Fixed initialization once|unstarted -> running -> completed; once|A failed initializer needs an explicit published result/policy; completed is not “value valid.”|
|Capacity3 resource ownership|three tokens, acquired/released exactly once; permit pool|The fourth caller fails without changing its output; an abandoned token needs cleanup.|
|Reusable phases|participants and generation; barrier|One missing participant prevents ordinary phase completion; break is explicit.|
|Read-mostly mutable state|readers coexist, writer excludes them; read/write lock|Exclusion alone does not specify preference, scheduling or bounded waiting.|

`TWPermits` uses a mutex and a three-bit ownership mask. Acquisition is
nonblocking, gives a distinct token0..2 or `EAGAIN`, and preserves output on
failure. Release validates range and ownership; duplicate release gives
`EALREADY`. Tokens are linear caller-owned capabilities within one active
episode; stale copies after release/reacquisition violate the contract.
Do not use a semaphore to implement the teaching pool.

For the asynchronous-cancellation variant, acquire with cancellation disabled,
install token cleanup, and enable asynchronous cancellation only inside the
isolated pure compute region. That region calls no library except the
async-cancel-safe state change; a local atomic readiness flag is lock-free on
the selected target. Cleanup first disables cancellation, then releases exactly
the owned token. The initial capacity remains3. General asynchronously
cancelable allocation/locking remains unsupported: a cleanup handler cannot
repair an arbitrary half-completed library operation. GCC/Clang's explicitly
labeled `no_sanitize` attribute applies only to that compute region because
instrumentation runtime calls have no async-cancel-safety promise. Clang also
requires `disable_sanitizer_instrumentation` here: `no_sanitize("thread")`
alone still instruments atomic operations. Inspect the
generated calls; all surrounding ownership code remains instrumented. Publish
the readiness flag only after the state-change call returns; the sole canceling
controller waits for that flag. Clang's sanitizer builds resolve that
call through its named interceptor trampoline. Record that symbol separately;
its successful selected run is not a general async-safety guarantee for
sanitizer runtimes or for calls canceled before the readiness handoff.

For fairness, define policies independently of the host read/write lock.
Reader preference admits a new reader while a writer waits; one reader overlaps
each successor indefinitely. Writer preference admits the next queued writer
before a waiting reader. A permit stealer reacquires the just-released permit
before the earlier waiter. Construct a state cycle for each, prove it can
repeat without violating safety, and name the absent fairness assumption.
Check1000 repetitions as a model regression, not as proof of infinite native
starvation or a statement about every POSIX scheduler.

## Deliverables, verification and rubric

D1: pre-experiment predictions, predicate/ownership diagrams, lock graph and
source-authority table. D2: implementations in `learner.c` and the explicit
primitive-selection/rejected-trace table. D3: ordinary and changed-condition
traces for every exact objective, initialization ledger, mutation evidence,
generated async-region call audit and source/tool hashes. D4: delayed artifacts
with changed assumptions, fresh reasoning and separate assessment date.

Compile the public `*tests.c` files against `learner.c` with C17, pthreads and
strict diagnostics. The stub must compile then fail with `TW_ASSERT`, not a
link error. Run each named case: bank, order, lifecycle, queue, wake, deadline,
barrier, fairness, permit. Use debug, optimized, AddressSanitizer/UndefinedBehaviorSanitizer
and ThreadSanitizer profiles under both compilers. The separately isolated
known-race control must trigger the detector. A sanitizer absence does not
prove all schedules race-free. Match the old ring fixture independently and
the stated forced traces, not only terminal return codes.

For every original criterion: level0 violates its invariant, precondition or
evidence boundary; level1 reproduces a positive case only; level2 supplies the
criterion's causal argument, ordinary/failure control and correct ownership;
level3 independently derives the exact delayed variant and its new limits.
The existing nine distinct rubric criteria remain authoritative. Finite author
observations cannot award level3 or absolute mastery. Resources are O(1) queue,
bank and permit storage, O(participants) worker cells, constant work per queue
transition, and potentially unbounded lock/wait/retry delay. Report stack and
sanitizer overhead separately; no hard latency or fairness bound is asserted.

## Graduated hints and containment

First write every predicate using only fields protected by its mutex. Then
mark the point at which an operation/reference becomes admitted, and the last
point at which cleanup owns the lock. Finally construct the smallest cycle
that returns to the same safety state while one participant remains waiting.
For snapshot trouble, ask whether every payload access itself is legal before
considering a version retry. For cancellation, move unsafe work outside the
cancellation-enabled region before changing resource counts.

Use only these original programs and synthetic integers in local processes.
Wrong-order demonstrations use trylock/model edges rather than an uncontrolled
deadlock; uninitialized reads are not executed. The race control is a separate
intentional detector target. No third-party service, credentials, kernel
policy, realtime priority, deployed equipment or physical board is involved.

## Validation status

The current author campaign is
`research/v5/expansion/legacy_thread_waiting/validation.json`; an absent,
failed or stale report does not establish execution. The exact original
criterion/variant bindings are in `completion_dispositions.json`. Assessor
implementations and technical solutions are withheld under
`assessor/thread_depth/waiting/`. Independent review, learner attempts and
physical observations remain separate dimensions.
