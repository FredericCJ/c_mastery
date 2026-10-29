# PR-T24-04 assessor solutions and delayed assessment

This file and all private reactor sources/oracles are assessor-only. Assess all
nine existing ordinary IDs and their exact `-DELAYED` variants. An author run
closes reference implementation evidence only; it is not an independent learner
attempt or a seven-day assessment.

## Rubric by mechanism

Score each leaf 0 for incorrect state/ownership or unsupported semantics, 1 for
ordinary output without its causal argument, 2 for correct implementation plus
the listed discriminator, and 3 only after independent changed-condition work
at the original minimum delay. A single aggregate green test does not assign
the same score to all nine leaves.

| Leaf | Required level-2 evidence | Changed-condition discriminator |
|---|---|---|
| T24.07.a | Actual read2/EAGAIN/read1 with cursor2 preserved and exact bytes; EOF distinguished | Another descriptor consumes readiness, immediate EAGAIN is tolerated |
| T24.07.b | RX and TX advance independently; TX buffer survives EAGAIN and partial cancellation | Slot generation, direction and operation sequence; stale same-connection completion cannot advance a new submission; all six settlement orders retain storage |
| T24.07.c | Persistent producer does not monopolize bounded turns; limited flawed drain shows starvation horizon | Zero-byte callbacks still consume work budget; conditional fairness proof |
| T24.08.a | Actual LT/ET partial-drain contrast; remaining work carried to EAGAIN | ONESHOT arrival between drain and rearm is recovered after MOD |
| T24.08.b | Actual unregister/close/descriptor-number reuse; old token rejected before payload access | Reduced-width generation retires; operation sequence exhaustion separately refuses reuse |
| T24.08.c | Exact order and separate per-participant/global/work quotas with rotating start | Callback publication enters next turn, including self-publication; nested turn rejected |
| T24.11.a | Same overload trace under three policies, explicit rejected ownership and accepted terminal ledger | Variable byte budget and empty-frame bound; lifetime ledger exhaustion distinct from queue capacity |
| T24.11.b | Every accepted ID settles once through drain/cancel/release and repeated stop | Pending message owners remain OUTBOX while borrowers exist, not merely a deferred service destructor |
| T24.11.c | Empty/full/peer paths have mutex enrollment, predicate recheck and joins | Stop/timeout races and forced orders publish one immutable result with cleanup |

Follow the three graduated learner hints per leaf and record hints used. During
delayed assessment provide only the original task, public contracts and necessary
prerequisite artifacts. Retain prediction time, source identity, attempted date,
raw observations, reasoning and assessor findings. Do not disclose this solution,
the independent oracle or mutant outputs in the learner package.

## Reference reasoning and alternatives

`struct rd_operation` preserves `done <= goal <= 8`. Positive progress is bounded by the
remaining extent; would-block changes nothing. Receive bytes are copied before
advancing its cursor. Separate TX storage is unchanged by RX and remains live
until its own terminal event. EOF inside an extent is terminal partial progress,
not a successful full transfer. A single combined RX/TX cursor cannot satisfy
the native three-received/two-sent-and-cancelled trace.

The registry keeps metadata stable while tokens may remain cached. Only after
validating slot, connection generation, direction, operation sequence and pending
state may completion touch operation storage. Connection identity alone was an
insufficient initial design: duplicate completion1 could affect submission2 in
the same connection. The current tests retain the old token and expose exactly
that counterexample. Per-direction sequence never wraps; new connection generation
is required after exhaustion. These identifiers prevent accidental stale reuse,
not forgery by an actor able to manufacture a currently valid token.

Close marks the slot unavailable to new work. Target RX, target TX and one callback
borrow can finish in six orders; all are executable checks. Reuse occurs only
after the last obligation, and the new generation differs. The native epoll probe
uses slot2 generation7, explicitly DELs and closes, forces the old descriptor
number onto a different owned pipe with dup2, and opens generation8. The cached
event remains a valid C value but no longer identifies a live connection.

For the scheduler, each turn has an entry snapshot and an accumulating next-turn
ready set. Within the snapshot, the reference visits participants in rotating
order, spending at most two successes per participant and the selected callback
budget. With three ready all-progress participants, six successes are below the
global cap of eight. A zero-result callback can consume the remaining callback
budget but cannot make the turn infinite. Every continuously ready participant
is first within three turns; this proof assumes finite callbacks and continued
dispatch, and is not a bound on OS scheduling latency. Alternative ready queues
are valid if they preserve the declared service order or explicitly implement
and assess an equivalent fairness contract.

The independent schedule model uses ordered sets and arithmetic quota counts.
Its 8 ready sets ×3 start positions ×8 work budgets ×4³ callback-type triples
give 12,288 exact rows. It checks service order, successful/callback counts,
next readiness and next start. Reentrant publication and nested-dispatch rejection
are separate concrete tests, not falsely inferred from that nonreentrant alphabet.
The flawed unbounded-drain policy is run for a disclosed 16-callback horizon,
all spent on the persistent producer; the liveness failure is then argued from
its missing handoff condition rather than from a finite nontermination claim.

`struct rd_service` imports the actual fixed outbox. The terminal ledger includes all
accepted IDs in its finite epoch, so it has its own exhaustion policy. Count-only
admission cannot implement a variable byte budget; 297 independent limit/length
cells cover limits0–32 and lengths0–8 in the service wrapper, while the prerequisite
outbox has its own broader 2,313-cell campaign. Do not reuse the prerequisite
number as newly executed service coverage. Three policies run the same nine-frame
trace. A paused service needs the native adapter to suppress actual input admission.

During shutdown, cancellation phase prevents new work but does not release
pending message ownership while a callback may access it. An initial implementation
deferred only service release and published cancelled messages too early; the
current owner assertions and `RD_MUTANT_EARLY_MESSAGES` reject that defect.
The last borrower finalizes pending messages and then releases the service.
The native workload has 32 accepted IDs: one complete wire frame, 31 cancellations,
and no extra terminal records after repeated stop. Actual elapsed time is retained
without declaring a universal maximum or promising rollback of kernel-copied bytes.

For `struct rd_port`, enrollment and the state predicate share a mutex. A controller
obtaining it after an entry announcement cannot slip a stop notification into a
check/unlock/wait gap. The notification-without-predicate test requires reentry,
then a real value42 handoff. The timeout/stop test has 200 barrier-started races
per profile, plus deterministic stop-first and timeout-first paths. Neither race
outcome is mandatory in every run. Successful transfer, stop, peer-gone and timeout
are application outcomes selected under the lock; late notifications do not
rewrite the returned result. No pthread cancellation is sent to these workers.

## Falsification and cost

Compiled core mutants turn EAGAIN terminal, ignore connection identity, ignore
per-operation identity, wrap a retired generation, exempt CPU-only callbacks
from the work cap, release a borrowed service, or publish pending messages early.
Threaded mutants select the wrong stop outcome or trust a notification without
rechecking. Every mutant must compile and exit1 at the intended assertion.
The predicate mutant may first hit the bounded enrollment timeout because it
incorrectly completed instead of reentering; this is a deliberate failed state
obligation, not a compiler or process-crash substitute.

Correct waiter references run under GCC/Clang ThreadSanitizer, with an original
known-race control required to produce the intended diagnostic/exit66. That race
has undefined ISO C behavior; only the stated instrumented diagnostic is expected.
ASan/UBSan and TSan remain bounded dynamic observations, not proofs of all lifetime
or concurrency properties. Compiled learner stubs and a missing prerequisite
stream implementation must also be rejected.

Core storage is bounded by four registry slots with two eight-byte operations
each, one three-participant scheduler, the actual outbox/messages and 64 terminal
records. Use actual `sizeof`/object-size/stack reports for the selected ABI; do
not equate payload capacity with process memory. Registry lookup is O(1), core
dispatch is bounded by callback budget, terminal lookup O(64), and imported
outbox invariant/duplicate validation has generalized O(K²) cost. Condition wait
and OS scheduling have no hard latency bound supplied by these tests. A stalled
callback defeats quiescence regardless of byte budgets; the contract requires
cooperative finite callbacks or a different containment boundary.

## Validation and transfer limits

The producer is `tools/run_reactor_followthrough.py`; its final source-bound
report, separate rejection guard and real CLI export check form author evidence.
Pilot reports precede operation-token and borrowed-message corrections and are
retained as historical development observations. Later code needs a fresh full
campaign. Independent learner providers, non-author review, seven-day attempts,
other-OS portability and physical measurements remain separate.

For transfer, ask the learner to change one premise: a callback publishes itself
and then reports terminal, a frame has zero payload at a zero-byte budget, a stale
completion has the correct connection but wrong operation sequence, or stop
arrives while both async directions and a callback are live. Require a new
state argument and discriminating case; simple constant substitutions do not
establish independent transfer. No finite campaign proves absolute mastery.
