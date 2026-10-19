# Integrated expansion author validation

Status: **passed**. Source SHA-256: `1fd8b84957dfaa2a6119269aa81cf5a3ea5434cbdebbf8e7176fbe73adea1f27`.

These checks assess authored contracts, scoped laboratory observations and publication tooling. They do not assess a learner or accept the whole edition.

| Check | Exit | Tests reported | Skipped | Exact command and output |
| --- | ---: | ---: | ---: | --- |
| exercise_workflow | 0 | 29 | 2 | [reviews/v5/expansion/integrated/runs/20261019T121048211267Z/exercise_workflow.log](runs/20261019T121048211267Z/exercise_workflow.log) |
| exercise_workflow_linux | 0 | 29 | 0 | [reviews/v5/expansion/integrated/runs/20261019T121048211267Z/exercise_workflow_linux.log](runs/20261019T121048211267Z/exercise_workflow_linux.log) |
| exercise_integration_linux | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261019T121048211267Z/exercise_integration_linux.log](runs/20261019T121048211267Z/exercise_integration_linux.log) |
| release_guards_linux | 0 | 81 | 0 | [reviews/v5/expansion/integrated/runs/20261019T121048211267Z/release_guards_linux.log](runs/20261019T121048211267Z/release_guards_linux.log) |
| publication_routing_linux | 0 | 25 | 0 | [reviews/v5/expansion/integrated/runs/20261019T121048211267Z/publication_routing_linux.log](runs/20261019T121048211267Z/publication_routing_linux.log) |
| baseline_learning | 0 | 45 | 0 | [reviews/v5/expansion/integrated/runs/20261019T121048211267Z/baseline_learning.log](runs/20261019T121048211267Z/baseline_learning.log) |
| legacy_cli | 0 | 19 | 0 | [reviews/v5/expansion/integrated/runs/20261019T121048211267Z/legacy_cli.log](runs/20261019T121048211267Z/legacy_cli.log) |
| authoring_contracts | 0 | 101 | 0 | [reviews/v5/expansion/integrated/runs/20261019T121048211267Z/authoring_contracts.log](runs/20261019T121048211267Z/authoring_contracts.log) |
| legacy_assessments | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261019T121048211267Z/legacy_assessments.log](runs/20261019T121048211267Z/legacy_assessments.log) |
| authored_content | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261019T121048211267Z/authored_content.log](runs/20261019T121048211267Z/authored_content.log) |
| modern_coverage | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261019T121048211267Z/modern_coverage.log](runs/20261019T121048211267Z/modern_coverage.log) |
| expansion_campaigns | 0 | None | 0 | [reviews/v5/expansion/integrated/runs/20261019T121048211267Z/expansion_campaigns.log](runs/20261019T121048211267Z/expansion_campaigns.log) |

Structure: 238 exercises; passed.

Full publication checks passed against this source identity: True.

Observed laboratory campaign: {'families': 62, 'passed': True, 'source_current': True, 'failed_families': [], 'distinct_execution_receipts': 55}.

Exact parent adjudication: **passed**. The three active row hashes and any failures are in `validation.json`; this result does not change child source gaps or external evidence.

Exact argument arrays, timings and log hashes are in `validation.json`. `expansion_campaigns.json` links each original source-bound campaign, its compiler profiles, oracle results, mutation controls and separate target audits. Shared campaign observations are not multiplied by the number of consuming domains.

Historical receipt preservation is checked separately in `historical_preservation.json`. Physical validation: not run. Learner mastery: not assessed. Scoped agent review, if present, is not an independent human or full-edition review.
