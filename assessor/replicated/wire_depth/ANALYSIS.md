# Nested-wire assessor analysis

The grammar is intentionally stricter than Protobuf: one field per tag,
required1/2 at every known record and explicit critical-tag/feature policy.
Use the inspected primary passages for comparison, not as authority for this
original schema. Unknown type2 data is opaque; it cannot be normalized as a
submessage without changing the identity domain. The byte counterexample
`10 01 08 01` versus `08 01 10 01` can mean equal future records yet remains
different current opaque values. Projection of root1/2 proves why a field-wise
reconstruction is not a transparent relay.

RPW1 is an explicitly selected separate schema, not a promise that an original
flat tag3 integer can become a nested record without a version transition.
The feature mask is application admission under the selected profile; it does
not authenticate the profile choice or negotiate it over an untrusted channel.

The decoder creates a zeroed unpublished arena. Physical outer length is tested
once; each recursive call receives an already bounded slice. `x > n-at` avoids
overflowing an end-offset calculation. One shared field counter covers all
records. The count reservation happens before recursive descent, bounding arena
use. Operational error discards the entire local document. Native valid-object
preconditions do not allow reading trap/uninitialized caller members.

Encoder validation uses a visited bitmap as both cycle/shared-child detection
and final reachability witness. Each record first validates unique tags/kinds,
required bits and resource counts, then sorts by scanning the fixed tag range.
Nested output is staged before the parent length is emitted. Each record's
buffer is at most256 bytes, recursion depth at most4 for valid input. The arena
itself is much larger than the wire: measured host document6212 bytes and
stream308 bytes are concrete ABI observations, not universal sizes. The
reference favors inspectability over a packed representation; a delayed compact
arena must preserve all semantic/failure conditions before claiming a saving.

Canonical uniqueness is induction on known nesting depth: unique tags give a
unique ordered sequence, minimal integer encodings are unique, known child
encoding is unique by induction, and opaque fields preserve exact bytes.
Presence remains significant. This proof does not include future schema meaning,
normalization, duplicate merge semantics or origin authentication. ZigZag uses
the actual retained primitive; signed-left-shift pseudocode is not copied into C.

Stream failure has two different publication domains. `consumed` records actual
transport progress, even on failure; output document remains unchanged. The
explicit empty-frame terminator prevents a raw valid prefix from being mistaken
for the whole intended message. Done/failed/cancelled phases are terminal until
explicit init. All retained bytes are owned copies, so cancellation can retire
them without external quiescence. It does not prove a future asynchronous I/O
adapter can free kernel-retained buffers. The actual imported RfParser stops at
one frame; the wrapper loops and stops at one document terminator.

Integer binary64 classification never evaluates signaling NaNs. The sign-zero
and NaN choices are application policy; all other bits are preserved. Strict
UTF-8 validity is distinct from canonical normalization. The supplied decomposed
versus composed spelling remains distinct by design. Adding a text/float wire
type requires a new declared schema and identity-domain policy, not merely
calling a helper in a serializer that still claims the old proof.

The Python oracle shares the declared grammar but no C parser/encoder. It finds
a varint terminator, computes the integer via powers of128 and compares a fresh
minimal encoding; the C parser accumulates while advancing a local cursor.
Python dictionaries/recursion represent values independently of arena indexes.
Malformed-byte fixtures may have more than one defect, so the model follows the
documented first-check status policy; success bytes independently test the full
canonical map. Python's actual UTF-8 codec and struct/math classification provide
a separate normalization oracle. Agreement is finite evidence, not a proof of
all pointer lifetimes or language editions.

Mutants1/2 exercise minimality/duplicate admission in wire_nested;3..6 target
depth/bytes/required features/early output in wire_limits;7 accepts EOF without
terminator in wire_stream;8..10 lose unknown values, preserve negative zero or
accept overlong UTF-8 in wire_evolution. Each is selected independently and must
reach its semantic assertion. The positive control must first build and pass.

Assess every original R1–R4 criterion and the separately delayed variant.
For an unseen field becoming known, require a counterexample, migration rule and
new versioned hash domain. Reusing an old digest after semantic interpretation
changes can collapse distinctions the old profile deliberately retained. A
learner who only runs fixed fixtures has not demonstrated this reasoning.
No author-team inspection, sanitizer pass, timing sample or finite model is an
independent human review or learner mastery claim.
