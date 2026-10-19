# Function declarations, separate objects and typed callbacks

## Scope and prerequisites

This required successor implements exactly `AX-PR-T05-01-T05.02.a`, `.b`, `.c`
and their `-DELAYED` variants. Families A, B, C, D, F, I and J assess the
prototype, promotion, compatibility and callback mechanisms. Retain the parent
PR-T05-01 notebook and fixture predictions; prerequisite learner artifacts are
the learner's T02 function/type verdicts, T04 checked-addition argument and T05.01
source-authority ledger. No assessor implementation is supplied to learners.
The parent project and other T05.01/.03 tasks remain separately required.

The active implementation is ISO C17, hosted native profile A. The old objects
are deliberately compiled in C90 mode, and isolated C23 probes examine one
changed rule. These comparisons do not replace the baseline. No library may
implement checked addition, comparison or the insertion sort. Host `qsort` is
permitted only to check integration of the correctly typed callback. No dynamic
allocation, system privileges, external targets or hardware are involved.

## Representation and operations

`LpRecord` contains an `int key` and an `unsigned serial`; serial identifies
the original occurrence and must travel with its key. `LpOrder.reverse` is
exactly 0 or 1. Every pointer passed to a comparator refers to a live aligned
actual `LpRecord`, not an arbitrary byte encoding. The context comparator
borrows a live `LpOrder` for the call. Callers serialize mutation, keep records
and context alive and unchanged during a comparison, and do not reenter the
sort. The sort copies the order before moving records, retaining no pointer.

Implement `lp_sum(a,b,out)` for the entire implementation's `int` range.
Return `LP_INVALID` for null output, `LP_RANGE` if the mathematical sum is not
representable, otherwise `LP_OK` and publish that sum. Rejection leaves the
output unchanged; check before adding. Derive why the two comparison bounds
themselves cannot overflow. Inputs are values; output may alias caller input
storage because the values were already passed.

`lp_compare` returns -1, 0 or 1 by comparisons, without subtracting keys.
`lp_adapter` has exactly `int(const void *,const void *)`, converts object
pointers to actual record pointers and invokes the typed operation. Conversion
of an incompatible function pointer is not an adapter. `lp_context_adapter`
returns the ascending result for reverse0 and its negation for reverse1; the
bounded three-result representation makes negation safe.

`lp_sort(records,n,order)` admits n0..16. Null records are allowed only for n0;
order must be nonnull with reverse0/1. Invalid pointers/order return INVALID;
n>16 returns RANGE after those checks. Every refusal preserves all records.
Success orders keys in the requested direction and preserves serial order
among equal keys. Use stable insertion sort, O(n squared) comparisons/moves,
O(1) extra records, no recursion. Empty and singleton cases must be defined.

## Six exact tasks and experiments

1. For T05.02.a, predict the supplied `old_sum.c`, then modernize it in
   `migration.c` with the shared header. Explain explicit parameter/return
   types, whole-int-range refusal and unchanged output. The original fixture
   uses identifier-list syntax but explicitly declares its return/parameter
   types; implicit return type is a separate compile-only negative control.
   Compare 65,618 defined old/new calls. For the delayed task, compile producer,
   consumer and driver as separate objects, inspect symbols/disassembly, and
   show the shared-header conflicting-return-type control is diagnosed before
   linking. A successful linker match alone does not check C types.
2. For T05.02.b, write a per-mode prediction for `int f();`, `int f(void)` and
   `int f(double)`. Execute only the admitted float1.25f-to-double call. The
   modern double prototype and the C17 unprototyped call are positive controls;
   the void call with an argument is a diagnostic control. For the delayed
   task, contrast the actual C23 empty-list rejection with the double-prototype
   positive. Report syntax/type checking separately from linking and execution.
3. For T05.02.c, replace the compile-only incompatible callback cast with the
   actual object-pointer adapter. Test minimum, zero and maximum keys, both
   operand directions and duplicates. For its delayed task, add the explicit
   reverse context and prove its lifetime and no-retention contract. Exercise
   the first-principles sort and the hosted qsort adapter separately. Stability
   is required of this insertion sort, not promised by qsort.

The source comparison uses [N1570](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf)
6.5.2.2p1-10 and 6.7.6.3p1-15: empty nondefinition declarations supply no
parameter information; calls without a prototype promote float to double.
Compatibility and call validity still constrain types and counts.
[N3096](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3096.pdf)
6.7.6.3p1-14 instead makes an omitted parameter list equivalent to void and
provides a prototype. Both documents are drafts. Exact published C90, C17 and
C23 text remains an access gap; neither compiler mode nor these passages prove
whole-edition conformance or universal binary compatibility.

The recorded Clang policy also rejects the old unprototyped declaration under
`-pedantic-errors`. Keep that rejection. Only the isolated old C90 objects and
C17 unprototyped comparison are retried with `-pedantic` to retain diagnostics
and observe the admitted call. The complete modern implementation continues
to use C17 and strict warnings as errors. Explain this policy difference;
do not erase the diagnostic or call the warning-free baseline an old-style API.

## Verification, evidence and costs

Build the unfinished public `migration.c` with the adjacent CMake file; its
intended tests compile and fail semantically. Build a completed learner source
in its place. An explicit `CMASTERY_ASSESSOR_ROOT` is available only to assessor
campaigns and must never be an implicit learner fallback. The CTest anchors
are `portability.sum`, `.adapter`, `.reverse`, `.stable`. Four compiled controls
prematurely publish on refusal, compare unsigned keys, ignore reverse, and
move equal keys out of order. Each must fail the intended `LP_CHECK` assertion,
not a compiler error or sanitizer failure.

The independent Python oracle enumerates all 39,062 five-key sequences of
length0..6 in both directions and 65,618 checked sums. It derives stable order
with a separate algorithm and exact Python integers. Include n16, n17, null,
bad reverse, extrema and unchanged-refusal tests outside that enumeration.
Run GCC and Clang debug, release and address/undefined-sanitizer profiles.
Bind actual source, include closure, compiler, library, flags, logs and artifacts.
Record sizeof/stack output, object undefined symbols, assembly and code size;
distinguish harness/library costs from the O(1) core. No universal performance
ratio or target execution is inferred from native artifacts.

The companion must compile all four old/modern objects separately at O0 and
O3 with both compilers, link/run the positive calls, and retain intended
diagnostics for shared-header mismatch, void arguments, C23 empty list and
implicit return type. The incompatible function-pointer specimen is compiled
only, never invoked; undefined execution is not an oracle.

## Deliverables, rubric and hints

D1 is the edition/type/promotion table with exact passage labels and missing
assumptions. D2 is the checked core, typed adapters and stable context sort.
D3 contains independent expected cases, actual commands/diagnostics, failing
controls and a separation of ISO semantics, compiler policy and observed ABI.
D4 is the separately compiled migration, C23 changed-rule and borrowed-context
delayed work, attempted again after at least seven days without assessor code.

Score each original criterion and its delayed task separately: 0 for a wrong
contract or incompatible executed call; 1 for reproducing output without a
type/lifetime/range argument; 2 for correct implementation, explicit independent
counterexample and intended diagnostic/failure evidence; 3 for independently
deriving the changed conditions and defending all limits. Passing the finite
campaign is author evidence, not learner mastery or a proof over all programs.

Hints: first write the function type visible at each call; next apply only
that mode's promotions/conversions; then compare with the actual definition.
For callbacks, adapt object pointers inside a correctly typed function. For
stable reverse sorting, equal keys must still stop movement. The private
technical analysis is published separately from this learner package.

## Status and limits

Authored executable support; the source-bound report records which commands
actually ran. No physical validation, external human review or independent
learner assessment is claimed. Future compiler versions, other int widths,
different calling conventions and differently laid-out records require new
arguments and evidence. The explicit native oracle uses 32-bit int; the core
contract itself uses the actual implementation's INT_MIN/INT_MAX.
