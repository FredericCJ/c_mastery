# P5: readiness, ownership and actual quiescence

This required successor supplies the eight ordinary and eight delayed criteria
already identified in PR-P5-01 and PR-P5-02. Their IDs, tasks and seven-day delay
remain unchanged. Families B/C/D/F/G/J dominate: construction, ownership boundaries,
diagnosis, falsification, recovery and composition. Explain the semantic premises
(A) and changed OS assumptions (I); measure costs (H) only with equivalent contracts.
The new adapter is hosted C17 on Linux with POSIX threads, nonblocking pipes, poll
and epoll. It is not a strict freestanding target. C23 does not add affine ownership,
generation freshness or automatic cancellation acknowledgement to these interfaces.

Bring P1's bounded storage/queue dossier, P3's full CRC-32 framing/streaming
implementation, T18 ownership, T21 operation state, T24 reactor and T25 predicate
artifacts. Record each actual provider's hash, public contract and test result.
The author campaign imports the existing `checkpoint_depth`, `checkpoint_async`,
`checkpoint_linux`, `checkpoint_operation`, `integration`, `foundation` and `algo`
implementations explicitly. Those are assessor references, never learner work.
The P1 baseline `algo_ring` and T21 `FAsync` are used by `in_shutdown`; the richer
CPA pool is a distinct four-slot representation, not a claim that P1's typed pool
API was called. A learner substituting another representation must map its capacity,
ownership and retirement behavior at the adapter boundary.

Primary reading: [POSIX Issue8 condition waits](https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_cond_wait.html),
DESCRIPTION and RATIONALE Condition Wait Semantics; [Linux epoll(7)](https://man7.org/linux/man-pages/man7/epoll.7.html),
LT/ET, descriptor identity and cached events; [Linux close(2)](https://man7.org/linux/man-pages/man2/close.2.html),
multithreaded close and error returns. The scoped register is
`research/v5/expansion/legacy_checkpoint_linux/resources.json`. ISO C object lifetime,
POSIX synchronization, Linux descriptor behavior and application ownership rules
are separate arguments. Do not infer them from a successful sanitizer run.

## Derive the state and storage model

A readiness event is permission to attempt nonblocking I/O. A connection identity
is `(slot,generation)`, not its reusable integer descriptor. Validate the token
before touching parser or queue state. Retire an exhausted generation rather than
wrapping it onto an old token. The P3 parser may own a complete frame even after
the pipe is empty. If admission stops on a full queue, maintain an application
ready/resume obligation. Waiting for another edge can strand that frame forever.

Use separate ledgers for parser-held, queued, worker-owned and terminal messages.
For accepted CPA messages, `accepted = queued + owned + completed + discarded`;
`releases = completed + discarded`, and a stale finish changes neither. The
parser-held frame has not yet been admitted. Rejection count is not admission count.
The queue contains at most two handles; its four slots also cover worker ownership.
Ownership transfer is not the same operation as deallocation. When all reusable
generations are exhausted, waiting cannot manufacture capacity.

For a copied design, ingress owns its parser and copies a complete immutable value
into the queue. A worker receives its own local value; resetting the parser is safe.
For a shared mutable design, specify every mutable field and the mutex that protects
it. The comparison workload is sequence0..63, two payload bytes `[i,i XOR A5]`,
exactly-once processing and sum2016. One worker reports an error for sequence17
without reopening ownership. Compare the same values using two copied workers and
one shared mutable cell. Their different capacities and concurrency preclude an
unqualified speed comparison. A blocked producer releases the queue mutex through
condition wait and holds no other lock the consumer needs.

Wait for a predicate, not a notification. Signal-before-wait, spurious wakeup,
another consumer taking the item, closure, and timeout all require rechecking
protected state. Bind each condition wait to its designated mutex. A monotonic
absolute deadline bounds waiting policy; it grants no lifetime permission.
`OPEN -> CLOSING -> DRAINING -> CLOSED` preserves admitted work. `ABORTING`
discards queued work but retains worker-owned work until its owner completes.
Closing wakes both producer and consumer predicates. Join workers before destroying
their control storage, mutexes, descriptors or retained input.

## Implement the actual callback bridge

Implement `bridge_stub.c` against `bridge.h`. Keep the public tests unchanged.
The caller serializes owner API calls; the worker is the sole asynchronous actor.
All operation, queue, callback, error and byte state is protected by the bridge
mutex. Do not copy an initialized bridge or enter it from an async signal handler.
No `pthread_cancel` is permitted: cancellation is a cooperative wake-pipe request.
Invalid object lifetimes and broken synchronization primitives are outside the
recoverable protocol; retain the object and report failure rather than pretend it
is quiescent. No callback may reenter the bridge API.

`init` borrows an already nonblocking input descriptor just long enough to create
its own close-on-exec duplicate and owns a nonblocking wake pipe. The duplicate
shares the open file description: the caller must not change its blocking flags.
Construction failure closes acquired resources. No container or async library may
replace the taught state machine. libc and pthread operations are permitted OS
adapters; Python and sanitizers are host verification tools.

Before starting, offer integer work1,2 to the four-element `algo_ring`. `begin`
calls the actual `f_submit` and creates one worker that polls the owned read and
wake descriptors, reads at most32 bytes, and publishes its final quiescence under
the mutex. The public fixture writes `ABC`. A lab-only acknowledgement gate holds
the completion notification after I/O has stopped; this lets the owner test what
it can safely know. The gate does not authorize earlier reclamation.

`cancel(ticket)` calls the actual `in_shutdown` with completion false and wakes
the worker. Both queued items and operation context remain live. `observe(ticket)`
may pass completion true only after synchronized matching acknowledgement. Reject
stale tickets before invoking the synchronous helper: its already-DONE branch does
not itself revalidate a ticket. One accepted operation has one release. Repeated
observation may report the terminal result but cannot release again. Separate
quiescence observation from `pthread_join`: the worker may still be returning
after publishing acknowledgement, so destruction remains forbidden until join.

A failed `pthread_create` means no worker exists; settle that operation locally,
preserve the caller's output ticket and queued work, and permit retry or explicit
destruction. Destruction after known absence/join abandons queued never-submitted
work exactly once. Report late close errors and invalidate owned descriptor fields.
Under the Linux profile do not retry close, even on EINTR: the number may already
identify another open file. The inspected Linux manual explicitly distinguishes
this behavior from POSIX.1-2024. Do not export the rule as universal POSIX behavior.

The bridge has fixed32-byte capture, four integers, three owned descriptors and one
joinable thread. Queue and token operations are bounded; OS scheduling, poll wakeup
and join latency have no derived upper bound. An observed deadline or scheduler
maximum is not a termination proof. Counter totals in the bounded exercises stay
representable; an unbounded service needs an explicit counter saturation policy.

## Required evidence and independent assessment

Submit source, provider manifest, ownership/state diagrams, exact traces, build
commands, tool identities and raw positive/negative results. Include these inputs:

| Objective | Required distinguishing observation |
|---|---|
| T24.08.b | Actually close descriptor7/g1, reuse7/g2, deliver stale readiness before current readiness; repeat LT/ET with a parser-held frame and explicit resumption. |
| T25.02.b | Admit1,2,3,5 and reject4,6 under the specified pressure trace; account every state and show a blocking producer releases consumer-needed locks. |
| T25.02.c | Execute the64-message copied/shared comparison; expose all shared mutable fields, two-worker local decoding and different resource contracts. |
| T18.04.c | Reset original input after copying; finish an old slot after reuse; use128-byte external payload, fail allocation, preserve output and live state. |
| T24.11.b | Empty/full close, late offers, graceful/abort, failed reporting, repeated shutdown, withheld acknowledgement, deadline, join and descriptor cleanup. |
| T25.04.c | Signal-before-wait, spurious wake, blocked-producer close and deterministic competing consumption before the waiter can rely on the predicate. |
| T21.06.b | Actual FAsync plus queue1,2: false completion and timeout retain both; real worker acknowledgement permits exactly one release/discard. |
| T21.06.c | g1 then g2, duplicate/stale g1, finite generation retirement, reporter error and allocate/report/free order; reentrant operations are explicitly refused while reporting. |

Run GCC/Clang at O0/O3 and ASan/UBSan, then selected TSan executions and an original
detector-sensitivity control. A compiling incorrect implementation must fail the
relevant assertion, not merely fail translation. Delayed private tests change
notification, ownership and failure assumptions; passing the public fixture alone
does not satisfy the reasoning obligations. Reasoned alternatives may use different
representations if all exact contracts and observables are preserved.

`run.py` requires explicit `--bridge`, `--integration`, `--foundation`,
`--algorithms`, repeated `--include` directories and `--build`. It never searches
for an assessor implementation. `bridge_stub.c` is intentionally unfinished.
The retained public checkpoint tests and headers accompany this supplement; use
your own complete prerequisite providers, not a renamed author reference.

Security scope: only original local teaching executables, synthetic bytes, process
owned pipes and descriptors. The stale-token and early-free mutants demonstrate
contract violations, not arbitrary code execution. Race-detector sensitivity runs
an intentionally racy original process separately; no UB numeric output is a golden
answer. Do not connect to external services, touch credentials, or alter host settings.

Hints: first write the four ownership counts and terminal predicates. Next mark
every mutex release/reacquire and every descriptor owner. Finally find the last
possible worker access before deciding where acknowledgement, release and join
belong. Assessor solutions and delayed schedules are packaged separately.

Author validation is recorded in the scoped current report; external review,
independent learner attempts and actual seven-day transfer remain pending. Physical
target evidence is not established by these hosted tests. Finite passing traces
and clean sanitizers never establish absolute mastery or every interleaving.
