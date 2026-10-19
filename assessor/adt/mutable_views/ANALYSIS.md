# Assessor: Exact mutable rectangles and widened reduction

A collision needs positive `dr*rs=dc*cs`, because both strides are positive and
the nonzero coordinate differences have opposite signs. Divide by gcd to get
the least positive difference pair `(cs/g,rs/g)`. It fits the rectangle iff
`rows>cs/g && columns>rs/g`; negating gives the admission predicate. Empty and
singleton cases are separate, so no modulo-by-zero or pointer arithmetic is
used. Bounding only the largest offset is insufficient for injectivity.

For2×2 strides2/3 the offset set is{0,2,3,5}. Neither stride is at least twice
the other, so this accepted finite mapping does not meet the inspected C++
layout_stride constructor's permutation requirement. Assess the exact source
precondition rather than overgeneralizing its uniqueness observers to all
possible C index maps. The installed mdspan header is absent; recorded negative
availability probes are not execution of an invalid mapping or a library defect.

Transpose preserves the map by exchanging coordinate roles and strides.
Subview keeps the original strides and shifts by an already admitted coordinate.
For empty children, retaining the base instead of calculating the requested
end-corner avoids forming a pointer outside the real object. A valid empty
descriptor has no accessible coordinate, so that representation is sufficient.
No descriptor grants exclusive alias rights to backing storage; assess that
separately from injectivity within one coordinate rectangle.

The independent model enumerates coordinates and compares their set cardinality,
never using the gcd test. It reads through actual retained getters, compares
every reduction policy and mutates transpose/subviews against an independent
whole-backing checksum. Narrow uint32 vector additions can wrap before a later
cast; widening input lanes first bounds each pair by2×MAX and each block by4×MAX.
Block admission against `MAX-current` makes scalar and vector overflow policies
equivalent for nonnegative inputs; their precise first failing read need not be
identical, because no callback, volatile access or concurrent mutation is admitted.
The results count logical coordinates even if the read-only map overlaps.

VECTOR refuses a noncontiguous logical row. AUTO can fall back; pretending a
transposed view has contiguous rows would read the wrong coordinates. Full
four-element blocks stay within a row, scalar tails handle the remainder, and
the output is assigned only after all checks. An overflow in a final scalar
tail must preserve all result fields, not merely the sum. The widened backend
has actual packed64-bit adds at both O0/O3 for GCC/Clang, verified separately
from the source extension spelling. Disabling automatic vectorization does not
disable explicitly declared vector arithmetic.

The first object audit assumed no unresolved helper and found
`__stack_chk_fail` in the GCC debug object. Its raw failed audit log remains.
The final declared hosted audit permits exactly that observed protection helper
and records it; it does not disable protection or claim a freestanding image.
The core's strict diagnostics and the backend's GNU extension have distinct
authority. A successful x86-64/SSE2 run is not an ISA/OS capability detector for
an arbitrary foreign deployment.

Use the four original rubric criteria and the exact thirteen rows in VIEWS.md.
The delayed padded/transpose/tail and overlap cases must retain the old result
on overflow and explain which mapping/backend precondition changed. Source
inspection, finite model results, author execution, missing mdspan tool support,
non-author review and learner assessment remain separate evidence dimensions.
