# Exact T51 wait enrollment and terminal arbitration

This required successor supports `AX-PR-T51-01-T51.01.a/b/c` and
`AX-PR-T51-13-T51.13.a/b/c`, including their original changed-condition IDs.
Keep their original reasoning, rubric and seven-day learner-delay contract.
Families A/B/C/D/F/G/I/J address lost notification, ownership, stale work and
port changes. The six criteria remain independently assessed. An authored or
executed reference is not a learner attempt or independent review.

## Prerequisites and explicit imports

Use the actual `PR-T51-01` / `PR-T51-03` / `PR-T51-13` CMX learner artifact,
`labs/execution/include/cmx.h`, its semantics and your task/object ownership
dossier. Implement the public `wait.h` interface in the compiling `wait.c`
stub. Supply the completed CMX source explicitly; the public runner has no
assessor default. The reference composes the unchanged actual CMX semaphore,
mutex, timeout, cancellation, close, exit/reap and donation implementation.
Replacing these operations with an invented stub is not import evidence.

The baseline CMX `CMX_BLOCKED` means the result is published later in the task;
the caller must not re-enter the original wait to retrieve completion. The
wrapper records that publication. CMX's smaller numeric priority is more urgent.
Cancel returns CMX_CLOSED at the model layer; wrapper `cause` distinguishes
cancellation from object close. No historical enum or fixture is rewritten.

## Representation, ownership and execution assumptions

C17, hosted Linux GCC/Clang at O0/O3 and ASan/UBSan/TSan. `struct cw_world` is stable
caller-owned storage containing two object records and one node per CMX task.
It borrows a live CMX kernel. `cw_init` requires nonnull fresh storage/kernel,
with no outstanding references from an older world. Do not copy a live world,
kernel, registered mutex or token owner. No heap allocation is permitted.
Target code uses no pthread API; native verification is a separate host adapter.
No new strict freestanding link claim follows from the hosted campaign.

Every core call, including predicate examination, enrollment, event publication,
result query and task lifecycle, must share one external exclusion domain. A
saved interrupt mask can implement that on an appropriate uniprocessor; it is
not exclusion against another CPU. The native harness uses one actual pthread
mutex around the entire CMX/wrapper transition and real condition waits. It
pins two owned worker threads to two allowed Linux CPUs and observes their
placement. It does not execute CMX context switches on those threads.

All task waits against registered objects must use this wrapper; do not mutate
CMX task/object state behind it or attach an untracked condition-reacquisition
node. The current adapter supports semaphores and mutexes only. Pointer tokens
require a live world and cannot validate dangling or arbitrary forged pointers.
Array indices, task generation, object generation and operation sequence are
checked after the live-owner premise. Input/output/control storage is distinct.

## Enrollment and identity

Initialize a closed/unregistered object before use. Object generation starts
nonzero and increments on successful reinitialization; maximum retires it.
Each accepted operation gets a nonzero monotonically checked node sequence and
captures the current task generation and object generation. Task slot reuse
after successful exit/reap changes the CMX task generation. An old callback
must not affect a new operation even when task slot and object address match.

`cw_begin` either acquires immediately or enrolls once. Failed admission
preserves the output token and adds no node. A second enrollment while waiting
fails. Timeout before blocking, deadlock, invalid task, object state and
saturated sequence/counters are explicit refusals. Global CMX wait-sequence
exhaustion forbids new blocking enrollment; an available resource can still
succeed. The wrapper's own operation-sequence saturation forbids its next
operation, including immediate success. Count/counter overflow never wraps.

The ordinary deadline policy permits immediate acquisition at an expired
deadline; `strict_deadline=1` rejects it before taking the resource. Both are
specified contracts, not interchangeable interpretations. Modular 32-bit
deadline comparisons require the existing CMX half-range horizon/elapsed-time
assumptions. Test wrap from UINT32_MAX-2 to2 and reject backwards/early callback
time. No clock comparison establishes real elapsed time after an unbounded stall.

The protected recheck matters: a stale cached false predicate can enroll while
a credit already exists. Supply an explicit split-check counterexample, then
identify the indivisible protected transition which the correct wrapper imports.
Do not treat a passing condition-variable stress test as a weak-memory proof.

## Completion, cancellation, resource and donation rules

Ready, timeout, cancel and close linearize under external exclusion. The first
terminal transition wins. Same-logical-time contenders are ordered by the
actual serialized invocation order; timestamps do not create a universal tie
priority. Each linked node is removed once and receives one terminal result and
cause. Later callbacks return CW_STALE without altering the current operation,
logical clock, resource count, owner or donated priority.

`cw_post` publishes an independent semaphore credit. `cw_ready_callback(token)`
instead represents a callback belonging to that enrollment and discards stale
registrations. If live, it publishes to the object's FIFO, not exclusively to
that token's task: an older waiter may receive the credit. Clients must choose
the correct event meaning; discarding a genuinely independent resource event
because an old wait ended would lose work. For initial credit I, accepted posts
P, current free credits F and completed acquisitions A, assert I+P=F+A.

A valid timeout calls the actual CMX tick, which can expire other currently due
waiters too. Its callback time must not move the model clock backwards under
the half-range rule. An early or stale callback does not advance time.
`cw_cancel` delegates actual pending-wait removal and donation recomputation;
it must not unlock a mutex which has already been handed to the completed
operation. This is a real ownership transfer, not a reservation still belonging
to the losing cancellation. The L9/H3/other5 tests show cancellation restores
the surviving donation and a stale operation cannot remove a new H3 donation.

Semaphore close wakes current waiters but does not overwrite already terminal
results. Closed-object tokens remain queryable until object reinitialization
changes generation. Mutex close retains the actual CMX owner/reference refusal.
Exit/reap refuses a pending wrapper node, current task, maximum generation or
owned resources under CMX's contract; owner-death recovery remains a separate
mechanism. Node terminal-count capacity is reserved by checked admission so a
later terminal callback cannot wrap that node's count. The shared acquisition
counter is checked separately: at UINT64_MAX an unlock with no waiter remains
permitted because it does not acquire another resource. A pending handoff
returns FULL without changing ownership. Cancel the pending waiters, then
unlock and close; do not retain ownership forever or wrap the diagnostic count.
Tests exercise immediate acquisition from MAX-1 and a final handoff from MAX-1
to MAX with a second waiter still pending, plus invalid/nonowner unlocks.

## Supplied inputs, outputs and verification

`tests.c` supplies immediate/expired deadlines, duplicate enrollment, stale
operation and task callbacks, close/reinit, maximum sequence, clock wrap, all24
orders of four terminal contenders, two FIFO consumers and actual mutex
donation/handoff. Require exact task state, result/cause, one terminal count and
resource conservation after each transition. Predict before running.

`model.py` is an independently authored list/credit oracle, not a call back into
the C implementation. `trace.c` supplies observations for every length-five
history over eight events: beginA/B, post, timeoutA/B, cancelA/B, close. There
are32,768 histories and163,840 checked transitions. Report the bound and omitted
generation exhaustion, interrupts, native schedules and weak-memory behavior.
The ordinary C cases cover generations and saturation separately. Including
close as a fourth terminal contender does not justify pruning all event orders.

`native.c` executes1000 actual publications with predicate rechecks under one
mutex on two pinned CPUs, then100 native ready/timeout/cancel contender rounds.
Its timeout argument is logical CMX time, not a measured pthread deadline.
Record the observed winner counts, including zero if some winner never occurs;
do not infer exhaustive native scheduling from those counts. ThreadSanitizer
is an additional finite check of this hosted exclusion implementation.

Compile the original unsolved CMX producer only with its existing
`-Wno-unused-parameter` scaffold allowance; completed-reference runs remain
strict. The producer stub must then fail an ordinary `CW_CHECK`, not a compiler
diagnostic. The adapter stub likewise stays unsolved and must compile then fail.
Controls remove operation checking, double-complete, use a stale predicate,
ignore the strict deadline, unlock completed ownership, retain stale donation,
overwrite completion on close and publish an inconsistent record. Reject
arbitrary crashes or sanitizer diagnostics as substitutes for semantic failure.

```sh
python run_wait.py --execution-source /path/to/completed/cmx.c --execution-include /path/to/include --adapter-source wait.c --case enrollment
```

Repeat for terminal, donation, native and model. Native requires at least two
allowed CPUs; unavailable affinity is a failed environment prerequisite, not
an unreported skip. Use synthetic local state only. No real service, unrelated
process, system scheduling policy or machine-wide affinity is modified.

## Assessment and transfer

Deliver representation/ownership tables, the split-check counterexample,
state/priority traces, implementation, independent oracle evidence, exact
commands/source identities and the saved-mask versus cross-CPU exclusion
argument. The imported kernel is bounded by CMX_TASKS; wrapper scans are O(T).
CMX priority propagation retains its own bounded task/mutex traversal cost.
Record stack/object sizes under the actual data model. No lock-free progress,
latency bound or scheduling fairness is inferred from constant capacity.

Assess each original leaf separately: level0 violates an identity, ownership or
completion invariant; level1 reproduces a happy trace; level2 explains the
earliest causal failure, supplies all applicable boundary/negative evidence and
distinguishes model/native/target guarantees; level3 derives and implements the
original changed condition unaided after its recorded delay. Hints: write the
four identity coordinates; mark the exact enrollment/publication step; then
ask which operation still owns each cleanup obligation. Assessor code and
analysis remain separate from the learner package.

The original .01a interrupt/mask transfer still requires its actual target
wrapper and generated-code/dependency audit; hosted mutex evidence does not
replace it. Exact target and source-bound status is recorded by the successor
ledger. Physical IRQ latency, unrelated weak-memory architectures, independent
review and delayed learner assessment remain separate observations. These are
required remaining obligations where claimed, not optional topic exclusions.

Security boundary: intentionally flawed local wrappers and synthetic callback
identities. Attacker-controlled order/stale tokens can demonstrate wrong
completion, lost credit or unauthorized release within this teaching target.
The success condition is an incorrect ledger/owner/result, not arbitrary code
execution. Repair identity/exclusion/ownership checks, rerun the counterexample
and state the residual live-pointer, external-lock and callback-trust premises.
