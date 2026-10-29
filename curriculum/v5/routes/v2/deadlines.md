# Pilot B: event-loop deadlines to measured representations

This path is authored as episodes in `contextual-v2`: EP-modular-deadlines
(U-T30-03-01, U-T30-03-02, X-T30-002), EP-deadline-scan (LAB-HS-03),
EP-deadline-heap (U-T14-05-01..04), EP-indexed-cancellation (U-T24-05-02),
EP-timer-wheel (LAB-RZ-05, LAB-RZ-06), EP-deadline-comparison (U-T24-05-03),
EP-sampled-records (U-T43-09-03) and EP-deadline-transfer (X-T75-003). Each
card's "Episode" section states the entering state, the limitation that
motivates the step and why the context changes afterwards.

Wrapped tick counters come first. An 8-bit tick decides due/future/equal only
under a less-than-half-range distance, and two multiwrap histories can produce
the same observations (X-T30-002). That limitation is why the collection below
stores absolute monotonic deadlines.

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

Work in your own initialized workspace, not in author tools:

```sh
python3 -m cmastery study next PROFILE
python3 -m cmastery init PR-T24-05 WORKSPACE --unit U-T24-05-03
cd WORKSPACE
cc -std=c17 -Wall -Wextra -Wpedantic -Werror -O2 -I labs/hosted_systems/timer_depth \
  labs/hosted_systems/timer_depth/collection_tests.c \
  labs/hosted_systems/timer_depth/collection.c labs/hosted_systems/timer_depth/timer.c \
  -o collection-tests && ./collection-tests
```

`timer.c` is your accepted timer from the earlier increment. Add
`-DTC_COUNT_WORK` for the operation-count build. Build
`collection_bench.c` separately with `-O3 -g` and no sanitizer or counter for
timing, and `collection_native.c` with `-O2` for the timerfd adapter. These are
separate builds: the correctness tests and the optimized comparison never share
a binary. `python3 labs/hosted_systems/timer_depth/run.py --mode core` builds
the timer's own public tests. The separate author campaign links assessor
implementations; it is author evidence, not a learner command, and its results
say nothing about your submitted sources.
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
