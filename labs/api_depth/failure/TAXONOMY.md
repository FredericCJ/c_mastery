# T21 error meaning, cause and public representation

Required PR-T21-01 T21.01.a/b/c and T21.02.a/b/c, with original AX identifiers and
seven-day variants. Families A/B/C/D/F/G/J. Prerequisites are T18 failure/output
contracts, T19 tagged parser results and T20 effect boundaries. Import the actual
validated `av_parse` producer for independent field validation; do not claim the
author reference import is a learner artifact. The error core uses caller storage
and primitive C17 operations; tests and old-client builds use hosted GCC/Clang
O0/O2/NDEBUG and ASan/UBSan. This scoped pilot has no target runtime-closure claim.

## Policy follows meaning, progress and retained obligations

`ae_boundary` accepts exactly one ASCII decimal digit, an internal count0..3 and
synthetic allocation/device failure flags0/1. Reject malformed external input
before use; distinguish an impossible internal count, exhaustion and device fault.
All errors preserve output. The flags select original teaching failures without
invalid pointer accesses or real hardware faults. Define outcomes separately:
invalid input is recoverable by replacement; no-progress busy is retryable;
device failure is terminal for that attempt; lost acknowledgment is degraded
because commit may already have happened. A retryable classification does not
by itself authorize replay of an externally visible effect.

For the changed trust boundary, `ae_admit_digit` validates externally supplied
bytes and creates an admitted value. `ae_trusted_digit` has a precondition that
this value came from successful admission and has not been forged or modified.
It may assert that internal invariant. Both debug and NDEBUG builds still reject
external malformed input; neither test calls the trusted function with a forged
capability. Its public C struct does not enforce non-forgery. Supply the concrete
counterexample showing why a cast or user-written admitted flag is not validation.

`struct ae_counter` makes retry consequences observable. Requests have IDs1..7, deltas1..7,
counter bound63 and effect count bound31. Busy and definite pre-send failure make
no effect. Accepted SEND commits once; LOSE_ACK commits but returns AMBIGUOUS.
Two entries retain (ID,delta) in FIFO order. A retained identical request returns
success without another effect; retained ID with different content is CONFLICT.
Admission/overflow rejection preserves state. After requests1,2,3, ID1 has expired:
replaying it commits again. Demonstrate value6 and four effects for deltas2,1,1,2.
This deliberately bounded dedupe window is not unbounded exactly-once delivery,
durable replay protection or a rule that all timeouts are safe to retry.

## First cause is not the last cleanup symptom

`struct ae_error` records stable public code/operation, resource stage0..3, backend0/1 and
first cause0..65535 where representable by int. Operation IDs1..31 are a declared
bound. Set primary only from an empty record; a later primary attempt conflicts
without changing it. Cleanup adds two secondary(stage,cause) records. Further
failures increment dropped through31; beyond that set a saturation flag. The
record owns values, contains no borrowed message, allocates nothing and performs
no logging. Forty secondary attempts after acquisition-stage2 cause11 must leave
the first cause intact, retain causes77 and78, dropped31 and saturation true.
The next constructor packet must connect these records to actual resource events;
this packet alone is not proof of rollback or successful release.

## Stable public code and optional diagnostic detail

Backend0 maps10/11/12 to timeout/exhausted/invalid. Backend1 maps101/102/103 to the
same codes;104 also maps timeout. Zero is success in either known namespace.
Unknown namespaces/codes map UNKNOWN. Preserve104 in the diagnostic snapshot
even though it shares a public code with101. Consumers must branch on the stable
public taxonomy rather than accidentally depend on backend integers.

`public_error.h` exposes only two uint32_t fields: code and operation. This is a
same-platform C ABI contract, not an eight-byte wire guarantee for all data models.
The private implementation has two different detail layouts. Compile the consumer
once against the installed public header, link it against each archive, physically
relocate the installed prefix and relink. No private-layout rebuild is allowed
for that old consumer. Failure to link, changed public fields or a stale pointer
is failure of the compatibility claim.

Deferred retrieval uses a copied eight-byte diagnostic encoding: version1,
backend,operation,stage,cause low byte,cause high byte,reserved0,reserved0. The
automatic private operation/detail lifetime ends when capture returns; the caller
owns the copied snapshot afterward. Decoder requires exactly8 bytes and validates
version, bounds and reserved bytes before writing outputs. It refuses a cause
that the host int cannot represent. Test every truncated prefix and unknown
version. A valid borrowed pointer to automatic detail would instead dangle.
The richer diagnostic encoding is separately versioned; it is not part of the
stable two-field error layout.

## Status and data need distinct channels

`struct ae_write_result` retains accepted count alongside status. An error after accepting
two of five bytes leaves an unaccepted suffix of three; replaying all five can
duplicate two bytes. Zero-length success is OK with accepted0. OK with accepted
less than offered is rejected by this complete-operation result contract.
AGAIN/DEVICE may accompany a valid partial count. `struct ae_lookup` distinguishes
present value0 from absent; a payload-zero sentinel cannot represent both.

For changed accumulation, validate four independent fields with the actual
T19 parser: `x1`, `32768`, `-`, `7`. Retain field indices0,1,2 and offsets0,4,1,
plus successful field3 value7. Do not use the first field's error as the location
of all failures. The accumulator is bounded at four records and contains no
heap pointers. Staging publishes the complete result once validation finishes.

Deliver classification/action table, progress and dedupe histories, bounded cause
ledger, public/private ABI comparison, copied diagnostic codec and field-error
evidence. Hints: distinguish request from committed effect; never replace primary
while cleaning; write which object owns a diagnostic after return. Per leaf0
wrong,1 nominal trace,2 justified ordinary contract plus discriminating tests,
3 independent changed-condition derivation. The secondary-overwrite and lost-ack
mutants must compile and then fail named assertions. Current author runs do not
provide independent review, delayed learner performance or hardware observations.
