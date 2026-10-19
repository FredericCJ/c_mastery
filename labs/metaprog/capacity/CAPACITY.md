# T68.02: constant proof and runtime admission

The original mp_storage_bound and its checked-product/budget contract remain the
producer for this required successor. Prerequisites are X-T68-001 repeated
inclusion, the actual containers.ring FIFO contract from PR-T14-01 and bytes.checked
reasoning from PR-T04-01. Retain those learner artifacts; implement specialization
locally and call your actual mp_storage_bound for runtime storage admission. No
ready-made ring, compiler constant-detection builtin, allocator or wider-integer
library implements this mechanism. The Python mathematical product is host-only.

Use ISO C17 core, exact uint32_t and size_t, GCC/Clang hosted six profiles. The
separate i386 assembly entry/raw Linux write/exit adapter is an explicit target
ABI extension, not ISO startup or hosted libc. It provides actual32-bit size_t
execution when its recorded toolchain/emulator is available. Distinguish that
from a hosted multilib availability probe and analytic16/32/64-bit models.

## Exact operations and representation

Mpc1/Mpc4/Mpc8 hold actual inline arrays, head and count; zero-initialize them.
Their template has a positive integer constant-expression capacity, power-of-two
assertion and storage-extent assertion. Push rejects FULL unchanged; pop rejects
EMPTY without touching output. Missing control/output pointers return INVALID.
Success appends/removes one uint32_t in FIFO order. No slots own resources and
unused slots need not be zeroed. A fixed ring can be copied as an independent
value only with caller exclusion; it has no borrowed backing pointer.

MpcRuntime borrows a caller uint32_t array and records capacity/head/count plus
mask and the constructor-derived power2 flag. Init needs a fresh/nonowning
control and live backing for the requested capacity. Null control/data is INVALID;
zero capacity, unrepresentable capacity*sizeof(uint32_t), or product beyond the
byte budget is RANGE. Refusal preserves all control and backing fields. Success
publishes empty control and does not read or clear backing. The byte budget is
an application limit, not evidence that native backing storage exists. Runtime
capacity may be nonconstant and may be any positive value passing these checks,
including3,5,17; this API does not silently round it up or reject all nonpowers.

After init, controls are live, initialized, unmodified except through these
operations; storage remains live/stable until the ring is no longer used. Controls,
backing and outputs are disjoint. Calls are externally serialized. Runtime ring
copies would alias backing and need a separately defined protocol; this exercise
does not permit them. No callback, allocation, destructor or retained pop borrow
exists. Pop copies one value and the caller may then reuse that slot.

Derive head<capacity and count<=capacity. For positive power2 capacity,
`(head+distance)&(capacity-1)` is the required index; the largest representable
power-of-two bound makes the admitted sum representable. For other capacities,
let remaining=capacity-distance with distance<=capacity. If head>=remaining use
head-remaining, otherwise head+distance. This avoids an overflowing sum even
before modulo; derive each branch's range. The fixed template uses only its
constant mask, while the runtime path branches on the validated stored flag.
The nonconstant caller never reaches a compile-time assertion disguised as a
runtime validation. Forged public fields remain invalid caller state.

Examples: fixed4 push0..3, pop0, push4 then pop1..4; runtime3 repeats0,1,2 and
wraps without power-of-two masking. Capacity4 with budget15 refuses unchanged;
budget16 succeeds on the8-bit-byte/32-bit-word profiles. count=SIZE_MAX,width2
refuses before product; count=SIZE_MAX/4,width4 is representable independently
of whether the application budget admits it. Zero count/width is invalid for
this original storage API even when the mathematical product is zero.

## Diagnostics and changed assumptions

The inspected N1570 C11 draft5.1.1.3 requires a diagnostic for a syntax/constraint
violation, not necessarily translation rejection. Clauses6.6/6.7.10 constrain
integer constant expressions and assertions;6.7.6.2 separates positive fixed
array extents from variable length arrays and notes conditional VLA support.
These are bounded C11 draft passages, not newly accessible published C17/C23.
Keep exact-edition access gaps. Current C2x compiler acceptance is not normative
C23 evidence; record it separately if explored.

Translate the actual template at capacities1,4,8 successfully, and capacity3/0
with the named MPC_POWER_OF_TWO diagnostic. A function parameter used as an
assertion operand is not an integer constant expression; capture that diagnostic
at syntax analysis, not a missing-link-symbol failure. Separately compile a
valid but unused local with and without -Werror=unused-variable: that policy
rejection is not a C constraint. Demonstrate a block-scope VLA accepted by the
available compiler and rejected by the chosen -Werror=vla policy; no blanket
portable-VLA guarantee follows. Compare actual Rust const usize parameter storage
and a disallowed floating const-parameter type under its inspected restrictions.
Neither macros nor a constant array prove an abstract trait/resource law.

## Required verification, artifacts and grading

D1 classifies each diagnostic/guard and captures positive controls. D2 supplies
product/budget and indexing proofs plus backing lifetime. D3 runs the retained
product controls, independent mathematical and FIFO models, actual32-bit target
adapter where available, and exact compiler/dependency/assembly observations.
D4 implements both constant specialization and arbitrary runtime fallback with
unchanged failure state, then compares equal capacity4 workloads.

Four anchors separate fixed, runtime, admission and actual retained product.
Eight compiled controls break fixed tail/head, mask a nonpower capacity, bypass
budget, publish before rejected init, overwrite empty output, and omit retained
overflow/budget guards. Require ordinary MPO_CHECK failure, successful build and
the intended selected anchor; a signal or sanitizer finding is not that result.
The Python oracle models a logical queue and unbounded multiplication, not C
indexing or wrap arithmetic. Its16/32/64 analyses must remain labeled analyses;
the separately linked/executed32-bit program checks its actual sizeof(size_t).

Score R1 diagnostic classification, R2 bounds/constant-expression/source limits,
R3 actual independent falsification/changed-width evidence, R4 real fallback and
specialization. Public anchors alone earn at most level1. Level2 requires each
proof and output/failure ledger. Level3 requires the seven-day delayed attempt:
change capacity from4 to3, reduce byte budget below representable storage, and
change size_t width; identify the invalidated masking/arithmetic argument before
repair. Author support is not an unaided attempt. Hints: divide before multiply;
separate representability from budget; prove the nonpower wrap branch without
forming a potentially overflowing sum. Assessor code stays separately packaged.

## Thirteen costs and enforcement boundaries

Representation: record fixed inline storage and runtime descriptor separately.
Reachable storage: runtime borrows capacity words; include those bytes in totals.
Lifetime: caller retains backing, while fixed values embed it. Mutation/aliasing:
serialized disjoint outputs, no shared descriptor-copy protocol. Allocation:
none in core; target/harness dependencies are audited separately. Movement/copy:
one word per push/pop, fixed descriptor copies also copy array whereas runtime
copies are forbidden. Destruction: no owned resource or cleanup loop. Dispatch:
fixed constant masks versus runtime validated branch and nonpower conditional
wrap; inspect actual O0/O3 instructions, not a presumed optimization. Concurrency:
no atomic publication/reclamation or multi-producer guarantee. Failure: full,
empty and invalid init preserve specified objects. Compilation: actual constant,
VLA/policy/Rust probes and clean/unchanged builds. Binary/stack: object sizes,
symbols, stack records and32-bit final-link undefined-helper audit. Boundary:
ISO core versus compiler diagnostics and i386/Linux ABI, application budget
versus pointer validity, analytic width versus executed target width.

Seven100000-round fixed4/runtime4 samples each push/pop four values with exact
checksum600000. Include runtime initialization and report storage/code/branches;
do not remove required checks from either measured implementation. No arbitrary
speedup, physical timing, unavailable published-standard inspection, non-author
review or learner mastery is inferred. Local synthetic arrays/values only;
wrong FIFO or unchanged-output violations are harmless teaching outcomes, not
arbitrary execution or attacks on an external target.
