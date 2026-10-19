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

## Actual solver continuation: SV1–SV3

Retain PR-T61-01–03 and the B06 shared contract. Prerequisites are the learner
checked-add, sorted lower-bound and ring implementations and their invariants.
Use CBMC6.11.0 with the exact target bit width, pointer/bounds/overflow checks,
unwinding assertions and command lines in `solver_validation.json`. A solver is
a host verification dependency, never a freestanding runtime dependency.

Implement the harness from `cbmc_structures.c`: arrays have0..8 sorted unsigned
elements, queries are unrestricted; assert the returned partition, not only one
index fixture. Ring capacity is1..8, valid initial head/size, and eight nondeterministic
push/pop choices. Assert valid state, exact accepted transitions and unchanged
refusal. D1 is each mathematical invariant and assumption justification; D2 is
the harness and every property/command; D3 is a reachable witness assertion,
contradictory-assumption vacuity control, insufficient-unwind rejection and three
semantic mutants. Run `python3 tools/run_b06_solver.py`. Require actual proof
success for the reference bounds, mutant counterexamples, a reachable false
assertion failure, a vacuous success explicitly rejected as evidence, and the
unwinding-assertion failure at bound2. A successful command is not enough without
classifying why it succeeded.

SV1 maps T61.01.a/b/c to checked arithmetic/unchanged failure; SV2 maps
T61.02.a/b/c to sorted partition; SV3 maps T61.03.a/b/c to ring transitions.
Level0 calls bounded/vacuous success an unconditional proof; level1 only repeats
the tool invocation; level2 supplies the exact bounded argument and all controls;
level3 changes comparator or operation trace after seven days and re-derives
assumptions/unwinding. Hints: assert the postcondition independently; then ask
whether any path reaches it; finally count loop exits as well as loop bodies.
The separate analysis is `assessor/verification_fuzzing/targets/ANALYSIS.md`.
Changed conditions require multiplication/mixed widths, descending opaque
comparators, borrowed payload destruction and two-actor memory-order reasoning.
Those are not established by these scalar bounded proofs. No external target,
hardware, independent learner assessment or unbounded correctness is claimed.

Inspected reference bindings: R-B06-FOLLOWUP-CBMC-UNWIND, R-B06-FOLLOWUP-CBMC-PROPS.
Exact passages and inspection limits are in
`research/v5/expansion/b06/followup_resources.json`; source titles alone are not
inspection evidence. Retain those locator bounds when changing assumptions.

Required implemented changed-condition continuations are specified in [TRANSFER.md](TRANSFER.md).
