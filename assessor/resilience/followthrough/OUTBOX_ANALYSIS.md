# Outbox technical analysis

The complete X-T75-009 successor also requires
[SERVICE_IO_ANALYSIS](SERVICE_IO_ANALYSIS.md),
[COMPOSITION_ANALYSIS](COMPOSITION_ANALYSIS.md), and the learner-facing
[cost protocol](../../../labs/resilience/followthrough/COST.md). The outbox
alone cannot establish partial-I/O or transport/hedge composition behavior.

The journal's atomic logical unit is the new effect and its event. A second
volatile queue insertion cannot fail because externally serialized preflight
reserved one of four credits. Mutant91 removes that preflight: allow four
notifications to remain unacknowledged while receiver retention expires, then
submit a fifth distinct ID. The effect commits, but no event slot exists. The
post-effect FULL result is too late to repair that violation.

ACKs reference the ordered sequence of nonduplicate effect records. Requiring
ACK[i] to match exactly event[i] proves a cumulative acknowledged prefix, rather
than merely accepting an increasing number that could skip a lost notification.
Body flush precedes footer publication; final flush precedes credit return.
Mutant92 returns success with only cached footer bytes. Reset then redelivers an
event after the API promised durable acknowledgment. An uncertain flush can
leave either durable outcome; keeping the live event while poisoned avoids
guessing. Recovery is the only transition that selects the actual prefix.

Restoring all unacknowledged events to WAITING gives at-least-once delivery.
The event sequence stays constant across resets. A consumer with an external
non-idempotent effect needs a durable consumer-side transaction or dedup record;
the producer cannot prove exactly-once arbitrary external behavior by a send or
ACK convention. A stale ACK before current redelivery is deliberately refused.

The baseline finite receiver window is an explicit replay hazard. A seen bitmap
adds no-result-after-expiry safety. Mutant93 forgets that safety and reapplies an
expired ID. Persisted kind3 rotation clears both the receiver and reconstructed
seen set, preserving accumulated total. The epoch is a stable part of logical
identity, not an authorization secret. Coordinating old-worker quiescence before
rotation is caller discipline; an old token's later input is nevertheless STALE.
The45-request test exercises actual ID reuse under changed epochs, not merely a
larger integer type. Finite128-record storage eventually returns FULL.

The logical oracle uses ordered events and a map of retained results. It does
not duplicate the C ring or ACK codec. Native fork/file execution, byte-cut
tests and the logical histories establish different bounded facts. Full-record
corruption is fail-closed. A checksum is not evidence against an active forger,
and the stated durable-prefix storage model is not a universal power-loss model.

Assess R1 by demanding ownership/clock/storage premises, R2 by a fifth-credit
counterexample and epoch proof, R3 by exact controls and independent results,
and R4 by an original consumer-policy analysis after delivery-before-ACK reset.
Do not award independent mastery from an author-supplied implementation.
