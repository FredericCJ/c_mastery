# Worked example – two atomic counters on the same cache line

Suppose two threads each increment their own counter. A process-level stopwatch includes allocation, thread creation and synchronization. To test whether the counter addresses change steady-state cost, keep the worker loop and number of increments fixed. Change the distance between counters. Predict how the result should differ when the threads run on different cores and when they share a core.

The public worked driver lives in `dossiers/concurrent_state/`. The contract checks each final value including warm-up and tail increments. Each worker sets its affinity, opens its own user-space counter group, warms up and waits at a start barrier. The timed loop calls the same relaxed atomic increment function in every layout. Record the actual CPUs, intervals and context switches. If the other worker finished early, its tail interval indicates how much of the measured window still had a contending peer.

Here is one pair from an archived author run on a Ryzen 3 7330U. The measured thread ran on CPU 2 and its peer on CPU 4. Both groups scheduled for their full enabled time. The original executable SHA-256 is `ea4879881edff3548f2ceb581aed434cbc507fe01a976d992aa55a7cdf8020ef`.

| Counter placement | Measured increments | Elapsed ns | Running/enabled ns | Retired instructions | L3/peer-L2 demand fills |
|---|---:|---:|---:|---:|---:|
| Adjacent slots, one 64-byte line | 262,144 | 6,427,143 | 6,438,455 / 6,438,455 | 655,716 | 152,176 |
| Slots 128 bytes apart | 262,144 | 370,718 | 383,403 / 383,403 | 655,703 | 3 |

Subtract the first elapsed time from the second and divide by 262,144. The difference for this pair is −23.10 ns/increment. Inspecting `cs_increment` in that build showed `lock addq $0x1,(%rdi)` in both variants. The instruction count hardly changed. On these cores, the demand-fill event includes fills from the shared L3 **or** a different core's L2 in this CCX; its value alone cannot tell which path served any given load.

Repeating the comparison with alternating order gave a median difference of −22.638 ns/increment across 31 accepted pairs. Repeating it on SMT siblings produced −11.549 ns/increment with almost no difference in those fills. That second result challenges the idea that transfer between private caches accounts for the whole delay. Serialization inside the shared core is a hypothesis requiring another experiment. Neither interval establishes a worst-case deadline. The C atomic contract describes correct values under concurrency; the particular instruction and observed costs belong to this build and CPU.

## Reproduce the method

From `labs/performance_followthrough`, use fresh output directories. These commands show the deterministic test and local hardware paths.

```sh
python3 dossiers/run_dossier.py test --spec dossiers/concurrent_state/spec.json --pmu-source pmu.c --out build/dossier-tests
python3 native.py doctor
python3 dossiers/run_dossier.py run --spec dossiers/concurrent_state/spec.json --pmu-source pmu.c --out build/campaign
python3 dossiers/run_dossier.py analyze --dir build/campaign --output build/rechecked.json
```

The checked-in spec describes the archived AMD PMU and topology. Read the event descriptions and placement premises first. On a different processor the runner can report unavailable event groups or refuse a placement. Use `native.py run` with your completed component for a separate generic-event experiment on that machine. Report each experiment's binary, placement, event group, paired raw rows and uncertainty.

## Practice with a different mechanism

The compact `dossiers/replay/layout.json` provides historical rows for a representation experiment. It contains the measured event identities, counts, calibration and 31 paired comparisons, and excludes an interpretation. Start by predicting what a longer stride could change. Compare the per-key instruction, fill and elapsed differences within each pair. Inspect schedule coverage, timing thresholds and checksums. Then run the public reader and reconcile your estimates.

```sh
python3 native.py report dossiers/replay/layout.json
```

Use `row.raw[i]` with `events[i]` and the matching `row.ids[i]`. IDs are local to a particular open. Divide elapsed time by `units` for ns/key, or by `repetitions` for ns/whole scan. Keep replay analysis, your own hardware observations and independent assessment as separate entries in your submission.
