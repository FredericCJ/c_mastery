# Solver and fuzz obligations

Build the learner core with cbmc_add.c and run CBMC with pointer, bounds, signed
overflow and unwinding assertions. Record version, exact command and every
property; no installed solver means no proof result. Repeat on the overflow
mutant and on an intentionally vacuous assumption to distinguish the claims.
Add loop invariants/unwinding bounds for lower_bound and the ring transition.

Clang invocation: `clang -std=c17 -Iinclude src/core.c targets/fuzz_frame.c
-fsanitize=fuzzer,address,undefined -g -O1 -o fuzz-frame`. Use a temporary corpus
with empty, valid, truncated and checksum-corrupt frames; retain the seed and
bounded run count. The fuzz harness checks semantics, not just crashes. Prove
the oracle's length/checksum conditions independently and test it with mutants.
Fuzz process state must reset between inputs. Minimize a found case while
retaining the same failure predicate, then add a deterministic regression.
