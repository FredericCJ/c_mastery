# Barrier reference analysis (withheld)

The retained CMX barrier is a useful actual producer but has no membership
registration, per-consumer ticket or generation exhaustion policy. The adapter
adds these rather than modifying the accepted producer. Raw arrived0 means the
cohort was released, not that all former users collected their results. Each slot
therefore retains round and terminal state until one collection. Configuration
epoch invalidates released handles across quiescent reconfiguration; raw round
never resets and saturates at a deliberately small selectable limit.

`settle` samples the actual selected CMX result before any decision. Admitted
timeout uses CMX abandonment to mark shared broken state and wake peers, keeping
initiator TIMEOUT. Cancellation similarly keeps CLOSED. An already due deadline
before CMX enrollment is a non-admission; it is intentionally different. A reset
requires both result drain and explicit registration release, not merely a ready
task or zero arrival count. Members cannot run unrelated task operations or exit
while these borrowed registrations/results remain live.

The36 two-round permutation pairs include a fast next-round arrival before other
old results are collected. Mutation1 ignores the round and lets an old ticket
observe the new pending result; mutation2 releases before cohort completion;
mutation3 ignores cancellation; mutation4 stages reset before drain; mutation5
wraps at the reduced limit. These are semantic controls, not compiler errors.

Native publication relies on real pthread exclusion/condition reacquisition.
Each pre-arrival plain write precedes releasing the node mutex; final arrival
has acquired the sequence of preceding releases. All released threads acquire
the published result. A second full logical barrier prevents next-round writes
overlapping old reads. Removing that reuse barrier is a separate data race even
if the arrival barrier is otherwise correct.

In the tree, one leader per leaf carries both leaf writers to root, then carries
all root-visible writers back through the leaf release. Nonleaders must wait for
that second leaf phase. Fixed four-thread execution observes32 rounds and512
plain reads per topology, with exact per-node arrival/generation counts. It does
not establish all weak-memory histories or a performance superiority. A general
tree storage/hop proof is an analysis obligation, not a larger executed fixture.

No pthread cancellation or unexpected valid-control mutex/condition failure is
injected by this narrow native campaign. Such failures require port-fatal retention,
not a claim that cleanup/retry is transactional. Core timeout/cancel/reset failures
are concretely executed separately. M4 execution checks target ABI/helper closure
with logical tasks; it does not provide physical latency or task-switch evidence.
