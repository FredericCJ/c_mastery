# PMU observations on the connected Linux host

The unprivileged Intel Core i9-12900K campaign used logical CPU 2, the `cpu_core` PMU (sysfs type 4), Linux 7.0.0-43-generic and the original host policy. Generic event codes carry the PMU type in their upper 32 bits. A strict pair counts only the calling thread's user-mode instructions and cycles. The separate `perf stat` check selects the same PMU and uses an acknowledged enable/disable window around identical work.

| Observation | Evidence |
|---|---|
| The full GCC/Clang campaign passed 843 checks. Of 24 regimes with 31 pairs, 22 met all five interpretation gates. The smallest allocation and numeric regimes exhausted calibration. | [Per-regime timings and gates](sweep.csv) |
| The numeric cross-check agreed on checksum 34,359,214,080. The course group counted 2,097,627 instructions and 537,387 cycles. A separate perf run counted 2,099,881 instructions and 552,337 cycles, both fully scheduled. | [Source hashes, measurements and checks](author-checks.json) |
| 74 recorded samples mapped to instructions in the exact campaign ELF. All six bounded saturation trials showed throttling, preventing a total loss estimate. | [Full capture](campaign.tar.xz) |
| The learner runner executed a 31-pair layout experiment with the supplied reference explicitly labelled TEST IMPLEMENTATION. At 200,000 cycles/sample the published example observed throttling; at 1,000,000 cycles/sample the kernel reported zero loss and 33 retained samples without a throttle record. | [Build](learner-build.json) · [Paired run](layout-count.json) · [Shorter period](layout-sample.json) · [Longer period](layout-sample-slower.json) |
| The deadline timer-only campaign judged 2 of 36 comparisons supported and 34 inconclusive after compiler-specific timing probes. The eight build profiles verified effective compile, link and runtime policy. | [Deadline raw report](deadline.json.xz) · [Build profiles](build-profiles.json.xz) |

The full capture contains the original receipt, its 1,915 command logs, exact measured ELF and 48 pinned source files. Extract it into a fresh directory to verify the hashes and reproduce the analysis without a live PMU. Run these commands from the repository root.

```sh
p=reviews/v5/expansion/main_integration/pmu
mkdir -p build/pmu-replay/archive
tar -xJf "$p/campaign.tar.xz" -C build/pmu-replay/archive
python3 tools/analyze_native_pmu.py --archive build/pmu-replay/archive --output build/pmu-replay/analysis
python3 labs/performance_followthrough/native.py report "$p/layout-count.json" "$p/layout-sample-slower.json"
```

Separately, an extracted laboratory with no adjacent assessor or tools directory rejected the unfinished scaffold, passed its deterministic protocol using an explicitly supplied reference test provider, built and ran that provider, and rechecked its two-pair smoke observation and the public historical replay. The [author checks](author-checks.json) retain commands and expected exit codes. The connected-host experiment made no independent learner or external assessment.

## Historical Ryzen dossiers

The [representation](../../polish_revision/dossiers/representation_locality/DOSSIER.md), [dispatch](../../polish_revision/dossiers/dispatch_predictability/DOSSIER.md) and [concurrent-state](../../polish_revision/dossiers/concurrent_state/DOSSIER.md) reports use the original machine's event definitions. Reanalysis reproduced all 60 regime/group results from 4,038 raw rows. Each original measured executable is missing. The original compiler inputs are recoverable at handoff commit `1cf5f5cf4`; the final handoff runner differs from the at-capture runner. The current Intel host correctly reported the AMD layout event groups unavailable. Fresh GCC and Clang tests exercised all three driver correctness contracts on the ported source.

```sh
python3 assessor/performance_followthrough/dossiers/check_history.py --git-sources
python3 assessor/performance_followthrough/dossiers/extract_replay.py --check
```

The first command also supports verification and reanalysis without a checkout of the source branch by omitting `--git-sources`. [Historical verification results](historical-recheck.json)
