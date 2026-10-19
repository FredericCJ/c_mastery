# Delayed persistence experiment design

This is the exact analysis-only changed task of AX-PR-T23-05-T23.12.c-DELAYED.
Do not run it against host storage. Its authoring is complete only when the
reviewer can identify both the intervention and an independent recovery oracle.
Actual guest/device execution and physical observations are separate evidence.

Use a new disposable guest and dedicated disposable virtual data disk. Keep the
controller, stage receipts and expected synthetic payload hashes outside that
guest data disk. Never attach an operational disk. Record guest kernel,
filesystem, mount options, virtual device/controller, cache mode and exactly
which flush/FUA requests the emulator honors. A process-kill run is a positive
control; it is not the power/cache-loss intervention.

Prepare old generation0, payload length and a checksum; sync file and directory,
then checkpoint the disk image at quiescence. For each stage/cut, restore a fresh
copy of this checkpoint, run the original generation1 writer, and send an
acknowledged stage record over a separate control channel. The controller's
receipt must distinguish "before operation", "operation returned success", and
"flush accepted by the emulated device". Inject failure only after the selected
event has been recorded. Do not let orderly guest shutdown flush the pending
cache before the intervention.

Two experiments answer different questions. (A) Force termination of the entire
disposable VM with the documented hypervisor cache behavior, then boot/recover
from a copy of its virtual disk. (B) Use a dedicated teaching block-device model
with separate volatile and committed sectors: ordinary writes update volatile
state, successful modeled flush commits the specified prefix, and reset drops
volatile state. Enumerate allowed reordered/torn writes between flushes under an
explicit sector atomicity assumption. A model that simply throws away nothing
on reset is a necessary negative control. Neither experiment alone proves what
a physical SSD, power supply or controller does.

Recovery is read by a separate minimal reader that does not call the writer or
its update state machine. It reports pathname existence, inode/metadata, exact
length, generation and independently calculated checksum/bytes. Retain image
hashes before/after, raw controller events and recovered bytes. Compare with a
separate allowed-state table: prior to rename the old durable name remains under
the stated assumptions; after rename before directory-sync the durable outcome
is not uniquely promised; after all honored file/directory synchronization the
new generation must be recovered. Partial/torn bytes must never be silently
classified as old/new by looking only at a generation field. A discrepancy at
the last cut falsifies at least one declared stack/model premise; it must not
be suppressed by widening the expected set after seeing the output.

Repeat each legal cut from a fresh checkpoint. Add a writer that omits file-sync,
one that omits directory-sync, and a block backend that acknowledges flush but
discards pending writes. The independent oracle must distinguish these broken
premises where the selected model permits the counterexample. Report models
that cannot discriminate a mutation instead of claiming universal sensitivity.
The small Python oracle here verifies only the original logical stage matrix;
it is not evidence that these VM/block-device interventions were performed.
