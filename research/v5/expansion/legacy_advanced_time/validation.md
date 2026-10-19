# PR-T29-05 author execution record

The source-bound campaign passed229 checks in WSL Linux using GCC15.2,
Clang21.1.8, Arm GCC14.2, GNU Arm ld and QEMU10.2.1. Exact versions, uname,
argv, exit statuses, expectations, input identities, raw output paths/hashes and
all generated artifacts are in
`reviews/v5/expansion/legacy_advanced_time/current_validation.json`.
This record describes author references, not independent learner work.

Executed producers:

```
wsl --exec python3 tools/run_interrupt_time_depth.py --report reviews/v5/expansion/legacy_advanced_time/current_validation.json
wsl --exec python3 tools/check_interrupt_time_depth.py --output reviews/v5/expansion/legacy_advanced_time/support_validation.json --self-test
wsl --exec python3 tools/check_interrupt_time_exports.py
```

The campaign covers GCC/Clang O0/O3 and ASan+UBSan profiles; five compiling semantic
mutants and the incomplete public stub per profile; an independent Python oracle
per profile with2100 coupled-counter histories,4096 checked calibrations,
400sequence cases,1296 interval-order cases and360event-delay set cases.
Calibration inputs use deterministic LCG seed0x58b49d23; the other spaces are
explicitly enumerated in `probe.c`. The oracle uses actual modeled middle instants,
big integers and possible-value sets rather than the implementation's concatenation
or quotient/remainder code. Enumeration remains bounded.

Actual Xi, Xd, CPFClock and Dp sources execute six profiles with four discriminating
provider mutations. Six native and two TSan runs publish5000 complementary
high/low values through the actual SC sequence clock. TSan reports only those runs.
The explicit odd-writer schedule also checks bounded refusal and retirement before
sequence wrap; it does not depend on a scheduler happening to expose the state.

Four MPS2 M4 images execute7 real timer interrupts each, retain4records/drop3,
check acquisition intervals and execute4large checked-arithmetic boundaries.
Both compilers O0/O3 import the original T11 unsigned division helper; omission
of its already-compiled provider objects produces the required unresolved-helper
link rejection. Final ELF undefined-symbol sets are empty. Object imports,
assembly, linker maps and stack-usage files are retained. No libc/libgcc/compiler-
rt/libatomic or HAL is linked. Memory support and the division ABI wrapper are
original repository implementations. Discarded unrelated CPF functions are not
claimed to have been exercised merely because their translation unit was compiled.

| Whole teaching image | text bytes | vector bytes | BSS bytes |
|---|---:|---:|---:|
| Arm GCC O0 |16072|1024|416|
| Arm GCC O3 |11888|1024|416|
| Clang O0 |14748|1024|416|
| Clang O3 |13316|1024|416|

These are image costs including diagnostics, original helpers and actual providers,
not isolated abstraction overhead or board performance. Raw section sizes and
compiler stack-usage records support a learner's separate call-path/stack budget;
an ELF debug-section total is not flashed code size. The target intervals use the
inspected25MHz virtual timer and fixed instruction clock. They do not establish
silicon timing, a64-bit hardware counter or a worst-case latency bound.

The first development campaign retained227checks and failed eight expected
rejections: the queue mutant released at `take`, but the test immediately called
`release`; the touching-endpoint mutant saw only a nonzero-width self interval.
Both were real test-depth defects. The corrected integration pauses the consumer
while full; the target compares intervals `[1,2]` and `[2,3]`. All compiled controls
now reject for the intended assertion. The failed `development_validation.json`
and its raw runs remain available; no failure was relabeled a pass.

The support guard passed22 rejection controls, including stale source/output,
missing oracle/TSan/helper/actual-provider include, lost runtime closure and weaker
negative expectations. The actual CLI export passed8commands: init, public native
stub/reference, exported target stub/reference build/execute and capture checker.
No assessor file/content leaked into the learner tree; private sources were
explicitly selected from outside it. The capture checker rejects the unobserved
template and12synthetic malformed variants, checks raw hashes and interval/event
conservation, and explicitly does not authenticate physical measurements.

Remaining evidence is specific: exact published ISO text, qualified board counter
coupling/reset/clock information, physical marker/analyzer captures, non-author
review, independent learner and original delayed attempts. Their protocols and
assessor analyses exist; their observations do not. All other recursive inventory
work remains required and is not closed by this topic's results.
