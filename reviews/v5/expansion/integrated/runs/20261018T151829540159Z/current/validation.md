# Integrated expansion author validation

Status: **passed**. Source SHA-256: `1ca37319eeaf42ef53d9e248583aa243bdaf88980a7cff9ceb145eb890240e02`.

These checks assess authored contracts, scoped laboratory observations and publication tooling. They do not assess a learner or accept the whole edition.

| Check | Exit | Tests reported | Skipped | Exact command and output |
| --- | ---: | ---: | ---: | --- |
| exercise_workflow | 0 | 17 | 1 | [reviews/v5/expansion/integrated/runs/20261017T001514002203Z/exercise_workflow.log](runs/20261017T001514002203Z/exercise_workflow.log) |
| exercise_workflow_linux | 0 | 17 | 0 | [reviews/v5/expansion/integrated/runs/20261017T001514002203Z/exercise_workflow_linux.log](runs/20261017T001514002203Z/exercise_workflow_linux.log) |
| exercise_integration_linux | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261017T001514002203Z/exercise_integration_linux.log](runs/20261017T001514002203Z/exercise_integration_linux.log) |
| release_guards_linux | 0 | 30 | 0 | [reviews/v5/expansion/integrated/runs/20261017T001514002203Z/release_guards_linux.log](runs/20261017T001514002203Z/release_guards_linux.log) |
| publication_routing_linux | 0 | 21 | 0 | [reviews/v5/expansion/integrated/runs/20261017T001514002203Z/publication_routing_linux.log](runs/20261017T001514002203Z/publication_routing_linux.log) |
| baseline_learning | 0 | 45 | 0 | [reviews/v5/expansion/integrated/runs/20261017T001514002203Z/baseline_learning.log](runs/20261017T001514002203Z/baseline_learning.log) |
| legacy_cli | 0 | 19 | 0 | [reviews/v5/expansion/integrated/runs/20261017T001514002203Z/legacy_cli.log](runs/20261017T001514002203Z/legacy_cli.log) |
| authoring_contracts | 0 | 69 | 0 | [reviews/v5/expansion/integrated/runs/20261017T001514002203Z/authoring_contracts.log](runs/20261017T001514002203Z/authoring_contracts.log) |
| legacy_assessments | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261017T001514002203Z/legacy_assessments.log](runs/20261017T001514002203Z/legacy_assessments.log) |
| authored_content | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261017T001514002203Z/authored_content.log](runs/20261017T001514002203Z/authored_content.log) |
| modern_coverage | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261017T001514002203Z/modern_coverage.log](runs/20261017T001514002203Z/modern_coverage.log) |
| expansion_campaigns | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261017T001514002203Z/expansion_campaigns.log](runs/20261017T001514002203Z/expansion_campaigns.log) |

Structure: 191 exercises; passed.

Full publication checks passed against this source identity: True.

Observed laboratory campaign: {'families': 36, 'passed': True, 'source_current': True, 'failed_families': [], 'distinct_execution_receipts': 29}.

Exact argument arrays, timings and log hashes are in `validation.json`. `expansion_campaigns.json` links each original source-bound campaign, its compiler profiles, oracle results, mutation controls and separate target audits. Shared campaign observations are not multiplied by the number of consuming domains.

Historical receipt preservation is checked separately in `historical_preservation.json`. Physical validation: not run. Learner mastery: not assessed. Scoped agent review, if present, is not an independent human or full-edition review.
