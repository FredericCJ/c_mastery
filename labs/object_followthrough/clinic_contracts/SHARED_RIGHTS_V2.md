# Shared clinic rights contract, revision2

This versioned clarification supplements `labs/object_followthrough/clinic/CONTRACTS.md`
for `PR-T03-05`, especially `AX-PR-T03-05-T03.14.a-DELAYED`. Original function
signatures, implementation, fixtures, criterion IDs and historical execution
receipts are unchanged. These are caller preconditions that ISO C does not
enforce; the API is not an affine type system or checked handle-replacement API.

`mc_shared_retain(source, destination)` creates one additional owning right.
Before the call, `source` must be protected by a live right held by this caller
through the call, and `destination` must point to a distinct, writable handle
slot initialized to `NULL` that owns no right. That slot is not concurrently
accessed and does not alias an existing owning handle. On successful retain,
the destination owns the newly counted right. Retain does not release or
replace a previous destination right. Capacity refusal preserves the empty
destination. The initialized control stays at a stable address throughout all
operations; its mutex is never copied.

The distinction is observable by reasoning about the original implementation:
with creator right1, retaining into empty `extra` makes2; calling retain again
into that same already-owning `extra` makes3 but overwrites the only slot that
records one acquired right. Dropping creator and `extra` then leaves count1.
That misuse loses ownership accounting and prevents normal final destruction;
it does not demonstrate arbitrary execution. Correct callers either use a
different empty destination or explicitly drop the old right before a new
retention, while another live right protects the control. Merely copying an
owning pointer does not create a counted right.

Every `mc_shared_add` and `mc_shared_read` caller must hold its own live counted
right for the entire call. `mc_shared_drop(&handle)` consumes exactly the live
right in that exclusive handle slot and nulls it; a copied alias cannot later
drop the same right again. Moving a right into a worker context is a deliberate
handoff: assign its pointer, clear the sender slot, then publish the context
under the thread-start synchronization. Count synchronization preserves payload
lifetime; the same mutex separately synchronizes payload access. Neither role
can be inferred from the other.

`mc_shared_finish` is an owner-control operation, not a payload borrower call.
It requires the control object itself still live, no outstanding counted rights,
and externally established quiescence: no current or future add/read/retain/drop
can race with mutex destruction. Join the worker and prevent new entrants.
The count alone does not establish this entry gate. In the selected normal-mutex
contract, valid lock/unlock calls succeed; an unexpected synchronization error
after an operation's commit is not permission to retry a consumed right blindly.

The actual existing tests already obey these premises: `extra`, `third`, and
`excess` start `NULL`; successful handoff clears `extra`; main and worker each
use their held right; the worker is joined before final control destruction.
No executable input or expected result changed, so the source-bound C campaign
is retained without claiming a new run. A fresh isolated learner export checks
publication of this clarification. A bounded automated second-agent source
review found the omission; it is not human-independent curriculum acceptance.

The original seven-day delayed task, objective rubric, security containment,
ownership/resource analysis and learner deliverables still apply. Require the
learner to explain this two-retains/one-slot counterexample and the distinction
between a live control address and a held ownership right. Reference counting
and finite tests do not establish complete memory safety or absolute mastery.
