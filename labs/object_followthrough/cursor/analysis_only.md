# Original analysis-only pointer counterexamples

For a live `int matrix[2][3]`, analyze `matrix[0] + 5` without evaluating it. The mathematical row-major index5 names a valid location through `matrix[1][2]`, but a pointer derived from the first row has that row's bounds. Casting `matrix` to `int **` does not create the row pointer objects which that interface would read.

For `int *p`, analyze `memset(&p,0,sizeof p)` separately from `p=0` and `p=(void*)0`. Zero bytes are not a portable null-pointer construction. The variable expression `zero` in negative_zero.c is not a null pointer constant merely because its stored value is0.

A live n-element array permits formation of base+n, equality as an end marker and subtraction only when the result is representable in ptrdiff_t. It does not permit dereferencing that end or forming base-1. Relational pointer ordering between unrelated fragmented buffers cannot implement logical stream order.

Function-pointer conversion to another function-pointer type and back has a separate ISO rule; invoking the intermediate incompatible type is not admitted. Object-pointer/function-pointer conversion used by a named dynamic-loader interface is a platform boundary, not an ISO round-trip theorem.
