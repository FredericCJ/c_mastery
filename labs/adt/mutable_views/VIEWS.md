# T67.06: Mutable layouts and equivalent checked reductions

This required X-T67-006 successor retains the actual read-only `ad_view` and
`ad_view_get` producer. Implement the supplied `mutable_views.c` and `vector.c`
stubs after the original `bytes.checked` and ADView work. The five anchors are
`view_geometry`, `view_transform`, `view_vector`, `view_reduction`, `view_base`.
The separate assessor directory contains the reference and technical analysis.

## Exact storage and arithmetic contract

`AdvView` borrows a live writable uint32 array with a real capacity in elements,
shape and positive element strides. It owns no storage. A nonempty shape uses
offset `r*row_stride+c*column_stride`; required extent is one beyond the largest
offset, not the logical element count. Empty shapes have required extent0 and
allow NULL and zero strides. No empty operation performs pointer arithmetic.

`adv_geometry` is a pure arithmetic admission calculation: reject missing output,
nonpositive strides for a nonempty shape, unrepresentable products or required
extent, and required extent beyond capacity. Return required extent and an
injectivity flag otherwise, preserving outputs on refusal. This function may
test SIZE_MAX numerically without claiming an array of that size exists. Real
constructors still need real backing storage. For positive strides `rs,cs`,
let `g=gcd(rs,cs)`. A nonempty rectangle is injective exactly when
`rows<=cs/g || columns<=rs/g`. Derive this from the smallest positive solution
of `dr*rs=dc*cs`; do not substitute a merely sufficient row-major condition.

`adv_view` calls the actual ADView constructor and refuses noninjective layouts
with ALIAS. It permits holes and unusual injective layouts. For example2×2 with
strides2/3 has offsets0,3,2,5 and required extent6;2×2 with strides2/2 aliases
offset2 and is rejected for mutation. The retained read-only constructor permits
overlap, and the reduction below counts every logical coordinate, including
repeated physical elements. That is deliberate, explicit repeated-read semantics.

Every nonnull pointer/control/output denotes live aligned storage of its stated
type. Outputs must be disjoint from descriptors and backing arrays. Descriptor
fields are constructor-derived and not modified or forged between operations.
All operations are externally serialized; injectivity of one view does not
establish exclusive access against another copied or overlapping descriptor.
The caller retains storage lifetime and controls mutation. No view copy extends
lifetime, transfers ownership or enforces a borrow checker in ISO C.

## Transpose and submatrix

`adv_transpose` swaps both dimensions and strides, borrowing the same backing
array. `adv_set` checks row/column and writes exactly the selected element. A
subview admits `start<=extent` and `count<=extent-start` independently in each
dimension, without overflowing an addition. Nonempty subviews shift the data
pointer by the admitted original offset and reduce remaining capacity by it.
An empty subview retains the original data/capacity and inherited strides; this
canonical empty representation never forms NULL+0 or an otherwise invalid
end-corner pointer. Missing output is INVALID; out-of-range coordinates return
RANGE unchanged. These functions take admitted views, not fabricated geometry.

Concrete transformation: backing `[1,2,3,99,4,5,6]`, shape2×3,strides4/1. Setting
transpose(1,0) changes backing1. Subview(start1,1;shape1×2) starts at backing5 and
has capacity2. An invalid subview leaves its old descriptor untouched. The
independent model also applies these writes for every admitted small layout and
compares a weighted checksum of the entire backing range, including holes.

## Reduction and vector boundary

`adv_reduce` receives an admitted immutable-for-the-call ADView and returns a
transactional `AdvReduction {sum,backend,vector_blocks,scalar_elements}`. The
mathematical sum must fit uint32; any overflow leaves the whole output unchanged.
Empty shapes return0 without access. Logical count must fit size_t, so check
`columns<=SIZE_MAX/rows` before counting nonempty work. No per-element allocation,
callback or hidden I/O exists. Out-of-range modes/missing outputs are INVALID.

SCALAR traverses logical row-major coordinates with checked accumulation.
VECTOR requires column_stride1 for a nonempty shape; otherwise return LAYOUT
unchanged. AUTO chooses VECTOR for that stride and SCALAR otherwise. Each vector
block consumes exactly four values within one row, with scalar tails. Row holes
are skipped. A transpose often requires AUTO's scalar fallback; calling it
VECTOR must not pretend its logical row is contiguous. Reduction does not
mutate backing storage, including when aliases represent repeated coordinates.

The separate backend is an original GNU/Clang vector-extension implementation,
not ISO C or a library reduction. It uses two uint64 lanes, widens four uint32
inputs before addition and horizontally combines the two lane results. One
block is at most4×UINT32_MAX and fits uint64. Check that exact block against the
remaining uint32 result capacity before publishing any result. A 32-bit lane
addition before widening silently loses information and is a named control.
`adv_vector4` requires exactly four accessible uint32 objects, ordinary uint32
alignment and no16-byte alignment promise. It does not cast their addresses
to vector pointers. Generated code must be inspected for actual packed adds.

The executable profile is hosted Linux x86-64 with its working SSE2/XMM ABI,
GCC or Clang and `-msse2 -mno-avx`. The build refuses a missing x86-64/SSE2
compile-time profile. Automatic loop/SLP vectorization is disabled so the
deliberate vector primitive can be compared with the declared scalar traversal.
This is not dynamic dispatch for arbitrary CPUs or an OS-state detector. Running
the same binary on another ISA/ABI requires a new backend and target admission.
The compiler extension's permitted scalar fallback is why a source vector type
alone cannot prove emitted SIMD. Actual O0/O3 object audits check packed64-bit
adds and XMM registers. A GCC debug object imports `__stack_chk_fail`; that is
an observed hosted compiler helper, explicitly allowed rather than removed or
misreported as dependency-free. No studied reduction mechanism is delegated.

## Language comparison and non-equivalence

The inspected C++ span overview describes borrowed contiguous access, and its
constructors require a valid range. The actual C++20 probe copies a mutable
span, takes a subspan, mutates through it, and rejects removal of const element
qualification with a compile-time constructibility assertion. A const span
descriptor alone does not make its referenced elements const.

The inspected rolling `layout_stride` draft additionally requires positive
strides, representable span and a dimension permutation satisfying its stated
stride-ordering condition. The2×2 strides2/3 example is injective here but does
not satisfy that constructor precondition. Do not construct that invalid C++
mapping or claim this C API is a drop-in implementation of all mdspan contracts.
Current installed GCC/Clang standard-library headers lack `<mdspan>`; the exact
failed availability probes are recorded, and no mdspan execution is claimed.
Published C17/C23 texts remain unavailable; extension semantics are not ISO C.

## Required evidence and assessment

Submit the extent and gcd proofs, alias/lifetime ledger, code and prediction
before execution. Enumerate small shapes/strides/capacities using a coordinate
set oracle independent of the gcd implementation. Compare retained admission
and getters, mutable admission, transformed writes, scalar/vector/AUTO sums,
overflow preservation, tail counts and noncontiguous refusal. Test empty/NULL,
singleton huge strides, SIZE_MAX multiplication/addition, zero strides, holes,
read-only overlap, interior unaligned-to16 input and widened maximum values.

Run six GCC/Clang debug/release/ASan+UBSan profiles. Ten controls independently
omit injectivity, omit gcd reduction, retain old transpose strides, calculate a
row-major subview origin, narrow before widening, omit a vector lane, wrap the
result, omit tails, assume retained row-major access or omit retained capacity
admission. Each must compile and fail an ordinary ADO_CHECK assertion; a crash
or sanitizer finding is not the required semantic rejection. Five learner
anchors must compile and reject. Preserve raw code-generation and dependency
observations, not just an asserted backend name.

R1 assesses extent versus logical count and a true injective/overlapping
counterexample. R2 requires checked arithmetic and the exact distinction from
span/mdspan preconditions. R3 requires independent coordinate/model evidence
and both retained controls, not only successor tests. R4 requires actual mutable
transpose/subview writes, fallback/forced-vector admission, widened equivalent
overflow and alias semantics, and measured generated backend. Level1 is public
tests only; level2 includes proofs/contracts/full artifacts; level3 adds the
unseen changed condition and an explanation of which old argument broke.

For X-T67-006-DELAYED change contiguous rows to padded rows with nonmultiple-of4
width, transpose a submatrix, and preserve a read-only overlapping source.
Require exact mathematical sums and refusal outputs across SCALAR/VECTOR/AUTO,
then force an overflow in the last tail. A vector load across row padding is not
a valid repair. Separately compare the unique interleaved2/3 layout with the
stronger C++ mapping premise. The supplied model supports these changes; an
unaided delayed learner attempt and its rubric assessment remain unobserved.

Hints: (1) find the smallest positive coordinate difference producing equal
offsets; (2) keep logical shape separate from backing extent; (3) widen before
adding, and keep output publication after every failure boundary.

## Thirteen cost and enforcement dimensions

| Dimension | Evidence and bound |
| --- | --- |
| Representation | View size/alignment measured; typed pointer plus five size_t fields, ABI padding retained. |
| Reachable storage | Borrowed caller array and stack descriptors; no owned heap graph. |
| Lifetime | No destructor or ownership extension; caller keeps stable backing through each operation. |
| Mutation/aliasing | Exact single-view injectivity; cross-view exclusion is caller discipline; repeated read-only coordinates are explicit. |
| Allocation | No allocation in core/backend; inspect actual unresolved symbols and hosted harness dependencies. |
| Movement/copying | Descriptor copies only for transpose; subview changes base; no backing copy, vector primitive reads exactly4 elements. |
| Destruction | No owned resource; invalidation follows caller storage lifetime, not a hidden reference count. |
| Dispatch | Direct scalar loop/vector4 calls, AUTO layout branch and observed XMM packed adds. |
| Concurrency | External serialization, no atomics/publication/reclamation guarantee; a simultaneous writer violates the contract. |
| Failure | Whole output unchanged for range/layout/overflow; set changes only one admitted coordinate. |
| Compilation | Actual clean/unchanged builds, strict-core diagnostics and separate extension/code-generation observations. |
| Binary/stack | Recorded executable size, stack-usage and disassembly; constant scratch space, no portable exact ABI size claim. |
| Boundary | Hosted x86-64/SSE2 extension versus ISO scalar core; pointer extent/lifetime and C++ library preconditions remain explicit. |

Metrics use one scalar/vector warmup followed by seven1000-reduction samples of
the same padded32×64 array of ones. Each result is2048, each aggregate checksum
2048000; report raw monotonic durations, layout, compiler and flags. Both paths
have the same checked sum/read-only contract. No measured speedup is assumed,
and instrumentation builds are not production performance estimates. Runtime
validation, compiler diagnostics, external algebra and caller discipline are
different enforcement mechanisms. Finite models, silent sanitizers and emitted
instructions establish neither all inputs nor absolute mastery.

Security uses only these original local synthetic arrays and controls. Incorrect
alias admission or a wrong sum is a harmless teaching property violation; no
external service, deployed equipment or arbitrary code-execution claim applies.
Author validation status comes only from current source-bound reports. Physical
execution, mdspan library execution, independent learner assessment and non-author
review remain unclaimed.
