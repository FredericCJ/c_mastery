# Executor-owned demand and bounded admission — X-T73-010

Implement `src/stream_follow.c` through `stream.h`, importing the actual completed
`struct msg_hub` copied-unsigned channel and `struct msg_stream` demand state machine. This
required successor supports R2/R3, the multithreaded R4 investigation and the
slow-subscriber assumptions in X-T73-010-DELAYED. It does not replace the unaided
delayed assessment. Use C17, POSIX pthreads and the declared unique context owner;
no reactive-stream or executor library implements the studied mechanism.

## Representation and lifetime

`struct mfq_stream` owns one native executor thread, one `struct msg_stream`, a real `struct msg_hub`
whose channel0 has capacity1–4, a control mutex/condition variable, pending
control signals, and a unique `struct mfo_value` callback context. Its initialized
control address is stable until destruction; never copy it or use the embedded
core/hub through another API. Mutex order is control then hub. Callbacks run
only on the executor and outside both locks. All pthread operations after
successful construction must meet the ordinary platform contract; asynchronous
pthread cancellation, owner termination and injected mutex/condition failures
are not modeled. The supplied first-principles code does not claim recovery from
those environmental failures.

`mfq_init` requires fresh control storage, valid next/terminal callbacks and a
nonempty unique context with a destructor. Success consumes that context only
after the executor starts. Any failure leaves the input owner intact and no live
stream. The explicit `fail_start` switch tests cleanup at the thread-start
boundary after mutex/condition/channel initialization; it does not claim an
actual operating-system resource-exhaustion event. Invalid capacity is rejected
before construction. Allocator/destructor dependencies remain live until the
context's eventual destruction. The destructor returns normally and cannot
reenter the destroyed stream.

`mfq_acquire` requires the administrative owner to protect the live stream while
acquiring a unique `struct mfq_handle`. Success increments its lease count; refused
admission leaves output unchanged. The output must not already own a handle.
A handle may move to another thread with an explicit happens-before handoff, but
must not be copied or used concurrently. Its lifetime covers every call through
it, including a blocked submission. `mfq_release` clears the handle and decrements
the count only after its operations finish; releasing an empty handle is harmless.
These tokens do not make an arbitrary stale pointer safe. C does not enforce
unique handle ownership or correct cross-thread transfer.

`mfq_close` stops new admission and requests cancellation; `mfq_cancel` does so
through a handle. Neither waits for an in-progress callback or drops the context.
The administrative owner calls `mfq_join` after closing or selecting terminal
completion. Joining while admission remains open returns `MSG_BUSY`; calling it
on the executor also returns `MSG_BUSY`. After the actual thread join, repeated
joins are harmless. Administrative join calls must be serialized externally;
there is one joining owner. A join failure retains resources for inspection.
`mfq_destroy` requires both a successful join and zero outstanding handles, plus
external quiescence of every administrative pointer/call. Otherwise it returns
`MSG_BUSY`. Only successful destruction releases the context and synchronization
objects. No later call is permitted before fresh initialization.

## Signal and delivery contract

Requests from producer threads accumulate under the control mutex with saturated
unsigned64 addition; `UINT64_MAX` is this course's unbounded sentinel. Only the
executor calls the actual demand core. Request0 closes admission and schedules
terminal `MSG_INVALID`. A callback may request or cancel using a separately held
handle; it cannot recursively invoke an item callback. The callback context is
borrowed through each callback return. Do not release its owning stream from a
callback. Terminal and next callbacks always return normally.

Items are copied into channel0 on successful `mfq_submit`. The executor removes
an item only when its core has positive demand, then invokes `msg_emit`, which
consumes finite credit before callback entry. One item may be in flight in
addition to the bounded queue. Accepted means admitted to this buffer; it does
not promise delivery after explicit cancellation or terminal selection. All
policies share the invariant:

`accepted = delivered + queued + in_flight + explicitly_discarded`

The rejected count is separate and saturates; accepted count exhaustion refuses
new items before channel mutation. Delivered and discarded cannot exceed the
nonwrapping accepted count. Handle count also refuses before wrap. An item can
be delivered at most once, only with credit, in channel acceptance order. No
ordering is promised between concurrent producers before mutex admission.

`mfq_end` accepts only OK, FAILED or INVALID, freezes admission and queues one
terminal signal independent of demand. The next executor selection prioritizes
cancellation, invalid request, terminal completion, accumulated requests, then an
eligible item. Cancellation before an unstarted terminal suppresses notification;
a callback already started finishes before any subsequent action. Pending items
are explicitly discarded when cancellation or terminal handling ends the worker.
No later next or terminal callback is delivered. This policy intentionally does
not drain accepted buffered items before completion; a producer needing that
property must first establish quiescent delivery, then request completion.

`mfq_idle` is a diagnostic synchronization point: with producers stopped, it waits
until the executor has processed available controls and credit-eligible items,
or has finished. An idle worker can still own queued items with zero demand.
Calling idle from the executor returns BUSY. Idle is neither a cancellation
mechanism nor a deadline; a nonreturning callback can prevent it indefinitely.
`mfq_stats` returns a mutex-protected accounting snapshot. No latency/fairness
bound follows from these predicates. The white-box test uses `stats.waiting`
under the same mutex solely to know a blocking submit has enrolled.

## Slow subscriber and explicit overload alternatives

Capacity is identical under all three policies; demand and delivery accounting
are identical. Only admission on a full queue changes:

* `MFQ_WAIT` enrolls a condition wait and retries the predicate. Space admits the
  item; cancellation/terminal selection wakes it with CLOSED. A full-queue wait
  attempted by the executor callback returns BUSY rather than waiting for itself.
* `MFQ_DROP` rejects the newest unaccepted item with FULL and records rejection.
  Previously accepted items and demand are unchanged.
* `MFQ_FAIL` rejects the new item with FAILED, freezes admission and schedules
  terminal FAILED. Accepted but not yet delivered items are explicitly discarded.

T75 owns selection of the production overload policy and its service-level
tradeoffs. Here implement and compare all three; do not call a dropped item
successfully delivered or silently change buffer capacity to favor one policy.

Pause the first callback with unbounded demand and capacity2; admit two more
items. For WAIT, a fourth producer must actually enroll, then succeed when the
callback resumes: four accepted/delivered. For DROP, the fourth returns FULL:
three accepted/delivered and one rejection. For FAIL, the fourth returns FAILED:
three accepted, one delivered, two discarded, one rejection, one terminal FAILED.
Check the accounting before resuming and after joining. Also cancel while an
actual producer waits on a full queue; it must return CLOSED while the paused
callback/context remain live. Test callback-origin full-queue WAIT refusal.

## Verification and assessment

`messaging.stream_executor` tests construction refusal, no delivery at zero
credit, finite/unbounded requests, request0 without demand, terminal uniqueness,
callback request/cancel, actual paused-callback cancellation, count exhaustion
and retained handles after join. `messaging.stream_overload` tests the three
identical-capacity alternatives, cancellation wakeup and two real producers
supplying2,000 distinct values through one executor. Assert every callback uses
that executor and callbacks never overlap; after successful joins every value
appears once. Require context destruction exactly once, after all leases release.

The independent Python demand oracle enumerates19,608 operation words of length
zero through five against the actual core: request1, unbounded, request0, emit,
cancel, complete and fail. It is a sequential protocol model, not a model of all
pthread schedules. Four compiled controls invent credit, ignore outstanding
leases at destruction, incorrectly reject a woken blocking sender, or omit
terminal failure on overload. Their named assertions must fail; a timeout,
compiler failure or sanitizer report is not the intended oracle.

Submit D1 demand and lifetime/state proofs; D2 actual channel/executor adapter;
D3 model, schedules, ownership/accounting and exact executions; D4 all three
policy comparisons and an unseen changed subscriber with all thirteen cost and
enforcement dimensions. Explain capacity plus one in-flight item, O(1) admission
work versus unbounded wait, callback duration, context retention, thread stack,
condition wakeups and saturation. Include explicit residual starvation and
nonreturning-callback examples rather than inferring progress from stress runs.

The inspected Reactive Streams rules motivate demand, serial signals and delayed
cancellation. This C interface uses status returns, an unsigned64 sentinel and
an optional blocking producer policy; it is not its JVM API, non-obstructing
contract or certification suite. ISO C does not enforce callback return, affine
leases, shutdown or deep context ownership. Diagnostics and runtime checks have
narrower guarantees. Level1 is nominal output; level2 requires every listed
accounting/lifetime/failure condition; level3 solves an unaided changed delivery
contract and explains why the former queue/policy proof no longer applies.
Hints: separate acceptance from delivery; serialize control and next invocation;
then retain the context until actual callback and producer quiescence. Assessor
solutions remain separate. Only original local targets and synthetic values are
used. Matching source-bound campaigns report execution; no independent-human,
physical, Java-runtime or learner-mastery result is implied.
