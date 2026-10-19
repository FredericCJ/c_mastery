# T29.01/.02 author validation

Current source-bound campaign: **208 checks passed**. Native GCC/Clang O0/O3
and ASan+UBSan run the reference, six compiled semantic controls and learner
stub. Each profile checks12288 priority partitions and41472 selection cells
against the separate bit-string oracle. Eight positive M4 images execute16
FP/lazy/padding/grouping cases each; three compiled negative images fail the
named frame property. Final images have no undefined symbols and use only
explicit original runtime objects. Each positive image supplies object imports,
map, disassembly, static usage and QEMU interrupt log; every trace has24 explicit
tail-chains. FPCAR is checked against the captured lazy frame's S0 address.

The architectural late-arrival model passes its two hand-derived traces and
rejects two wrong context/tail-chain variants. It does not inject a measured
mid-stacking event into physical hardware. Seven actual CLI export operations
pass: init, native stub/reference runs, target stub/reference builds and runs.
Private material is excluded by path and whole-file content identity. Reference
providers are explicitly selected; no learner acceptance is inferred.

Evidence guard:18 negative controls passed. Exact original-task/delayed and
six-dimension package validation is recorded in completion_dispositions_validation.json.

Commands (repository root, WSL tools; Windows Python used for pure generation):

```sh
python3 tools/close_interrupt_frame_depth.py --integrate
python3 tools/run_interrupt_frame_depth.py --report reviews/v5/expansion/legacy_advanced_frame/current_validation.json
python3 tools/check_interrupt_frame_depth.py --output reviews/v5/expansion/legacy_advanced_frame/support_validation.json --self-test
python3 tools/check_interrupt_frame_exports.py
python3 tools/close_interrupt_frame_depth.py
python3 tools/check_interrupt_frame_package.py --output reviews/v5/expansion/legacy_advanced_frame/completion_dispositions_validation.json --self-test
```

Exact tool versions, commands, exit statuses, markers, source hashes, raw logs
and object/output identities are in current_validation.json, support_validation.json
and exports.json under `reviews/v5/expansion/legacy_advanced_frame/`. Current
GCC15.2/Clang21.1.8, ArmGCC14.2, QEMU10.2.1; C17 portable core and explicit
GNU-compatible Arm assembly/architectural target contract. `.su` plus the actual
closed direct-call graph gives these conservative bounds, not observed watermarks:

| Profile | MSP normal two-level upper bound (bytes) | PSP hardware maximum (bytes) |
|---|---:|---:|
| arm-none-eabi-gcc-O0-fp0-v0 | 480 | 108 |
| arm-none-eabi-gcc-O0-fp1-v0 | 552 | 108 |
| arm-none-eabi-gcc-O3-fp0-v0 | 496 | 108 |
| arm-none-eabi-gcc-O3-fp1-v0 | 592 | 108 |
| clang-O0-fp0-v0 | 528 | 108 |
| clang-O0-fp1-v0 | 600 | 108 |
| clang-O3-fp0-v0 | 356 | 108 |
| clang-O3-fp1-v0 | 452 | 108 |

The bounds include asynchronous B hardware state and compiler frames; they exclude
unexpected-fault recovery and future interrupt/callback additions. Per-function
maxima may overcount outgoing argument space that is not simultaneously live.
All are below the configured reserve; this does not establish a physical WCET.

`development_validation.json` retains the preceding208-check attempt with one
failed expectation: the extended-offset mutant reached a valid-looking xPSR in
old FP words and was rejected at interrupted-PC range, not at the structural
parser check predicted by the initial campaign. The final campaign requires that
specific PC-range rejection and reruns all resulting sources. The model/target
itself was not weakened. Early manual pilots are under build/frame-pilot and
build/frame-public-pilot; they are development artifacts, not closure evidence.

Teaching, implementation and bounded author execution are supported. Non-author
technical review, independent ordinary/delayed learner performance and physical
board observations remain pending. Exact final ISO source access remains an
inherited gap; no older draft is presented as C17. Later T29 projects remain
required and are not discharged by this frame package.
