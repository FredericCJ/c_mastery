# T29.03–.06 author validation

Current source-bound campaign: `reviews/v5/expansion/legacy_advanced_handoff/current_validation.json`, **286 checks passed**. Exact argv, exit/expected status, raw-log path/hash, all source/output identities and limits are in that record. Six native GCC/Clang debug/optimized/ASan+UBSan profiles, four actual TSan thread/mutex executions, selected GenMC RC11, and eight M4 configurations were executed. No external review, learner/delayed attempt or physical observation is claimed.

Reproduction from repository root through WSL:

```text
python3 tools/run_interrupt_handoff_depth.py --report reviews/v5/expansion/legacy_advanced_handoff/current_validation.json
python3 tools/check_interrupt_handoff_depth.py --output reviews/v5/expansion/legacy_advanced_handoff/support_validation.json --self-test
python3 tools/check_interrupt_handoff_supplement.py
python3 tools/close_interrupt_handoff_depth.py --integrate
python3 tools/check_interrupt_handoff_exports.py
python3 tools/close_interrupt_handoff_depth.py
python3 tools/check_interrupt_handoff_package.py --output reviews/v5/expansion/legacy_advanced_handoff/completion_dispositions_validation.json --self-test
```

Observed gates:22 campaign-guard controls;16 supplemental checks (eight actual queue placements, ten negative capture cases within one self-test, one unexecuted-template refusal and six source bindings);16 actual CLI/export operations across both projects;12 package controls. These counts describe check granularity, not independent competencies.24 exact original ordinary/delayed rows are bound by `completion_dispositions.json` and `objective_mapping.json`.

Each native profile checked49152 independent deque operations and4096 mask triples. The unchanged CMX provider checked1984 records including unsigned wrap; the widened-index host fixture checked3000 records. Two producer threads publish4000 records through separate queues. The positive GenMC core explored20 complete executions, while premature publication and two missing release edges returned its explicit race diagnostic/exit42. Logical mutants compiled successfully before semantic rejection. Widened M4 load/store helpers intentionally failed a strict link; this is a separate dependency rejection, not a semantic-mutant pass.

Actual M4 source-to-image closure includes original runtime helpers only, and all eight queue symbols have252-byte size/four-byte alignment under the selected ABI. Conservative handler-increment bounds, including the declared nested IRQ/hardware frames, were GCC O0:768bytes; GCC O3:312; Clang O0:604; Clang O3:272. These sum reachable compiler locals conservatively and exclude foreground depth, FP and NMI/fault nesting. They are not watermarks or physical WCET.

All target configurations produced40 accepted/0dropped for the spaced workload and24accepted/16dropped for the equal-average burst workload. With publication probes, observed nearest-rank median enqueue-to-consume intervals in **virtual timer ticks** were GCC O0:[11,13] spaced/[96900,96902] burst; GCC O3:[2,3]/[96899,96900]; Clang O0:[9,10]/[96900,96901]; Clang O3:[3,4]/[96900,96901]. Overhead brackets observed0–1ticks. Timer model tick40ns does not convert these into real silicon timings. Raw distributions and both instrument configurations are retained in the campaign artifacts.

Earlier development campaign `development_validation.json` recorded271 checks and six failures caused by an expected-text mismatch after formatting the deliberately failing DMA assertion. The negative program actually returned1 with `XI_DMA_FAIL out.sequence == 42`; the corrected exact marker and expanded final campaign passed. `exports_attempt1.json` retains an invalid shared-contract path rejection; the board protocol was moved into the allowed curriculum guide tree and both actual exports rerun. No exporter allowlist was weakened.

`optimized_model_attempt.json` retains a current-source O1 GenMC compatibility failure, with exact output and no validation credit. Earlier tool crashes in the pilot are retained under `build/handoff-pilot/genmc-attempt1.json` and `genmc-O1.json`; their earlier whole-translation-unit source was subsequently refactored, so those old logs alone are not a source-reproducible success claim. The actual queue core is shared by native, target and the explicitly selected O0 model. Extending that checker to optimized whole-program IR remains a stated tool limitation, not evidence for unexamined service/frame behavior.

Physical/source/external boundaries remain in `source_access.json` and the required [board protocol](../../../../curriculum/guides/interrupt-handoff-board-protocol.md). T29.07–.10 and later domains remain required implementation work; this package does not claim them complete.
