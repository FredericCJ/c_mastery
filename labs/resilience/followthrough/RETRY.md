# X-T75-005 required retry and retained-transport successor

This shared contract adds executable support to X-T75-005, T75.05.a-d and
X-T75-005-DELAYED. The original rubric, inspected references, security rules and
13-dimension cost contract remain applicable. It does not replace independent
learner assessment. Use the exact source locators in X-T75-005, particularly
R-MOD-RZ-RETRY and R-MOD-RZ-IDEMPOTENCE; the arithmetic below is an original
derivation, not a claim that the referenced services use this generator.

## Dependencies, supplied artifacts and environment

Complete X-T75-001's owned `struct rzf_scope`, X-T75-004's fractional `struct rzf_bucket`, and
the original `struct rz_retry` first. Import the actual `communications.tcp_fault`
artifact from PR-T41-06/LAB-EX-09: `v4_com.c`, `net.c`, their declared HTTP and
EDU7 dependencies and public headers. The test calls actual `v4_tcp_delivery`,
`net_tick`, `net_receive` and `net_link_down`; do not replace them with a
lookalike retry counter. The io.deadline and measurement.capture prerequisite
contracts inform clock and measurement arguments; this bounded adapter does
not claim to call a nonexistent deadline or measurement API.

Implement `src/retry_follow.c` against `followthrough/retry.h`. Public supplied
tests are `retry_test.c`; the independent Python arithmetic/event oracle is
`tools/verify_resilience_retry.py`. Private implementations and analysis are
separate assessor material. CMake accepts explicit communications and messaging
workspace roots and explicit assessor roots for author checks. Learner builds
must use their prerequisite implementations.

Use C17 hosted Linux, GCC/Clang, debug/release, address/undefined-behavior and
thread instrumentation profiles. The adapter is externally serialized; the
test does not create concurrent access to its ordinary fields. The transport
uses hosted library functions and a large fixed parser object. This is not a
strict freestanding target or a physical network experiment. Python and CMake
are host verification tools, not target dependencies.

## Representation and ownership

`struct rzr_random` owns a 32-bit state and nonwrapping 64-bit draw count. `struct rzr_retry`
owns one `struct rzf_borrow`, the original `struct rz_retry` schedule, its random state and a
single pending schedule. It borrows a stable-address initialized fractional
limiter and a stable-address `struct net_stack` until successful finish/abort. An output
controller must not already own an operation. It may not be copied while live.
Only this adapter may advance or mutate its borrowed transport; sharing a
limiter requires serialization of every user and one common clock origin.

Initialization requires one established retained flight, packet length1..1518,
zero transport retries, RTO1..60000, retries1..3, base1..8 and cap base..32.
The fixture deliberately seeds the retained-packet state, as the imported
fault model does; it is not a full TCP connection handshake. No pending HTTP
response or new application send may create another flight during this
operation. The supported completion input is a peer acknowledgment for this
flight. The synthetic payload is `{client=0,id=1,delta=3}`; receiver fence1 and
retention16 units are explicit. Sender milliseconds are divided by1000 for
that independent receiver clock. All demonstrations finish within retention.

Sink callbacks synchronously borrow the packet only for the call. They must
not retain it, reenter/mutate/destroy the controller or transport, or abandon
the call. Zero reports delivery to the teaching receiver; nonzero reports a
locally injected loss. Delivery is separate from acknowledgment. The scope
borrow remains owned even after cancellation, deadline expiry or exhausting
the retry schedule. `finish` refuses BUSY while a flight or plan survives.
An acknowledgment may settle the flight after cancellation; it does not undo
an already committed receiver effect. Only then may finish acknowledge the
borrow and release its scope. `abort` uses the actual synchronous in-memory
`net_link_down` reset and releases the borrow. It requires no concurrent
callbacks and no queued packets surviving into reuse of this teaching
instance. This reset is not an OS socket cancellation or a remote rollback.

## Required operations and refusal semantics

`rzr_reduce(sample,bits,span,out)` selects from a source interval of2^bits,
bits1..32. Validate sample and span before output. Reject the first
`2^bits mod span` samples with PENDING and unchanged output; map accepted
samples by remainder. Derive equal accepted residue counts. Mere `% span`
without rejection is the deliberate defect. The generator recurrence is
`state = 5*state+1 mod 2^32`, followed by two invertible xor shifts and odd
multiplication to permute the word. `uniform` tries at most64 draws, preserves
output on PENDING/FULL and refuses draw-count wrap. Internal state and draw
count advance on rejected samples. This is a deterministic teaching generator,
not entropy, cryptographic randomness, independent clients or a timing-safety
guarantee. Unbiased reduction is conditional on a uniform source word; a seed
and one finite histogram cannot establish that premise for a deployment.

`original` charges one limiter token and emits the retained packet once. A
repeat call returns DONE without another charge/send. Planning is permitted
only afterward, for a safe-to-repeat operation and an outstanding flight.
`plan` stages the actual `rz_retry_next`: window `min(cap,base*2^attempts)`;
full jitter draws0..window, no jitter chooses window, and equal jitter chooses
floor(window/2)..window. The final due time is the maximum of this result,
`now+retry_after`, and the transport RTO deadline. Starting at or beyond the
shared deadline is rejected; retry-after never extends that deadline.
An admitted plan increments the attempt budget even if later cancelled.
Refusal preserves the attempt count, pending flag and output, but a random
draw already consumed during the computation stays consumed. A second plan
while pending returns BUSY. Original dispatch is counted separately.

`dispatch` before due returns PENDING without a send or limiter charge. At due
it charges the common fractional limiter then calls the actual transport
fault adapter. A limiter refusal retains the plan and flight for a later
admission decision. Cancellation/expiry cancels the plan but retains the
flight. Backward time or time above `UINT64_MAX-240000` is refused; this
explicit horizon protects the imported transport's bounded unchecked deadline
additions. Scope expiry governs starts, not proof of completion by deadline.
An acknowledgment before dispatch removes the flight and plan. Duplicate or
stale acknowledgments may leave the flight outstanding. The supported bounded
operation sends at most one original plus three retries, so its dispatch/drop
counters cannot wrap. Admission errors cannot be repaired by editing public
fields after initialization.

## Deliverables and assessment

1. D1: derive the remainder rejection threshold, full-period recurrence and
   permutation argument; distinguish each from statistical independence.
   Draw the plan/dispatch/flight/borrow state product and show which transition
   owns each token and reference. Derive the upper bound of four sends and
   compare it with nested layer budgets whose work multiplies.
2. D2: implement the API and actual imported adapter. Demonstrate the16 loss
   masks for original plus three retries. With every acknowledgment withheld,
   a delivered effect occurs once under the finite receiver identity contract;
   retransmission is visible as duplicate delivery. Demonstrate late valid
   acknowledgment after cancellation and after creator-reference release.
3. D3: independently enumerate all residues of an8-bit source for spans1..32,
   draw histories/windows2/4/8, deadlines, invalid draws, exhausted attempts,
   64-draw bound and draw-count retirement. Compare the actual C result against
   the Python models, including output preservation and allocator balance.
   Reject the modulo-bias and uncharged-retry mutants after the positive passes.
4. D4: execute the32-client deterministic population for three jitter policies,
   distinct/correlated seeds, planning before/at the RTO and five outage cutoffs.
   Record every retry time, total transmissions, delivered clients, peak work
   per virtual tick and first/last completion. These are virtual event
   measurements, not host execution latency. Apply the remaining cost-dossier
   dimensions separately; show the borrowed transport/parser footprint too.

Run `resilience.retry_transport` and its two named mutation controls, then
`python3 tools/verify_resilience_retry.py --driver <build>/resilience-retry`.
The model reports312 seed/span sequences (65 selections each),6000 admission
states,10080 draw histories and60 population experiments. Retain raw output
and exact compiler/source pins. Passing requires exact model agreement, no
early scope destruction, exactly one charge per send, no receiver duplicate
effect within the declared retention window, and both intended semantic
controls rejected. Unexpected compilation or sanitizer failures are failures,
not valid mutant rejections.

R1 assesses D1's attempt/nested-work and timeout reasoning. R2 assesses the
actual D2 adapter and ownership/limiter invariants. R3 assesses the exhaustive
bounded D3 result and the effect-without-ack trace. R4 assesses D4's measured
tradeoffs and correlated-seed counterexample. A nominal schedule alone earns
at most level1. Level2 requires all relevant reasoning and evidence. Level3
requires the independent changed-condition solution and explanation; author
execution cannot award it to a learner.

Graduated hints: first separate a scheduled start from a retained flight;
then compare residue counts before and after discarding a prefix; finally
check whether an RTO lower bound erases your jitter and whether acknowledgment
can outlive cancellation. Assessor solutions remain outside the learner tree.

## Required delayed transfer and containment

X-T75-005-DELAYED supplies retry-after9000 at now1000 with deadline10000.
Reject the start at10000 without consuming an attempt; keep the retained
flight and later accept its valid acknowledgment. Also move the deadline to
one tick before/after a computed due time and change the seed correlation
assumption. The delayed assessment must be scheduled separately after the
minimum interval; these public fixtures are preparation, not that assessment.

The target is only the original retained-packet/receiver program with synthetic
bytes in one process. The adversary controls the16 bounded loss masks, retry
timing, duplicate delivery and correlated seeds; there is no external endpoint,
real credential or other person's data. The property under attack is bounded
dispatch/ownership and duplicate-effect suppression. The harmless success
observation is a limiter charge omitted by the mutant or concentrated retry
work; unsafe repetition can change only the synthetic total. Exploitability
requires the stated scheduling/identity conditions. Restore the charge,
deduplication and reference-lifetime rules and retain all regression traces.
Finite retention, unauthenticated synthetic identity, model reset assumptions
and arbitrary external mutation remain explicit limits. No crash or sanitizer
finding is called arbitrary code execution.

## Validation limits

The public bounded test/oracle can be executed; current source-bound campaign
receipts separately record which profiles actually ran. Exact published C17,
different data models, physical networks, arbitrary weak-memory executions,
deployment entropy and independent learner assessment are not established.
Other T75 successors remain required. This contract closes no broader claim
merely because its anchor passes.
