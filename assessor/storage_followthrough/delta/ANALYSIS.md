# SDW1 assessor analysis

The difficult change is the authority relation, not the loop bound. CST2 seals
every home against one global generation, so skipping a home without changing
its interpretation is incorrect. SDW1 deliberately supplies a separate raw-page
format: metadata and marker commit the checksum of every logical page while a
mask chooses only the physical sources requiring installation. This preserves
one transaction API for the full-versus-sparse comparison. It is not a migrated
CST2 tree and does not inherit its key/value capacity or structural proofs.

Before publication the old metadata/homes remain authoritative. A complete
new marker can be durable only after the selected WAL flush under the stated
backend premise. Recovery first validates the entire prospective image, so an
unchanged home is part of the authority proof. After successful home flush,
publishing alternate metadata makes the new homes sufficient; only then may
the marker be reclaimed. A cut during recovery either leaves the marker for
another install or leaves complete new homes under new metadata. The repeated
matrix checks that induction for each selected pair; it does not prove an
arbitrary corrupt device or every C execution.

Independent expected results are in the Python oracle, using struct/zlib and
separately enumerated phase outcomes. A final marker write or its flush may
have persisted despite error, so SD_UNCERTAIN is necessary. In the selected
medium, even/all persistence at either of those last two events yields the new
image; other selected modes leave the old image. Checkpoint cuts must all
recover new. Check every byte, not just the changed first bytes or a checksum.

The all-page checksum scan costs O(PB) even in sparse mode. Exact traffic for
D=3 is commit0/5/3 versus0/34/3 and checkpoint36/5/3 versus65/34/3, measured as
reads/writes/flushes. Buffer size remains PB=131072 bytes. A statement that this
is universally faster fails the rubric: checksum CPU work, extra header design,
cache behavior and physical flush latency remain. The supplied failure medium
has an explicit128-event tracing limit; arm/reset between bounded operations.

The four mutants are real compiled changes. Omitted final flush violates the
acknowledged durability premise. Early marker reclaim loses redo authority
while old metadata may still win. Omitted validation writes before learning the
prospective image is corrupt. Ignoring bit31 loses a sole last-page update and
demonstrates why testing only low mask bits is inadequate.

Review max-generation evidence from a valid resealed metadata fixture, not
only by mutating a live control. Check that an empty transaction and cleanup
remain available at the limit. Output preservation applies to admitted public
read buffers; inactive scratch may be partially filled after a failed begin.
Generation, checksum and mask do not authenticate hostile resealed bytes.

Initial author pilots caught a wrong inherited field spelling and a C typedef
name collision before execution. The optimized pilot subsequently passed the
six ordinary anchors and four semantic controls. Final claims must come from
the current source-bound report; no pilot is a substitute for that report or an
independent assessor. No external source inspection or physical measurement is
invented for this original protocol.
