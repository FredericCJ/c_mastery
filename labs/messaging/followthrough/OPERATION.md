# Inline completion and owned successor chains

This required X-T73-003-R2/R4 successor composes the actual `MsgOperation` with
the owning payload descriptor/drop operation from X-T73-002 (`owned_value.h`) and
an explicit backend start callback. Implement
`src/operation_follow.c`; do not replace the imported operation's terminal-state
protocol. The backend and receiver operate on one serialized executor. Cross-thread
delivery needs an external serialization/quiescence adapter, not just these APIs.

`MfoValue` is a unique owner `(data, drop)`; non-null data requires a drop function.
A null data value may have a drop function (it is still called). No ownership is
copied by copying this C representation. A borrowed view does not become owned
because it fits this struct. The operation, backend state and receiver state have
stable lifetimes through their last access, subject to the completion rule below.
Initialization requires unused/quiescent control storage and non-null backend and
receiver. Input descriptors and output operation do not overlap live state they
would overwrite. Callbacks return normally; no nonlocal exits or asynchronous
thread cancellation. `mfo_drop` clears the descriptor before invoking its saved
drop callback, which must not retain or invalidate unrelated borrowed control.

Start transitions the imported operation to STARTED before invoking the backend.
It saves all information needed for that invocation first. The backend may
complete inline, and the receiver may free the operation and its enclosing backend
context. After backend invocation, start must neither inspect nor modify them.
Do not write a post-callback `completed` flag in freed storage. Repeated start
retains the producer's INVALID result. Stop only sets a request on a started
operation and returns INVALID otherwise; it neither delivers completion nor
licenses freeing context. The backend chooses OK/FAILED/CANCELLED according to
the actual commit/cancellation outcome. A request does not retroactively undo a
committed result.

Finish validates status and descriptor before transfer. Only OK, FAILED and
CANCELLED are accepted; non-started returns BUSY. Rejection leaves the descriptor
and operation unchanged. On acceptance it takes the descriptor, clears the caller
descriptor **before** invoking the receiver, and supplies the value to that
receiver even on failure/cancellation. The receiver must drop or transfer it once.
The input descriptor passed to finish must be outside operation/receiver/backend
storage that the callback might destroy. The imported terminal state is committed
before user code. The adapter caches receiver/context/value, clears its pending
owner, and performs no access through them after receiver return.

The changed-condition fixture allocates two distinct operations and one actual
heap payload. The first completes inline, transfers the payload to the successor,
frees its own enclosing operation and starts the successor. The successor also
completes inline and frees itself. Expected accounting is two starts, two receiver
calls, two operation frees and one payload drop, with payload value 42 retained.
The deferred fixture requests cancellation, observes no terminal callback yet,
rejects an invalid status without consuming the payload, then explicitly delivers
CANCELLED and releases the payload once. Duplicate completion is refused.

`messaging.operation_inline`, its two compiled controls, and the actual sanitizer
profiles are required. Controls omit the start transition or lose the handed-off
owner. A finite passing sanitizer run is not a proof of arbitrary callback code.
Required reasoning includes a last-access table for every pointer and the fact
that C neither enforces linear ownership nor diagnoses a copied owner. Hints:
(1) callbacks can destroy their context, (2) cache before calling, (3) inspect the
imported completion implementation's post-callback behavior. Assessor analysis is
separate. Record control size, two allocations and synchronous call-chain stack
growth; this bounded two-stage fixture is not a stack-safe unbounded trampoline.
