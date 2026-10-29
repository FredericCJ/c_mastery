# Buffered select with explicit waiter registration

This required X-T73-002 comparison implements the buffered subset of `struct msg_hub`
from mutexes, condition variables and fixed arrays in `src/registration_follow.c`.
It is a distinct algorithm, not a call through to `msg_select`. The original
channel exercise remains responsible for rendezvous and blocking sends.

Use C17 hosted POSIX threads, the declared GCC/Clang profiles, and two channels
of 1 through 4 copied unsigned values each. Zero capacity is invalid here.
`mfr_send` never waits for space: FULL preserves the queue. `mfr_select` accepts
mask 1, 2 or 3; it blocks only when requested, returns the selected channel and
value only on OK, and otherwise preserves both outputs. These outputs are
distinct live unsigned objects outside the hub. The source payload is copied;
there is no borrowed object pointer. Selection rotates its starting channel
after success, preserves each channel's FIFO order, drains a closed channel,
returns CLOSED only when all selected channels are closed and empty, and gives
cancellation priority at its guarded admission check. A concurrent close or
cancel does not retroactively undo an already committed transfer.

## Representation, obligations and failure

The channel mutex protects its queue. The registry mutex protects cancellation,
the rotating start, a linked list of live stack waiter nodes, and diagnostics.
Whenever both locks are held the order is **channel then registry**. No code may
hold the registry while waiting for a channel mutex. Publish a waiter node
before scanning; clear its notification flag before each scan; when no value is
available, reacquire the registry and recheck the flag before entering the
condition wait. Every queue/close/cancel transition marks registered nodes and
broadcasts while holding the registry. Remove the node under that lock before
the function returns. The list never owns or extends a node's stack lifetime.

Explain the two wakeup-window cases: a transition before flag clear must be
observed by the following scan; a transition after flag clear either appears
in the scan or sets the flag before the wait/recheck. Condition waits release
the registry atomically with sleeping and must tolerate spurious wakeups.
Omitting either registration lifetime or flag recheck invalidates this argument.
The two-channel scan is not an atomic snapshot of readiness; each successful
dequeue has a guarded commit point. No fairness claim follows merely from
round-robin channel preference or from POSIX condition broadcast.

Initialization cleans up already initialized synchronization objects on a later
initialization failure. Post-initialization lock errors, POSIX `pthread_cancel`
(deferred or asynchronous), owner death and process termination are outside this adapter's
contract. `mfr_cancel` is cooperative and does not call `pthread_cancel`.
All callbacks return normally. A hub cannot be copied or moved while
live. `mfr_destroy` requires an external gate preventing new API calls and all
non-select users quiescent; it returns BUSY while registered selects remain.
Cancel, join those users, and retry destruction. BUSY is not a replacement for
that external gate. A successful destroy ends the hub lifetime. Waiting-count
overflow refuses admission; the diagnostic sleep counter saturates.

The diagnostic `mfr_select_probe` supplies an optional CLOCK_REALTIME absolute
deadline and one normally returning hook after the first empty scan, outside
all locks. The hook may send, close or cancel but must not destroy this hub or
start a blocking select on it. After timeout the function performs a final
predicate scan; EMPTY means that final scan found no selected value. This is a
bounded diagnostic interface, not a monotonic-time production timeout contract.
Its hook and context remain live through return. `mfr_observe` copies counters
under the registry; outputs are distinct and outside the hub.

## Work and assessment

Submit the implementation, lock-order/wakeup proof with a missing-notification
counterexample, exact waiter ownership argument, and the cost comparison in
`MEASUREMENTS.md`. Queue storage is fixed; registered stack storage is O(W) for
W active selects, and a transition traverses O(W) nodes. pthread bookkeeping
and scheduler costs are platform contracts and measurements, not ISO C bounds.

`messaging.select_registration` must demonstrate a notification strictly
between empty scan and wait admission, with **zero condition-wait calls** in
that controlled case; closing must still drain the accepted value; two real
producers must deliver 2,000 distinct values exactly once; cancellation must
release a registered waiter before destruction. Separate compiled controls
remove the notification flag and discard closed-channel data. A timeout, crash
or compiler failure does not count as a successful semantic-control rejection.
The separate oracle enumerates all seven-operation words through length five
against a Python queue/close/cancel model. That sequential enumeration does not
prove all threaded histories.

Hints: (1) enumerate the empty-scan-to-wait window; (2) put the node in the list
before the scan and make notifications sticky until the next scan; (3) write
down which lock protects removal before allowing any stack frame to return.
For delayed transfer, replace the broadcast list with a mask-indexed registry
and justify safe removal when two registrations alias one waiter. Retain both
close/drain and cancellation behavior and rerun the same model and window test;
a timing-only observation is not the wakeup proof. The separate assessor
analysis names the admitted comparison subset and its unresolved progress
limitations. Author support is executable; learner transfer is independently
assessed only with a separate attempt and recorded evidence.
