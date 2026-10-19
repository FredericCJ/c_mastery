# T19 registry assessor analysis

T19.03.a: Behavior is the pair(code,context). The same function updates separate
0 and10 fields to1 and11. A global context conflates instances. An early-return
stack pointer cannot be retained merely because the callback code remains loaded.
Owned contexts transfer on API_OK only; slot admission failure must not invoke
the release hook. Drops occur only after the provider no longer invokes them.

T19.03.b: Snapshot generation handles retain B's turn even if A marks B removing.
In immediate-suppression mode the wrapper's enabled flag supplies an additional
eligibility predicate. The provider still owns the context until its deferred
drop; freeing on suppression would be wrong. A adding C is outside the original
snapshot and waits. Recursive dispatch is BUSY. These four behaviors distinguish
semantics; do not reuse a single self-removal test as evidence for all of them.
All eight policy×operation histories execute both initial and next dispatch.

T19.03.c: api_destroy called inside A sets closing and returns PENDING. The outer
api_dispatch skips B, clears dispatching, releases all live slots, marks closed,
then returns to the adapter. Only now may adapter terminal notification free its
storage. Save the provider status first, clear/copy terminal fields before
invocation, then return that local status. ASan executes the real free path and
the external receiver log supplies subsequent assertions. Reading r->closed or
resetting a terminal flag after notification would be use after free. A retained
closed object supports idempotent destroy; a consumed pointer does not.

Alternative valid design: a separately owned control block outlives registry data
and receives the final acknowledgment. It must still specify which allocation
the receiver can consume and who frees the control block. Simply documenting
that callbacks may free anything is insufficient. Disallowing terminal free
would avoid the hazard but fail this changed-condition task. Assess each exact
leaf and seven-day variant; source-bound reference passes do not substitute for
independent learner or external technical review evidence.
