# Historical native PMU campaign: bounded author interpretation

This is the original elevated run, preserved byte-for-byte in `original-receipt.json`.
Its original SHA-256 is `768398ef8af6d89f0061f38dde1a7075a345a2f5b2d501863255c22e500482eb`.
`campaign.json` remaps evidence locations while preserving original argv, raw receipt,
all 1833 command logs, all 19 bound source files and the observed ELF binary.
The original gates are immutable. Counting and sampled-ring acquisition succeeded;
the initial standard-perf comparison did **not** validate: perf exited zero but
reported `<not counted>`. This archive remains an honest historical result; the
later control/ACK campaign supplies the repaired standard-tool comparison.

The source-bound author runner verified 31 complete alternating-order pairs for
each of 24 regimes, equivalent-result checks, strict group IDs, positive running
time and cleanup. All 1488 paired count records had running/enabled = 1; that
rules out observed multiplexing in this run, not other measurement error. There
were 102 endpoint CPU changes across the paired records; equal endpoints do not
prove no migration. Every record is retained. CPU placement, peer placement,
frequency and concurrent host load were not controlled. Root UID 0 is an explicit
experiment condition; no global perf policy was changed.

The host was Ryzen 3 7330U, family 19h/model 50h, 4 cores/8 threads, 8 MiB shared
L3, native Ubuntu Linux 7.0.0-38, GCC 15.2.0, unsanitized `-O3 -g`.
`logs/cpu.txt` and the environment platform/build/compiler objects retain
these identities. The ELF hash is
`3858a9d8946cf3c882d73e204773704ecd3b9c947be69fa9c94c3d5fc831d8d1`.
Only caller-thread user instructions and cycles were grouped; kernel work and
the concurrent peer were excluded. Wall duration has its own recorded bracket.

The following largest-regime medians are per repetition; search means a batch
of 256 queries, concurrency means two counters each receiving n increments,
and service means one complete n-deadline lifecycle. V0/V1 identify the exact
variants below. CPI is median within-observation cycles/instructions, not the
ratio of two table medians. Exact raw counts, scaling fractions, calibration,
paired differences and all smaller regimes are in `analysis.json` and `summary.csv`.

| Family | n | V0/V1 time (µs) | V0/V1 instructions | V0/V1 CPI |
| --- | ---: | ---: | ---: | ---: |
| allocation | 4096 | 75.258 / 1.125 | 1485507 / 22850 | 0.215 / 0.208 |
| concurrency | 65536 | 1356.667 / 196.883 | 165460 / 165457 | 33.146 / 2.984 |
| dispatch | 262144 | 440.280 / 498.736 | 3670357 / 5243211 | 0.501 / 0.401 |
| layout | 262144 | 105.163 / 642.557 | 1048675 / 1048677 | 0.422 / 2.624 |
| numeric | 1048576 | 312.260 / 282.718 | 2621790 / 2097508 | 0.505 / 0.555 |
| search | 65536 | 2073.804 / 12.183 | 43500042 / 46407 | 0.204 / 1.110 |
| service | 1024 | 730.261 / 301.644 | 11903788 / 4519119 | 0.261 / 0.282 |
| sort | 1024 | 125.444 / 16.194 | 2277741 / 304460 | 0.234 / 0.224 |

## What these interventions establish

**Search:** V0 linear and V1 binary preserve lower-bound results. At n=65536,
binary needs dramatically fewer retired instructions despite greater CPI. Both
retain data-dependent branches in `execute` (`0x3b0d` linear and `0x3ef4` binary).
The dominant instruction-count explanation is supported for this query corpus;
“binary wins because CPI is lower” is refuted. At n=16 the benefit is modest
(25/31 pairs); no portable crossover or branch-miss cause is established.

**Stable sorting:** V0 insertion and V1 merge retain actual `algo_insertion`
and `algo_merge` calls at `0x3e60`/`0x3ce5`. At n=1024 merge executes about 7.5
times fewer instructions and wins all pairs, supporting reduced algorithmic
work on this repeated-key corpus. n=16 favors insertion descriptively but its
batch failed the bracket target and remains provisional. Results do not prove
that every input distribution has this crossover or that scratch is free.

**Layout:** V0 8-byte packed and V1 64-byte wide records use scalar loops with
8-byte/64-byte address steps (`0x3a9b`, `0x3ea8`). At n=262144 their instruction
counts differ by less than 0.001%, but the wide layout has substantially greater
cycles, CPI and elapsed time. The 2-MiB versus 16-MiB representation intervention
supports a memory-access-cost explanation and rejects instruction-count-only.
Cache level, miss count, prefetching, TLB activity and bandwidth attribution
remain unresolved because those events were not measured. n=256 is provisional.

**Allocation:** V0 allocates each value separately plus a pointer array; V1
uses one block. Both measure complete allocate/initialize/sum/free lifecycles.
Disassembly retains per-object `malloc`/`free` calls (`0x3d59`, `0x3d96`), while
block loops are vectorized. At n=4096 the block needs far fewer instructions
and wins every pair. Reduced allocator calls, indirection and compiler loop
changes jointly explain this implementation contrast; the data cannot isolate
a pure allocator cost. n=16 and n=256 failed the calibration target.

**Numerical reduction:** V0 one source accumulator and V1 four source
accumulators are both vectorized. V0 has `paddq` at `0x3a2f`; V1 uses two vector
accumulator chains at `0x3de5` and `0x3de9`. V1 has fewer instructions and wins
24/31 pairs at n=1048576, with smaller improvement than at n=8192. This supports
an instruction/dependency change for the compiled loops. “Scalar versus SIMD”
is refuted, and a unique latency-versus-bandwidth cause is unresolved. n=256
remains provisional. Higher CPI in V1 at the largest size does not imply slower
completion when instruction count is also lower.

**Dispatch:** V0 calls retained noinline direct functions; V1 uses an actual
indirect call (`0x3c64`), so optimization has not erased the intended distinction.
The values are even and the repetition index controls parity: the target is
constant within each repetition. This is a predictable dispatch experiment.
V1 executes more instructions and loses 29/31 pairs at the largest size;
extra dispatch work is supported. A branch-misprediction or unpredictable-target
claim would exceed this workload and its measured events.

**Concurrent state:** V0 adjacent atomic slots and V1 separated 64-byte-aligned
slots have the same relaxed atomic update and thread lifecycle. `increment`
contains `lock addq` at `0x8e63`; the caller retains the same locked updates
(`0x8f68`, `0x8f80`, `0x8f85`). The binary aligns its stack to 64 and selects
adjacent offsets 0x40/0x48 versus separated 0x80/0xc0. Almost identical caller
instruction counts with much lower padded cycles and elapsed time support
layout-dependent sharing interference. This is consistent with false sharing
on this target, but peer CPU placement and coherence events were not recorded;
it is not a unique attribution or a universal padding ratio. Wall time includes
thread creation/barrier/join, and the caller's counters omit peer instructions.

**Integrated service:** V0 scan and V1 heap feed identical live deadlines into
the real segmented parser, copied event queue and independent sum consumer;
slot multiples of five are cancelled before dispatch. Calls to `tc_begin`,
`ev_feed`, `evq_pop`, `evs_event`, `ev_finish` and `tc_finish` remain visible in
`pf_service_work` (`0x9254` through `0x9461`). At n=1024 heap reduces instructions
by about 62% and wins 30/31 pairs, supporting reduced deadline-selection work
inside the same integrated lifecycle. At n=8 the paired delta is near zero
and 16/31 favor heap: the ranking is inconclusive. This is a bounded in-process
service, not socket throughput, real-time deadline assurance or WCET.

## Samples and measurement limits

The sampled workload actually runs the numeric n=65536 reduction 256 times;
the supplied sample adapter fixes that batch independently of the count-mode
repetition argument. It captured 86 samples with no reported loss or unknown
record, positive equal enabled/running times, and `precise_ip=0`.
`analysis.json` translates actual process mappings through the archived ELF
LOAD segments and resolves all 86 addresses: 85 at `execute+0x113` (ELF 0x3a33,
loop compare), one at ELF 0x3a2b (pointer advance). The retained
`attribution-elf-loads.txt` and disassembly make the conversion reviewable.
This locates the hot loop; skid prevents interpreting the compare instruction
as the cause of the cycles. A small sample set and no lost records do not prove
all activity was captured or that the period would be safe at another rate.

Allocation n=16/256, layout n=256, numeric n=256 and sort n=16 reached the
bounded 1024-repetition cap below the calibration target. Their results remain
provisional; no selective deletion or enlarged resource budget repairs them
retroactively. All larger-regime diagnoses above use calibration-sufficient
records. Standard-perf was invalid in this original run, independently of these
successful local hardware records. There are no raw branch/cache/top-down/IBS
claims, no portable counter constants, and no independent learner assessment.

The status of this document is **author-reviewed, bounded interpretation**.
It assesses stated hypotheses against source, disassembly and actual counters;
unresolved causes are explicitly inconclusive. It does not change the original
campaign's `microarchitectural_diagnosis` or `independent_learner` gates.
Reproduce derived integrity checks with:

```sh
python3 tools/analyze_native_pmu.py --archive reviews/v5/expansion/native_revision/pmu_hardware
```
