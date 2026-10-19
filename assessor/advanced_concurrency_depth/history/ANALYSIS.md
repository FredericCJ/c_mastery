# History/progress assessor analysis

## T26.01.a

The overlapping insertions admit order11,22,remove11 even though22 returns
first. Nonoverlapping insertion11 before22 forces that order, so remove22 fails
FIFO. A pending insertion11 may be completed after the recorded prefix and
linearized before its completed removal. Rejecting every incomplete history
or dropping all pending calls misses this case; completing every pending call
is also unjustified. The checker explicitly searches subsets and publishes only
a valid witness. Its independent oracle uses permutations and Python lists/sets,
not the C recursion. Exponential bounds limit the claim to the chosen corpus.

## T26.01.b

Sequential consistency of atomic accesses does not choose the abstract data
type. The executed atomic stack has a perfectly defined SC execution returning
22 after inserting11 then22; its FIFO history is illegal. Removal of named22
from the corresponding SET is legal, and duplicate insertion/absent removal
have newly specified statuses. A missing payload release/acquire edge is a
different failure: conflicting ordinary C accesses need happens-before. The
relation model identifies the missing edge without assigning a reliable result
to an undefined native execution. Neither claim can repair the other.

## T26.01.c

For the corrected linked queue, the successful link CAS makes a node reachable;
Tail is an assisting cursor and may lag. The actual CMX consumer experiment
starts from a documented valid prefix with Head==Tail but next!=sentinel, helps
Tail, swings Head, returns the item and retires the old dummy. This is real
consumer execution from a constructed prefix, not a claimed paused producer
thread. Empty observation needs the implementation's validation/protection
argument; the single prefix test is not a full reclamation proof.

The fixed pool admits only31 queued nodes while retaining one dummy, and retired
nodes can reduce admission further. The sequential test shows31 retired nodes
and zero logical elements before collection. The scan's failed result must not
be equated with the capacity2 logical FULL in the history checker. Concurrent
free-slot changes can defeat any claim that a failed scan observed a globally
full pool at one instant. The bounded adaptation therefore specifies permissive
try-admission failure; exact full-pool/logical-capacity semantics require a
different admission protocol. Successful insert/remove points remain distinct
from this permitted no-change failure.

## T26.02.a

An indefinitely stopped mutex owner blocks another active thread. An operation
whose intent collision makes both participants withdraw has an indefinitely
repeatable collision cycle but succeeds in isolation. Under suitable atomic and
resource progress premises, repeated genuine CAS interference entails other
successful changes, not the losing request's completion. Three bounded own
actions give a bound conditional on those primitives; stopped scheduling does
not give a wall-clock guarantee. A retry cap changes a success-only operation
to a try operation with an abandonment result. Reject any proof that silently
counts abandonment as the required enqueue.

## T26.02.b

The frozen implementation has bounded node and hazard arrays, but queue retry
loops are not bounded merely by array dimensions. Collection has bounded scan
work at each call; safe eventual reuse still depends on participant hazards and
the generation protocol. There are no callbacks or malloc in the retained
selected path. Target helper imports are implementation-specific: report the
whole object and selected image separately. Adding malloc/free reintroduces
allocation failure and unspecified blocking/cost behavior; its strict link
rejection demonstrates a changed dependency profile, not that every hosted
allocator necessarily blocks. Weak CAS can fail spuriously, so paper-level
progress arguments need more than the fact that the C primitive is available.

## T26.02.c

The repeating contention trace gives the losing thread infinitely many steps
in its extension while a competitor repeatedly completes. Even scheduling both
does not impose successful-CAS fairness. The finite1024-round experiment is a
witness prefix; the repeated state relation supplies the extension argument.
Actual retirement pressure is a separate failure mode: call collection to
recover this quiescent fixture, without claiming that arbitrary stopped hazards
can safely be cleared. A per-request service proof needs a helping/wait-free
algorithm or a separately justified admission/service policy; for a deadline
it additionally needs bounded primitive/effect costs and scheduling. None is
created by timing a few tests or returning RETRY after a cap.

## Evidence boundaries

Assess each original task and delayed condition under its own ID. Host models,
native reference execution, selected target links, external review, independent
learner and physical observations are different dimensions. The source paper's
type-preserving allocator assumption and corrected algorithm must not be
silently substituted for a C hazard-pointer proof. Those construction criteria
remain in the separate required T26.04–.08 packet.
