# Inline relocation and opaque generation authority

Required X-T66-003/R1-R4 and X-T66-003-DELAYED after X-T66-002. Use actual
ow_move/ow_consume and the declared V4Allocator/ow_allocator typed provider.
Hosted C17 is the implementation baseline. GCC/Clang warning attributes and the
separate C23 nodiscard translation probes are comparisons, not ownership rules.

## Ordinary move and the changed representation

The retained OwOwner move transfers three allocated arrays and their allocator
to an empty, distinct destination, then empties the source. Reject self-move,
occupied destination and empty source unchanged. Repeated consume is DEAD.
Compile the ordinary copied-owner function, but never consume both copies as a
native double-free experiment. ISO C17 does not intercept structure assignment.

OmmValue is a live fixed-address owner with four inline int32_t cells, a data
pointer, length, matching allocator, self pointer and live flag. Init requires
fresh zero or previously consumed empty storage, a valid typed allocator, disjoint
input and1..8cells. Length1..4uses inline cells;5..8calls the actual typed provider.
Any invalid request or null allocation preserves the destination. Success copies
the complete input then publishes a live self pointer and data representation.
The raw input does not become a retained borrow.

Move copies value/allocator metadata, rebinds self to the destination, and for
inline representation rebinds data to destination.small. Heap representation
transfers the existing pointer without allocation. Then clear every active source
field. The destination must be empty and disjoint; a copied or corrupted control
is not an authentic source. Read checks live/self/length/representation before
indexing and preserves output on refusal. Consume caches the heap pointer and
matching allocator, empties the control before release, and releases only for
the heap representation. Inline storage remains part of its C object; protocol
borrow expiration is not a claim that the array's C lifetime ended.

An ordinary copy of an initialized OmmValue has self pointing to the old object
and is rejected without dereferencing its data. This runtime consistency check
does not prevent a malicious client from rewriting public fields or using a raw
alias. All outstanding borrows expire at move/consume by contract; the API cannot
find copies retained elsewhere. Output/input/control/allocator context storage is
live and disjoint. Calls are externally serialized and callbacks do not reenter
or exit nonlocally. No thread-safe or automatic move-only type is claimed.

## Opaque representation with checked, copyable tokens

OmmRegistry is incomplete in the public header. Create takes an explicit raw
allocation callback pair for its private control plus a separate typed cell
allocator for values. The raw callback returns suitably aligned untyped storage
of the requested size or NULL; the reference host adapter uses malloc/free.
This is not the V4 typed-cell allocator disguised as an arbitrary-object allocator.
The private initializer establishes the actual registry object type. Empty output
is required, and allocation failure preserves it. Callback context lives through
destroy; a permitted release observer can verify the owner was already NULL.

The registry owns two fixed slots. Each slot contains an OmmValue, generation,
active flag and permanent retirement flag. Create initializes both generations
to the supplied seed; a MAX seed retires both immediately. Open selects the first
inactive nonretired slot, initializes its value transactionally, then increments
generation and publishes an owner-pointer/slot/generation token. A token output
must be all-zero empty. Neither allocation failure nor invalid input consumes a
generation. Ordinary full capacity returns FULL. All registry/ticket addresses
must remain live; comparisons cannot validate forged or dangling native pointers.

Token move is a transfer of logical authority, not physical relocation of the
private payload. Check source identity and an empty destination, refuse at MAX,
otherwise increment the slot generation, publish the new token and empty source.
Every old copied token now rejects. Consume releases the value, deactivates the
slot and empties the supplied token. A slot at MAX retires permanently; mandatory
consume remains legal at MAX even though another move/open cannot wrap. Destroy
requires no active slot, clears the registry owner before raw release and is
idempotent for NULL. All tickets must be abandoned before registry destruction;
even a stale-token check needs the live registry argument.

Opacity does not make a pointer or token unique. Two identical valid tokens are
indistinguishable to this protocol. The first holder can transfer or consume the
resource, making the other's copy stale. The checks prevent repeated consumption
through the API; they do not authenticate which holder was intended to act.

## Diagnostics and edition comparison

Compile tests/diagnostic.c twice: OMM_DIAGNOSTIC calls actual annotated ow_consume
and deliberately ignores its result; the ordinary mode ignores the unannotated
omm_consume result. Both include a legal struct-copy function. Require actual
GCC/Clang observations, including compiler flags and versions. The first uses a
documented compiler extension; the plain C17 return-value rule permits discard.
The comparison is not a claim that every included header uses ISO-only syntax.

The separate C23 nodiscard.c probe tests ignored versus explicitly void-cast
results. N3096 6.7.12.2 recommends diagnostics for the first case; it does not
make ordinary discarded results a linear-use type violation. With -Werror the
observed compiler warning is a build error, not a new ISO ownership constraint.
Published C17/C23 texts remain unavailable; N3096 is a draft. The negative opaque
field-access probe must fail on incomplete type; neither diagnostic catches every
duplicated handle, logical misuse or memory corruption.

## Deliverables, verification and scoring

D1 gives the ownership/physical-copy/poisoning/diagnostic enforcement matrix and
inline-pointer repair derivation. D2 implements the exact headers, allocator
failure semantics, authentic-control preconditions and explicit raw/typed adapters.
D3 runs self/occupied/empty/repeated-use cases, all lengths1..8, allocation failure,
retained original controls, negative translations and source-copy acceptance.
D4 seals the registry generation protocol, demonstrates duplicate authority and
attempts an independently changed case after seven days.

Six anchors assess original moves, inline/heap relocation, admission, opaque
registry transfer, duplicate-holder limits and retirement. Six compiled controls
omit inline rebinding, omit source clearing, omit authority generation increment,
ignore generation, or compile actual OW_MUTANT3/13. Require exit1 with OW_CHECK;
compiler failure or sanitizer abort is not a semantic rejection. The harness owns
an emergency raw-allocation ledger so a failing control can return its assertion
without a test-harness leak; normal tests separately assert the API's real release
counts. This teardown does not turn an implementation leak into a passing test.

The independent630-case integer model varies lengths0..9, seven generation seeds
including the maximum boundary, and0..8transfers. It computes old-token authority
and expected typed allocation/release counts independently of private C layout.
It is not a proof of arbitrary lifetimes or concurrent callers. The supplied
learner sources compile and fail each selected positive anchor.

R1 assesses enforcement limits, R2 exact movement and consumption, R3 boundary/
diagnostic evidence and original controls, R4 inline repair and opaque-token
changed assumptions. Nominal reproduction earns1, justified bounded evidence2,
independent transfer with a counterexample3. Double ownership, an unrepaired
inline pointer, reused exhausted generation or overstated authentication is a
hard failure. Hints: identify which pointer points inside the old object; separate
physical movement from authority movement; reserve final consumption at exhaustion;
compare identical token copies before either acts. Solutions remain assessor-only.

## Local adversarial demonstration

Target: original OmmRegistry in one local teaching process with synthetic value51.
The adversary receives an exact copy of a currently valid token; it cannot forge
native pointers, corrupt the registry, access external data or use another process.
Isolation is a disposable ordinary process, no privilege/security-setting changes.
The defect in an overstated guarantee is treating field secrecy/generation checking
as holder authentication. Exposing transition: copy the token, transfer with the
copy first, then try the nominal original. Harmless success is the copied holder
reading51 while the old token returns STALE. No crash or code-execution claim.
Repair the claim and ownership discipline: forbid unauthorized copies, restrict
the trust boundary, or add a separately specified authenticated principal model
where required. Regression tests retain stale rejection and exactly-once release.
Residual limit: code with identical live authority in the same trusted address
space remains indistinguishable; generation alone cannot solve that problem.

## Cost review (thirteen dimensions)

1. Representation: sizeof OmmValue/token, private raw allocation size, two slots.
2. Work: O(n) inline copy up to4cells; heap authority transfer is fixed metadata;
   lookup scans two slots, not a general scalable handle table claim.
3. Space: fixed control plus up to two typed payload regions; no recursive move.
4. Allocation: none for inline init/move; one typed allocation for5..8cells; one
   explicit raw allocation for registry control; failure keeps prior ownership.
5. Traffic: value copy and pointer repair versus token metadata/generation update.
6. Dispatch: typed and raw allocator callbacks have distinct matching releases.
7. Code size: preserve objects and disassembly with flags, including compiled controls.
8. Stack: preserve native .su observations; changed data models require fresh audit.
9. Timing: seven10000inline-init/move/read/consume samples, checksum210000 and no
   typed allocations; this is not comparable to a heap-allocating workload by default.
10. Failure: invalid, stale, NOMEM, FULL and RETIRED distinguish different authority.
11. Scheduling: external serialization, no atomic publication or concurrent disposal.
12. Builds: exact source/dependency closure, negative/positive translation controls
   and unchanged incremental build observations.
13. Portability: C17 baseline, explicit compiler attributes/C23 comparison, finite
   uint32 authority and raw allocator alignment premises.

Author executions, independent assessment and physical validation remain separate.
