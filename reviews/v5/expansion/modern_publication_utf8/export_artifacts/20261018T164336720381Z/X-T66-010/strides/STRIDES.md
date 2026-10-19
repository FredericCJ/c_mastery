# Exact bounded borrows over strided and padded storage

Required X-T66-004/R1-R4 and X-T66-004-DELAYED after X-T66-003. Retain the actual
ow_borrow/ow_release/ow_split implementation and its interval contract. Add the
public strides.h API using hosted C17, no borrowed-set or matrix library. The
hosted verification process may use Python integer sets. This is not a
freestanding image or a concurrent borrow checker.

## Representation, inputs and ownership

An OsvShape describes indices offset+r*row_stride+c*column_stride for
0<=r<rows and0<=c<columns in one declared element extent. Rows and columns are
each at most64 and their product at most64. Each touched index must be below
extent; offset may equal extent only for an empty set. Empty means either
dimension is zero. Strides are nonnegative size_t values; zero stride is legal
only when it introduces no duplicate touched index. Reject self-aliasing shapes,
including two rows and two columns with both strides1. This is an explicit
bounded unique-element model, not all possible strided matrix representations.

osv_indices validates completely before writing the caller's64-entry output
prefix and count. Refusal preserves both. Success writes exactly count entries
in row-major logical order, leaving the suffix unchanged; empty success writes
count0 without evaluating a derived element address. Derive checked multiplication
using remaining extent before adding the last row/column. Do not form overflowing
intermediate arithmetic and then check it. Numeric SIZE_MAX cases do not imply
an actual SIZE_MAX-element allocation exists.

OsvBook contains four loan slots and the extent. Initialize fresh zero storage,
then set extent before first use. Its address and extent stay fixed until every
loan has been released. A loan owns a shape, reader/writer flag, active bit,
uint32 serial and permanent retirement bit. Controls, outputs, backing arrays
and callback contexts are live, valid and disjoint; no callback is used by this
core. Calls are externally serialized. Direct control edits, copied books and
raw data accesses bypassing guards are invalid callers, not detected safety.

Acquire admits writer exactly0 or1 and returns an owner-pointer/slot/serial
token. Two active loans conflict exactly when their touched sets intersect and
at least one is a writer. Empty sets conflict with nothing but still consume a
slot. Readers may overlap. Invalid shape/flag, conflict and capacity yield
INVALID, BUSY and FULL respectively without changing book or token output.
Choose the first inactive, nonretired slot with serial below MAX, increment
before publication, and never wrap. Release validates exact live book/slot/serial;
it deactivates once and retires a MAX slot. A stale copy cannot release a later
loan. The token does not authenticate identical holders or extend book lifetime.

## Exact splitting and real array access

Split consumes one live writer, selecting axis0 for rows or axis1 for columns.
cut lies within that logical dimension. The two child shapes partition the
parent touched set exactly; padding is never part of a loan. A nonempty right
child starts at parent.offset+cut*selected_stride. An empty child keeps the
parent's offset, avoiding an unnecessary potentially out-of-extent padding
address. Build both children and slot transitions in candidate metadata. If
either acquisition fails, preserve every parent/book field and both output
canaries. At MAX the released parent slot is retired even in the candidate, so
two other available slots may be needed. Commit only after both acquisitions;
rebind both child token owner pointers to the original book, not the local copy.

Example: extent18, shape(0,3,4,6,1) touches rows0..3,6..9,12..15. Splitting rows
at1 gives right offset6. Splitting that right tile's columns at2 gives its right
child offset8 and touched indices8,9,14,15. Actual writes must leave indices
4,5,10,11,16,17 and all other unselected cells unchanged. Interleaved writers
(0,1,4,0,2) and(1,1,4,0,2) have overlapping bounding intervals but disjoint sets.
A bounding-interval rejection is conservative, not exact for this API.

Read/write require the actual live int32_t backing array to contain exactly the
book's declared extent, and logical row/column in the selected loan. Empty loans
admit no element access. Read preserves its separate output on refusal; write
requires writer authority and preserves storage on refusal. Release invalidates
access through that token. The reference takes no raw pointer from a numeric
SIZE_MAX test. Holding a token does not make an arbitrary pointer valid or hide
other aliases. A false logical extent remains a caller violation.

## Required evidence and assessment

D1 derives set-intersection exclusion, lifetime premises, finite retirement and
the difference between exact and conservative overlap. Compare the inspected
Rust RefCell runtime borrowing contract with this explicit C protocol: RefCell
guards use language lifetimes and destruction for the safe API, while C needs
caller discipline for raw aliases, token copying and release. Neither comparison
turns this externally serialized set book into a thread-safe general verifier.
D2 implements the exact API and the retained interval implementation. D3 gives
independent integer-set results, semantic mutation rejection, complete logical
book comparisons on failure, output canaries and actual padded-array checks.
D4 implements both tile axes and seals a changed assessment after seven days.

The oracle enumerates35084ordinary interval pairs,31200shape descriptions,
9216read/write intersections over46distinct admitted small sets plus invalid
shapes, and576split/capacity/serial cases. It uses integer Cartesian products and
set equality, not the C overlap predicate. These are precisely bounded models;
they do not establish all C lifetimes. Actual tests additionally exercise
SIZE_MAX arithmetic,64-cell maximum, self aliases, stale and cross-book tokens,
full tables, padding, readonly access, bad extents and final serial release.

Run all seven C17 anchors under GCC/Clang debug, optimized and ASan/UBSan modes.
Reject eight compiled controls: conservative bounding overlap substituted for
exactness, omitted conflict, admitted duplicate elements, partial failed split,
ignored serial, incorrect row offset, and actual retained OW_MUTANT4/14. Each
must fail exit1 at OW_CHECK. A compile error or sanitizer failure is not the
expected semantic rejection. The compiling learner stubs fail every anchor.

R1 requires the enforcement matrix and a false-positive bounding example; R2
requires exact and transactional operations; R3 requires independent models and
boundary falsification; R4 requires real padded-array transfer, exact union and
disjointness with empty children. Nominal runs earn1; justified complete bounded
evidence earns2; independent changed assumptions plus a counterexample earns3.
Hard failures include overlapping writer admission, partial failure publication,
generation reuse, touched padding or claiming pointer lifetime from a number.
Hints: first enumerate actual indices; subtract before multiplying; compare
candidate state, not only return codes; empty right tiles need no derived pointer.
Detailed solutions remain assessor-only.

The local adversarial control targets only this original teaching book and
synthetic arrays in one ordinary process. Its capability is API input selection,
not native pointer fabrication or memory corruption. A missing conflict check
admits two incompatible writers to one known cell; the harmless observation is
the duplicated logical authority, not code execution. Repair with exact admitted
sets and writer policy, retain the regression and report residual raw-alias and
serialization premises. No external targets or security settings are involved.

## Cost review (thirteen dimensions)

1. Representation: four fixed descriptors, extent and three-field tokens;
   record sizeof with the actual data model.
2. Work: O(N squared) uniqueness and intersection for N<=64; split validates
   candidates twice. Four slots bound scans, not general scalable complexity.
3. Space: fixed64-index staging arrays and candidate book; no recursive traversal.
4. Allocation: none in core; host Python oracle allocation is not target dependency.
5. Traffic: explicit index materialization versus interval metadata comparisons.
6. Dispatch: direct C functions, no hidden callback or set-library dispatch.
7. Code size: preserve objects/disassembly and exact optimization flags.
8. Stack: preserve emitted .su observations; candidate copies have real stack cost.
9. Timing: seven1000-pair interleaved32-element writer samples, checksum448000;
   do not compare this workload with a simpler interval-only correctness contract.
10. Failure: shape validity, conflict, capacity, stale authority and write permission
    remain distinct; all ordinary refusal paths preserve outputs/state.
11. Scheduling: serialization required; no atomic publication or preemption proof.
12. Build: actual dependency closure, controls, clean and unchanged incremental runs.
13. Portability: C17 size_t arithmetic, finite uint32 identities, exact typed arrays;
    changed width/representation needs fresh generated-code and bound analysis.

Authored support, actual author execution, independent assessment and physical
validation are separate. Exact published C17 remains unavailable.
