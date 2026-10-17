# B02 first-wave validation

Actually executed on 2026-10-17: Python `cmastery.exercises.parse_brief` and `validate` on all eight new canonical briefs succeeded. This checks structural contracts, references, paths and rubric facets; it does not assess learner knowledge.

Executed through WSL from `/mnt/f/Learning/Curricula/c_mastery`:

```
cmake -S labs/b02_depth -B /tmp/cmastery-b02-author -DCMASTERY_ASSESSOR_ROOT=/mnt/f/Learning/Curricula/c_mastery/assessor -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/cmastery-b02-author -j4
ctest --test-dir /tmp/cmastery-b02-author --output-on-failure
```

Observed 17/17 passed: eight semantic anchors, eight compiled mutants rejected by named semantic assertions, and one independent host-oracle campaign. The oracle executes 65,536 Q7 pairs, 4,686 counted spans, 3,280 stable-sort sequences and 20,000 deterministic map transitions. A prior attempted incremental build found its temporary build directory absent; reconfiguration into the explicit author directory above succeeded. Early builds reported misleading-indentation warnings; source formatting was repaired and warnings promoted to errors before this passing run.

The source-bound six-profile campaign is produced separately by tools/run_b02_depth.py. This author report is not a fabricated receipt or independent review. Additional parser streaming, rehash/chaining, cleanup-failure and owning-variant work remains required under the parent projects and disposition ledger. Physical validation, delayed learner assessment, performance calibration and exact C17 access remain open.
