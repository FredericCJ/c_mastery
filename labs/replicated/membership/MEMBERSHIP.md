# Retained counter membership and authenticated component admission

This required X-T76-004 D4 / X-T76-004-DELAYED support retains all four original
rubric IDs and the ordinary last-writer-register work. Families A/B/C/D/E/F/H/I/J
cover identity, representation, boundary, security and transfer; asynchronous
cancellation, allocator failures and board behavior do not occur in this core.

## Prerequisites and problem

Complete PR-T76-04's three-slot PN counter and register merge, PR-T76-03's
incarnation reasoning, T03/T04 byte-span/unsigned arithmetic, and the preparatory
streaming HMAC slice in [HASHING.md](../hashing/HASHING.md). That preparatory
slice does not require completion of the later Merkle/transport exercise.
Import the actual `rp_pn_add`, `rp_pn_merge` and `rp_pn_value` functions; their
containing file explicitly depends on the retained DOD implementation. Import
the actual first-principles `rph_hash`, `rph_hmac` and full-tag comparison. Do not
substitute a crypto or CRDT library or copy an assessor implementation.
Read R-RP-CRDT's registered counter/register passages and R-RP-HMAC's named
construction/key-policy passages. The identity-tagged membership and formats
below are original teaching contracts, not an attributed distributed membership
protocol from those sources.

Implement `struct rpm_counter`: nonzero uint64 namespace and local writer identity,
six sorted unique identity tags, two actual three-component RPPN blocks, and
count1..6. Unused tags/components are zero. The self identity must occur in the
active representation. Every retained component has one globally provisioned
writer incarnation. Changing self does not delete old components. The metadata
does not prove unique provisioning or prevent copied state from creating two
writers with the same label; those remain explicit caller/administrative premises.

Use C17 hosted Linux,8-bit bytes, exact uint32/uint64 and GCC/Clang diagnostics.
All control/byte/output objects are live, aligned, externally serialized and
disjoint. Struct outputs may exactly equal a supplied counter input because
every operation stages before publication; partial object overlap is invalid.
Spans are fixed160 or64 bytes as declared by the API, not runtime-measured arrays.
Contexts own no pointers; value copies are snapshots, not new write authority.
Host Python/oracle/sanitizer tooling is separate from target dependencies.

## Component operations and invariants

`rpm_make` creates one zero component. `rpm_add` increments only self's positive
or negative component through the actual checked PN producer. Overflow is
RANGE; invalid sign is INVALID. `rpm_replace` requires a new nonzero identity
absent from retained history, inserts its zero component in sorted order and
changes self. Retained components and the total remain unchanged. Reuse is
IDENTITY; needing a seventh entry is FULL. Namespace/identity provisioning
does not wrap, recycle or silently remove tombstones. This bounded exercise
retains history; it does not solve unbounded membership or garbage collection.

`rpm_merge` accepts already-admitted compatible states, joins by identity and
calls the actual PN maximum merge on aligned slots. It never adds two received
states. Missing identities are inserted if the union fits; namespace mismatch
is IDENTITY and excess union size is FULL, with no output publication even if
some earlier insertions fit. The receiving local self remains its authority.
Lattice equality compares identity-to-component maps; local self is not part
of that lattice. Sum over six positive minus negative uint32 components fits
int64. Repeated delivery is idempotent; rejecting a capacity-exceeding union
does not promise convergence outside the admitted finite state universe.

All refused operations preserve output and caller state. Required null controls
or output are INVALID. Structural checks detect malformed count/order/self and
unused components; they do not validate arbitrary pointers or establish the
history of values in a well-formed public struct. Local mutation, trusted
snapshot merge and untrusted message admission are distinct interfaces.

## Canonical snapshot and authenticated packet

The160-byte snapshot begins ASCII RPN1,version1,count,zero bytes6..7,
namespace BE64 at8,self BE64 at16,zeros24..31. Six16-byte entries begin at32:
identity BE64,positive BE32,negative BE32. Unused entries are zero. Bytes128..159
are the actual SHA256 of bytes0..127. Decode checks magic/version/reserved
bytes, checksum, sorted unique IDs, zero unused state and presence of self
before publishing. It stages the whole object. Bit corruption is detectable
in the tested cases, but this unkeyed checksum is not authentication: an
attacker can recompute it. Decode alone does not authorize merge or recovery
of old writer authority. Serialization does not imply disk durability; use
the separately assessed storage protocol before claiming persistent publication.

The64-byte cumulative-component packet begins ASCII RPA1,zeros4..7,
namespace BE64 at8,writer BE64 at16,positive BE32 at24,negative BE32 at28,
and HMAC-SHA256 over bytes0..31 at32. `rpm_seal` signs only self's component.
There is no native-layout cast, unauthenticated field or forwarded third-party
component claim. `struct rpm_permit` is trusted local policy: namespace, one writer ID,
synthetic32-byte key, positive/negative absolute limits and active0/1.

`rpm_admit` checks canonical framing, namespace and exact permit writer,
active permission, all32 tag bytes and both component limits before inserting
or joining the authenticated component. Wrong namespace/writer is IDENTITY,
inactive/excess limit is POLICY, wrong tag is AUTH; a missing seventh component
is FULL. The key proves possession only to a verifier with that key; any holder
can generate messages. Local policy chooses who may write which component and
within what limits. Replay of an older permitted cumulative value is admitted
but cannot decrease the maximum. Revocation rejects later packets, while the
old component remains in the total. Changing limits does not retroactively
undo already-admitted values. Signing an entire received snapshot with a peer's
key would not establish individual writer authority; this interface deliberately
authenticates only the named owned component.

## Deliverables and verification

D1: Implement the public scaffold, derive the tag-to-slot abstraction function,
single-writer premise, component monotonicity, finite-union refusal and total
bound. Explain why replacement must preserve history and why a copied counter
cannot allocate a fresh globally unique writer label by itself.

D2: Implement canonical snapshots and packets with the actual hash/HMAC
producer. Retain external expected bytes from the independent Python oracle;
prove unchanged output on every admission failure and distinguish checksum,
key possession, authorization, freshness and durability.

D3: Run four anchors and nine compiled controls under all declared profiles.
The actual C suite enumerates16 small two-writer states and4,368 idempotence,
commutativity and three-way-associativity cases. The separate Python model
also enumerates those laws and checks12,416 actual C transition outputs across
128 histories: every resulting canonical160-byte snapshot and64-byte MAC packet
is compared using independent dictionaries, big integers, hashlib and hmac.
Exercise positive/negative exhaustion, reused/fresh identity, sorted insertion,
union capacity, old-message replay and inactive policy. Flip all1,280 snapshot
bits individually and verify refusal with unchanged output. Preserve exact raw
commands, toolchain/source/import pins, code size, disassembly and stack reports.

D4: Conduct the local security experiment below; compare retention costs with
an operation log under the same retry/failure assumptions. A fixed six-entry
state includes retired components forever, while an operation log grows with
updates unless a separately justified checkpoint/acknowledgment scheme exists.
Compare full160-byte snapshots with64-byte component packets and record the
extra hashing/policy work; do not compare a protected packet with an unchecked
one as if they offered identical correctness contracts.

## Local security scope

Target only `membership_auth` in this original process, using synthetic keys,
namespace9 and identities10/20/30. The attacker can alter packet bytes, replay
old packets, and possess writer20's teaching key; it cannot change trusted
permit memory or obtain another writer's key. Recompute a valid MAC over a
packet naming writer30 using writer20's key: exact identity binding must reject
it. The compiled omitted-binding control admits it and demonstrates unauthorized
component creation confined to the test counter. Invalid tags are rejected
without publication. No network, real identity, privilege escalation or arbitrary
code execution is involved.

Then let the legitimate writer20 sign positive UINT32_MAX. An unrestricted
permit admits it, giving that component a permanent maximum and preventing its
writer from incrementing again. A cap100 permit rejects the identical validly
tagged packet. This demonstrates the difference between authentication and
semantic entitlement; a cap is an explicit laboratory policy, not evidence the
underlying real-world increments occurred. An authorized dishonest writer may
still lie within the cap. Repair the missing ID/tag/policy checks, retain
regressions and describe the residual key provisioning, revocation, replay,
finite-history, compromised-key and trusted-snapshot assumptions.

## Assessment, hints, transfer and cost

Keep X-T76-004-R1/R2 on lattice/authority reasoning, R3 on ordinary and expanded
law/boundary evidence, and R4 on replacement/snapshot/authentication support.
Level0 is an incorrect result or missing mechanism; level1 is nominal tests
without the ownership/proof/refusal argument; level2 meets D1–D4 and all stated
boundaries; level3 independently changes membership order, delivers an old
signed component after replacement/revocation, and explains the exact weakened
assumption without copying an assessor solution. Every required criterion must
reach2, with separate unaided/delayed evidence. Passing the public suite does
not demonstrate independent mastery.

Hints progress from (1) naming the slot's identity before comparing values,
(2) staging a union before publishing, (3) signing the bytes that the receiver
actually interprets, to (4) separating a trusted permit from untrusted packet
claims. Private technical analysis is `assessor/replicated/membership/ANALYSIS.md`.
The changed-condition tasks use identities inserted before/after existing tags,
retired writer messages, full capacity and authenticated excessive values.
Use an unseen order and policy on X-T76-004-DELAYED after the required delay.

For all thirteen modern cost dimensions: (1) record actual counter/permit layout;
(2) value ownership and external key authority; (3) bounded linear lookup and
insertion/merge work; (4) sorted tags and monotone components; (5) transactional
INVALID/FULL/RANGE/IDENTITY/AUTH/POLICY/CORRUPT; (6) six-entry join versus growing
operation history; (7) fixed stack/state and secret copies; (8) two retained PN
blocks and sequential wire access; (9) actual HMAC cost versus unkeyed checksum,
without physical/constant-time inference; (10) compiler/helper/stack audit;
(11) actual PN and hash composition; (12) ISO types versus runtime checks,
external verification and administrative discipline; (13) change identity/order,
data model or framing and re-establish arguments. No allocation, I/O, scheduler
or asynchronous cleanup cost exists in this core; their absence is scoped
non-applicability, not coverage of the separate durable/transport exercises.

## Validation limits

Authored support; use only matching `replicated_membership_validation` evidence.
GCC release pilot and independent oracle passed before the full campaign.
Physical validation, independent human review and learner assessment are not
claimed. Final C17/C23 source access gaps remain. Finite hash tests do not prove
collision resistance or MAC unforgeability, and this local policy is not a
complete Byzantine membership or consensus protocol.
