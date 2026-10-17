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
