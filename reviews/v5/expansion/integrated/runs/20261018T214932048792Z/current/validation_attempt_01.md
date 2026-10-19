# Integrated expansion author validation

Status: **failed**. Source SHA-256: `64a8e116cf6a1387dad2e950c326971c1ee8ef36c708933148f5a3a1ea5d031c`.

These checks assess authored contracts, scoped laboratory observations and publication tooling. They do not assess a learner or accept the whole edition.

| Check | Exit | Tests reported | Skipped | Exact command and output |
| --- | ---: | ---: | ---: | --- |
| exercise_workflow | 0 | 17 | 1 | [reviews/v5/expansion/integrated/runs/20261016T231436990907Z/exercise_workflow.log](runs/20261016T231436990907Z/exercise_workflow.log) |
| exercise_workflow_linux | 0 | 17 | 0 | [reviews/v5/expansion/integrated/runs/20261016T231436990907Z/exercise_workflow_linux.log](runs/20261016T231436990907Z/exercise_workflow_linux.log) |
| exercise_integration_linux | 1 | None | 0 | [reviews/v5/expansion/integrated/runs/20261016T231436990907Z/exercise_integration_linux.log](runs/20261016T231436990907Z/exercise_integration_linux.log) |
| release_guards_linux | 0 | 30 | 0 | [reviews/v5/expansion/integrated/runs/20261016T231436990907Z/release_guards_linux.log](runs/20261016T231436990907Z/release_guards_linux.log) |
| publication_routing_linux | 0 | 21 | 0 | [reviews/v5/expansion/integrated/runs/20261016T231436990907Z/publication_routing_linux.log](runs/20261016T231436990907Z/publication_routing_linux.log) |
| baseline_learning | 0 | 45 | 0 | [reviews/v5/expansion/integrated/runs/20261016T231436990907Z/baseline_learning.log](runs/20261016T231436990907Z/baseline_learning.log) |
| legacy_cli | 0 | 19 | 0 | [reviews/v5/expansion/integrated/runs/20261016T231436990907Z/legacy_cli.log](runs/20261016T231436990907Z/legacy_cli.log) |
| authoring_contracts | 0 | 62 | 0 | [reviews/v5/expansion/integrated/runs/20261016T231436990907Z/authoring_contracts.log](runs/20261016T231436990907Z/authoring_contracts.log) |
| legacy_assessments | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261016T231436990907Z/legacy_assessments.log](runs/20261016T231436990907Z/legacy_assessments.log) |
| authored_content | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261016T231436990907Z/authored_content.log](runs/20261016T231436990907Z/authored_content.log) |
| modern_coverage | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261016T231436990907Z/modern_coverage.log](runs/20261016T231436990907Z/modern_coverage.log) |
| expansion_campaigns | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261016T231436990907Z/expansion_campaigns.log](runs/20261016T231436990907Z/expansion_campaigns.log) |

Structure: 191 exercises; passed.

Full publication checks passed against this source identity: True.

Observed laboratory campaign: {'families': 36, 'passed': True, 'source_current': True, 'failed_families': [], 'distinct_execution_receipts': 29}.

Exact argument arrays, timings and log hashes are in `validation.json`. `expansion_campaigns.json` links each original source-bound campaign, its compiler profiles, oracle results, mutation controls and separate target audits. Shared campaign observations are not multiplied by the number of consuming domains.

Historical receipt preservation is checked separately in `historical_preservation.json`. Physical validation: not run. Learner mastery: not assessed. Scoped agent review, if present, is not an independent human or full-edition review.
