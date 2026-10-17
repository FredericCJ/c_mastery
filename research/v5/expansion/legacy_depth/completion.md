# Retained-domain depth expansion: author scope

This scope is authored and structurally validated. It contains 103 additive project supplements and 633 distinct leaf-objective criteria across T01-T06, T12-T17 and T30-T35. Every assigned existing leaf has a specific task, misconception, independent expected relation, criterion rubric, changed-condition assessment and separate assessor analysis. The accepted modules, project identities, unit identities, historical hours and acceptance receipts are retained. This document does not establish independent review, learner achievement or universal mastery.

| Domains | Projects | Criteria |
| --- | ---: | ---: |
| T01-T06 | 24 | 189 |
| T12-T17 | 44 | 240 |
| T30-T35 | 35 | 204 |
| Total | 103 | 633 |

Canonical contracts are `curriculum/v5/expansion/legacy_assessments/PR-*.json`, with the shared 14-field contract and schema in the same directory. Technical analyses are withheld under `assessor/legacy_depth/PR-*.md`. Per-leaf teaching and implementation dispositions in the B02/B03 depth records point to the actual supplement criteria rather than treating a topic heading or test name as independent evidence. Prerequisite artifacts and baseline parent-unit bindings remain explicit. Retained elective metadata does not exempt these additive leaf assessments from the requested expansion.

## Actual validation

- `.conda/python.exe assessor/legacy_depth/validate_author_scope.py` passed: 103 supplements, 633 criteria, 633 distinct assigned objectives, no missing assigned leaf and no validation errors. The source-bound result is `reviews/v5/expansion/legacy_depth/author_scope_validation.json`. This checks the current shared validator, exact baseline and source bindings, teaching links, required fields, separate assessor anchors, delayed variants and supplied-artifact containment. It does not execute every assessment.
- `.conda/python.exe assessor/legacy_depth/verify_author_cases.py` passed three independent mathematical fixture checks: the finite membership filter, CRC polynomial division and run-length encoding. These are host oracle checks of exact added fixtures, not learner C execution.
- `wsl -e python3 /mnt/f/Learning/Curricula/c_mastery/assessor/legacy_depth/verify_format_demo.py` passed six executions: original and repaired implementations under GCC optimized, Clang optimized and GCC ASan/UBSan profiles. The receipt records all 15 version/build/run commands and their output. A first attempt failed compilation because the test initializer was longer than its output array under warning-as-error; the initial receipt is preserved, the initializer was corrected, and no warning or semantic check was disabled.

The formatting demonstration admits only the original literal `%n` with an intentionally supplied live, correctly typed `int *`. The observable defect is a synthetic marker changing from 77 to 0. The repair prints `%n` literally and preserves 77, including separately checked output truncation. No arbitrary-address write, arbitrary code execution, privilege escalation, writable-format hardening bypass or third-party target is claimed. The original target and input contract are learner fixtures; the repair and executable oracle remain assessor material.

## Reference and execution gaps retained

Only the precise passages registered in `resources.json` and discussed in `sources.md` were inspected. Their gate pins those files. Bibliography titles and prior historical inspections are not silently upgraded to fresh whole-source reads.

- Exact published C17 and C23, C90/C95 normative originals, selected C23 library/freestanding clauses and proposal-specific provenance semantics remain source gaps. N1570 is explicitly C11; N3096 is a C23 draft. No draft is presented as the published C17 baseline.
- Fresh musl retrieval failed. CLRS, Hanson, Hoare, complete Numerical Algorithms proofs, Roofline and the historical Bernstein case were not newly inspected in this wave. The earlier EWD249 retrieval did not provide a readable inspected passage. These limits do not erase retained historical records.
- OpenGroup normative open/write/exec retrieval failed. Inspected Linux man-pages bound the Linux adapter; they are not represented as a newly inspected universal POSIX specification.
- Board revision/errata deltas, the full Arm fault bitfields, complete Stop/Standby entry recipes, AVX OS-state enablement and PMU definitions require further exact source work where used. The known synthetic masks, baseline port excerpts and revised board manual pages do not cover those missing details.
- No flashing, host security policy change, physical target timing, current/energy, MPU enforcement or electrical measurement was performed. Host models and original local demonstrations do not validate physical hardware.
- The formal-method tasks now have concrete mathematical models, proof obligations and counterexample boundaries, but no new CBMC/Frama-C solver campaign was performed in this wave. Runtime, solver and platform experiments remain required learner/assessor work where specified.
- The underlying retained construction laboratories support only their named mechanisms. The additive contracts intentionally require the remaining per-leaf analysis, implementation variants, failure injections and independent transfer; their mere authoring is not evidence that those tasks were executed or assessed.

The newly added format target and three mathematical fixtures have executed author checks. Other new security demonstrations, descriptor races, update-verifier models, formal proofs and physical experiments are specified but not newly executed here. No unimplemented target experiment is reclassified as an equivalent weaker completed task.
