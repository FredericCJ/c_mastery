# T68.06 assessor analysis

Keep this solution separate from learner exports. Grade X-T68-006-R1..R4
individually against SCHEMA.md; public tests do not establish level3.

The registry expands into declarations, descriptors and typed access switches.
The descriptor's offset is descriptive only: no lvalue is made by adding an
offset to an arbitrary byte pointer. Stringification exposes a declared name;
it is not runtime type discovery. Native width and wire width differ for note,
whose four-byte native value uses the actual imported five-octet codec.

Encoding visits present fields in the trusted increasing-tag registry, stages
header/payloads and checks capacity before copying. Decoding bounds every header
and payload relative to remaining bytes, requires exact known widths and order,
arithmetically obtains values through the imported signed codec, and publishes
only after complete validation. No prefix can change caller output. Native
padding is neither a wire field nor part of equality. Present-only equality
must still distinguish different presence masks. A forged unknown mask is
INVALID; absent fields may contain different initialized values.

The quality changed-schema specimen adds an actually new range/version/visitor
obligation. Value16 must fail although the C uint16 type accepts it. The old
handwritten equality compiles against the new declaration and ignores quality;
generated equality rejects that counterexample. Upgrading a version2 absent
quality record leaves its bit clear and native value0; downgrading a present
quality record refuses rather than silently losing information.

Ordinary mutants1..6 omit optional equality, publish decode prefixes, ignore
visitor stop, reverse unsigned wire bytes, repair an invalid XOR before checking,
and ignore required bits. Controls7/8 compile actual retained MOD11/12 byte-order
and missing-descriptor defects. Each must fail its named ordinary assertion.
The Python oracle independently creates and parses bytes; its current finite
case counts and trace are checked from actual reports, not assumed here.

Fingerprint guards address accidental stale build artifacts, not malicious
consumers. The static check binds four expected words. The old-object test must
stop at link failure: executing an old ABI layout against the new implementation
would be outside the experiment. SHA-256 host support is allowed because hashing
internals are not this exercise's mechanism. Hash equality is not a universal
proof of content equality, and long linker symbol uniqueness is an observed
toolchain premise. Consistent packing changes offsets while preserving wire
bytes; inconsistent cross-object packing is not tested by executing invalid calls.

The Rust specimen uses declared macro_rules syntax and actual derived PartialEq.
Its type-aware generated equality includes the added field automatically. C's
local registry achieves a bounded similar operation; it does not inherit Rust
macro hygiene, typed procedural syntax, borrowing or arbitrary reflection.

Sealed assessment decision: introduce an optional signed priority sinceversion4
with range-4..4 and specify whether version3 writers refuse or project it away.
Require the learner to justify one policy, derive bytes independently, reject a
stale visitor that still compiles, preserve old version3 decoding and demonstrate
failed range/output publication. Supply this altered case only after the delay;
do not reuse the assisted quality case as evidence of independence.

First evolution pilot stopped because optimized GCC introduced memcpy. That is
an observed hosted code-generation dependency, not evidence of a strict runtime.
The corrected audit permits and records only ar_encode/ar_decode, ordinary
copy/zero helpers and stack protection; no library supplies schema operations.
Retain that failed pilot log and the later source-bound successful campaign.
