# Explicit SIMD and arithmetic-model experiments

Implement ns_vector_add in learner source; vector_test.c compares every length
0..67 against the scalar contract using reproducible inputs. Compile extension
code separately from strict C17 core. The assessor vector implementation is
separate. Check integer lane width, compare-mask representation, alignment and
tail proof from the registered GCC passage. Inspect assembly to establish which
vector instructions actually occur; a vector source type alone is insufficient.

Compile compensated reduction with ordinary flags and with -ffast-math. Retain
the expected failure as a changed arithmetic contract, not an optimization win.
Measure only variants meeting the same declared correctness contract; pin CPU,
compiler, frequency policy, alignment, data size, repetitions and warm/cold state.
Report distributions, code/stack size and traffic model; no universal speed claim.
For fenv, subnormals and FTZ record actual compiler/platform support. No host
security or persistent system setting changes are authorized by this exercise.


## Required P6 dispatch and measurement composition

Reuse the actual learner artifact contracts in
`labs/checkpoint_depth/include/checkpoint_numeric.h` and `checkpoint_vector.h`
and the numerical section of `labs/checkpoint_depth/CONTRACTS.md`. Implement the
same staged FIR with scalar, compiler-vectorized and explicit AVX backends;
choose using actual CPU OSXSAVE/AVX and guarded XGETBV XMM/YMM state checks.
Run `tools/run_checkpoint_numeric.py`, retain its exact source-pinned report,
disassembly and raw timing samples, and compare equal finite-domain, overlap,
rounding, failure and history contracts. This continuation maps T64.03.a/b/c,
T64.07.a/b/c and T64.08.a/b/c to actual tail/admission, dispatch/fallback and
measurement evidence. T64-N1 requires every length0..256, double alignment offset,
scalar/vector equality and unsupported-CPU fallback. T64-N2 requires repeated
samples, retained work in disassembly and a cost explanation including staging.
Level0 reports speedup without equivalence; level1 ordinary timing only; level2
satisfies these concrete obligations; level3 independently changes arithmetic
or target after seven days and revisits the contract. Hints: derive the scalar
expression order; isolate target-attributed code; guard XGETBV before calling it.
The author report at
`reviews/v5/expansion/legacy_completion/checkpoint_numeric.json` is reusable
bounded evidence only while its source pins match. A QEMU no-AVX result is not
physical no-AVX hardware, and timings establish no universal performance claim.
Q3.12 policy, directed rounding, reduction/blocked-matrix variants and FTZ analysis
remain separate required work; this FIR does not silently cover them.

The Q3.12, fenv, tree, matrix, exact SSE2 and parallel continuations are implemented and specified in [EXTENSIONS.md](EXTENSIONS.md), with separate source-bound reports.
