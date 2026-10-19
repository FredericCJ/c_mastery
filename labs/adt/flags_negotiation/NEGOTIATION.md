# Required versioned flag negotiation: X-T67-003

This is the required T67-03-INVESTIGATION and T67-03-TRANSFER successor, including
X-T67-003-DELAYED. Complete the original ADFlags constructor, X-T67-002 typed
value reasoning, and bytes.checked artifact first. Import the learner's actual
ad_flags implementation through adt.h; the reference campaign imports the actual
retained provider. Do not substitute a library flag-set or negotiation engine.
Keep the original eight-state constructor contract and accepted records.

Use hosted ISO C17, uint32_t/uint64_t, GCC and Clang debug/release/ASan+UBSan.
The core has no heap, I/O, callbacks, threading or target runtime dependencies.
Host Python sets supply the independent oracle; they are not target dependencies.
The original null-output pointer remains outside ad_flags's valid-call domain.
The new APIs explicitly return ADF_INVALID for null output. Otherwise outputs
must name live writable objects; numeric validation does not validate pointers.

## Representation and exact operations

ADFlags is a set of independently selectable bits0..2, including the empty set.
It has eight abstract states, not three mutually exclusive variants. New union
and intersection operations validate both by-value operands through ad_flags,
then publish the corresponding set operation. A forged public ADFlags{8} is
refused, even when intersection would erase its reserved bit. Output may be the
same ADFlags object from which a by-value operand was copied. Failure preserves
the output, including an initially invalid sentinel. No ownership is transferred.

AdfCapabilities{version,bits} has version1 with known mask7 or version2 with known
mask63. The bits are the capabilities actually offered by that participant;
version2 alone does not imply all six are present. Any other version or any bit
outside its version's mask is invalid, including version0/mask0. The constructor
validates before publication; version1 delegates to actual ad_flags. Negotiation
revalidates capabilities because public fields can be forged.

AdfOffer{required,advisory} permits all32 bit positions. Local and peer required
sets are combined by union. All required bits must occur in BOTH participants'
capabilities, or negotiation returns ADF_UNSUPPORTED unchanged. Advisory bits
are combined by union and preserved exactly, including unknown high bits.
The resulting AdfAgreement has four values:

- common: intersection of the two capability sets;
- required: union of the two required sets;
- advisory: union of the two advisory sets, without clearing unknown bits;
- enabled: intersection of common with required union advisory.

Required/advisory overlap is legal and redundant. An unsupported required bit
cannot be downgraded to advisory. Unknown advisory preservation is forwarding
metadata, not permission to execute a feature. Reversing the two participants
and their offers leaves the agreement unchanged. Null output or invalid
capabilities yield ADF_INVALID before required-set admission. Any failure leaves
all four output fields unchanged. Inputs are values, so partial-output aliasing
and borrowed input lifetime do not arise within a valid call. Calls sharing an
output require caller synchronization; this API is not an atomic protocol.

Examples: local(version1,7), peer(version2,63), offers(1,8) and(2,0x80000004)
produce common7, required3, advisory0x8000000c, enabled7. Changing peer required
to8 refuses: peer support does not create local support. With both capabilities
(version2,63), required32 succeeds. Empty capabilities and empty offers succeed.
Advisory UINT32_MAX is preserved, but only common bits are enabled.

## Derivation, boundaries and enforcement

Derive constructor validity, closure/idempotence/commutativity/associativity and
absorption for the original set operations. Explain why checking only the
result of intersection admits an invalid operand. State the negotiation proof
per bit: enabled membership implies membership in each capability set; required
membership implies enabled membership after successful admission. Offer union
alone cannot establish either participant's capability.

Read the named N1570 C11 draft clauses on complement and bitwise AND/OR.
Complement performs integer promotions; its result need not have the source's
narrow type. Show the types and values for ~(uint8_t)7 and ~UINT32_C(7) on the
measured implementation, then revisit on a wider-int model. The exact-width
mask is a value contract; a uint32_t typedef need not always avoid promotion on
every conforming integer model. Use the explicit unsigned masks and derive
the low32-bit membership rule, instead of assuming all implementations have
32-bit int. N1570 is a C11 draft, not an inspected published C17/C23 text.

ISO structure types distinguish different wrapper names but public construction
does not enforce valid masks or truthful capabilities. Runtime constructors
enforce admission for calls that use them. Rust enum exhaustiveness concerns
alternatives; it does not establish independent flag combinations or negotiated
support. A library flag-set abstraction can enforce a chosen unknown-bit policy,
but no library is used here and no blanket equivalence is claimed. Protocol
truthfulness, authorization and concurrency remain separate caller obligations.

## Required artifacts and verification

D1 supplies the set/sum distinction and counterexamples to masking, forged
values and capability union. D2 supplies the core, invariants and unchanged-
output proof. D3 records original and extended tests, independent set oracle,
all32-bit raw constructor enumeration, generated code and dependency audit.
D4 implements two-version required/advisory negotiation and forwards an unknown
bit into a later version that can support it without changing its meaning.
Demonstrate both refusal before support exists and later explicit activation.

Three anchors isolate negotiation, set algebra and the actual original provider.
Six compiled defects use capability union, erase unknown advisory bits, swap
union/intersection, silently mask original invalid raw bits, or reject the
original empty set. Require successful compilation and normal ADO_CHECK failure
in each matching test. A compiler error, signal or sanitizer finding is not a
semantic rejection result. Compiling learner stubs must fail all three anchors.

The set-based Python oracle checks all262144 combinations of three-bit
capabilities and offers, version/admission/high-bit corners and seeded32-bit
advisory values:283808 total cases. Its expected outputs use sets of bit
positions; do not derive them by calling the C implementation. Check exact
status and all four result fields, including77/78/79/80 refusal sentinels.

The separate optimized GCC and Clang campaign calls the actual original
constructor once for each of4294967296 values, using a uint64 loop bound so
the terminal condition is representable. Expected counts:8 admitted,
4294967288 refused, admitted checksum28, every refusal preserving0xdeadbeef.
Compile provider and driver separately without link-time optimization. Retain
commands, dependencies and controls: silent-mask fails at8; empty rejection
fails at0. This finite constructor exhaustion does not enumerate negotiation
histories, arbitrary compiler behaviors or learner reasoning.

R1 assesses the distinction and policy, R2 the exact invariant/promotion/forged
value argument, R3 actual original+extended falsification, R4 changed-version
composition. Public test success alone is level1. Level2 requires all explicit
arguments and implementation/refusal obligations. Level3 requires an unaided
delayed version change and a correct counterexample to the old proof. Retain
the original seven-day delayed policy and record assistance. Neither exhaustive
finite inputs nor a finite mutant set proves absolute mastery.

Graduated hints: enumerate the eight original sets; separate capability and
requested-set types; then derive each agreement bit from two participant
capabilities before writing any output. Private ANALYSIS.md supplies the proof
and changed-condition assessor case and is excluded from learner export.

## Costs, security scope and evidence limits

Thirteen dimensions: capabilities store version+mask, offers two masks,
agreement four masks; there is no reachable heap graph; values have caller
storage lifetime; no borrow survives a call; allocation/destruction do not
apply; copies have fixed size; dispatch is direct; concurrent independent
outputs need no shared state; failure is transactional; actual compile/object
work is retained; stack and binary sizes are ABI/compiler observations; the
enforcement boundary separates type diagnostics, runtime admission and caller
truthfulness; integration requires the actual retained ad_flags contract.
Record seven100000-negotiation timing samples per profile, verify checksum350000,
inspect assembly/stack/undefined symbols, and report noise and optimization.
No comparative zero-cost or device-latency conclusion follows from those samples.

Security target: this original local negotiation test with a synthetic counter.
Environment/isolation: a local process, synthetic capability/offer integers,
no external service or credentials. Attacker capability: select peer offers
and claimed capabilities; local capabilities remain trusted configuration.
Defect: unioning participants' capabilities admits an unsupported local feature.
Exposing transition: local mask7, peer mask63, peer required8; the flawed core
publishes enabled8 and increments the harmless test counter. Success means
only that unsupported-operation property violation; it is not code execution,
authentication bypass against a real peer, or privilege escalation. Repair:
intersection-based capability admission, exact advisory preservation and atomic
output publication. Regression: counter stays0, unsupported status and unchanged
sentinels; compiled control must expose the wrong result. Residual limits:
truthful peer claims, authenticated transport and feature implementation are not
provided. Current author results require matching source-bound reports; physical
execution, non-author review and delayed learner performance remain unobserved.
