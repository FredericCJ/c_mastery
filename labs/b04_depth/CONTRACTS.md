# B04 bounded mechanism contracts, revision 1

These original exercises extend the existing algorithms, communications,
architecture and memory investigations. Existing accepted implementations remain
the integration targets. This is C17 with exact uint32_t and 8-bit octets, hosted
Linux x86-64 GCC/Clang. Profile C applies to original mechanism code: only the
standard type/limit headers, no allocator, parser, graph, DMA or memory-accounting
library implements the studied mechanism. Profile A host tests may use libc,
Python and ctypes. This family does not claim a strict target link audit.

Caller objects and arrays must be live, correctly typed, sufficiently sized and
initialized. Required pointers are nonnull; input and output/control objects do
not overlap unless expressly allowed. Passing a fabricated native pointer is
outside the contract; numeric rejection cannot establish native pointer validity.
Operations are serialized; no callback, ISR or thread may concurrently mutate
the controls. No operation allocates. Scalar output and complete mutable controls
remain unchanged on a nonzero result. Errors are the named B4 constants in the
header. B4_UNIMPLEMENTED is a learner stub result, never an accepted solution.
These metadata models do not themselves enforce ownership in ISO C. Violations
outside the checked state remain programmer obligations. C23 does not add a
borrow checker, native DMA ordering, or modular total ordering to these APIs.

## Serial and receive window

Use 2..32 serial bits and values in [0,2^bits). b4_serial returns order -1 for
first preceding second, zero for equal and +1 for succeeding under the bounded
serial interpretation. Exactly half-range separation returns B4_CONFLICT, leaving
output intact. Invalid bit counts/null output return INVALID; out-of-space
values return RANGE. No signed conversion of an unrepresentable difference is
permitted. b4_window accepts spans strictly below half-range, including empty
span zero, and tests membership in the forward half-open modular interval.
Serial comparison is not a transitive total ordering over the entire ring;
never pass it as a general sorting comparator. O(1) time and storage.

## Shortest-path certificate

B4Graph holds 1..6 vertices. UINT32_MAX means no edge or unreachable distance;
all actual edge weights are 0..1000000. This weight bound ensures every simple
path fits below the distance sentinel. Source index is valid. Inputs contain n
distances and n unsigned predecessor indices. Source distance is zero and its
predecessor UINT_MAX; every unreachable predecessor is UINT_MAX. Accept only
when every finite predecessor chain is rooted at source, each predecessor edge
realizes its distance, and every graph edge satisfies the relaxation inequality.
Reject unsupported weights with RANGE, malformed extent/control with INVALID,
and false certificates with CONFLICT. The validator must not call an existing
shortest-path implementation. Derive both proof directions. O(n^2) time and O(1)
additional storage are sufficient; explicitly count parent-chain work.

## Framing boundary

Input is the COMPLETE header field section only, at most 512 octets, terminating
with a blank CRLF line. It contains only Content-Length and Transfer-Encoding
fields, or just the empty final line; it is not a whole HTTP request parser.
Names are case-insensitive ASCII letters/hyphens, with no whitespace before the
colon. Trim only SP/HTAB at value ends. Content-Length is nonempty decimal digits,
without sign/comma/interior spaces, within body_limit; leading zeroes are allowed.
Transfer-Encoding is exactly case-insensitive chunked. Reject duplicate lengths
even if equal, duplicate TE, unsupported coding and CL+TE with CONFLICT. Reject
other names, bare LF, folding, trailing bytes and incomplete input with INVALID.
Reject excessive decimal magnitude with RANGE. On success publish {chunked,body}
(zero body when CL absent). A caller closes the original teaching connection on
any error: returning a status alone does not close a real connection. O(n) time,
O(1) space. RFC9112 permits choices this intentionally stricter boundary excludes.

## DMA ownership and cache extents

Initialize B4Dma as {B4_CPU,0,1,1}. The model is a serialized noncoherent receive
buffer lifecycle: CPU write marks dirty; CPU clean clears dirty; submit requires
clean CPU ownership and transfers to DEVICE, invalidating the CPU view; matching
completion transfers to DONE; invalidate makes that completed CPU view valid;
release requires DONE+valid and advances generation, returning CPU ownership.
Every event carries the exact generation. Releasing generation UINT32_MAX fails
permanently; recycling would alias an old completion. Duplicate completion and
CPU access events while device-owned fail. Native barriers, bus reachability,
actual cache maintenance, descriptor layouts and IOMMUs are not implemented.
These require an explicit adapter and part/revision/API evidence before porting.

b4_cache_extent takes offsets in a caller-owned capacity, a nonzero power-of-two
line size and two distinct output pointers. Empty extent returns [offset,offset).
Nonempty extent rounds outward to cover whole touched lines; reject when the
requested or rounded extent exceeds capacity. Rounding can include neighboring
objects: the caller must establish exclusive ownership of entire maintained
lines. O(1) arithmetic with no overflow; no pointer is formed by this function.

## Allocation event ledger

Initialize B4Ledger to all zero. Four entries represent original synthetic
allocation identities, not pointer addresses; identity zero is invalid. ALLOC
requires absent identity. Failed allocation records no object and succeeds as a
recorded event. Successful allocation needs a free slot and checked live-byte
sum. RESIZE requires live identity; failed resize leaves ownership and byte count
unchanged; success replaces its size without changing identity. FREE requires a
live identity, cannot fail in this model, ignores bytes/succeeded and removes it.
Zero-sized logical objects remain live entries; this is not a claim about C17
realloc(p,0), whose language/library contract must be examined separately.
live is sum of current sizes, peak is monotonic maximum. Duplicate/foreign events
return STATE, capacity/arithmetic exhaustion RANGE. O(4) time, fixed storage.
The ledger does not prove absence of real leaks, escaped aliases or UB.

## Evidence and publication

Implement only src/depth.c; public tests/build/header are trusted harness inputs.
Assessor code and mutant switches reside outside learner packaging. A command
pass demonstrates that source revision under the stated host assumptions only.
The separate oracle derives finite expected values without calling the studied
algorithm; it is still authored by the same team, not an independent examiner.
Submit raw commands, source hashes, counterexample traces, complexity arguments,
and a delayed changed-condition assessment. Passing tests alone earns no claim
of absolute mastery. Existing accepted project hours and receipts remain intact.
