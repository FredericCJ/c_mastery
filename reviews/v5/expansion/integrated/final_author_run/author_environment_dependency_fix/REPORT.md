# Exact author-environment dependency repair

Finding `FIX-AUTHOR-ENV-CORRECTION-CAPTURES`, under `G-V5-DOMAIN-TOOLS`.

The actual extracted build from candidate `candidate_20261019T075134830133Z` failed in `resource_identity_v5.corrections`: `build/replicated_sources/hlc.pdf` was absent. The original manifest collected exact active completion-record pins but omitted a separate authority used unconditionally by `integrate_v5.py`. Four selected resource-correction records require captures: HLC, CRDT, delta-state CRDT and operational transformation. Their local bytes match the previously recorded hashes; no passage was newly inspected for this repair.

`tools/candidate_author_environment.py` now reuses the actual correction validator, requires its policy, predecessor registry, evidence records and consumer sources in the source archive, and records the exact selected capture edges. It stages those captures only through the existing explicit opt-in environment. Neither source archives nor learner archives gain these PDF payloads. Private-path and no-overwrite guards remain enforced. Unselected records and adjacent files grant no authority.

Only that tool and `tools/check_candidate_author_environment.py` changed in the canonical source. The current identity and exact two-file comparison are in `source_frozen.json` and `discovery_comparison.json`. All 912 existing input rows remain byte-for-byte equivalent; the four additions total 3,502,768bytes. The 916-input qualified environment totals 410,764,293bytes. Two absolute external inputs remain uncopied. This is not a self-contained archive claim.

The actual checkout passes 32 focused tests on Linux with 0 skips; Windows passes 30 with 2 explicit symlink-permission skips. Reports retain exact source hashes and per-test results. The old executable discovery logic fails the intended four-capture assertion with no import/translation errors and then reproduces the actual missing-HLC error when the real checker reads the staged synthetic environment. See `pre_fix_rejection.json`, its raw log and the portable replay under `control/`.

Bounded author-team source reviews covered the correction consumer, selected legacy-source dereferences, publication/release consumers and scoped campaign pins. They found no further omitted capture in those inspected paths. These are collaborating-author reviews, not independent external technical review.

The previous build, six campaigns and integrated validation remain observations of `de97e549...`; they are not relabelled with the new identity. `retained_receipts/manifest.json` preserves 160 active reports before replacement. The failed extracted run's original log, execution record and passing historical-byte preservation are retained under `failed_candidate/`. The failed candidate remains failed.

Full build, refreshed campaigns, integrated validation and actual extracted-source reproduction must finish successfully on the repaired source before this finding's author-validation dimension closes. No release acceptance, promotion, learner performance or physical observation is asserted by the focused results.
