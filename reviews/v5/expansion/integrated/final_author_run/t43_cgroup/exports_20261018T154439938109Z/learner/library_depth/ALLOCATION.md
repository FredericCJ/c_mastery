# T12.07: allocation is an ownership transition

Required PR-T12-03 criteria T12.07a/b/c and their original delayed transfers,
families A/B/C/D/F/G/H/I/J. Prerequisites are the actual learner checked-size
primitive, sole ownership, alignment, effective type and error-result contracts.
Complete allocation.c using allocation.h and the prior f_checked_size artifact.
This C17 hosted adapter investigates allocation contracts; it does not implement
an allocator's free-list mechanism. The callbacks are permitted primitives here.
Keep target allocator implementation exercises under their own dependency rules.

An empty la_owner is all-zero metadata. A live owner uniquely holds data, logical
length, allocated capacity and required alignment; capacity>=length>0. Its same
operations table/context must outlive every operation, and all outstanding borrows
must end before resize or release. C enforces none of these ownership rules.
Callbacks are synchronous, non-reentrant, respect extents and alignment, and do
not retain source/destination borrows. Release cannot fail under its valid-input
contract. A copy failure may modify only the new destination, never the original.

`la_create(count,width,alignment)` rejects zero count/width, checks multiplication
through the actual predecessor, admits a supported power-of-two alignment, and
rounds physical capacity up with a separate checked addition. The conservative
size-multiple contract works across the inspected editions. A backend's support
predicate is part of its declared contract, not a portable query supplied by ISO C.
No allocation call occurs for arithmetic or alignment rejection. A null result
leaves the empty owner unchanged. The bytes are uninitialized until the caller
establishes an admitted representation/value. calloc's zero bits do not universally
construct a null pointer or floating zero; use semantic member initialization.

`la_resize_native` accepts only a live owner with fundamental alignment and a
nonzero requested logical length. The selected realloc-like callback must preserve
fundamental alignment and the prior prefix on success, and retain the old owner
on nonzero failure. Hold its result temporarily; do not overwrite the sole pointer
before testing failure. After success, the old object's lifetime has ended even
if the returned address happens to be unchanged. Invalidate all old borrows.
Extended alignment instead requires `la_resize_copy` with the explicit adapter.

Zero size is not overloaded onto resize. Both resize operations reject it before
calling the backend; the caller chooses the separate `la_release` operation.
Release clears the metadata after exactly one valid release. Releasing an already
empty owner is a no-op. Examine an original adverse callback whose zero request
returns null while retaining storage: the adapter never delegates that request,
so cleanup remains unambiguous. Do not execute an unspecified/undefined library
case merely to choose the ownership policy you prefer.

For fallible allocate-copy-free, allocate fresh rounded capacity; copy only
min(old logical length,new logical length); on any copy failure release the fresh
allocation and retain the original owner/bytes. Only after copy success release
the old allocation and publish the new owner. Peak owned storage during this
transition is old_capacity+new_capacity, and copy cost is linear in the preserved
logical prefix. A callback that can corrupt source, release old storage or fail
after transferring ownership requires a different contract; this adapter cannot
roll back those effects. Explain the resource tradeoff with native realloc, whose
internal movement and allocation policy remain implementation-specific.

Derive and test: SIZE_MAX*2; rounding SIZE_MAX up to64; zero count/width; unsupported
alignment3/128; first allocation failure; ABCD resized4→8 with first native failure
then retry; copy failure after every prefix0..4; shrinking; and a real alignas64
concrete array resized through fresh aligned storage. Distinguish logical length
from capacity and prove that no uninitialized extension bytes are read. The
bounded synthetic backend records live slots and exact releases, enabling safe
semantic mutants without creating dangling accesses. The separate hosted notebook
uses actual malloc, calloc, realloc and aligned_alloc on admitted arguments.

Read R-LLIB-N1570-ALLOC, R-LLIB-DR400-460 and R-LLIB-N3096-ALLOC. The C11 draft
describes zero-size allocation as implementation-defined. WG14's October2017
DR400 record clarifies nonzero failure retention, makes zero-size deallocation
implementation-defined and calls that use obsolescent. Its final DR460 proposal
differs from its initial proposal; do not conflate them. The April2023 N3096 draft
makes realloc(nonnull,0) undefined. These are exact draft/defect-record comparisons,
not a claim to have inspected the published C17 or C23 standards. The selected
adapter avoids the disputed boundary and records its own stricter alignment policy.

Deliver code, owner-state diagram, callback assumptions, cost derivation, fault
trace and allocation/free balance. Run allocation_tests.c with the actual prior
checked-size artifact under GCC/Clang O0/O2 and hosted sanitizers; separately
record native library/ABI behavior. Require intended rejection of lost-owner,
overflow and leaked-staging mutants. A sanitizer pass cannot establish an ownership
proof or callback discipline.

Rubric:0 loses/leaks/duplicates ownership;1 passes one resize;2 derives every
failure transition, alignment and initialization premise;3 independently solves
the original over-aligned, adverse-zero-callback or fallible-copy condition after
at least7days. Hints: preserve the old owner first; distinguish logical and rounded
sizes; locate the only safe commit point. All demonstrations use original local
programs and synthetic bytes. No independent learner/reviewer or physical evidence
is inferred from author execution; assessor solutions are packaged separately.
