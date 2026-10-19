# Exact PR-T01-02 assessor solutions

Keep this directory private. The `analysis_only.c` translation unit is never
linked or executed. Successful translation or a missing warning is an observation
about a tool, not an expected value for those undefined paths. The two `wrong_*.c`
files instead violate pointer argument constraints and must diagnose under the
declared strict compiler invocation. Distinguish these negative categories.

## t0103a

For an actual three-element array at its first element, -1 and4 cannot be formed
by the corresponding pointer addition, 0 and2 are accessible elements, and3 can
be formed but not dereferenced. A signed negative converted first to size_t is
not thereby an admissible nonnegative index. The implementation compares signed
negativity first, then uses uintmax_t only for nonnegative comparisons; the latter
can represent every unsigned integer type's values. The 117-case metadata oracle
does not create a zero-element C array. At length0 its ONE_PAST result is a
classification of a logical range, not permission to do arithmetic on null.

`const int *` plus length borrows elements; `const int (*)[3]` points to a row.
`int *a[3]` is an array of pointers, not a row pointer. For two rows use
`const int (*)[3]`, not `const int **`. The actual test matrix has rows1,2,3 and
10,20,30, yielding66; repeatedly traversing row0 yields12 and is the defined
wrong-stride mutant. Passing a plain int pointer or pointer table is rejected in
the negative translation specimens; a cast changes diagnostics, not layout.
The correct nested traversal never computes a first-row pointer beyond its own
inner-array boundary. Outer `rows+1` has the row type and advances one row.

Every addition is preceded by a representability check; failure leaves output91.
Maximum cancellation across a row boundary is rejected too. Empty sum has a
defined zero result and no input dereference; `fs_row` still needs an actual row.
Space is constant, work3*row_count (arithmetic feasibility and memory extent
remain preconditions). Alternative traversal is valid only with an equally clear
object and failure argument. The one-past mutant changes classification only,
so its test rejection does not require executing an out-of-bounds access.

## t0103b

For `fs_off_by_one` with a real three-element array, n=3 and small positive
elements, iterations0,1,2 are defined and the read at3 is the first invalid
operation. Merely computing the one-past pointer is not the defect. For n=0 with
null input the first read is invalid immediately; the repair uses i<n and avoids
even null arithmetic. The cumulative addition also needs its separate range
precondition or checked implementation; fixing the loop bound alone is not a
universal sum repair.

The no-address-taken automatic locals in `fs_uninitialized` and the false path
of `fs_partial_init` satisfy the exact N1570 6.3.2.1p2 premise for undefined
uninitialized lvalue conversion. Do not generalize this explanation to every
possible character-byte/indeterminate representation question. The branch proof
has an assignment on each admitted path: true stores7, false stores11. Invalid
admission performs no publication. The mutant stores0 on false: it is fully
defined but semantically wrong, so ASan success alone cannot repair the policy.
Both branches must be tested and explained, not inferred from a nominal true run.
Observed warning silence with default GCC options is recorded; turning warnings
on can add diagnostics without changing the source's semantic classification.

## t0103c

In `i++ + i++`, both updates to i are unsequenced relative to each other; each
postincrement also reads i. The plus operator supplies no ordering edge, so the
execution is undefined. There is no permitted portable output set to enumerate.
For `0 && (1/0)`, left evaluation determines false and the right operand is not
evaluated; result0 is defined. Compile-only warnings about unreachable expressions
would not change that rule.

For `first()+second()`, each function body's execution is indeterminately
sequenced relative to the other. Starting x1,y2 yields x2,y4,sum6 for either
order, with observation12 or21. The shared observation field remains valid
because its updates occur inside noninterleaved function executions. Calling
these effects unsequenced merely because their call expressions are operands of
plus conflates body execution with argument evaluation. Shared state alone is
not a sufficient UB argument.

The comma operator in `(i++,i++)` sequences the first update before the second
operand; starting1 gives v2 and final i3. The commas in `sum(i++,i++)` separate
arguments and provide no such edge. Its two modifications remain unsequenced
before entry to sum, so that example is analysis-only. A repair can assign
`first=i; ++i; second=i; ++i; result=sum(first,second);` with checked limits and
a chosen order. Its correctness belongs to the new explicit contract.

## Delayed administration and discrimination

Issue the exact three unchanged delayed AX tasks only after a recorded study event
and at least seven days, without this directory or the prior solution source.
For .a supply a fresh2x3 matrix and a pointer table; require the full parameter
declaration, stride and object-boundary proof before running either candidate.
For .b provide a branch that assigns only on true and require the11-policy false
path; reject a “repair” that merely initializes to zero. For .c give both comma
syntaxes and demand separate event graphs. A changed numeric constant is not the
transfer mechanism. Retain each prediction, implementation/analysis, minimized
counterexample, exact command record and assessor judgment. Author reference
execution is not that independent delayed evidence.

Five compiled wrong-but-defined implementations discriminate one-past admission,
row selection, false-path policy, comma result and call-sum result. Negative
translation cases assess incompatible argument types instead. Neither category
proves learner understanding; require the row-specific reasoning at rubric2/3.
