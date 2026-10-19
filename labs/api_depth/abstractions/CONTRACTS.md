# Required T19 abstraction implementation extension

This successor implements the original T19 criterion tasks without replacing
baseline identities, hours, historical receipts or T19.08 stream work. The active
edition is C17. C11/C23 generic-selection comparisons are explicitly limited
to the named compiler probes and inspected DR481/source passages. A successful
native build is not a freestanding dependency audit or proof of language safety.

| Original objectives | Required teaching and learner artifacts | Ordinary and changed assessor path |
|---|---|---|
| T19.01–02 | [Session](SESSION.md): state, construction, opacity, exact/versioned tables | session_tests.c; parent PR-T19-01 AX criteria |
| T19.03 | [Registry](REGISTRY.md): context ownership, mutation policy, terminal destruction | registry_tests.c; PR-T19-02 |
| T19.04–05 | [Generic](GENERIC.md): typed/generated/erased forms, owning sums and errors | generic_tests.c; PR-T19-03 |
| T19.06 | [Reducer](REDUCER.md): effects, retry identity, hot strategies | reducer_tests.c; PR-T19-03 |
| T19.07 | [Traversal](TRAVERSAL.md): borrowed views, deferred visits, explicit continuations | traversal_tests.c; PR-T19-03 |
| T19.08 | Existing [stream contracts](../stream/CONTRACTS.md) and [variants](../stream/VARIANTS.md) | Existing stream_tests/variants_test; PR-T19-04 support remains separate |
| T19.09 | [Handles](HANDLES.md): finite identity, strong/weak lifetime, payload access | handles_tests.c and private concurrent transfer tests; PR-T19-04 |
| T19.10 | [Dispatch](DISPATCH.md): actual provider composition, code/cost/maintenance evidence | dispatch_tests.c and private measurement/third-backend variants; PR-T19-04 |

Headers define supplied representations and operations. Public `.c` files are
intentionally unimplemented learner mechanisms; never replace them with private
answers. Import the learner's previously validated foundation, API registry,
T18 ownership and T19 stream components. Author campaigns instead identify and
pin the corresponding assessor reference sources; they do not call them learner
artifacts. Execute positive, malformed, boundary and compiled semantic-negative
checks before measuring. Preserve raw commands, flags, outputs and source hashes.

For each original AX criterion submit prediction, implementation or rigorous
reasoning, invariant/ownership/failure proof, independent oracle/counterexample,
cost and portability account, and changed-condition design. The named packets
give concrete inputs and acceptance rules. The original four project dossiers
and per-criterion 0–3 rubrics remain binding. Level3 requires independent transfer,
not copying an answer or accumulating passing tests. Delayed variants retain
their minimum seven days and require a different learner attempt; author execution
of their reference implementation is only preparation for that assessment.

Compiler function types and incomplete structs diagnose some misuse; they do not
enforce linear ownership, nonescape, alias validity, buffer extents, protocol order,
callback quiescence or data-race freedom. Runtime checks operate only after valid
object/lifetime premises. Generated tables prevent some omissions, not incorrect
semantics. Match every language-native abstraction comparison with a counterexample
to claims that C supplies equivalent enforcement.

The source register is `research/v5/expansion/legacy_abstractions/resources.json`.
Actual K&R5.11 is a historical function-pointer strategy example; its cast of a
string comparator to an incompatible function signature and its pointer-array
cast are not portable C17 adapters. Use an exactly typed wrapper and actual typed
storage. Inspected N1570 is the2011 draft, not the published C17 standard. The
public Modern C third edition is the October2024 C23 text with its own pagination.
Exact remaining source limitations stay in the source-access ledger.

Author validation uses `tools/run_abstractions_followthrough.py`, its strict
receipt checker and actual CLI exports. It preserves six distinct dimensions:
teaching, implementation, author execution, external review, independent/delayed
learner performance and physical observation. No final three are inferred from
the first three. Original local defective specimens use synthetic data only;
the payload-race diagnostic is not a code-execution or privilege-escalation claim.
