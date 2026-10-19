# X-T51-002 delayed journal effect

This companion imports actual MBOnce and retains both its permanent and retry
policies. The separate frozen CDOnce adapter supplies target CMX enrollment,
blocking/cancellation and eight-entry publication. This journal component is
externally serialized; it does not replace that scheduler adapter or claim a
lock-free implementation. Required output/control/input objects are live and
disjoint, initialization occurs only before any caller/ticket survives, and
the journal remains live throughout every once operation using it.

Zero-initialize SxJournal. It stores at most3 synthetic append records, each
ID1..3 plus tag0..255; append and retained identity occur in one atomic model
transition. There is no disk or crash persistence. IDs never evict or recycle.
Initialize SxOnce with one fixed ID/tag and selected permanent/retry policy.
Successful entry elects participant0..2. Step0 appends the record unless that
exact ID/tag already exists; a conflicting tag fails the attempt. Subsequent
eight steps build t[i]=17*i+1, and the last step seals checksum484 and publishes
through actual MBOnce. READY copies all entries/checksum. BUSY, recursive,
failed or invalid entry preserves outputs. Attempts retire at UINT_MAX.

Cooperative owner cancellation before final publication clears unpublished
table state and finishes failure through MBOnce. Permanent policy stays FAILED;
retry policy admits a new owner but reuses the same journal operation identity.
No ordinary waiter can cancel the initializer. Cancellation cannot roll back
the already appended effect. Only successful final publication increments the
publication count. Readers use the actual MB acquire operation under the same
external serialization discipline.

Required witness: append then cancel, then retry. There are2 attempts and1
successful publication. A naive append-per-attempt produces2 records; the
idempotent journal keeps1. Failure before append also keeps exactly1 after a
successful retry. A completed publication rejects later cancellation. Retaining
the journal across an owner retry is essential; erasing it during full-machine
reset permits the effect to repeat. A durable service needs atomic durable
effect/identity before acknowledgment, which this synthetic journal does not
provide. The unchanged permanent variant is tested at every construction cut.

Author runs of these cuts are not an unaided seven-day learner attempt. Supplied
learner source is intentionally incomplete; assessor implementations remain
outside exports. Native/dedicated emulated adapter evidence and physical evidence
retain their separate original bounds.
