# Exact changed-premise assessor arguments

These are assessor analyses for the original criteria, not executed undefined
cases, new learner scores or claims that a compiler proves the arguments.

## X-T02-001-R1 through R8

For `f()+g()` the addition groups two calls; `(f())+(g())` changes no sequencing.
By contrast `f(),g()` creates a sequencing edge and returns g's result, so it is
not merely a parenthesized version of the sum. A valid sum-preserving repair uses
separate full expressions `a=f(); b=g(); r=a+b;`, when those local types and
arithmetic are admitted. `sum(f(),g())` still does not select which body executes
first. Separate-state f/g give6; shared-state bodies give(5,4) or(4,3). Replacing
the body calls with `sum(s++,s++)` removes the indeterminate sequencing between
bodies: the argument-expression modifications conflict without sequencing.

With unsigned-short maximum65535, a32-bit int model computes65536 in int and
then narrows to unsigned-short0. A16-bit int/unsigned-int model computes0 in
unsigned int. This is a proof under stated models, not a claim that either host
compiler has16-bit int. FDStep removes the impossible simultaneous old/new writes
when the two output pointers alias; C does not solve that postcondition by a cast.

The pointer-constraint source is compiled under both a continued-diagnostic
policy and strict rejection. A warning-severity choice does not remove the ISO
diagnostic obligation, and an object from continued translation is never run.
The foreign Packet argument is a distinct type constraint. A well-typed Packet
with index==length can translate while violating the live/index contract. A
checked wrapper admits truthful metadata and index before access, stages result,
and preserves output on refusal; arbitrary forged-pointer validation is not
provided. The existing ten-case analysis supplies each verdict and its misleading
explanation rebuttal. Undefined outputs are never used as expected values.

## X-T03-001-R1 through R3

Nested views preserve membership in the same live owner by integer interval
containment. For admitted position p and remaining r, offset<=r and n<=r-offset
establish p+offset and p+offset+n without evaluating a bad pointer first. On an
implementation permitting an actual array extent greater than PTRDIFF_MAX, two
positions may each be legitimate size_t indices although subtracting their
pointers has an unrepresentable result. The offset/remaining proof does not need
that subtraction. This is a changed-model argument; no fictitious large native
array or unrelated pointer comparison is executed. The null-empty case must
avoid even null+0. A form-before-check implementation fails the proof regardless
of whether a particular machine later traps. The tagged consumer stages its
cursor, while the underlying take keeps its advancing contract.

## X-T03-002-R1 through R4

The same member values can have different valid object representations because
of target byte order and unspecified padding. The recorded little/big-endian
observations use the same8-byte wire, and neither treats observed padding as a
promise. Representation equality is valid for the canonical wire array only
after validating version, reserved flags, exact length and unique encoding; it
is not a substitute for arbitrary FDRecord member equality. The pair of swapped
count adapters is a concrete counterexample to round-trip sufficiency. Replacing
an arbitrary typed-pointer cast by byte assembly changes the access proof; the
captured valid decoder assembly/size supplies a real cost question without ever
executing an unaligned or wrong-effective-type access. Exact and least-width
storage retain the same mathematical wire ranges in the withheld transfer.

## X-T03-003-R1 through R4

No-declared-type allocation, typed initialization, and declared-FDValue array
reservation have different type premises. The new character-copy callback uses
a live FDValue source and the actual allocation, while the existing transfer
revises the premise to declared storage. Arbitrary zero bytes are not a general
constructor. FDSlot owns storage only once; a struct assignment can copy its
ownership fields, so ordinary C acceptance does not enforce noncopyability or
language-level destruction. A valid move revokes the source before either
wrapper is reset. The A/B initializer traces distinguish rollback before B and
after B from calling a full destructor on failed construction. Callback costs
and normal-return/no-reentry premises are not hidden inside constant-time claims.

## X-T03-004-R1 through R4

Equal pointer bases with count0 access no object, so the disjoint positive-access
restriction is vacuous for that execution. Equal bases with a positive count
perform overlapping source/destination accesses and do not satisfy the original
restrict transform contract. The named single-pointer XOR operation changes the
contract explicitly. The unchanged-code-generation outcome is legitimate: the
compiler may already prove disjointness, choose the same loop, or not exploit
available alias facts. It does not remove the semantic caller obligation.
Snapshot movement and positive-stride movement choose direction based on logical
indices; compaction changes the semantic view epoch even if the backing address
stays live. General permutation layouts would need a different cycle/snapshot
argument. The six captured optimization comparisons measure only admitted calls.

## X-T03-005-R1 through R4

There are at most9successful acquisitions for3slots with3generations and no
reset. The independent issue-count model covers interleaved allocations, saved
handles, copies, repeated release, full slots and retirement. A numerical address
reused after owner destruction does not make evaluating an old pointer legitimate;
that destroyed-owner example remains analysis-only. With a separate logical
registry, issue epoch1/local generation1, retire through epoch3, reset to1 and
bind a fresh local generation1: a saved serialized token can revive. Maintenance
therefore needs all old tokens, persistent copies and in-flight operations to be
unreachable or a genuinely nonreused namespace. The existing concrete registry
never resets epochs. Relocation requires exclusive access and no surviving direct
borrows/native local handles; logical registry handles survive because the
registry entry changes owner while retaining its admitted epoch. A const borrow
does not extend either lifetime and is discarded before stale-lookup checks.

## X-T04-001-R1 through R4

Before conversion, negative intmax_t rejects. A nonnegative source converted to
uintmax_t preserves magnitude; uintmax_t represents every unsigned integer type,
including size_t. Comparing against SIZE_MAX therefore does not assume signed
rank ordering. For a hypothetical wider positive source, test its mathematical
magnitude before narrowing. The existing M4 compilation is a32-bit size_t
comparison, not an executed full-width test on that target.

An unsigned wrapped product can be defined C and still violate the requested
extent predicate. Offset-first subtraction and division bound the later product
and addition. ff_interval stages both endpoints. Two individually admitted
payloads can exceed a shared budget: header12,7*9 and10*11 each fit180 in
isolation, but their combined185does not. Inclusive last-index SIZE_MAX cannot
be incremented into this half-open representation. The existing builtin and
C23-draft comparisons prove their temporary-result adapters against the same
predicate; the primitive's overflow flag does not promise unchanged output.

## X-T35-001-R1 through R7

The asset is the synthetic counter. Bytes, lengths and fragment order are
attacker-controlled; trusted authorization is not. A truncation or timeout can
have accidental origin while violating the same completion precondition as an
adversarial fragment schedule. Cancellation revokes staged authority and bytes;
it is not a claim that every incomplete request is malicious. Both publication
before denial and wire-flag authority have concrete independent-oracle witnesses.
The property demonstrated is denied local state change, never code execution.

For the changed variable-payload reasoning case, define a **different** frame:
two big-endian count octets followed by exactly3*count payload octets, with a
budget of32records. Header2/count21845 would mathematically need65537bytes; on
a16-bit size model, premature multiply/add can wrap to1and incorrectly fit5.
First bound count<=32, then checked total<=98, then require complete input before
publication. A partial parser owns at most98staged bytes, rejects trailing data
under a one-frame policy, and cancellation resets both used count and authority.
This is the requested changed-model analysis, not a relabeling of the executed
eight-byte protocol or a claim that this additional variable-length parser is an
implemented production protocol. The original protocol's count is arithmetic
units, not a payload length. Its exact eight-byte decoder really calls the extent
and cursor provider. int64_t covers old+count*value; the final int32_t gate is
still required. The independent2474-case oracle checks that distinction.

The delayed eight-byte machine captures authority on the first nonempty fragment,
commits at most once per completed frame, consumes a failed complete frame, and
rejects over-capacity input without any new retained bytes. A fresh later complete
frame is a new request, not secretly deduplicated; the contract has no request-ID
replay protection. QUERY shares syntax admission but skips update arithmetic and
authority, so public query at INT32_MAX can succeed while UPDATE rejects. These
precise limits must remain in the learner's resource and threat arguments.
