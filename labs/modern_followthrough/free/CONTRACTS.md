# Free effects at the resource boundary

This companion completes the explicit identity-bind portion of
`X-T72-010-DELAYED`. Use the unchanged public `higher_order.h` and its complete
`labs/higher_order/CONTRACTS.md` contract. Implement the original public
`labs/higher_order/src/free.c`; this companion adds no substitute implementation.
The assessor implementation is selected explicitly at compilation. All inputs
are original local C17 teaching objects. No external service or target is used.

Build one Pure node, followed by 63 Choose nodes whose two children are the
previous root. There are 64 physical nodes and 2^63 answer paths. Before running
the probe, derive the summary for terminal values -1, 0 and 1. The sum is signed
64-bit: the first value fits exactly, the second is zero, and the third cannot
be represented. Distinguish that arithmetic boundary from unsigned path count.

An identity constructor callback calls `ho_pure` with the received value. Bind
the full arena through this callback, rather than merely calling a constructor
directly. Predict the callback count, status, consumed allocation attempts,
unchanged output, arena mark and continued validity of the old root. Compare
meaning with resource behavior: running the old root succeeds while this
reconstructing identity bind fails. This does not refute the admitted monad law.

Next use Emit(7, Choose(Pure(5), Pure(5))) with a shared Pure node. Enumerate each
of the four allocation failure cuts in the reconstructing bind. Check that the
old three-node prefix is unchanged, the old root still runs, the output root
remains its sentinel, and the attempted allocations and callback counts remain
observable. Test a callback that allocates then returns failure, and one that
allocates then returns an invalid root. Their external call counters do not roll
back. Newly allocated roots may not escape these failed binds.

`probe.c` emits records in addition to checking publication and old-root
invariants. The separate assessor oracle derives their expected integer results
without calling C. The runner compiles GCC/Clang at O0, O2 and ASan/UBSan, rejects
the original unfinished learner source, and checks three staged provider mutants
for the specific new boundary. A compilation failure is not a semantic rejection.

Example ordinary compilation from the repository root:

```text
gcc -std=c17 -Wall -Wextra -Wpedantic -Werror -O2 -Ilabs/higher_order/include labs/modern_followthrough/free/probe.c assessor/higher_order/free.c -o build/free-followthrough
build/free-followthrough
```

For the delayed assessment, retain the original seven-day requirement and an
assistance log. Change compiler/optimization as the exercise requests; use the
separate higher-order cost report for actual frames, calls and compilation
observations. The author campaign does not supply an independent learner attempt,
an external review, a proof over all callbacks, or a physical target observation.
