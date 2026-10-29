# T19 iteration, callbacks and continuations

This required PR-T19-03 packet binds T19.07.a/b/c and their original seven-day
changed tasks. Families A/B/C/F/G/H/J. Import the actual T18 `struct ac_epoch_owner` and
foundation `struct f_view`, not a replacement implementation. Prerequisites: T18 borrow
lifetimes, value versus structural epochs, and T19 callback/reducer contracts.
Use C17 hosted GCC/Clang O0/O2 and ASan/UBSan; host inspection tools are separate
from dependencies of the mechanism. No hardware or OS asynchronous I/O is used.

## Pull owns progress; the owner owns storage

Implement `struct at_cursor` with a borrowed owner control, its captured view/generation,
position, threshold0..255 and terminal status. The owner control must remain alive
until termination, even after logical destruction or structural invalidation.
On each active `at_next`, check logical liveness and structural generation before
testing end or reading. Return ITEM and advance only if the next byte is at most
the threshold. Otherwise return STOP without visiting that byte. END/STOP/STALE
are idempotent and preserve the output. Terminal calls do not access the owner.
Invalid arguments preserve caller output. Value-only updates are observable by an
existing cursor; structural reset invalidates it. A generation check on a freed
control pointer is already too late. ISO C does not enforce this borrow lifetime.

For `[2,4,6,8]` and threshold5, require exactly2,4, sum6, then STOP. Test empty,
singleton, repeated terminal calls, value updates, generation3 to4 reset and
logical destruction with live control storage. Compare all lengths0..4 and
thresholds0..9 with an independent prefix calculation, not only with another
implementation. No portable result is assigned to actual use-after-free.

## Push owns control; callbacks retain obligations

`at_push` visits synchronously using the same cursor rules. CONTINUE advances;
STOP_NOW stops after the current visit; REMOVE returns DENIED without removing
anything. Unknown callback results are invalid. The visitor may perform a
value-only update or a structural reset; the next iteration observes their
different effects. Owner/control storage must remain live throughout the call.
Nested visitors share `struct at_push`; allow at most three active levels and reject
the fourth before calling it. Reset visits/max-depth at the outer entry, not at
recursive entry. Count callback work separately from representation bytes.

The changed task uses `struct at_deferred`: zero-initialize, explicitly start, then pump
with an item budget. Zero budget leaves the operation active and makes no callback.
Cursor and callback context survive between pumps. All methods are serialized.
Cancel outside a callback is a synchronous quiescence acknowledgment: clear
callback/context, mark terminal, and guarantee no later callback. Reentrant pump
or cancel while running rejects. The visitor cannot free the control or escape
nonlocally. Test budgets0,1,2, partial progress, cancel after one item, repeated
cancel/pump and owner invalidation. Before start, pump/cancel reject. This local
scheduler contract is not equivalent to concurrent OS cancellation; changing that
assumption requires an in-flight count, synchronization and a distinct acknowledgment.

## Replace the C call stack with an explicit continuation

Nodes have byte value and one-based child indices; zero is absent. Bound the
fixture at seven nodes, traversal depth at three and output capacity at0..7.
Implement recursive preorder and a three-frame continuation stack containing
node and next action. Validate every child index before indexing. On depth or
output exhaustion, return FULL with the exact emitted prefix; malformed child
returns INVALID with partial progress. Invalid initial parameters preserve the
output count; admitted traversal initializes it to zero. A cycle reaches FULL
at the depth bound; this is containment, not general cycle detection.

For the seven-node complete tree, independent preorder is1,2,4,5,3,6,7. Verify
all32 depth/capacity combinations against its bounded prefixes and expected
status, then empty root, invalid child99 and a self-cycle. Report actual cursor,
owner, push and deferred control sizes; separate shared storage, explicit frames,
and compiler `.su` stack estimates for O0/O2 under both compilers. Stack estimates
describe this build, not a portable worst-case hardware stack bound.

Deliver implementations, ownership/state diagrams, independent prefix oracle,
negative implementations, partial-progress traces, object/stack evidence and a
transfer design for truly concurrent cancellation. Hints: preserve terminal state;
check owner liveness before even empty completion; store the return continuation
explicitly. Rubric per leaf:0 wrong contract,1 nominal trace only,2 correct ordinary
invariants and discriminating tests,3 independent changed-task derivation. The
original delayed variants remain withheld for at least seven days. Source-bound
author runs are neither independent learner attempts nor absolute mastery proof.
