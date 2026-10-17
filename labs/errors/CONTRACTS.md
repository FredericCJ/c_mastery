# T65 error-model laboratory contract

The component uses ISO C17 on a hosted implementation with `uint32_t` and
`SIZE_MAX`. GCC and Clang compile with extensions disabled and warnings treated
as errors. `<setjmp.h>` is a deliberate hosted dependency for project 06. This
family is **not** a strict freestanding runtime; no target link-audit claim is
made. Test programs may use stdio and Python; target mechanisms may use only the
header's primitive C types, functions and the declared nonlocal-jump boundary.
Do not replace parsing, validation, arithmetic policy, composition or fraction
normalization with a library that implements that mechanism.

Every pointer denotes a live object of the declared type with sufficient extent.
The component can reject null and numeric violations; it cannot validate an
arbitrary dangling pointer. Output/control objects do not overlap input arrays,
other outputs or callback context objects. Callbacks may mutate their explicitly
owned context, but do not reenter the active operation or retain borrowed input.
No concurrent access is permitted without an external protocol. A returned
error is a value; the API does not automatically unwind or release resources.

`er_result` has the tag set ER_OK through ER_NOT_IMPLEMENTED. ER_OK selects
`as.value`; every other valid tag selects `as.offset`. `er_error` rejects ER_OK
and out-of-domain tags by returning ER_INVALID. An unknown input tag is rejected
before interpreting its union member. Copying a well-formed value is permitted;
tag/payload consistency remains an application invariant, not a C guarantee.
No wire layout, padding-byte comparison or ABI-size equivalence is promised.

The nine briefs specify their APIs and independent obligations. Every learner
submits D1 (reasoning and representation), D2 (implementation and regression
tests), D3 (oracle, mutants and exact experimental logs), and D4 (changed-condition
and delayed transfer). Each objective is graded separately: 0 missing/incorrect,
1 nominal behavior or unsupported explanation, 2 justified contract plus relevant
boundary and independent evidence, 3 fresh transfer with a counterexample and
explicit limits. A wrong failure-side effect, invalid lifetime, hidden dependency
or unsupported equivalence is a noncompensable failure. Overall rubric also
scores clarity, reasoning, correctness, verification, cost and transfer using the
existing shared exercise contract. Tests alone cannot award level 3.

Record compiler version, flags, platform/data model, source hashes and exact
commands. Debug, optimized and ASan/UBSan observations are distinct. Mutants must
compile, run, and fail a named semantic assertion: compiler errors, timeouts,
unrelated crashes or sanitizer aborts are not substitute mutation evidence.
The public anchors are examples; independent expected results and additional
cases are mandatory. A finite exhaustive reduced model proves only that model.

Measure `sizeof`, `_Alignof`, discriminator branches, callback calls, stack use,
code size and copies under equivalent correctness/failure contracts. Record zero
dynamic allocation from inspection and actual allocation observations separately.
T66's counting allocator is not yet an imported implementation. Revisit that
comparison after its producer is available. Do not claim all abstractions are
zero-cost from one optimized build or subtract away required validation.

Security experiments use only these original local teaching programs, synthetic
inputs and in-memory counters. They have no network, credentials, privilege or
external target. A wrong counter transition establishes a bounded integrity
violation; a crash or undefined behavior does not establish code execution.
Analyze dangerous jump destinations and invalid pointer lifetimes without
executing undefined behavior as an oracle. Mitigate by a valid boundary and
regressions, then state what remains caller discipline.

Independent transfer variants are withheld under `assessor/errors/analyses`.
Attempt the named delayed variant at least seven days after assisted study and
record assistance honestly. This schedule is an engineering default, not a
measured optimum. Authorship, executable code, actual campaign execution,
non-author review, learner assessment and physical evidence remain separate.
