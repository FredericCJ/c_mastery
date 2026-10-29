# Pilot B: event-loop deadlines to measured representations

PR-T24-05 already has an absolute deadline adapter and `struct td_timer` state machine.
Keep its timer identities, lowest-slot tie policy, periodic accounting and callback
borrow semantics. A cancellation request while a callback is running is not
permission to destroy that callback's storage.

Start with the existing bounded unsorted collection. Find the earliest armed
registration by scanning N entries: O(N) query work, constant auxiliary storage,
and no index repair. This is a justified design for three timers. Count actual
operations for the service's arm/cancel/expire mix before replacing it.

The next workload has many idle timers and frequent earliest queries. Derive a
binary min-heap invariant over (deadline, stable slot identity), then implement
parent/child arithmetic without overflow. Root order is insufficient: verify
every edge and the permutation of live handles. The collection extension is
specified in `labs/hosted_systems/timer_depth/COLLECTION.md` with learner source
`collection.c`, public tests and a separate assessor implementation.

Cancellation changes the representation obligation. An indexed heap maintains
both heap-position-to-slot and slot-to-position mappings; every swap updates both.
A generation-checked handle rejects a notification for a reused slot. Capacity,
generation exhaustion and rollback on failed rearm must preserve the old state.
Compare with lazy deletion only if tombstones and cleanup work have explicit
bounds; unbounded deferred work is not equivalent service behavior.

Change deadline precision and distribution to introduce X-T75-003's finite
hashed wheel. Derive bucket placement, collision handling, long-gap advancement
and generation lifetime before scaling its storage. An exact-deadline wheel
must still enforce the same due-time/tie contract; a rounded wheel is a different
contract and must be measured as such. Dense collisions can eliminate the
expected advantage. No representation is required to win.

Use `python3 tools/run_deadline_pilot.py --help` for the author campaign's current
options. Its correctness tests and optimized comparison are separate builds.
Use identical deterministic event traces for scan, heap and wheel: steady sparse
arrivals, clustered expirations, cancellation-heavy churn and periodic catch-up.
Vary capacities through small/cache-resident/larger regimes within bounded memory.
Check identical delivered identities and effects outside the timed interval.
Retain raw times and actual PMU enabled/running counts when hardware is accessible.

The independent transfer brief describes a service with a maximum number of
outstanding jobs, cancellation guarantees, coarse acceptable wakeup precision and
bursty overload. It names no collection. Require selection and rejection of at
least two plausible alternatives, then change precision or cancellation rate.
Score the invariant and cost argument, borrower lifetime, native measurement
protocol and willingness to keep an inconclusive performance result.
