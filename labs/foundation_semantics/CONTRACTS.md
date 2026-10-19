# Bounds, initialization and sequencing diagnostic

This successor implements the three ordinary PR-T01-02 tasks and their three
unchanged seven-day variants. Keep the original AX identifiers and the shared
legacy-assessment rubric. Historical fixtures, project hours and acceptance
records remain unchanged. The public files contain typed interfaces, compiling
learner stubs and tests of defined repairs. The private assessor package contains
analysis-only faulty examples and the technical solutions.

## Premises, dependencies and workflow

Use hosted ISO C17, profile A, Linux with GCC or Clang. Standard runtime startup
and libc belong to the harness; no POSIX call or allocation is required by this
component. Record exact compiler path/version, command arguments, dependencies
and source identities. All C values use the implementation's `int` limits, not
an assumed width. Pointer representation and `sizeof(int)` are measured or left
symbolic. C17 does not make a pointer-to-row interchangeable with `int **`.

Prerequisites are the original diagnostic notebook, the PR-T01-01 before/after
state discipline and basic declaration syntax. Submit your own `learner.c` and
criterion-indexed predictions, proof/diagnosis notebook and raw commands. Run:

```
python labs/foundation_semantics/run.py --build build/semantics
```

The runner builds only the repaired sources and public tests. Do not use a warning
suppression or a cast to make a type-invalid argument appear repaired. Do not run
an undefined example to discover its “expected” value. Translation diagnostics,
semantic classifications and observed defined executions are separate evidence.

## T01.03.a: a pointer carries fewer facts than its contract

Begin with a real three-element array. Classify indices -1,0,2,3,4 in signed
arithmetic *before* converting to an unsigned count. An element requires
`0 <= i < n`; one-past formation permits `i == n` but not dereference. A negative
offset or offset greater than n must be rejected before forming the pointer.
`fs_index` performs this metadata classification without forming any pointer.
Its zero-length one-past result does not authorize `NULL+0` or prove that an
object exists; a logical empty view can use a real object or make no pointer
operation at all. Actual storage and lifetime are caller preconditions.

Implement both `fs_sum(const int *,size_t,int *)` and
`fs_row(const int (*)[3],int *)`. The first borrows a live n-element array and
permits null input only for n=0. The second borrows one actual three-element row,
requires a nonnull pointer and obtains its extent from the pointed-to type.
Both require a live, disjoint output object, check each signed addition before
evaluation, and publish only after success. Empty sum publishes zero. The type
does not validate nullness, lifetime or actual allocation extent.

For the changed condition implement `fs_matrix(const int (*)[3],size_t,int *)`.
The count is the number of actual row objects, each containing three ints.
Traverse rows then columns, checking every intermediate total. Zero rows makes
no input access and publishes zero. For a real `int matrix[2][3]`, `matrix+1`
advances by one row, whose size is `3*sizeof(int)`; it does not advance by one
int or one pointer. A table of pointers is a different representation. Explain
why a constraint diagnostic is appropriate for passing it to this interface.

The success invariant is a representable total of exactly the preceding elements
in row-major traversal, with output unchanged. At each nested-loop transition
identify the remaining row/column work; runtime is linear in visited elements,
with constant auxiliary state and no allocation. Rows need not be flattened
through a pointer derived from the first inner array. All ownership is borrowed
for the call; nothing is retained. Tested overflow refusal is a return status,
not execution of undefined overflow.

## T01.03.b: a path obligation is not an observed default value

Create two analysis-only examples: a loop that reads at `i <= n`, and an automatic
local whose value is used without initialization. State the first invalid access
or value computation and the exact premises; then implement the corrected bounds
and assignment. A compiler can emit no warning because the language does not
require a diagnostic for every undefined execution. Record warning configuration
as part of the observation; silence is not a proof.

For transfer, put initialization behind a branch. Use the exact two-result policy
`branch=1 -> 7`, `branch=0 -> 11`; any other branch value or null result pointer
returns invalid without publication. Implement `fs_branch`. Before coding, draw
the control-flow paths and associate every result read with its dominating
assignment. A default-zero assignment can prevent an uninitialized read while
still implementing the wrong policy. Test both branches, invalid admission and
preserved output, and retain a wrong-but-defined candidate to distinguish this.

## T01.03.c: syntax grouping does not establish sequencing

Make an event graph containing each scalar read/write and the guaranteed
sequenced-before edges. Classify `i++ + i++`, `0 && (1/0)` and two calls whose
bodies update independent objects. No portable output is assigned to the first.
For the second, explain why the right operand is not evaluated. For the calls,
separate unspecified ordering from conflicting unsequenced scalar modifications.

Implement `fs_calls`: start x=1,y=2, call one function returning `++x` and another
returning `y*=2`, sum their results, and record their body order as12 or21. The
functions may append to one observation field because their body executions are
indeterminately sequenced. The final x,y,sum are fixed by this contract even
though the observed order need not be. Tests accept both permitted orders; a
compiler observing one order does not prove that the other is forbidden.

For transfer, compare a comma *operator* with comma-separated arguments.
`fs_comma` evaluates `int i=1; int v=(i++,i++);`, then publishes v and i through
two live disjoint outputs. Null or equal output pointers reject without change.
Analyze `sum(i++,i++)` separately; do not execute it. A separator comma is not a
sequencing operator. For a general repair choose and state the intended order,
split the operations into full expressions and recheck integer bounds; changing
the order of a formerly undefined expression is choosing a contract, not
recovering an original portable result.

## Verification, assessment and hints

The public tests execute valid element/one-past operations, typed row traversal,
all117 metadata combinations over lengths0..8/indices-2..10, arithmetic refusal,
both initialization paths and defined sequencing. Their coverage is finite and
does not validate arbitrary pointer provenance. Independent evidence must include
one new boundary and one causal counterexample for each objective.

For .a require the signed admission table, legal/illegal formation distinction,
two correct declarations and row stride derivation. For .b require the first
invalid operation, path proof and wrong-default counterexample. For .c require
the event graph, correct category and the operator/separator distinction. Level0
is a wrong or missing argument; level1 repeats nominal observations; level2
explains and verifies the exact mechanism; level3 independently reconstructs the
changed-condition argument after the recorded delay. A compiled program alone
does not earn the reasoning or delayed-assessment score.

Hints: first mark the actual array object and its lifetime; then mark accesses
versus mere pointer formation. For initialization, enumerate the path where the
assignment is absent. For sequencing, circle the scalar object of each event and
draw only edges supplied by language rules, not by visual left-to-right reading.

Sources are the exact N1570 C11 draft passages in the successor source register:
array conversions/arithmetic, automatic uninitialized lvalue conversion, function
execution sequencing, logical AND, comma operator and adjusted array parameters.
The active implementation remains C17; N1570 is not relabeled published C17.
No later-edition extension is assumed. This local laboratory uses original
synthetic examples, never external targets. Faulty examples stay compile-only;
no crash or diagnostic is treated as an exploitability claim. Actual author
execution, non-author review, learner attempts, seven-day evidence and physical
observations remain distinct.
