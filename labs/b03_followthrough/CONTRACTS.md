# B03 followthrough contract, version 1

These are original local C17 teaching components. Import the unchanged version 1
`labs/b03_depth/CONTRACTS.md` and `include/depth.h`. Non-atomic state objects have
one owner, stable lifetime, valid initialization and no concurrent/reentrant
access. Required pointers refer to live, disjoint objects. Null required pointers
return INVALID without publication. No MMIO address, OS cancellation primitive,
remote service, persistence or physical timer is used.

## X-T23-002: fair turns and an ambiguous message outcome

`struct bf_round` contains two valid B3 writers and next=0 or1. Both callbacks and context
array entries describe synchronous backends. `bf_turn` selects next, publishes
that index, advances next modulo2, and calls the actual b3_pump with quota0..64.
Even WAIT, terminal or zero-quota turns advance the scheduler. Admission failure
changes nothing. Callback execution must itself return; quota cannot preempt a
blocked callback. With positive quota and a ready cold backend making one byte
of progress per call, the cold connection gets a turn within two invocations.

`struct bf_message` starts all-zero. Requests have ID1..3, 0..6 bytes; quota0..6 is a work
budget. An accepted ID owns a copied request until the next completed request is
replaced. Only the latest ID may be retried, with exactly matching length/bytes.
A new ID must be the next sequential ID and the old operation must be complete.
Old/future IDs return STALE; ID0/4+ are INVALID; conflicting same-ID bytes FAILED.
These failures leave state/output unchanged. Byte values are application increments.
Each step atomically adds one byte to total and advances retained done/effects.
At most18 application steps and4590 total can occur over this owner's lifetime.
This atomic pair is an explicit in-memory service-model premise. It is not a
crash-safe storage transaction. Partial effects are externally observable through
total; a timeout is represented by WAIT, preserving result. `lose_reply=1` returns
WAIT even after completion. Retrying the same ID completes only missing steps;
a final repeated request returns the retained total without repeating effects.
No reset, restart, eviction, parallel client or ID recycling is admitted. A durable
service would need atomic persistence of effect/progress/identity before reply.

## X-T24-002: queued is not quiescent

`struct bf_operation` starts zero; at most three starts. Start while owned or after ID3
fails unchanged. TIMEOUT moves ACTIVE to REQUESTED; repeats retain REQUESTED or
QUEUED. REJECT is admitted only in REQUESTED and returns to ACTIVE, retaining all
ownership: completion remains possible. QUEUED_ACK only in REQUESTED moves to
QUEUED with ownership/report count unchanged. COMPLETE wins in every owned phase;
QUIESCENT cancels only from QUEUED. Only these final events guarantee that the
backend will no longer access the borrowed resource, release ownership and add
one report. Mismatched generation, unexpected acknowledgments, and all terminal
duplicates return STALE unchanged. A late final event may arrive after completion
but cannot publish again. Invalid events return INVALID. This extra phase models
an API contract; it does not manufacture a real kernel's cancellation guarantee.

## X-T25-002: explicit claims

`struct bf_box` is initialized before publication, exactly two producers and one consumer,
live until all threads join. No payload escape or concurrent init. EMPTY0 ->
WRITING1 is a strong CAS claim; only its winner writes ordinary payload. Release
FULL2 publishes the payload. Consumer CAS FULL2 -> READING3 acquires it; consumer
release EMPTY0 publishes completion of the read before a later producer's acquire
claim. Failed CAS uses relaxed order and returns WAIT, preserving output. Claim
success uses acquire (also valid for an RMW); both publication stores use release.
BF_SC=1 selects seq_cst for these operations under the same ownership contract.
Calls use a bounded number of atomic operations but atomics may need implementation
helpers. A producer paused after claim blocks this one slot. There is no fairness,
wait-free or system-wide lock-free guarantee. Cancellation cannot steal a live
claim. BF_ATOMIC_MUTANT is an assessor fault injection, never learner behavior.

## X-T28-002: consuming reads and W0C

`struct bf_register` starts with logical status0..15 and zero access counters. clear_read
returns all current bits and clears them in one synthetic read, including empty
reads. A debugger's first read therefore consumes evidence a later read cannot
recover. w0c accepts mask0..15, makes exactly one write/no read, and encodes an
8-bit command with reserved bits7..4 required1: wire=255 XOR mask. Device action
clears a status bit only where the command bit is0. Invalid masks change nothing.
last_write exposes the command. External event injection sets bits between calls;
same-bit events coalesce, and a selected bit arriving before the acknowledgment
can be cleared by it. Unselected bits survive. Counters do not overflow in the
bounded exercises. Logical model widths are not an asserted C struct bus layout.

## X-T30-002: coherent sampling is a separate obligation

bf_coherent invokes a synchronous half-read callback high,low,high, up to0..8
attempts. Halves must be0..255. Matching high reads publish high*256+low. Otherwise
retry; quota exhaustion returns WAIT without output. Out-of-range half is FAILED
without output. Callback context can change; no automatic rollback is promised.
The underlying device model increments a whole16-bit monotonic counter atomically;
individual half reads are atomic and ordered. Between the two high reads fewer
than256 high-word changes occur, so equal high values cannot hide a high-word
wrap. Under that premise the accepted low read determines a coherent sample.
The exhaustive test narrows increments to at most1 after each read. Software
updating low and high in separate stores does not satisfy the premise: it needs
a correct sequence protocol or interrupt masking with explicit nesting rules.

bf_due16 is the original half-range predicate widened to16 bits; delay0..32767,
true elapsed<32768, detectable delta>=32768 AMBIGUOUS unchanged. Full wraps remain
undetectable. Coherent sampling cannot establish this history premise.
bf_ticks computes ceil(milliseconds * denominator / numerator), where one tick
is numerator/denominator milliseconds. Zero numerator/denominator or overflowing
product is INVALID unchanged; positive rounding prevents an early deadline in
the ideal rational clock. It does not bound oscillator error or scheduling delay.

Changing8-bit/1ms ticks to16-bit/0.5ms ticks preserves a selected127ms delay at
254 ticks, below32768. The original elapsed<128ms sampling promise becomes
elapsed<256 new ticks when retaining that same conservative real-time horizon;
the representation could admit a larger horizon only after an explicit contract
change. Read coherence, modular ambiguity, units and physical accuracy are separate.
