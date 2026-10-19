# Counter membership assessor analysis

The state is a finite map ID→(P,N); max join is a semilattice on compatible
maps. Local self is write authority, not part of lattice equality. Moving an
entry in the sorted array must move both components, including across the
boundary between the two real RPPN blocks. A staged union can fail at entry7
without changing any caller state. Replacement retains the retired map entry;
erasing it loses its contribution and permits stale received history to change
the interpretation. A fresh ID must be provisioned globally, not just absent
from one receiver's incomplete current view.

Each of six signed differences lies within ±UINT32_MAX, so the total magnitude
is at most6*UINT32_MAX, safely inside int64. Checked local additions prevent
component wrap; using addition to merge state violates duplicate tolerance.
The C law enumeration uses identical local authority so canonical snapshot
equality is meaningful, while the independent model explicitly ignores self
for lattice equality. Neither a random campaign nor the finite enumeration
establishes all possible histories.

The snapshot digest detects corruption but can be recomputed. It is not safe
to treat a successfully decoded arbitrary peer snapshot as already admitted
state. The untrusted interface instead accepts one exact owned component,
binds all interpreted identity/value bytes into the MAC, and applies trusted
writer/cap/revocation policy before maximum merge. Replaying an older cumulative
message cannot lower state; it is not rejected as fresh-invalid merely because
it is old. This is an explicit idempotent replay policy, not an anti-replay
sequence-number protocol.

HMAC authenticates the packet to a key holder. A holder who signs another
writer's ID is stopped by the permit binding; a holder who signs its own maximum
can still poison an unrestricted counter. The cap rejects that packet but
cannot prove that every smaller claimed increment corresponds to reality.
Revocation stops future admission while preserving prior contributions. A
malicious issuer, stolen key or arbitrary trusted snapshot is outside the
provenance premise; do not claim Byzantine tolerance from these local checks.

Controls1/2 erase retired history or add received states;3 ignores checksum;
4/5/6 omit writer/tag/cap checks;7 overwrites rather than joins an old packet;
8 ignores revocation;9 accepts noncanonical reserved bytes despite a recomputed checksum. Each is a compiled normal semantic failure in its named
anchor. The output/state refusal contract and qualified security explanation
are grading gates. Require a separately attempted changed identity/policy order
before assigning the independent level; this author campaign is not that evidence.
