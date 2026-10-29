# Compact foundation: a bounded record tool

In `contextual-v2` this context is taught through the authored episodes
EP-diagnostic-loop, EP-build-provenance, EP-checked-extents,
EP-declarations-lifetime, EP-borrowed-cursor, EP-algorithmic-vocabulary and
EP-function-contracts. Each card names its episode, step, motivating limitation
and transition. Units of this context outside those episodes remain unauthored
project blocks.

Begin with the C diagnostic, compilation workflow, live arrays and checked integer
arithmetic. Then use the algorithmic vocabulary in U-T01-01-01/02. The prerequisite
graph enforces this ordering without requiring the entire language catalog first.

A state is the information the next operation can observe. For a caller-owned
array of capacity C and length n, a representation invariant is 0 ≤ n ≤ C and
exactly the first n elements are initialized. The abstract sequence does not
contain the unused tail. A precondition describes an admitted caller state;
a postcondition describes each possible result. A failed bounded append must
leave the old sequence unchanged. These claims differ from the C obligations
that the pointer is live, arithmetic does not overflow, and accesses remain
inside the actual array object.

For linear search, before iteration i all positions below i have been examined
and do not match. The loop checks i < n before reading. The variant n-i decreases
on every unsuccessful iteration, proving termination. The unsuccessful case
uses n comparisons; an early successful search uses i+1. This is an operation
count, not a prediction of cycles. The size parameter is the number of admitted
records, not the largest integer value they contain.

For bounded insertion, reject n == C before moving anything. Move the suffix
backward so each source remains available until copied. If the contract admits
aliasing between the input record and the backing array, retain the value before
movement or explicitly disallow that alias. A correct mathematical shift can
still be an invalid C program if a size computation wraps or an object expires.

For stable insertion sort, keep a tagged record aside, move only records whose
keys are strictly greater, then insert. The prefix is sorted and preserves the
relative order of equal keys. The outer-loop invariant plus the inner-loop
invariant establishes the result. Count comparisons, moves and scratch storage
separately. Worst-case quadratic work does not mean every workload has that
cost. Explain why comparing signed keys by subtraction can violate the contract.

Before consuming these mechanisms, submit an invariant, termination argument,
normal/boundary/failure prediction and independent tagged-permutation check.
Later vector growth requires a separate amortized argument: charge each copied
item to capacity growth or use a potential function. A working vector alone does
not establish amortized constant append cost. Allocation failure and a maximum
capacity invalidate an unqualified infinite-growth proof.

These examples feed the record, streaming and deadline contexts. Reconstruct the
argument at those boundaries; do not repeat the introductory implementation merely
with different constants. Every later encounter must add a stated contract change.
