# B01 transfer implementation and falsification notes

This is assessor-only, same-author analysis. The parent analyses retain their
objective-specific rubrics and source assignments. The reference supplies concrete
implementations for previously prose-only variants, without filling learner code.

Layout uses two staged extent checks. The second starts at the first segment's
end, not at the original header. The separate signed conversion gate prevents a
negative count becoming a large unsigned request. The `FD_TRANSFER_LAYOUT`
control admits the two segments against separate budgets and must fail the
combined-total assertion. The reduced oracle sums in a deliberately wide signed
type over small operands; actual extrema are checked separately.
The three original extent defects also compile independently: premature unsigned
multiply/add, missing offset admission when count is zero, and writing output
before final admission. Their witnesses respectively use `(0,SIZE_MAX,2,SIZE_MAX)`,
`(1,0,1,0)` and `(0,1,1,0)` with output sentinel917. The latter two are minimal
nonnegative counterexamples for their violated admission properties, independent
of machine width. The wrapped-product witness depends on the actual unsigned
range and is checked separately from reduced enumeration.

Segmented copy proves total representability, cursor admission, availability and
destination capacity before copying. The index selects exactly one actual array.
No pointer is formed for an empty fragment. Nested views are checked against
integer intervals, including aliased parent/output and failed take preservation.
The base early-advance mutation is rejected by this deeper harness too.
The absolute-owner variant enumerates admitted integer intervals without rebasing
the owner. The tagged consumer operates on a private cursor copy and publishes
it only after payload success; `FD_TRANSFER_TAG_COMMIT` exposes the lost rollback
with a complete header and incomplete payload. A valid alternative saves/restores
the cursor, but must restore on every error and exclude reentrant observation.

The codec uses a separately fixed version2 golden vector and rejects the reserved
byte. Field-wise round trips supplement that vector; they do not independently
detect all symmetric defects. Greatest-width inputs model wider logical values
without pretending a typedef changes the machine's byte width.
The least-width wrapper admits the same mathematical ranges; widening a storage
type cannot widen the wire contract. Its staged decoder preserves all fields on
rejection, without comparing or assigning meaning to structure padding.

The typed-storage adapter deliberately changes the original allocator contract:
it reserves an existing declared `FDValue`, whose declared type remains in force,
rather than creating an untyped allocation. This is legal for this concrete typed
construction and is not a generic aligned-byte allocator. The unchanged slot
control logic is reused under that revised premise. Owner transfer is ordinary
C struct assignment followed by revocation of source; caller discipline still
prohibits copying a live owner. An operation-sequence model enumerates all five
actions through depth5, and verifies state, token ownership and backing ownership
at every step. Event traces separately distinguish partial rollback from full
destruction. Callback reentrancy and concurrency remain outside this contract.

Stride movement relies on the monotonic positive-stride mapping. The same logical
copy direction protects unread bytes; physical gaps remain unchanged. This proof
would fail for an arbitrary permutation mapping, which needs a cycle or snapshot
strategy. Compaction increments the semantic epoch before any later borrow can
be admitted. Exhaustion rejects; wider counters merely postpone exhaustion.
The separate in-place XOR interface has one accessible range, so its proof does
not contradict the disjoint-range restriction of the original two-pointer API.

Registry lookup checks the registry epoch before forming a current local handle.
Owner lifetimes remain a precondition. Unbinding does not reset a pool or reclaim
its values; those remain owned by its separate lifetime protocol. Epoch retirement
prevents a retained handle reviving when mapping changes return to an older owner.
The wrapping control fails the final bind rejection, rather than relying on a
use-after-free observation. Concurrent reclamation would require another protocol.
Relocation is a different transition from unbind/rebind: it preserves the logical
registry epoch and copies the local generations under exclusive access, while
revoking old storage ownership. The duplicate-owner control fails the old-live
slot invariant. Native local handles and borrowed pointers cannot survive the
move; this precondition is not enforced by ISO C. Borrowed lookup first validates
the handle and only then forms the returned pointer. The separate semantic dossier
requires a counterexample to resetting retired identities without quiescence.

Frame completion consumes the attempted frame even when decoding, arithmetic or
authorization fails. Capacity rejection consumes no bytes. The counter commits
only through `fd_apply`; wire flags do not grant authority. All128 nonempty
chunkings are checked under both initial authority states with opposite authority
on later fragments. The stale-authority mutation leaves authorization across
cancel, and a denied fresh record exposes it without undefined behavior. Original
vulnerable teaching programs remain unchanged.
The separate wire-flag authority control is rejected for flags1 and3 when trusted
authority is false. This is a defined logic violation; sanitizers need not notice
it. Public QUERY deliberately skips update arithmetic and authority after syntax
validation. Thus a valid query at `INT32_MAX` can succeed where UPDATE rejects.
An implementation that shares only decoding, then selects effect-specific
admission, is valid; one that changes UPDATE to accept a wire permission is not.

Costs: step/layout/codec/control operations have constant work and storage;
segmented and stride copies use O(n) operations and O(1) auxiliary storage;
framing copies at most8 bytes per admitted feed and uses one8-byte staging array.
Registry operations scan at most2 bindings plus the base pool's3 slots. Callback
costs are additional. These counts do not imply a latency maximum or a performance
ranking. `comparisons.c` measures paired restricted and unrestricted transforms
with disjoint arrays, identical correctness, warmups and alternating order. Each
is in a separate translation unit without LTO, preventing asymmetric inlining.
The receipt records five samples per size (4,64,4096,65536),128 repetitions,
clock resolution, exact flags, generated assembly and image size. Short samples
may quantize to zero; no speedup threshold or worst-case latency follows. Both
variants remain O(n); vectorization/versioning and target instructions must be
read from each captured executable, not inferred from restrict alone.

The C17 builtin adapter and separately compiled C23 checked-macro adapter use
temporary results to preserve the extent transaction. Their65536 reduced tuples
use a non-overflowing small-domain sum as an independent predicate; six explicit
actual-width cases supplement it. Both primitive addition APIs write zero for
`SIZE_MAX+1` while returning overflow. N3096 section7.20.1 describes wrapped output;
the draft is not the published C23 standard. No C23 facility enters the C17 target.

Reproduce with `python3 tools/run_foundation_transfer.py --report <new-receipt>` in
WSL/Linux. Both compilers run debug, optimized and ASan/UBSan profiles. Each
selected mutation must first compile and then exit1 at `FD_TRANSFER_CHECK`; a
translation failure, crash or sanitizer-only error fails the campaign. Cortex-M4
objects and an image retaining all functions are audited for dependencies; no
garbage collection may hide an unused helper. This is build evidence, not target
execution. Actual author receipts, not this prose, determine execution status.
