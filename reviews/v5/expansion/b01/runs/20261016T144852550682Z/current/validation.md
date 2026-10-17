# B01 author validation

Status: **passed**. Source SHA-256: `61d4ff3c6a76d2c8c232488393c0f47f04ed7684bad7b2563c989eb207483cc6`.

These checks assess the authored batch and its tooling. They do not assess a learner or accept the whole edition.

| Check | Exit | Tests reported | Skipped | Exact command and output |
| --- | ---: | ---: | ---: | --- |
| exercise_workflow | 0 | 17 | 1 | [reviews/v5/expansion/b01/runs/20261016T144852550682Z/exercise_workflow.log](runs/20261016T144852550682Z/exercise_workflow.log) |
| exercise_workflow_linux | 0 | 17 | 0 | [reviews/v5/expansion/b01/runs/20261016T144852550682Z/exercise_workflow_linux.log](runs/20261016T144852550682Z/exercise_workflow_linux.log) |
| exercise_integration_linux | 0 | None | 0 | [reviews/v5/expansion/b01/runs/20261016T144852550682Z/exercise_integration_linux.log](runs/20261016T144852550682Z/exercise_integration_linux.log) |
| release_guards_linux | 0 | 28 | 0 | [reviews/v5/expansion/b01/runs/20261016T144852550682Z/release_guards_linux.log](runs/20261016T144852550682Z/release_guards_linux.log) |
| baseline_learning | 0 | 45 | 0 | [reviews/v5/expansion/b01/runs/20261016T144852550682Z/baseline_learning.log](runs/20261016T144852550682Z/baseline_learning.log) |
| legacy_cli | 0 | 19 | 0 | [reviews/v5/expansion/b01/runs/20261016T144852550682Z/legacy_cli.log](runs/20261016T144852550682Z/legacy_cli.log) |
| foundation_depth_campaign | 0 | None | 0 | [reviews/v5/expansion/b01/runs/20261016T144852550682Z/foundation_depth_campaign.log](runs/20261016T144852550682Z/foundation_depth_campaign.log) |

Structure: 8 exercises; passed.

Full publication checks passed against this source identity: True.

Observed laboratory campaign: {'profiles': 6, 'assessor_test_executions': 102, 'compiled_mutant_rejections': 48, 'learner_stub_rejections': 48, 'independent_oracle_cases': 12288, 'target_audit': 'passed'}.

Exact argument arrays, timings and log hashes are in `validation.json`. Detailed compiler profiles, oracle counts, mutation controls and link audits are in `foundation_depth_campaign.json`.

Historical receipt preservation is checked separately in `historical_preservation.json`. Physical validation: not run. Learner mastery: not assessed. Scoped agent review, if present, is not an independent human or full-edition review.
