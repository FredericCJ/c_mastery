# T21.06: a deadline does not release the backend's ownership

Use `cancellation.h/c`, public tests and the actual `FAsync` prerequisite provider.
The author campaign imports the assessor foundation provider and audits its
`f_submit`, `f_cancel` and `f_complete` symbols. Learners supply their validated
provider; copied look-alike counters are not integration. Families A/B/C/D/F/G/J
apply to the original three T21.06 criteria and changed packets.

Zero-initialize a stable `AeOperation`. Start copies0..4 bytes, installs a terminal
receiver and admits a generation1..3. A second start while pending conflicts;
after generation3 the control retires. Every core call is serialized. The backend
owns access to retained operation storage while pending. A deadline records the
timeout winner and asks for cancellation, but makes no release or terminal report.
COMPLETE relinquishes the backend; CANCEL_ACK and JOINED do so only after a timeout
request. An acknowledgment before a request is ignored. Duplicate or stale events
are rejected while storage remains live. Invalid inputs preserve output tokens.

For DEADLINE,ACK,COMPLETE, timeout wins at DEADLINE, ACK makes the backend quiescent
and emits exactly one terminal report, then the duplicate COMPLETE is rejected.
For DEADLINE,COMPLETE,ACK, COMPLETE establishes quiescence but reports TIMEOUT;
the later ACK is rejected. COMPLETE before DEADLINE reports success. The selected
policy does not promise that requesting cancellation undoes an effect.

ACK/JOINED are strong **environment contracts**: the producer has stopped using
retained memory and unregistered future callbacks. They are not ordinary messages
that can be fabricated to make a test green. A physically late callback after
destroy cannot be made safe by reading a generation in freed storage. Tests of
duplicate notifications retain the control block. Real adapters must stop sources
or route through a separately retained control whose pointer can become null.

Before calling the receiver, mark terminal state, complete the actual provider,
copy payload and receiver arguments into local storage and perform the final
operation access. The receiver may free the heap operation. Do not log an operation
field or increment a counter in it after callback return. With best-effort cancel
and no acknowledgment, timeout cannot establish a finite reclamation time. Keep
ownership pending until completion or an independently justified backend join.
The20-request test intentionally leaves it pending before a modeled join.

The changed POSIX experiment has a stable outer mutex/control, an operation
pointer and a release/acquire start gate. Timeout and completion acquire the same
mutex. The receiver nulls the protected pointer and frees the operation while the
outer control remains alive. Join the completion thread before destroying the
outer mutex. This is actual thread execution, but it is not a real-I/O cancellation
adapter and does not substitute for the separate MC-REAL-IO work.

Deliver ownership and winner-state diagrams, all5461 event words of length0..6
with an independent transition oracle, two-generation stale/duplicate histories,
generation-exhaustion proof, callback-free test and thread handoff/reclamation
argument. Costs are constant state, at most four payload bytes copied and one
provider release per admitted request; mutex scheduling adds unbounded OS waiting
under an unstated fairness model. Hints: distinguish result winner from quiescence;
draw the last permissible dereference; keep the routing control separate from the
object the receiver can free. Level2 requires the complete ownership argument and
negative controls; level3 independently solves the threaded changed packet after
the original minimum seven days. Finite schedules and sanitizers do not prove
all concurrent executions or learner mastery.
