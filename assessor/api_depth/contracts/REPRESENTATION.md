# Withheld representation and ABI analysis

The incomplete pointer hides layout, not lifetime or allocation. The reference
stores the release callback/context with the allocation and copies that pair
before calling release, so it does not access the object afterward. Allocation
failure cannot overwrite an existing caller output; the mutant is caught with
a valid retained object pointer and then the original object is released safely.
No crash or leak sanitizer finding substitutes for this intended rejection.

The same separately compiled V1 consumer works with both narrow/wide private
layouts because it only passes an opaque pointer and uses unchanged exported
functions. The actual sizes and symbols are observations. Exposed layouts and
caller-owned storage have separate contracts and cannot inherit that conclusion.
V2 uses explicitly versioned typed storage. C clients can still access its
contract-private fields, so discipline/tooling, not ISO access control, protects
the abstraction. Serialization accesses only declared unsigned-char elements
and reconstructs values numerically; it does not create a hidden struct in a
declared byte array. Accept an alternative actual typed union only if its member
access/effective-type and alignment reasoning is correct for the exact edition.

The oracle is the independently written twelve-octet pattern; a round-trip using
matching wrong endian functions would be insufficient. The reference splits
the64-bit count into32-bit halves before variable shifts, then uses a constant
32-bit shift for recombination. Inspect the actual M4 image rather than assuming
that source arithmetic implies no helper calls. Host opaque allocation and its
callback closure are intentionally outside that strict image's selected entry.

Generated header checks fail for the actual enlarged-alignment type and for the
wrong ABI version. Missing headers or an unavailable compiler do not count as
those rejection tests. A stale consumer linked to an incompatible binary needs
a versioned link/runtime boundary too; compile-time checks only protect the
compilations that actually include them.

Delayed packet: after at least7days require the original serialized representation,
heap-free wider-field embedding and hidden-type generated-header tasks. Change
the public capacity or alignment and ask which installed consumers must rebuild,
which opaque consumers can remain, and which exact negative check detects the
stale pairing. Require a real outside consumer and failure ownership example.
Reject “opaque means ABI-stable forever,” “aligned char storage has the hidden
type,” and “same compiler output proves portable semantics.” No actual delayed
attempt or independent reviewer evidence is inferred from these author files.
