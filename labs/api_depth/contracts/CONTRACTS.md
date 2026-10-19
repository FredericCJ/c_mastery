# T18 criterion-specific implementation supplement

The existing `AX-PR-T18-01..04-T18.*` identifiers, tasks, project dependency
profiles, rubrics and minimum-seven-day delayed assessments remain authoritative.
This package supplies executable detail and worked teaching for all 30 ordinary
and 30 changed cases, without modifying historical acceptance records.

| Packet | Exact objectives | Dependencies |
|---|---|---|
| [Bounds](BOUNDS.md) | 01.a/c,02.a/b,07.a/b/c,09.a/b/c | Actual api_append and FBuffer |
| [Ownership](OWNERSHIP.md) | 01.b,02.c,03.a/b/c,04.b/c,05.a/b/c | Actual FView/FBuffer |
| [Representation](REPRESENTATION.md) | 06.a/b/c | Caller allocation contract |
| [Callbacks/evolution](ASYNC.md) | 04.a,08.a/b/c,10.a/b/c | Actual ApiRegistry/FAsync and ownership |

Use C17 and exact public headers. Hosted Linux experiments use GCC/Clang at O0,
O2 and ASan/UBSan; additional NDEBUG, real i386, generated-header and Cortex-M4
profiles are explicitly scoped in each packet. There is no implicit target libc,
libgcc/compiler-rt/libatomic allowance. Host Python/oracle/publishing tools are
not target dependencies. Executed hosted results, target builds, independent
learner attempts and physical observation are separate dimensions.

Inspect the seven records in
`research/v5/expansion/legacy_contracts/resources.json`: the exact T18 original
tasks; actual api_depth and foundation contracts/bodies; N1570 draft effective
type/assert passages; SQLite column lifetime and finalize error consumption;
Hanson's author-hosted Chapter3.1 interface sample. The source register records
precise locators and edition limits. N1570 is a C11 draft, not the published C17
standard. No C23 feature is needed for these implementations and no new ownership
or borrow-checking guarantee is attributed to C23. Published C17 and the original
Modern C reading remain explicit access gaps in `source_access.json`; do not
represent a landing page as a book passage inspection.

The library contracts choose additional semantics beyond ISO C: opaque types
control access to layout, not ownership; generations validate live control state,
not arbitrary dangling pointers; function types diagnose some mismatches, not
callback effects; runtime admission checks validate represented values under
caller lifetime/synchronization premises. External verification and programmer
discipline remain explicit. Compare these mechanisms with a language-level
affine/borrowed reference and give a counterexample to equivalent static safety.

Learner deliverables include prediction, implementation, contracts/invariants,
failure/ownership ledger, experiment commands/results, counterexamples, resource
analysis and changed-condition justification. Use public `.c` stubs as exercises;
they intentionally do not satisfy the tests. Test/oracle scaffolding and supplied
headers are inspectable. Withhold `assessor/api_depth/contracts` implementations,
solutions and transfer analyses. Use the existing CLI project initialization to
export learner material; never copy the assessor directory into it.

Author campaign (from repository root in the declared Linux environment):

```text
python3 tools/run_contracts_followthrough.py --module bounds --module ownership --module representation --module async --report reviews/v5/expansion/legacy_contracts/current_validation.json
python3 tools/check_contracts_followthrough.py --report reviews/v5/expansion/legacy_contracts/current_validation.json
```

The campaign must retain complete commands, exits, raw observations, source pins,
generated artifacts, positive and intended negative results. A source change
invalidates the prior receipt for that source. The linked support record
`research/v5/expansion/legacy_contracts/support.json` attaches only validated exact
criteria. Absence of a current passing support record means author validation is
pending, even if the specification is authored. Finite passes do not prove
absolute mastery. External review, independent/delayed learner performance and
physical measurements are never inferred from these runs.

All experiments concern original local programs, synthetic bytes/resource IDs,
and dedicated build artifacts. Do not call undefined C behavior a portable golden
output or equate a diagnostic/crash with demonstrated code execution. No external
targets or host security changes are authorized by this package.
