# PR-T43-10: required assessment supplement

Read-only cgroup scope and conditional control - explicit objective assessment

This supplements the bound project; historical hours are descriptive, not a ceiling. Use the shared assessment contract and the existing learner artifact. Assessor analyses are distributed separately.

## Environment and prerequisites

| Assumption | Contract |
|---|---|
| language | ISO C17 active baseline; C11 draft support must retain its edition limits |
| target | Parent target/profile: hosted Linux or original disposable QEMU kernel/Cortex-M teaching firmware; physical routes require their own evidence |
| profile | A |
| dependencies | Parent boundary applies. C components use permitted local primitives; hosted verification and packaging tools are separate. Audit generated helpers and link closure before any strict freestanding claim. |
| toolchain | Record actual GCC/Clang, CMake, Python and Git versions when used., NDEBUG on/off and O0/O2 for relevant semantic/diagnostic comparisons. |

Supplied artifacts:

- curriculum/modules/T43.md
- curriculum/projects/PR-T43-10.md
- fixtures/performance/PR-T43-10.json
- fixtures/performance/PR-T43-01.json
- fixtures/performance/PR-T43-02.json
- fixtures/performance/PR-T43-03.json
- fixtures/performance/PR-T43-04.json
- fixtures/performance/PR-T43-05.json
- fixtures/performance/PR-T43-06.json
- fixtures/performance/PR-T43-07.json
- fixtures/performance/PR-T43-08.json
- fixtures/performance/PR-T43-09.json
- fixtures/performance/PR-T43-11.json
- fixtures/performance/PR-T43-12.json
- labs/algorithms/include/algo.h
- labs/algorithms/src/algo.c
- labs/algorithms/src/x86_64.S
- labs/containers_depth/containers.h
- labs/containers_depth/learner.c
- labs/foundations/include/foundation.h
- labs/foundations/src/foundation.c
- labs/performance_followthrough/CONTRACTS.md
- labs/performance_followthrough/DELAYED.md
- labs/performance_followthrough/analyze.py
- labs/performance_followthrough/build.py
- labs/performance_followthrough/certificates.py
- labs/performance_followthrough/cgroup_observe.py
- labs/performance_followthrough/cost.c
- labs/performance_followthrough/cost.h
- labs/performance_followthrough/criteria.json
- labs/performance_followthrough/graph.c
- labs/performance_followthrough/observations.c
- labs/performance_followthrough/profile_helper.c
- labs/performance_followthrough/profile_main.c
- labs/performance_followthrough/stack.c
- labs/performance_followthrough/tests.c
- labs/performance_followthrough/trace_gdb.py
- labs/performance_followthrough/trace_main.c
- labs/performance_delegated/CONTRACTS.md
- labs/performance_delegated/build.py
- labs/performance_delegated/fixtures.json
- labs/performance_delegated/guest.py
- labs/performance_delegated/learner.c
- labs/performance_delegated/HINTS.md

Shared contracts:

- curriculum/v5/expansion/legacy_assessments/shared-contract.md
- labs/performance_followthrough/CONTRACTS.md
- labs/performance_delegated/CONTRACTS.md

## Sources and evidence limits

- R-LDIAG-T43 (research/v5/expansion/legacy_diagnostics/resources.json): Complete teaching and mastery/limits text; Project Contract, Inputs and expected results, Steps, Hints and recovery; associated complete fixtures
- R-LDIAG-PERF (research/v5/expansion/legacy_diagnostics/resources.json): DESCRIPTION and Arguments through event grouping; Generalized hardware event definitions
- R-LDIAG-CGROUP (research/v5/expansion/legacy_diagnostics/resources.json): memory.peak
- R-PERF-DELEGATED-CGROUP (research/v5/expansion/performance_delegated/resources.json): No Internal Process Constraint (selected source lines508-534); Model of Delegation (selected source lines540-569); Delegation Containment (selected source lines573-611); Organize Once and Control (selected source lines619-634)

Unavailable or uninspected material:

- Exact published C17 not inspected
- Only registered bounded source passages are claimed; historical book inspections remain separate
- Hardware, foreign ABI and dependency license claims require their exact selected authorities

Inspected baseline scope:

- curriculum/modules/T43.md: Teaching text; project Contract, Inputs, Steps and Hints; complete fixture data
- curriculum/projects/PR-T43-10.md: Teaching text; project Contract, Inputs, Steps and Hints; complete fixture data
- fixtures/performance/PR-T43-10.json: Teaching text; project Contract, Inputs, Steps and Hints; complete fixture data

- Unit identity/mapping inspected; this wave does not claim fresh reading of every unit body.
- Named anchor implementations are separately scoped; a project family binding alone is not full exercise execution.
- No fresh execution of the legacy implementation campaign in this audit.

Recorded baseline execution support: foundations: Read-only visible cgroup accounting; no quota experiment. Complete criterion-specific C, diagnosis and changed-condition evidence remain required.
Recorded baseline fresh execution: not-run for these supplemental tasks

## Current successor support

The baseline execution and validation statements below retain their original observation scope. Additional criterion support is tracked by these record locators. Use the supplied learner artifacts and shared contracts for the current implementation obligations. These locators do not copy or link assessor solutions, and their presence alone does not establish a passing current campaign, independent review, a learner attempt, or physical validation.

- `research/v5/expansion/legacy_performance_followthrough/support.json`: `AX-PR-T43-10-T43.11.a`, `AX-PR-T43-10-T43.11.b`, `AX-PR-T43-10-T43.11.c`
- `research/v5/expansion/performance_delegated/completion_dispositions.json`: `AX-PR-T43-10-T43.11.a-DELAYED`
- `research/v5/expansion/legacy_performance_followthrough/support.json`: `AX-PR-T43-10-T43.11.a`
- `research/v5/expansion/legacy_performance_followthrough/support.json`: `AX-PR-T43-10-T43.11.b`
- `research/v5/expansion/legacy_performance_followthrough/support.json`: `AX-PR-T43-10-T43.11.c`


## AX-PR-T43-10-T43.11.a: Inspect a permitted cgroupv2 hierarchy/controller/delegation snapshot and distinguish observation files from configuration and inherited restrictions.

Objective: T43.11.a. Families: A, C, F, I, J.

Write a read-only inventory/parser for a captured or permitted cgroupv2 hierarchy. Classify cpu.max, cpu.stat, memory.current, memory.peak, io.stat and ancestor policy; preserve absent controllers as unknown.

Why this task: A visible control file implies delegated permission or isolated ownership.

**Prerequisite knowledge:**

- T43 parent teaching and exact project contract
- Representation, lifetime and failure guarantees of imported artifacts

**Prerequisite learner artifacts:**

- PR-T43-10 learner-owned implementation/dossier and recorded initial fixture prediction

**Required reasoning:**

- Derive the stated result from pre/postconditions; name the first state at which the incorrect explanation fails.
- A visible control file implies delegated permission or isolated ownership.
- Account for ownership, failure preservation and resource/complexity bounds where the task changes them; justify genuine non-applicability.

| Evidence | Required artifact |
|---|---|
| D1 | Prediction and causal explanation addressing: A visible control file implies delegated permission or isolated ownership. |
| D2 | Write a read-only inventory/parser for a captured or permitted cgroupv2 hierarchy. Classify cpu.max, cpu.stat, memory.current, memory.peak, io.stat and ancestor policy; preserve absent controllers as unknown. |
| D3 | Independent counterexample and source-bound evidence for: No settings are written; policy and observation have separate fields; hierarchy and descendants are part of the scope. |
| D4 | Move the workload into an already prepared delegated subtree in a separate authorized experiment. |

| Verification | Obligation |
|---|---|
| procedure | Predict before execution. Retain complete observable state, not just return codes. Compare the stated expected relation with the original fixture and independently constructed cases. Preserve exact commands/tool/source identities for experiments; reason about undefined cases without executing them as expected-output tests. |
| expected | No settings are written; policy and observation have separate fields; hierarchy and descendants are part of the scope. |
| pass | All stated relations hold with a correct causal argument; a matching output with false lifetime/language reasoning fails. |
| limits | This criterion is newly authored. The listed legacy anchor does not assess all of its obligations; no fresh result is inferred. |

| Rubric level | Discriminator |
|---|---|
| 0 | Missing or incorrect contract/result. |
| 1 | Reproduces a nominal example but cannot diagnose the stated misconception. |
| 2 | No settings are written; policy and observation have separate fields; hierarchy and descendants are part of the scope. Explains the mechanism and supplies independent evidence. |
| 3 | Move the workload into an already prepared delegated subtree in a separate authorized experiment. Derives changed invariants without copying the previous solution. |

**Graduated hints:**

1. Availability, delegation and attribution are different questions.
2. Write the earliest changed ownership, state, dependency or observation explicitly; compare that point before comparing final outputs.

**Changed-condition assessment:**

- AX-PR-T43-10-T43.11.a-DELAYED (at least 7 days after recorded study): Move the workload into an already prepared delegated subtree in a separate authorized experiment.

| Coverage facet | Claim and limit |
|---|---|
| teaching | Retain inspected T43.11 teaching and the bound project contract. |
| implementation_or_analysis | Existing learner artifact plus the exact new task; no unrelated test is promoted to whole-leaf coverage. |
| boundary_adversarial | The named misconception, independent counterexample and changed assumption are explicit assessment obligations. |
| independent_assessment | Objective-specific 0-3 criterion and separate assessor analysis authored; not administered. |
| remaining | Execute or independently review all task-specific evidence and delayed transfer; exact reference/target gaps remain. |

**Security and containment:**

| Property | Contract |
|---|---|
| applicable | False |
| scope | Original local learner components and synthetic fixtures only. This is a contract/diagnostic assessment, not an exploit demonstration; shared containment applies. |


Current criterion successor record: `research/v5/expansion/legacy_performance_followthrough/support.json`: `AX-PR-T43-10-T43.11.a`.

## AX-PR-T43-10-T43.11.b: Interpret keyed CPU-time/throttle, current/peak memory, per-device byte/IO and pressure deltas with units and scope, retaining per-FD peak-reset and writeback-attribution limits.

Objective: T43.11.b. Families: B, C, D, F, J.

Parse keyed snapshots with reordered fields, new optional keys, a decreasing gauge and a reset cumulative counter. Derive per-device deltas and refuse a fresh-window memory peak without the required same-descriptor reset history.

Why this task: Every decreasing field is invalid or every historical peak belongs to the current run.

**Prerequisite knowledge:**

- T43 parent teaching and exact project contract
- Representation, lifetime and failure guarantees of imported artifacts

**Prerequisite learner artifacts:**

- PR-T43-10 learner-owned implementation/dossier and recorded initial fixture prediction

**Required reasoning:**

- Derive the stated result from pre/postconditions; name the first state at which the incorrect explanation fails.
- Every decreasing field is invalid or every historical peak belongs to the current run.
- Account for ownership, failure preservation and resource/complexity bounds where the task changes them; justify genuine non-applicability.

| Evidence | Required artifact |
|---|---|
| D1 | Prediction and causal explanation addressing: Every decreasing field is invalid or every historical peak belongs to the current run. |
| D2 | Parse keyed snapshots with reordered fields, new optional keys, a decreasing gauge and a reset cumulative counter. Derive per-device deltas and refuse a fresh-window memory peak without the required same-descriptor reset history. |
| D3 | Independent counterexample and source-bound evidence for: Gauge decreases are valid; counter reset requires a new baseline; absent fields remain unknown; device identities are matched before subtraction. |
| D4 | Add a disappearing device and shared-inode writeback attribution. |

| Verification | Obligation |
|---|---|
| procedure | Predict before execution. Retain complete observable state, not just return codes. Compare the stated expected relation with the original fixture and independently constructed cases. Preserve exact commands/tool/source identities for experiments; reason about undefined cases without executing them as expected-output tests. |
| expected | Gauge decreases are valid; counter reset requires a new baseline; absent fields remain unknown; device identities are matched before subtraction. |
| pass | All stated relations hold with a correct causal argument; a matching output with false lifetime/language reasoning fails. |
| limits | This criterion is newly authored. The listed legacy anchor does not assess all of its obligations; no fresh result is inferred. |

| Rubric level | Discriminator |
|---|---|
| 0 | Missing or incorrect contract/result. |
| 1 | Reproduces a nominal example but cannot diagnose the stated misconception. |
| 2 | Gauge decreases are valid; counter reset requires a new baseline; absent fields remain unknown; device identities are matched before subtraction. Explains the mechanism and supplies independent evidence. |
| 3 | Add a disappearing device and shared-inode writeback attribution. Derives changed invariants without copying the previous solution. |

**Graduated hints:**

1. Treat counters, gauges and policy values as distinct types.
2. Write the earliest changed ownership, state, dependency or observation explicitly; compare that point before comparing final outputs.

**Changed-condition assessment:**

- AX-PR-T43-10-T43.11.b-DELAYED (at least 7 days after recorded study): Add a disappearing device and shared-inode writeback attribution.

| Coverage facet | Claim and limit |
|---|---|
| teaching | Retain inspected T43.11 teaching and the bound project contract. |
| implementation_or_analysis | Existing learner artifact plus the exact new task; no unrelated test is promoted to whole-leaf coverage. |
| boundary_adversarial | The named misconception, independent counterexample and changed assumption are explicit assessment obligations. |
| independent_assessment | Objective-specific 0-3 criterion and separate assessor analysis authored; not administered. |
| remaining | Execute or independently review all task-specific evidence and delayed transfer; exact reference/target gaps remain. |

**Security and containment:**

| Property | Contract |
|---|---|
| applicable | False |
| scope | Original local learner components and synthetic fixtures only. This is a contract/diagnostic assessment, not an exploit demonstration; shared containment applies. |


Current criterion successor record: `research/v5/expansion/legacy_performance_followthrough/support.json`: `AX-PR-T43-10-T43.11.b`.

## AX-PR-T43-10-T43.11.c: Compare a bounded workload with unchanged observations or a prepared delegated quota experiment, explaining quota/pressure effects while preserving an explicit no-root unavailable-controller route.

Objective: T43.11.c. Families: A, F, H, I, J.

Compare a bounded workload with unchanged read-only observations and the object-lifetime ledger. Explain why quota ratio, observed CPU fraction, pressure and retained payload need not agree.

Why this task: Quota is a guaranteed usage fraction and PSI is exclusive component wait time.

**Prerequisite knowledge:**

- T43 parent teaching and exact project contract
- Representation, lifetime and failure guarantees of imported artifacts

**Prerequisite learner artifacts:**

- PR-T43-10 learner-owned implementation/dossier and recorded initial fixture prediction

**Required reasoning:**

- Derive the stated result from pre/postconditions; name the first state at which the incorrect explanation fails.
- Quota is a guaranteed usage fraction and PSI is exclusive component wait time.
- Account for ownership, failure preservation and resource/complexity bounds where the task changes them; justify genuine non-applicability.

| Evidence | Required artifact |
|---|---|
| D1 | Prediction and causal explanation addressing: Quota is a guaranteed usage fraction and PSI is exclusive component wait time. |
| D2 | Compare a bounded workload with unchanged read-only observations and the object-lifetime ledger. Explain why quota ratio, observed CPU fraction, pressure and retained payload need not agree. |
| D3 | Independent counterexample and source-bound evidence for: The result preserves inherited/shared scope and gives a causal hypothesis rather than forced equality; missing controllers remain unavailable. |
| D4 | Use a captured throttling history with burst and ancestor limits. |

| Verification | Obligation |
|---|---|
| procedure | Predict before execution. Retain complete observable state, not just return codes. Compare the stated expected relation with the original fixture and independently constructed cases. Preserve exact commands/tool/source identities for experiments; reason about undefined cases without executing them as expected-output tests. |
| expected | The result preserves inherited/shared scope and gives a causal hypothesis rather than forced equality; missing controllers remain unavailable. |
| pass | All stated relations hold with a correct causal argument; a matching output with false lifetime/language reasoning fails. |
| limits | This criterion is newly authored. The listed legacy anchor does not assess all of its obligations; no fresh result is inferred. |

| Rubric level | Discriminator |
|---|---|
| 0 | Missing or incorrect contract/result. |
| 1 | Reproduces a nominal example but cannot diagnose the stated misconception. |
| 2 | The result preserves inherited/shared scope and gives a causal hypothesis rather than forced equality; missing controllers remain unavailable. Explains the mechanism and supplies independent evidence. |
| 3 | Use a captured throttling history with burst and ancestor limits. Derives changed invariants without copying the previous solution. |

**Graduated hints:**

1. A policy constrains opportunities; it does not prescribe work performed.
2. Write the earliest changed ownership, state, dependency or observation explicitly; compare that point before comparing final outputs.

**Changed-condition assessment:**

- AX-PR-T43-10-T43.11.c-DELAYED (at least 7 days after recorded study): Use a captured throttling history with burst and ancestor limits.

| Coverage facet | Claim and limit |
|---|---|
| teaching | Retain inspected T43.11 teaching and the bound project contract. |
| implementation_or_analysis | Existing learner artifact plus the exact new task; no unrelated test is promoted to whole-leaf coverage. |
| boundary_adversarial | The named misconception, independent counterexample and changed assumption are explicit assessment obligations. |
| independent_assessment | Objective-specific 0-3 criterion and separate assessor analysis authored; not administered. |
| remaining | Execute or independently review all task-specific evidence and delayed transfer; exact reference/target gaps remain. |

**Security and containment:**

| Property | Contract |
|---|---|
| applicable | False |
| scope | Original local learner components and synthetic fixtures only. This is a contract/diagnostic assessment, not an exploit demonstration; shared containment applies. |


Current criterion successor record: `research/v5/expansion/legacy_performance_followthrough/support.json`: `AX-PR-T43-10-T43.11.c`.

## Original validation status

| Dimension | Status |
|---|---|
| content | authored additive assessment |
| execution | not-run for supplemental tasks |
| independent_review | not-run |
| learner_assessment | not-assessed |
| limitations | Baseline text, IDs, hours and historical receipts preserved.; Finite controls do not prove absolute mastery.; Each exact changed-condition experiment remains required. |

