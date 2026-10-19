# Integrated expansion author validation

Status: **failed**. Source SHA-256: `59d3c902835a2a61dd95baec894d612c14fbf2eea2a3160948bbad0dd39a356b`.

These checks assess authored contracts, scoped laboratory observations and publication tooling. They do not assess a learner or accept the whole edition.

| Check | Exit | Tests reported | Skipped | Exact command and output |
| --- | ---: | ---: | ---: | --- |
| exercise_workflow | 0 | 19 | 1 | [reviews/v5/expansion/integrated/runs/20261018T214932048792Z/exercise_workflow.log](exercise_workflow.log) |
| exercise_workflow_linux | 0 | 19 | 0 | [reviews/v5/expansion/integrated/runs/20261018T214932048792Z/exercise_workflow_linux.log](exercise_workflow_linux.log) |
| exercise_integration_linux | 1 | None | 0 | [reviews/v5/expansion/integrated/runs/20261018T214932048792Z/exercise_integration_linux.log](exercise_integration_linux.log) |
| release_guards_linux | 0 | 70 | 0 | [reviews/v5/expansion/integrated/runs/20261018T214932048792Z/release_guards_linux.log](release_guards_linux.log) |
| publication_routing_linux | 0 | 21 | 0 | [reviews/v5/expansion/integrated/runs/20261018T214932048792Z/publication_routing_linux.log](publication_routing_linux.log) |
| baseline_learning | 0 | 45 | 0 | [reviews/v5/expansion/integrated/runs/20261018T214932048792Z/baseline_learning.log](baseline_learning.log) |
| legacy_cli | 0 | 19 | 0 | [reviews/v5/expansion/integrated/runs/20261018T214932048792Z/legacy_cli.log](legacy_cli.log) |
| authoring_contracts | 0 | 101 | 0 | [reviews/v5/expansion/integrated/runs/20261018T214932048792Z/authoring_contracts.log](authoring_contracts.log) |
| legacy_assessments | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261018T214932048792Z/legacy_assessments.log](legacy_assessments.log) |
| authored_content | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261018T214932048792Z/authored_content.log](authored_content.log) |
| modern_coverage | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261018T214932048792Z/modern_coverage.log](modern_coverage.log) |
| expansion_campaigns | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261018T214932048792Z/expansion_campaigns.log](expansion_campaigns.log) |

Structure: 238 exercises; passed.

Full publication checks passed against this source identity: True.

Observed laboratory campaign: {'families': 62, 'passed': True, 'source_current': True, 'failed_families': [], 'distinct_execution_receipts': 55}.

Exact parent adjudication: **failed**. The three active row hashes and any failures are in `validation.json`; this result does not change child source gaps or external evidence.

Exact argument arrays, timings and log hashes are in `validation.json`. `expansion_campaigns.json` links each original source-bound campaign, its compiler profiles, oracle results, mutation controls and separate target audits. Shared campaign observations are not multiplied by the number of consuming domains.

Historical receipt preservation is checked separately in `historical_preservation.json`. Physical validation: not run. Learner mastery: not assessed. Scoped agent review, if present, is not an independent human or full-edition review.
