# T24.11: admission, terminal outcomes and quiescence

## T24.11.a — choose an overload policy before exhaustion

Import the actual `RfOutbox` from PR-T24-03 into `RdService`. Admit at most four
connections and eight frames per connection. Reject a fifth connection and ninth
frame with the caller still owning rejected input. Compare the same nine-frame
trace under three policies: drop newest leaves the existing queue; disconnect
cancels all queued work on that connection; pause-reading retains queued work
and disables further input until an explicit low-water resume below four frames.
Record terminal outcomes of accepted work separately from rejected offers.

Pause-reading is an application admission state; merely setting the bit does not
change a kernel registration. Your native loop must remove or suppress read
interest while paused and reestablish it after resume. The reference policy
tests verify this state contract; the native loop integration must show that
admission is actually suppressed. A disconnect cannot unsend bytes already
committed to a peer, and that peer may need to discard a truncated frame.

The changed-condition task varies payload lengths and adds a global byte budget.
Test empty frames, a zero-byte budget, exact exhaustion and rejection without
ownership transfer. Distinguish payload cost from headers, message objects,
connection state, terminal ledger and socket buffers. This finite service epoch
also caps lifetime accepted IDs at 64, even if earlier messages have completed.
Demonstrate that the 65th offer is rejected unchanged; bounded current queue
length does not by itself bound an accumulating audit log.

Admission occurs outside borrowed callbacks. A callback may request shutdown,
but a disconnect triggered by reentrant admission could release another borrowed
message; the supplied API rejects running-state admission while any callback
borrow exists. Explain this restriction and assess a new deferred-disconnect
design as a distinct contract if you choose to support that extension.

Hints: draw ownership on both sides of rejection; replay the same overload trace
under all three policies; then include completed IDs in lifetime resource cost.

## T24.11.b — settle every accepted ID exactly once

Implement running, draining, cancelling and released states. Stop-admission is
immediate. Preserve one absolute monotonic deadline; repeated stop may shorten it
but must not postpone it. Continue allowed progress while draining, then request
cancellation for the remainder. Every accepted unique ID ultimately has exactly
one completed or cancelled terminal record. Rejected offers are not accepted
work and must not appear as cancelled service-owned records.

For the native workload, use four owned stalled local connections and 32 queued
messages. Verify actual EAGAIN preserves queued objects. Begin draining, allow
one complete frame through, wait for the observed deadline, then settle the
remaining 31 as cancelled. Retain actual elapsed time and state transitions;
the experiment does not certify a hard upper latency bound. Partial bytes
already copied by the kernel cannot be rolled back by a local cancellation.

The changed-condition task invokes stop from inside a callback that still
borrows service state. Enter cancelling, but retain both the service object and
all pending message ownership until the last callback returns. An early
`RF_CANCELLED` publication could permit the caller to free a payload still being
read; deferring only the service object's destructor is insufficient. Test the
message owners and terminal ledger before and after the final return. Reject
new borrows once cancellation starts. Repeated stop must not double-release or
duplicate terminal records.

This callback protocol is synchronous. An asynchronous target can retain a buffer
after callback return, so its pending-operation tokens from T24.07.b/T24.12 must
also participate in quiescence. Do not compose the two by silently assuming
callback return means kernel completion. Write the extra product-state invariant
before attempting that integration.

Hints: distinguish requesting cancellation from publishing terminal ownership;
then keep a borrow across stop; finally let a caller free every object whose
owner is terminal to expose premature publication in a bounded model.

## T24.11.c — enroll waiters and arbitrate competing outcomes

Implement the capacity-one `RdPort` with one mutex protecting queue, stop/peer
predicates, enrollment counters and terminal decisions. Force an empty blocked
consumer, a full blocked producer, an actual peer EOF and repeated shutdown
notifications. The test announces entry while holding the mutex, then performs
the atomic release-and-wait operation. The controller observes enrollment under
that same mutex. Explain why a sleep would not prove this ordering.

Issue a notification without changing the predicate. Require the waiter to
recheck and enroll again, then perform a real producer/consumer handoff. This
deliberate notification is not a claim that the implementation spontaneously
produced a spurious wakeup. All state reads in the observer must be locked or
performed after join; the test harness is subject to the same data-race rules.

The inspected POSIX Issue-8 condition-wait passage specifies mutex release and
reacquisition, predicate rechecks, the chosen condition clock and timeout/signal
interaction. This adapter uses CLOCK_MONOTONIC and cooperative stop; it does not
support `pthread_cancel` or robust-mutex recovery. Those need separate cleanup
and owner-death contracts rather than an unreviewed additional return branch.

For the delayed task, race an already-expired timed wait with a stop thread
released through a barrier. Define the decision rule at mutex reacquisition:
observed stop, then peer-gone, then an available transfer, then timeout. The
first terminal decision is latched. Count both possible race outcomes without
demanding that a particular scheduler produce both in a finite run. Separately
force timeout-first and stop-first histories, and prove a later notification
cannot overwrite either result. Every worker is joined, waiters return to zero,
and conditions are destroyed only afterward.

Hints: identify the mutex-protected linearization point; keep enrollment separate
from notification count; finally distinguish a timeout event from the published
winner of the application policy.
