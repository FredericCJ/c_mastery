# X-T75-006 required window and tenant-capacity successor

This shared extension implements T75.06.a-d and X-T75-006-DELAYED without
replacing the retained consecutive-failure RZGate. Bring the actual original
gate, X-T03-005 finite identity reasoning and X-T75-001 owned scope. The
io.async and measurement.capture prerequisite contracts establish the separate
completion/measurement obligations; this model does not claim to call a
nonexistent backend API. Implement `src/breaker_follow.c` against
`followthrough/breaker.h`, supplied `breaker_test.c` and the independent list
model `tools/verify_resilience_breaker.py`.

The inspected primary comparison is [Resilience4j CircuitBreaker](https://resilience4j.readme.io/docs/circuitbreaker),
sections Count-based sliding window, Failure rate and slow call rate
thresholds, and the thread-safety discussion. It distinguishes retained health
observations and a minimum sample threshold from a concurrent-call limit.
[Bulkhead configuration](https://resilience4j.readme.io/docs/bulkhead) separately
defines concurrency and waiting bounds. These are comparison contracts, not
libraries imported into this implementation. Exact registered locators and
bounded inspection notes are in resilience_resources.json and
resilience_followthrough_sources.md. Do not infer arbitrary Java library
behavior or exact C17 rules from these pages.

## Environment, representation and policy

Use C17 on hosted Linux with GCC/Clang and declared debug/release/sanitizer
profiles. The core is externally serialized; callers provide one consistent
lock if used by concurrent workers. No hidden scheduler, backend, allocation
or ready-made breaker implementation is permitted. It retains an already
allocated scope borrow per admitted operation. Host Python verifies models;
there is no target/physical execution or weak-memory proof claim.

`RzbGate` owns two independent health records and four permit slots, two slots
reserved for each tenant. Reservations a,b are1..4 abstract resource units,
and an operation's cost is1..that tenant's reservation. Each tenant can have
at most two operations as well as the cost bound. One tenant never borrows
the other's reserved units or slots. Unused reservations can reduce aggregate
utilization; this is an explicit isolation policy, not a work-conserving
scheduler or fairness guarantee. Health is classified per tenant in this
variant. A shared backend-wide breaker would be a different documented policy.

The common policy has count-window size1..8, minimum observations1..window,
and rational failure threshold numerator/denominator with
1<=numerator<=denominator<=16. Keep the last N **classified completions**,
excluding neutral outcomes. Maintain count, head and failure total with
subtract-on-eviction; the independent oracle must instead recompute from its
retained list. Trip exactly when the minimum is satisfied and
`failures*denominator >= count*numerator`. These bounded products fit unsigned
int on the declared implementation. This is a count window, not a time window.

Health phases are CLOSED, OPEN and HALF_OPEN. Trip advances a nonwrapping
epoch and opens for four ticks. At exact cooldown equality, one newly admitted
current-epoch probe may enter. A successful probe closes and clears old
observations; a failed probe opens a new epoch; a neutral probe leaves
HALF_OPEN and releases its capacity so a later probe can be admitted. Old
epoch completions release their original cost/scope but contribute no current
health evidence. This includes an old success arriving while a new probe is
outstanding. Epoch exhaustion or unrepresentable now+4 retires that tenant's
health gate rather than wrapping into an old identity. The other tenant stays
independent.

## Admission, completion and cancellation

Each permit identifies owner address, slot and nonwrapping generation. The
slot retains the admission epoch, cost, cancellation flag and scope borrow.
The gate must remain at a stable address, cannot be copied/reinitialized while
any token survives, and must not be modified outside its API. An unused
generation UINT_MAX slot cannot be admitted again; its last live generation
may still complete normally. Fields are public for inspection/tests, not a
permission for clients to forge state. Arbitrary memory modification is outside
the capability check's guarantee.

`admit` rejects malformed tenant/cost/output, backward time, closed state,
cancelled/expired scope, unhealthy state, insufficient tenant cost capacity,
an already active half-open probe or retired slots. It retains the scope and
publishes the permit only after every refusal check. Failed admission preserves
gate and output. Current phase changes from OPEN to HALF_OPEN only on actual
probe admission. A free permit slot does not imply available cost units.

`complete` accepts only one valid live-generation permit, nonbackward time and
outcome -1 neutral,0 backend failure or1 success. It releases the saved cost
and scope exactly once, then applies the epoch/health rule. Classification is
explicit: client cancellation/error can be neutral, but merely requesting
cancellation must not hide a later genuine backend failure. Stale/duplicate
completion preserves state. The provided original scope allocator's callbacks
retain their no-reentry contract.

`cancel` sets a request flag only. `cancelled` observes that flag or the retained
scope's cancellation/expiry; it does not acknowledge quiescence or release
capacity. `close` refuses future admissions and requests cancellation of all
live operations. `quiescent` remains BUSY until each actual completion has
arrived. A backend that ignores cancellation can keep capacity indefinitely.
The owner must keep the gate and dependency objects alive; a timeout may
report this fact but may not free their storage. Closing does not prevent a
late completion or discard its cost ledger. This model supplies no fabricated
OS cancellation acknowledgment.

Prove per tenant that used cost is the sum of active saved costs and never
exceeds its reservation; prove slot count<=2 separately. Health history size
does not appear in either capacity invariant. Prove reference retention until
completion and that old epochs cannot alter a probe. C types/atomics in the
scope do not enforce ownership of copied permit values, serialization,
outcome classification or backend cooperation.

## Concrete work and acceptance

D1/R1: derive the health/window and cost/permit invariants independently,
explain neutral classification, and give a small example where a healthy but
slow backend occupies all cost units. Analyze why another tenant's reserved
capacity remains available even if the first tenant never acknowledges stop.

D2/R2: implement all operations, finite generations/epochs and cooldown
retirement. Exercise owner mismatch, duplicate completion, invalid outcome,
backward time, exact cooldown equality, a live UINT_MAX generation, exhausted
slot pairs, epoch UINT_MAX and now UINT64_MAX-2. Publication and active-owner
state must remain unchanged on refusal. Use real scope retain/release calls.

D3/R3: execute both completion orders for two admitted permits and all
neutral/failure/success classifications. Hold one old permit while other
completions trip the window; admit a new probe, then deliver the old success
and show the probe remains authoritative. Independently recompute the retained
observations after every event. The default4/min4/one-half policy trips on
F,S,F,S, whereas the actual retained consecutive-two-failure gate stays CLOSED.
These health policies deliberately differ; compare them under the same
two-permit admission cap and report recovery/capacity consequences without
calling their correctness contracts equivalent.

D4/R4: run two tenants with reservation2 each and cost1/cost2 requests.
Demonstrate full-cost refusal despite another tenant's idle capacity, then
prove that the other tenant still admits its own cost2 request. Request stop
for both active operations; complete only one and show the other remains
owned and prevents quiescence. Compare the four supplied window configurations
and record phase, epoch, retained observations and resource occupancy over
virtual time. Apply the13-dimension cost contract separately to measured host
costs; a virtual cooldown is not physical response latency.

Run `resilience.breaker_window`, its three semantic controls, and
`tools/verify_resilience_breaker.py --driver <build>/resilience-breaker`.
The model checks162908 structured histories and5000 seeded mixed-tenant
histories across four policy configurations, recomputing counts from lists.
Direct cases cover extreme counters and clock values rather than pretending
to enumerate2^32 generations. Three controls admit old-epoch health evidence,
forget subtract-on-eviction, or release cost at cancellation request. Each
must fail its intended assertion after the positive passes; unrelated build
or runtime failure is not successful mutation rejection.

Level1 reproduces a nominal trip but conflates health and capacity. Level2
requires all relevant implementation, invariants, model agreement, controls
and failure/lifetime cases. Level3 requires independent changed conditions and
the delayed assessment. Graduated hints: track an admitted cost separately
from a failure observation; then save the admission epoch; finally write the
capacity sum before implementing cancellation. Private technical analysis is
published only in the assessor tree.

## Delayed transfer and bounded security experiment

X-T75-006-DELAYED requests cancellation for two active cost2 operations in
different tenants. One backend acknowledges neutrally; the other ignores the
request until a later chosen completion. Close the gate, drop the creator scope
owner, and show that the second permit keeps both scope and capacity alive.
Only its later completion permits quiescence and final scope deallocation.
The public test supplies this controlled backend-event schedule; an actual
worker/reactor composition must retain the same rule and record its own run.
The independent learner attempt occurs after the required delay and changes
reservation/cancellation order without copying the worked trace.

The target is the original local synthetic gate. Adversarial inputs are bounded
cost bursts, stale completions and ignored cancellation; the property is
reserved-capacity and health-epoch integrity. The harmless success condition
is an incorrect used-cost counter or a stale result altering current health.
It requires those exact admitted events, not a third-party service. Restore
the epoch check, eviction subtraction and acknowledgment rule; retain all
regressions. Tenant identity here is a trusted index, not authenticated
security isolation. Arbitrary memory writes, caller misclassification,
unbounded backend delay and external scheduling remain limitations. No host
settings, external targets or real data are involved.

Source-bound receipts separately establish actual executions. Finite histories
do not prove all schedules, physical resource costs or learner mastery. Exact
published C17 and independent/physical evidence remain explicit gaps; other
T75 mechanisms stay required.
