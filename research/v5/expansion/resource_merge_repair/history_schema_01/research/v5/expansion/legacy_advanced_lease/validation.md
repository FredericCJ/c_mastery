# Source-bound author validation: T29.07–.08

This package closes author implementation of the six unchanged PR-T29-04
criteria and six unchanged delayed variants. Original IDs, criterion bodies,
baseline bindings and minimum-seven-day conditions remain intact. Actual
learner/delayed performance, non-author review, final ISO source access and
physical board observations are not created by these runs.

Executed in WSL on this checkout using GCC15.2, Clang21.1.8, Arm GCC14.2,
GNU Arm ld and QEMU10.2.1. Exact version output, command argv, return status,
raw logs and source/output SHA256 are in `current_validation.json` under the
matching reviews directory. No random seed: the independent oracle enumerates
all6561 four-event words and26244 transitions in its declared initial state.

Commands:

```text
wsl --exec python3 tools/run_interrupt_lease_depth.py --report reviews/v5/expansion/legacy_advanced_lease/current_validation.json
wsl --exec python3 tools/check_interrupt_lease_depth.py --output reviews/v5/expansion/legacy_advanced_lease/support_validation.json --self-test
.venv/Scripts/python.exe -X utf8 tools/close_interrupt_lease_depth.py --integrate
wsl --exec python3 tools/check_interrupt_lease_exports.py
.venv/Scripts/python.exe -X utf8 tools/close_interrupt_lease_depth.py
.venv/Scripts/python.exe -X utf8 tools/check_interrupt_lease_package.py --output reviews/v5/expansion/legacy_advanced_lease/completion_dispositions_validation.json --self-test
```

The current execution campaign passed269 checks: six native compiler profiles
(GCC/Clang O0/O3 and ASan+UBSan), five compiling core mutants, seven compiling
composition controls, independent symbolic transition comparison, four actual
MPS2 M4 executions, eight F439/H745 selected port closures, address audit
rejection, visibility models and public-stub rejection. The guard rejected20
weakened evidence records. Ten actual exported CLI operations passed, including
both target/native stubs and explicitly selected references, two port builds
and the synthetic capture checker. No private assessor bytes were exported.

The M4 image imports the unchanged original runtime, PdTransfer and cache/peer
providers. Object imports such as memcpy/memset and Clang __aeabi variants are
resolved by the explicit original runtime; final undefined-symbol lists are
empty. F439 port closures contain no H745 cache implementation or fictitious
cache registers. H745 generated addresses include both required maintenance
stores even when the compiler uses MOVW/MOVT or a base plus displacement.
The compiled wrong-register object is rejected after successful compilation;
that is an object-contract check, not a runtime semantic-mutant claim.

Paired virtual-timer measurements retain31 samples per condition/profile,
128 stores and selected barriers per sample, alternating order. Read the
four `object and interval audit` raw JSON outputs for complete samples and
medians. Unit:25MHz QEMU virtual timer, `-icount shift=0,align=off,sleep=off`.
These data compare adjacent duplicate versus single DSB instruction cost;
they are not physical bus latency, a DMA/cache measurement, or a WCET bound.

Development evidence remains in `development_validation.json`:257 checks,
three object-audit failures. Its literal-text search failed to recognize
valid compiler address lowering. The repaired local effective-address audit
retains the same required registers and adds an actually compiled wrong-address
rejection. No validator criterion was weakened. An earlier local composition
pilot caught an incorrectly expected barrier count12; tracing the actual cache
provider gave15 (three maintenance events before enable, one after quiescence,
each with DSB/DSB/ISB, plus release/two acquires). Current positives and the
missing-barrier mutant discriminate that count. Neither pilot is accepted
evidence for the corrected source.

Five named primary source artifacts were inspected and hash-bound in
`resources.json`; source-register metadata is not itself proof of wider reading.
The retained provider/timer source registers are identified as inherited
inspections. Exact physical/reset-domain and ISO gaps remain in
`source_access.json`. The physical template still says `not-run` and is
deliberately rejected as evidence. Its eleven structural checker controls are
synthetic only; the checker does not authenticate captures or verify their
raw-file hashes on behalf of an assessor.
