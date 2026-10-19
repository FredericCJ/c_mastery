# Foundation depth shared contract, version 1

This contract supplements the eight exercise briefs. The active implementation
edition is ISO C17. The codec requires `CHAR_BIT == 8` and exact-width
`uint16_t`, `uint32_t`, and `int32_t`; the header rejects an incompatible target.
Do not infer signed arithmetic semantics from a processor's representation.

All operations are synchronous and single threaded. Objects, output locations,
and borrowed input arrays must be live, initialized where read, suitably aligned,
and truthfully sized. Unless stated otherwise, outputs do not overlap input or
control objects. A successful bounds check cannot establish that a pointer names
real storage. Invalid native pointers are outside these APIs' admitted inputs.
Failure leaves outputs unchanged unless an explicit lifecycle transition below
requires cleanup. `FD_INVALID` rejects admitted malformed metadata or null
required control/output pointers; it cannot validate arbitrary pointer bits.

The portable component calls no library routines. Allocation is permitted only
through the lifecycle exercise's explicit callbacks. A hosted test harness may
use `malloc`, `free`, `memcmp`, Python and sanitizers as verification tools.
Do not replace studied operations with `memmove`, a codec library, an existing
container or allocation framework. Assembly, undefined-symbol and final-link
inspection are necessary to check implicit compiler dependencies.

## Semantic verdict and repair

`FDCase` ordering matches `fixtures/foundation_depth/cases.json`. Classify each
complete supplied case under its stated C17 assumptions; classify the ordering
of function calls for `CALL_ORDER`. Distinguish a required diagnostic from
undefined runtime behavior. No executed-output test is prescribed for an
undefined case. `fd_verdict` returns `FD_UNKNOWN` for an unknown case identifier.
The public test checks completion, not the correctness of the explanation or
every classification. Exact verdict checks and technical analysis remain in the
assessor tree. Claiming correctness requires the separate reasoning rubric.

`fd_repair_step(value, previous)` saves the old integer and increments it once.
The two objects must be distinct. Null/equal pointers return `FD_INVALID`;
`INT_MAX` returns `FD_RANGE` with both objects unchanged. It is a repair of an
intended state transition, not a prediction of an undefined source expression.

## Checked extents and counted cursors

`fd_extent(offset,count,width,limit,out)` admits exactly the mathematical
condition `offset + count * width <= limit`, with all arguments in `size_t`.
Zero width and zero count are valid; they do not excuse an offset above the
limit. Reject overflow before executing it. Write the exact extent only on
`FD_OK`; otherwise return `FD_RANGE`. `fd_from_signed` rejects negative values
and positive values not representable in `size_t` before conversion.

`FDCursor` describes a borrowed actual byte array with `position <= length`.
The all-zero cursor is valid; a null base with positive length is invalid.
`fd_take` returns the next `count` bytes and advances only on success. At count
zero, a non-null cursor returns its current (possibly one-past) pointer; the
empty null cursor returns null without evaluating null pointer arithmetic.
`fd_subview` uses an offset from the parent's unconsumed suffix and creates a
position-zero cursor. It does not advance the parent. `out == parent` is allowed:
capture the source metadata before changing it. Rejected ranges leave every
cursor and output unchanged. Costs are constant time and constant storage.

These extend the `bytes.checked` and `bytes.cursor` component concepts. Import
the learner's accepted implementation and adapt `FStatus`/`FCursor` or another
recorded representation at the boundary; record source and contract digests.
The stronger output-preservation, nested-view and extent-limit requirements need
fresh evidence. A new interface name does not discard prerequisite obligations.

## Canonical eight-octet record

Octets: `[version=1, flags, count_hi, count_lo, value_3, value_2, value_1, value_0]`.
Only flag bits 0 and 1 may be set. The count is unsigned big-endian 16-bit;
the value is the signed integer denoted by a big-endian 32-bit two's-complement
wire encoding. Derive that integer arithmetically. Object representation,
structure padding, alignment, union punning and a host's byte order are not wire
contracts. `fd_decode` and `fd_encode` admit exactly length/capacity eight;
other sizes return `FD_RANGE`. Non-null input/output is required. Malformed
version/flags return `FD_INVALID`. Failure changes no output bytes or fields.
Encoding's record is passed by value. Costs are constant time/storage.
Decoding requires the output `FDRecord` and input octet array to occupy disjoint
storage; overlapping decode is not admitted by this contract.
The decoder composes the checked extent and cursor operations; record imported
artifact identities instead of substituting unassessed implementations.

## Actual allocated typed storage

Initialize `FDSlot` with `FD_SLOT_INITIALIZER`. `FDValue` is one concrete type;
the slot is not a promise of generic arbitrary-alignment storage.
`fd_slot_acquire` requests exactly `sizeof(FDValue)` and `_Alignof(FDValue)`.
The allocator must return genuinely allocated, suitably aligned storage with no
declared type, or null. A cast from a declared character array does not satisfy
this contract. Callback context remains live until reset. The slot cannot
repair a lying allocator or detect invalid pointers by numerical alignment alone.

States are empty → acquired → initialized → published. Acquire outside empty
returns `FD_STATE`. Allocation failure returns `FD_NOMEM`, leaving empty.
Initialize is legal only while acquired; its callback constructs the actual
`FDValue` through typed stores and returns zero on success. On failure, the
callback rolls back its own partial resource acquisitions; the slot frees its
storage, becomes empty and returns `FD_INIT_FAILED`. A destructor is **not**
called for failed initialization. None of the allocator, initializer, destructor
or deallocator callbacks may reenter, reset, publish or otherwise mutate the
same slot. Their context may observe the documented state for tracing. The
callback must not retain or access borrowed storage after failed-initialization
cleanup or reset; a destructor/deallocator borrow ends at callback return.

Publish is legal in initialized or published states and returns a borrowed const
pointer. Reset destroys a successfully initialized value exactly once (if a
destructor was supplied), frees storage exactly once and restores empty. Reset
of empty is a no-op; reset of acquired frees without destruction. Published
borrows expire at reset. Do not dereference or compare an expired pointer to
prove identity. Retain callback ordering and effective-type/access timelines.
The slot owns one allocation, needs constant control space and has callback-
dependent initialization/destruction costs; no callback cost is silently bounded.

## Restricted transform and overlapping movement

`fd_transform(dst,src,count,mask)` writes `dst[i] = src[i] ^ mask`. When count is
positive, the accessed source and destination ranges must be disjoint live
arrays. The `restrict` obligation concerns accesses and associated objects,
not merely unequal base addresses. At count zero, null pointers are admitted.
Invalid restricted calls are analysis-only counterexamples, never observations
with a portable expected value.

`fd_move(base,length,destination,source,count)` admits two windows within one
actual byte array, including overlap, identical windows and zero-length
one-past positions. It has snapshot-copy semantics without dynamic allocation.
Null plus zero length is admitted. Validate offsets/counts before pointer
formation; rejected inputs preserve the whole array. No relational comparison
of unrelated pointers is needed. Both operations require O(count) byte work and
O(1) auxiliary space. Inspect optimized code only for valid calls.

## Finite handle pool

`FDPool` contains three actual `FDValue` entries. `fd_pool_init` requires a live,
nonnull pool object; its void interface does not report input errors. Call it once before
use; keep the owner at one live, stable address until all handles are discarded.
Copying/reinitializing a pool with outstanding handles is outside the contract.
This typed-storage adaptation extends the `memory.pool` concept; it does not
claim that generations repair dangling owner pointers or unbounded address reuse.

Acquire chooses the lowest-index free non-retired slot, stores a typed value,
and returns `{owner,index,generation}`. Generations run from 1 through 3 for
deliberate finite exhaustion. Release makes the handle stale; a release at
generation 3 permanently retires that slot. No generation wraps. All three
slots retired or live returns `FD_FULL` without changing the output handle.
Get/release reject foreign-owner, invalid-index, stale and duplicate-release
handles with `FD_STALE`; get preserves its output on failure. Pool owner
liveness is a caller precondition before evaluating a handle's pointer member.
Acquisition scans three entries; get/release are O(1); storage is fixed.

## Local authorization composition

`fd_apply` decodes one exact record, derives `old_counter + count * value` in a
wide mathematical range, checks that the result fits `int32_t`, and requires
trusted `authorized != 0` before publishing. Wire flag bits never confer
authority. Validation errors take precedence, then result overflow, then
`FD_DENIED`; all failures preserve the counter. Zero change still requires
authorization. The maximum arithmetic magnitude fits `int64_t`; justify it.

The target is solely `drills/authorization.c` and learner derivatives, run as a
local unprivileged child process with synthetic bytes. The attacker controls
wire bytes, never the trusted authorization argument. No network, real account,
device, credential or external target is involved. Its harmless success signal
is a denied request changing the synthetic counter from 10 to 16. The corrected
implementation returns denial and leaves 10. This demonstrates state-integrity
failure; it establishes neither arbitrary code execution nor privilege escalation.
Do not change host security settings. Repairs need malformed-input, overflow,
authorization, unchanged-output and delayed incremental-framing regressions.

## Evidence boundaries

The public finite tests are necessary anchors, not proof of universal correctness
or independent mastery. The supplied assessor code is same-author reference
material, not an independent reviewer. Compiled mutations must reach an explicit
semantic assertion; compile failures or arbitrary crashes do not count as killed
mutations. A sanitizer result cannot establish safety outside tested executions.
The strict Cortex-M4 artifacts are compile/link/assembly evidence only. There is
no reset startup, emulator run, flashing operation or physical board claim.
