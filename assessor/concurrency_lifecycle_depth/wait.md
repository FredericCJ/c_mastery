# T51 generational CMX wrapper analysis

This wrapper uses actual retained CMX operations. It adds stable caller-owned
object and operation identities around APIs that otherwise accept raw task
slots. Four coordinates are necessary for the stated lifecycle: world lifetime,
task generation, object generation and operation sequence. A live pointer alone
does not prove any of the last three, and integer generations do not validate a
freed pointer. Saturation is refusal, not permission to silently reset a counter.

The predicate recheck and enrollment are one protected call into CMX. Its
semaphore immediate acquisition precedes the due-deadline check; the strict
wrapper policy deliberately changes this order. Both policies must be explained
before comparing tests. CMX's global queue sequence concerns queued waits,
whereas the wrapper sequence identifies every accepted operation. Exhaustion
therefore has different refusal boundaries. A failed admission does not publish
a new token or erase the previous terminal record.

Completion synchronization observes the actual transition out of CMX_WAITING
and copies the published result exactly once. Late cancellation must not unlock
a mutex already handed to a successful waiter. After cancelling g4 and enrolling
g5, a g4 timeout must not call the global CMX tick at all: that tick could expire
g5 or other current work. The same identity test prevents a stale callback from
recomputing away a newly established donation. The tests observe actual CMX
priority and owner fields rather than a duplicate donation implementation.

For readiness callbacks, enrollment identity and resource identity have different
roles. A callback bound to a cancelled registration is stale; a new independent
resource event must use cw_post even if an earlier waiter disappeared. Within
the object, FIFO admission may choose another older waiter. Treating each signal
as a private reservation for the named task would contradict semaphore
conservation. This distinction is part of the assessor's reasoning rubric.

The finite oracle uses a waiting list and credit accounting. It does not call
CMX or copy its task layouts. Its32,768 histories check the advertised bounded
transition system, not interrupt delivery, long-lived generation wrap or C
memory ordering. Native verification runs real pthread publication/contenders
on two observed CPUs, but CMX task records remain logical tasks. Mutex exclusion
provides the publication relationship; CPU affinity merely confirms placement.
The target IRQ wrapper, compiler/link audit and any physical timing require
separate receipts. No model or TSan run is promoted to those evidence classes.

The additional M4 image supplies that target transfer separately: an actual
pending IRQ0 is deferred across enrollment and nested PRIMASK ownership, then
IPSR16 confirms exception execution. Its explicitly linked course memory
helpers are listed rather than concealed. This is still a logical CMX task
record, not a newly implemented context switch. The native suite separately
replays200 old timeout/cancel callbacks after new enrollment, on pinned workers.

No partial-order reduction is used by the oracle. With one shared credit, most
operations conflict, so commuting them without an independence argument would
be unsound. For terminal-count and credit-conservation safety only, an already
stale callback is a state self-loop and stutter elimination preserves these
state predicates; it would not preserve observed return-count traces or a
liveness/fairness claim. Adding close changes which steps are independent and
requires revisiting such a proof instead of importing it from the three-event
model.
