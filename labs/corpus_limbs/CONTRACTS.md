# Variable-length limbs and explicit ownership

X-T11-002 is a required refinement of PR-T11-02, not a change to its historical
hours or acceptance. Implement arithmetic whose limb count is a runtime value,
with no implementation maximum or hidden fixed-width integer. The executable
test harness bounds its generated inputs; that bound is not the API definition.

## Representation and admission

Use C17, exact uint16_t and uint32_t, size_t extents, and BL_BITS=16 or8. Radix
B is2^BL_BITS. BlView denotes the nonnegative integer sum(d[i]*B^i), with
least significant digit first. Each digit is below B; zero has used=0 and may
have NULL digits. Every nonzero value has a nonzero final digit. Reject padded
noncanonical inputs instead of silently changing their identity. In the8-bit
profile each digit still occupies one actual uint16_t object: rederive its
space cost rather than claiming byte-packed storage.

Every view has its stated live readable extent. A BlOutput control is initialized
with used<=capacity and live writable backing of capacity actual uint16_t objects;
capacity0 may have NULL backing. Its old payload need not be canonical. Inputs
may share readable backing with each other, including exact self-multiplication.
Inputs, outputs, control objects and scratch are otherwise disjoint and live.
Two division outputs are distinct, including backing. These are caller
preconditions, not fabricated dynamic overlap checks between unrelated pointers.
Calls are externally serialized; no library supplies the arithmetic mechanism.

All counters are size_t. Derive/check capacity sums before pointer offsets;
neither a wrapped product nor a small array is a legal substitute for the stated
extent. No API reads a nonexistent object to test SIZE_MAX. Exercise oversized
owner reservation through its checked byte-size admission instead.

## Operations and publication

bl_add, bl_sub and bl_mul implement magnitude sum, nonnegative difference and
product. bl_sub returns NEGATIVE for x<y. bl_div returns quotient q and remainder
r with x=q*y+r and0<=r<y; zero divisor is a distinct refusal. The reference uses
binary restoring division over variable-length limbs: after every admitted input
bit, the staged remainder is below y and the quotient prefix is exact. Compare
this derivation with HAC14.20 normalized digit-estimation division, including its
normalization and correction bounds. This implementation does not claim to be
that algorithm or to have its word-division cost.

Scratch requirements in actual uint16_t objects are max(n,m)+1 for addition,
n for subtraction, n+m for multiplication, and n+m+1 for division. A zero
magnitude product may therefore need scratch under this deliberately simple
contract. Addition of zero+zero needs one staged digit. FULL means required
scratch or actual result capacity is unavailable; INVALID means malformed
representation/control or a required NULL pointer after capacity admission.
Length-sum overflow is FULL. Do not infer a universal status precedence for
several simultaneous invalid premises. For division, representation/control
admission precedes zero-divisor and scratch checks.

Compute in scratch, trim to canonical result lengths, check every output capacity,
then publish. On every refusal, all caller output digits and control fields remain
unchanged. Division must not publish q if r cannot fit. Scratch is allowed to
change. On success only the result prefix and used are written; unused capacity
is unspecified prior storage and must not be hashed as part of the integer.
No memcpy/memset/malloc or external bignum operation is permitted inside the core.
The target audit must also catch implicit compiler helpers.

BlSigned accepts sign=-1/+1 for nonzero magnitude and sign=0 only for zero.
bl_signed_add combines magnitude arithmetic and signs. bl_signed_div truncates
toward zero; a nonzero remainder has the dividend's sign, including a negative
divisor. This differs from floor/Euclidean remainder conventions. Both sign and
magnitude outputs publish together after successful magnitude admission; -0 is
never published. C17's native integer division is not used to implement the
arbitrary-length operation.

## Owned capacity and failure

BlOwner initialization requires a fresh or dropped, nonowning, quiescent control.
Controls are live, uniquely owned and noncopied. The allocator receives a count
of uint16_t objects, returns aligned allocated storage of that extent or NULL;
the matching release receives the same count. Callbacks must not reenter or
mutate the owner, retain borrowed pointers, or return existing live storage;
release cannot fail. The host adapter uses malloc/free only for storage, never
arithmetic. In Profile C an explicit caller-supplied allocator is a dependency
boundary, not permission to silently import malloc into the target.

bl_owner_set copies a canonical view into a newly allocated reservation. reserve
must be>=used and its byte product representable. Old storage remains live until
the entire copy succeeds and the new owner is published. The input may borrow
the old owner, permitting a self-copy while growing. Allocation failure preserves
the old pointer, capacity, used and payload, so existing borrows stay valid.
Successful replacement invalidates old borrows even if a future allocator later
reuses an address. reserve0 is legal only for zero and drops prior storage without
allocating. drop revokes fields before calling release, is idempotent, and retains
the allocator configuration for later set. Raw pointer borrowing has no automatic
stale-pointer diagnostic; do not dereference a revoked borrow in a demonstration.

## Required derivations and experiments

Prove addition carry and subtraction borrow by prefix value. For schoolbook
multiplication prove each inner accumulation is at most B*B-1; with16-bit digits
this fits uint32_t exactly. Doubling a remainder digit plus carry fits17bits.
Restoring division needs at most m+1 staged remainder limbs before its subtraction;
afterwards it has at most m. Do not multiply n by BL_BITS for a loop counter that
can overflow; use nested limb/bit loops. The core contains no recursive calls or
variable-length automatic arrays. Scratch is caller-owned and charged explicitly.

Addition/subtraction cost O(max(n,m)); schoolbook multiplication O(n*m);
division O(BL_BITS*n*(m+1)), plus canonical validation/zeroing. Publish costs the
actual result size; owner replacement needs old capacity+reserve live storage at
its peak. A failed allocation performs no copy/free. Compare8- and16-bit profiles
under equal mathematical values and include actual storage/code/frame sizes.
Do not report a universal speedup or constant-time cryptographic behavior.

Concrete anchors include arbitrary-length all-maximum carry chains, one followed
by n zeros minus one, (B-1)^2,7/3,6/3,0/0, zero/one, divisor larger than dividend,
exact/insufficient result and scratch, and malformed leading-zero views. Check
all four dividend/divisor sign combinations and cancellation to canonical zero.
The allocator tests force failed growth, successful self-copy and final cleanup.
The independent oracle uses Python integers and divmod, not copied limb code.

V1 changes BL_BITS16 to8 for identical values and rederives carry/accumulator,
scratch and storage bounds. V2 changes unsigned caller storage into signed owned
results with allocation failure, self-borrowed growth and truncation-versus-floor
division counterexamples. Both are required implemented variants; independent
delayed learner reconstruction at least7days later remains unassessed.

## Assessment, hints and security boundary

Submit D1 representation/prefix proofs and false explanations; D2 complete core
and allocator adapter; D3 exact oracle, refusal, mutant and target closure logs;
D4 both changed implementations, cost ledger and delayed evidence. Grade0 violates
ownership/arithmetic or overstates results; grade1 reproduces examples; grade2
derives invariants and independently falsifies six causal controls; grade3 solves
the delayed changed assumptions without copying assessor code. Hints: trace one
carry/borrow by hand; represent the consumed division prefix as q*y+r; finally
draw old/new allocation authority across the sole commit point.

Security demonstrations use only these original local controls, synthetic values
and injected allocator failure. The adversary chooses admitted digits/counts and
resource budgets, not invalid pointers or other people's data. Harmless success
is a wrong arithmetic result, partial q/r publication or leaked synthetic owner.
Repairs restore checked bounds, staged publication and exact release. Regress
the original minimal counterexample and changed radix. Timing/size leaks remain;
this is not a cryptographic constant-time library or an authenticated protocol.
Passing finite tests does not establish absolute mastery or all implementations.

Validation starts authored. The current source-bound report records actual host
execution separately from strict target object/link audits. A linked Cortex-M4
image is not physical execution. Exact published C17/C23 and Knuth volume2
passages remain reference-access gaps; the actual inspected alternative is HAC14.
