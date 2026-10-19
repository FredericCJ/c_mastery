# Original PR-T36-08 depth audit and implementation map

Inspected actual parent project, all six original supplement criteria and V1
tasks, fixture, algorithm header, completed T14 public representations/contracts
and actual tree/ring/slot-queue functions, plus T18 OWNERSHIP.md. The generic
project working card mentions unrelated sort work; the concrete typed-interface
contract is operative. Existing declarations and abstract fixture replay did
not supply actual wrappers, backing-failure transitions or live-view admission.

The additive family retains the actual32-node CDTree as a unique-ID membership
index and layers rank-ordered typed storage over it. This distinction prevents
crediting an ID comparator with a rank-order invariant. The actual CDRing and
CDQueue remain unchanged and expose their different physical layouts through
seven observed states. A source-derived free-slot count is not hardware timing.

Six leaf dispositions: .02a typed layout plus generation/retirement; .02b actual
queue traces plus movable-backing success/failure; .02c readonly token lifetime;
.10a real value/pointer arrays and original System V/Thumb kernel plus finite
dispatch comparison; .10b copy/borrow/context admission plus empty epoch switch;
.10c duplicate/full precedence and all five fallible backing update boundaries.
All original criteria/V1 identities and baseline bindings remain byte-equivalent
objects. New execution lives in a successor, never overwriting accepted evidence.

N1570 primary PDF passages6.2.4 paragraphs1–2 and6.5 paragraphs6–7 were actually
read through the accessible PDF text on2026-10-18. They establish the precise
limits of declared object access and lifetime; this is a C11 draft, not final
C17. No new C17/C23 inspection is invented. The exact earlier bounded SysV and
Arm passages are reused explicitly from the source register. Private primary
copies stay outside publication; public references expose their original URLs.

First campaign passed Cortex and GCC but stopped at Clang misleading indentation
in the test harness. The failed115-command receipt is preserved. Formatting was
corrected without suppressing warnings. Target mutants now select the relevant
case so a different earlier assertion cannot count as the intended failure.
Final source-bound execution must include the settled public/assessor contracts.

External review, independent delayed learner assessment and physical observation
remain unperformed. The code operates on caller-guaranteed live, serialized
objects. It cannot detect dangling controls or enforce callback purity; no
universal language-level borrow-safety claim follows from the finite tests.
