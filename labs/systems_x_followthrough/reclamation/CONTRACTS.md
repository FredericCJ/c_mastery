# X-T55-002/003 exact container and registration changes

This original three-node append/read/remove list adds the concrete structure
missing from the original MB trackers. It imports actual MBEpoch for epoch
entry/leave/retirement/advance/collection; original MB hazard and epoch anchor
tests remain separately required. There are exactly2 participants and3 typed
unsigned payload nodes in both modes. Each participant has2 hazard slots for
adjacent-node traversal. All node/control storage stays live at a stable address
until all callers join. No native pointer to payload escapes; copied unsigned
values and generation/section references are the public outputs.

An acquire/release atomic_flag gate serializes every operation over ordinary
metadata and payload. Calls try once and return BUSY with outputs unchanged if
another operation owns the gate. No gate is held between calls or while a reader
is paused. This establishes a documented synchronization domain; it is not a
lock-free hazard algorithm or unsynchronized use of ordinary MBEpoch fields.
No concurrent/reentrant operations by the same participant are permitted even
though the gate excludes data races. Inputs/outputs/control are disjoint and
required pointers are valid. Init happens only before publication; no reset,
raw representation mutation, forged observations or cross-domain handles.

Register selects a free participant and increments generation1..3. Depart is
legal only while inactive and invalidates that registration. Saturation retires
the slot. A stale registration cannot enter, leave or clear a successor hazard.
Enter starts a new nonwrapping section identity and, in epoch mode, calls actual
mb_epoch_enter. Leave clears both slots and ends that section; all old references
then fail. Active departure is refused. Timeout or a pause cannot establish
quiescence. An external successor may clean up only after proving the original
owner has stopped and cannot resume; the ordinary API does not infer that fact.

Append initializes a free node, then links it at the tail under the gate. Remove
unlinks the head and retires it; collection is separate. Hazard node identities
use generation1..3 followed by retirement4. Epoch nodes follow the unchanged
MBEpoch policy: three one-use identities, epochs1..3 and permanent exhaustion.
The equal-pool experiment compares the first three retirements under identical
pause schedules; it does not equate different later generation/reuse policies.

A reader observes head, or the next link of a currently protected node. Protect
publishes its observed candidate in a free hazard slot and revalidates that exact
source link and candidate incarnation before issuing a section-bound reference.
Failure clears the just-published slot and returns RETRY; no payload is read.
An observed empty source returns EMPTY without an output observation. A reference
permits a copied read only while its section and registration remain valid and
the node remains protected. Both adjacent nodes need simultaneous slots in hazard
mode. Clearing protection before the last read invalidates that read. Epoch
entry protects admitted old retired nodes until leave; later sections cannot
recover an old reference by copying its bits. Metadata updates are serialized,
so the argument uses the gate's synchronization, not an invented weak-memory
proof from source order alone.

Hazard collection scans both slots of BOTH participants and retains every
protected retired incarnation. Epoch collection uses actual distance>=2 and
no active old-cohort rule. The fixture pauses reader0 on the head while the writer
removes all3 nodes: hazard retains1 and collects2; epoch retains3 and collects0.
After legitimate leave and the required advances both can reclaim their retained
nodes. Simultaneously protecting two adjacent nodes raises hazard retention to2.
Each experiment records metadata/storage separately from equal payload capacity.

For a long scan, leave is a quiescent point, so every old reference becomes invalid.
Reenter, reobserve the current link, republish/revalidate, then resume through a
new reference. Reusing the old reference is refused even if its numerical node
index remains present. Wrapping epochs would invalidate the ordered-distance
argument: e.g. old reader1, retirement1, synthetic wrap back to1 makes old and new
cohorts indistinguishable. This implementation retains permanent exhaustion at3;
it does not silently impose an unproved delay bound to justify wrap.

Required evidence includes all10 legal orders of observe/protect/read and
remove/collect, both reader identities, stale registration reuse, all9 hazard
node generations, distances0/1/2, two-node traversal, long-scan reacquisition,
identical native pause schedules, locked-model/history comparison and target
atomic/helper audit. Selected executions remain distinct from physical timing,
general progress proofs, independent review and unaided delayed learner results.
