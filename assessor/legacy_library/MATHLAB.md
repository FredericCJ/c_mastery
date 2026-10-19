# Withheld numerical-library analysis

Require the exact type table and ordinary/changed criterion bindings; an
observed NaN alone is not a complete domain-error explanation. tgmath's
integer rule selects double, while long-double-complex dispatch requires the
complex suffix/larger ABI. Inspect object calls with builtins disabled and run
the selected calls. Treat optimized instruction substitution as a compiler
fact, not a change in the macro's semantic type.

The observer stages a local sample, captures errno before any later library
calls can disturb it, and restores the caller before publication. A stale
errno mutant fails sqrt(4); a missing restoration mutant loses the saved
FE_DIVBYZERO; a fast-policy bypass calls a harmless identity callback and is
rejected before any dangerous inference from NaN/exception behavior. A failed
fesetenv cannot be repaired by claiming unconditional restoration: ENVIRONMENT
means no published sample and potentially unknown floating state. The public
contract states this limit. Legitimate implementations may use feholdexcept
only after explaining the additional non-stop/trap-mode semantics and restoring
the exact saved environment; disabling traps silently is not interchangeable.

MPFR arithmetic is independent of the scalar library algorithm being measured.
Exact double input plus a directed interval with both endpoints rounding to the
same double settles nearest rounding for that input. It does not turn a512-bit
printed midpoint into an exact transcendental value. Near zero the relative
metric requires its explicit convention; at binade boundaries local-upward-ULP
and representable-step distance need not be identical. Reject an oracle that
accepts18 repetitions of sqrt(0), ignores a trailing partial line, uses the
library under test to generate its own expected result or silently accepts an
ambiguous interval. The maintained checker includes each of those admission
boundaries relevant to this finite fixture; forged sqrt(2)=0 fails numerically.

The glibc manual distinguishes fully specified operations from its accuracy
goals for other math functions. The musl source uses range-dependent rational
approximations and sqrt-based correction near1. Neither reading proves every
scalar acos input is within a particular ULP count. Four representable steps
is an application qualification screen explicitly chosen for the finite set.
It is not taken from glibc's vector4-ULP paragraph. For the delayed move to an
implementation without a rounding promise, award full marks only if the learner
refuses to derive a fabricated guarantee from the name or a tiny sample and
states what additional bound, source proof or application qualification would
be needed. They must still run and analyze the supplied finite comparison.

After at least7days supply public interface, input list and the original three
changed tasks; withhold this file, observer reference and oracle implementation.
Require independent state diagrams and a fresh sensitive input justified by
analysis, not a copied error table. A learner may choose adaptive MPFR precision
or exact rational reasoning for exact operations if the error proof is valid.
Parallel authoring or successful author execution is not independent assessment.
