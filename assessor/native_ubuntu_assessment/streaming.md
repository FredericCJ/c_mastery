# Withheld packet: fragmented acquisition log

## Prerequisite check

Require accepted bounded-byte storage, segmented decoder, ownership/backpressure,
and SPSC publication artifacts. Request the learner's live imported bundle hashes.
For the optional native extension require perf ABI protocol competence separately;
a successful byte-ring test does not establish it. Ask the learner to explain
one empty/full transition and one release/acquire edge without opening old code.

## Initial task to release

An acquisition device produces records with a 2-byte big-endian payload length,
a 1-byte kind and 0–31 payload bytes. Length includes kind. Zero or length above 32
is malformed. Delivery may split anywhere, including between length bytes.
At most 128 input bytes and eight complete output records may be retained by the
service. A consumer accepts one record or refuses progress. It may retain an
accepted record until an explicit release, and the service must not overwrite it.

Design and implement an adapter. Report the exact consumed input prefix and
preserve accepted records across a full output queue. No read past the admitted
span, silent byte loss, duplicate delivery or allocation outside the supplied
storage is permitted. Malformed framing terminates that stream; already accepted
records remain valid. End of input distinguishes clean completion and truncation.
Select a representation and reject a plausible alternative before implementing.

## Assessor trace, not released initially

Use bytes `00 03 11 A0 A1 00 01 22 00 02 33 B0`. The three records have kinds
11, 22, 33 hex and payload lengths 2, 0, 1. Supply one byte per call, then all bytes
at once, then every two-part split. Stall after accepting record 1; release it only
after enough offers to fill the remaining slots. Verify the same record sequence
and exact accepted-prefix accounting independently of internal buffer positions.

Reject lengths 0 and 33, a final lone length byte and a truncated payload. Retain
a consumer borrow during shutdown. Force repeated stop and release and verify
that every accepted record has one terminal ownership result. A crash, timeout or
compile error is not equivalent to the expected malformed-frame rejection.

## Changed condition to withhold

After the delay, the producer and parser are separate threads and the consumer
can suspend indefinitely with two borrowed outputs. The input source supplies a
shared-memory ring whose published position is not an application record count.
Provide an unfamiliar documented metadata layout and require an adapter without
naming the earlier data structure. The learner must identify which ordinary
storage/invariant arguments transfer and which ordering/ABI rules must be reread.
If real perf is used, replace this synthetic ABI with the actual kernel contract
and keep lost-record and denied-hardware states explicit.

## Scoring discriminators

Mechanism 2 needs an explicit state machine and progress invariant; 3 adds a
minimal counterexample to an abandoned design. Correctness 2 handles every split
and failure; 3 adds a separate oracle or metamorphic trace not copied from the
implementation. Ownership 2 retains accepted borrows through pressure/shutdown; 3
explains the new publication and reuse edges. Resources 2 proves both byte and
object bounds; 3 accounts for stalled borrowers without hiding kernel buffers.
Diagnosis 2 locates the first invalid transition; 3 discriminates ordering from
parser corruption. Transfer 2 adapts unaided; 3 states exactly which prior proof
must be replaced and which remains valid.
