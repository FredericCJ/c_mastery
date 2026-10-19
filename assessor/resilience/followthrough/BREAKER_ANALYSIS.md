# Assessor analysis: count windows, capacity and cancellation

The four slots encode two independent resource partitions. Cost reservation
is not identical to slot count: reservation2 admits either two cost1 requests
or one cost2 request. One tenant cannot use an idle peer's units, which gives
bounded isolation at the explicit price of unused capacity. These guarantees
depend on trusted tenant assignment and cost classification; this is not
authenticated multitenant memory isolation.

In a full observation ring, subtract the overwritten Boolean before adding the
new Boolean. Before full, grow count. Compare integer products only after the
minimum sample count. The largest product is8*16=128. Neutral outcomes neither
evict nor increment observations. A count window is not an elapsed-time
window, and it does not bound active calls. The Python oracle keeps ordinary
lists and recomputes sum; it does not copy the ring's incremental total logic.

Trip changes epoch before any subsequent probe can be admitted. Each live slot
retains its old epoch until completion. Completion always releases its cost and
scope, but an old epoch returns before health changes. The adversarial case is
stronger than an old success during OPEN: allow the new current probe first,
then deliver the old success. Without the epoch check, that old success could
close HALF_OPEN while the real probe is still pending. Mutant61 is detected
by the required phase assertion there. Epoch overflow and cooldown addition
overflow retire only that tenant and never synthesize a new time/identity.

Mutant62 fails the F,S,S,S,S eviction trace with window4,minimum4,threshold3/4:
after the fifth observation, the correct retained list contains four successes
and zero failures. Mutant63 subtracts saved cost at cancellation request;
the delayed-capacity invariant catches it immediately, before eventual second
subtraction could corrupt the ledger. Neither condition is a sanitizer proxy.

The original core has deliberate small finite-generation/epoch domains and a
consecutive-failure policy. The new count-window gate is a separate specified
representation because adapting its policy by modifying the old counters
would break that retained contract. Tests call the original core for the
F,S,F,S counterexample: two-permit cap is equal, health decisions differ.
No throughput comparison may silently label these policies equivalent.

Closure sets cancellation request bits but keeps live references. A neutral
acknowledgment releases one tenant's cost; the other ignored request retains
its full cost and scope. Dropping the creator reference cannot free the scope
until that second completion. The controlled event trace models a backend
that refuses stop; it is not an actual OS cancellation experiment. A real
worker composition must independently establish its completion/quiescence.

All APIs are externally serialized. Epoch/generation checks do not make a
copied pointer safe after the gate's lifetime ends. Admission is staged through
scope retention before changing cost, phase, generation or output; diagnostics
cannot enforce that the caller stops using old borrowed objects. Per-event
work is bounded by two slots and an8-entry maximum window; model and direct
tests assess this finite implementation, not all histories or physical timing.
