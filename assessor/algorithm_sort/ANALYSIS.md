# Private PR-T36-03 analysis

These are author technical answers, not external review or learner acceptance.
The original criterion and seven-day V1 identities remain unchanged.

## T36.05.a: pending records and stable order

Before outer iteration i, prefix[0,i) contains its original full records in
stable sorted order. Save the complete record at i. After shifting predecessor
j−1 into j, regard j−1 as the hole; the duplicated physical predecessor is not
counted twice in the logical state. Array excluding the hole plus saved pending
record equals the original multiset. Each strict greater-key shift moves the
hole left; placement restores the complete multiset. The index decreases and
cannot wrap because it is tested before reading the predecessor.

Fixture completed prefixes are B,A,C,D,E; B,A,C,D,E; B,D,A,C,E; B,E,D,A,C.
The first identical-looking pair still represents a distinct completed
insertion with one comparison and one placement. The observer snapshots come
from actual assembled code after placement. During a shift the physical array
alone need not be a permutation; demanding that false invariant would reject
a correct algorithm. The code preserves the pending record in registers.

Descending order shifts strictly smaller predecessors, stops at equality and
produces A,C,D,B,E. Simply reversing the ascending output would reverse ties
and violate stability. The generated descending source changes only the strict
ordering predicate in the original assembly; all original stored sources stay
unchanged and both variants execute.

Actual inspected ODS merge uses `<0` to choose the left head and otherwise
chooses right. With `(1,B)` left and `(1,E)` right, it produces E,B. This source
is not evidence of the required stable policy. The actual prior T15 `ad_merge`
uses left on equality and the direct two-run checkpoint produces B,E,D,A,C.
The resource record distinguishes source comparison from implementation authority.

## T36.05.b: actual reuse and changed width

The original T15 producer admits signed32 keys and n<=16. Map unsigned u to
`INT32_MIN+u` when u<=INT32_MAX, otherwise `u−2^31`; each arithmetic operation
is performed only in its representable branch. This is an increasing bijection
of numeric values, not a representation cast. Its inverse subtracts INT32_MIN
only for negative mapped values, otherwise adds2^31 in unsigned arithmetic.
Copy full tags through the prior unsigned label field and assert its width.

The adapter actually calls frozen insertion/merge/heap sort implementations.
Stable outputs must match the assembly key/tag order on their common domain.
The frozen heap adapter uses the T14 max heap and writes successive extractions
from the end; its equal-key policy is not stable in final ascending order.
Compare sorted labeled multisets only and retain an observed nonstable example.
Outside n<=16, the new assembly is checked independently rather than claiming
the prior provider accepted an unsupported count.

Wide layout uses16-byte stride and tag at8. Native uses R8=i,R9=j,R10D=pending
key,R11=pending64bit tag,RAX=byte offset,ECX/RDX=predecessor key/tag. These are
all caller scratch; no call occurs and no callee-saved state changes. Thumb
saves r4–r10 andLR (32bytes): r4=i,r5=j,r6=pending key,r7/r8=tag halves,r9/r10
plusr3=predecessor fields,r2=record address. The full64bit tag moves with every
key; padding is not part of the value oracle. Existing preserved-register
probes actually call this new kernel at both architectures.

The truncation mutant retains only low tag bits. High-bit tags make it fail
even when key order is correct. Whole-record multiplicity and stability are
different properties, so a duplicate-drop or tag-swap cannot pass merely by
matching the final maximum or key sequence.

## T36.05.c: exact work and callback failure

For each original input position i, let g count earlier strictly greater keys.
Shifts=g; comparisons=g plus one if g<i (a stopping predecessor exists).
Placements=1 for each i>=1. The fixture gives g=1,0,2,3 and comparisons1,1,3,4:
six shifts, nine comparisons, four placements. Sorted5 gives4 comparisons and
no shifts; reverse5 gives10 comparisons,10shifts,4placements. The observer's
own copies/counter instructions are neither hidden nor mislabeled as kernel
record moves. A separate independent Python rank/inversion oracle consumes
every one of132 emitted metric rows and the four actual prefix snapshots.

The native trace thunk preserves nine caller-scratch registers,72bytes, after
the insertion call site reserves8bytes to align CALL. Thunk entry RSP mod16=8;
the72byte save makes it0 for the C observer call. Thumb instrumented insertion
already saves24bytes; its observer call saves r0–r3,r12,lr (24more), retaining
8byte alignment and all live caller-scratch values. Original saved-register
state remains restored at return. The extra copy/stack/call work makes this an
instrumentation build, not a fair timing substitute for the unobserved leaf.

The fallible comparator owns its context. A32-record staging buffer permits
array rollback on any callback failure or invalid relation. On error, report
attempted callback/shift/placement work and keep input unchanged. At a failure
after an earlier staged insertion, copying staged data back is a real violation
even if it remains sorted in part; the intended mutant is rejected there.
Do not claim rollback of callback I/O or context increments. Tests inject both
nonzero status and invalid relation at all nine comparison positions. A caller
that supplies contradictory order results cannot demand a sorted outcome.

## Evidence limits

Both native and actual Cortex-M4 executions assess original and changed
mechanisms; C sanitizers do not instrument assembly memory operations. Object
and final image helper audits are separate from hosted prior-sort comparisons.
No target timing, physical observation, external review or independent delayed
learner completion is inferred. Exact original tasks remain intact; every
other T36/T37 project is still separately required.
