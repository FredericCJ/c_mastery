# Exact T04 floating completion audit

The retained PR-T04-04 card had twelve criterion-specific ordinary tasks and twelve
seven-day variants, but no complete executable package for them. Existing T17
summation/environment work and T12 math-library work are useful comparisons, not
proof that T04's caller-state, changed-format, fixed-point and minimization tasks
were implemented. In particular the older standalone numerical environment probe
does not provide this library routine's save/restore/failure contract. This
successor introduces no reverse learner dependency on T12 or T17.

The new package distinguishes core construction, exact rational expectations,
directed MPFR intervals, actual compiler/ABI/evaluation experiments and raw timing.
An unchanged earlier fixed-point reference is imported with explicit headers and
source identities. All original criteria, expected results, hour metadata and
historical status remain unchanged; an additive attachment links the support.

Pilot findings are part of the evidence, not deleted successes. The first FTZ
pilot restored MXCSR before classifying its subnormal result; Clang's classification
set the denormal-operand status bit afterwards. The correction classifies before
the restoration check and restores again after output. Frozen-tolerance reduction
found a two-element counterexample instead of the initially assumed three-element
one. The first full pilot rejected Clang's unused link-only `-no-pie` flag on an
object compilation; the corrected producer uses it only at link time. Final
receipts must be generated after all package source edits.

The second full pilot also exposed Clang21's NaN/fast-math diagnostic. It now has
its own negative translation check; the warning remains in the observation build.
The third pilot showed that Clang's local NaN result differs from GCC's. The
correction records each compiler's raw output without assigning a universal result
to a violated relaxed contract; GCC still demonstrates the missing-data defect,
both demonstrate the sign-policy mismatch, and the strict adapter must return3.

Actual source inspection: N1570 PDF-extracted5.2.4.2.2,7.6/7.6.1/7.6.4,
7.12.2/7.12.3.1/7.12.13.1; GCC15.2 optimization option passages;
MPFR4.2.2 rounding, import/conversion, directed arithmetic and sum descriptions;
Intel2025 FTZ/DAZ page lines1566–1594. The Intel page was substantively accessible
through the browser, but direct Python download returned HTTP403. Record that
cache limitation without falsely calling the inspected passage inaccessible.
N1570 is an older C11 working draft, not published C17; the exact-standard access
gap remains elsewhere in the canonical source inventory. No textbook inspection,
decimal C hardware, physical observation or independent learner attempt is claimed.
