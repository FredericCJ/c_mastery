# Native PMU v3: verified acquisition and bounded diagnosis

The elevated native campaign completed on 2026-10-29: strict grouped counting,
sampled-ring acquisition and the repaired standard-perf comparison succeeded.
This is an author observation and interpretation, with independent learner
assessment still open. The raw campaign's diagnosis gate is intentionally
unchanged; `assessment.json` records the subsequent bounded author review.

`original-receipt.json` preserves the original receipt bytes and SHA-256
`5fe30281e4e67559a76aaa3e9f1d1d566bada68804b6512826ccd39b69c6376c`.
`campaign.json` changes only evidence locations and adds archive provenance;
it retains actual executed argv and original locations. All 1822 command logs,
22 bound source files, the observed binary and byte-identical build manifest are
retained. `build-provenance.json` verifies the standalone manifest matches the
campaign's embedded build. The ELF SHA-256 is
`118a6aa0a9b241beacdd0f4c09698b06c78e2b4abbc75f132b2ffcd701f020e2`.
`analysis.json` verifies every log/source/binary/receipt hash before deriving
`summary.csv`; exact counts and raw observations remain available.

The host evidence records Ryzen 3 7330U, family 19h/model 50h, 4 cores/8 threads,
8-MiB L3, Ubuntu Linux 7.0.0-38 and GCC 15.2.0. The binary is unsanitized `-O3 -g`.
The experiment ran as UID 0 without changing perf policy. Each regime contains
31 complete pairs with alternating execution order and warmed variants, equal
results checked against an independent oracle, distinct kernel event IDs and
successful cleanup. Instructions and cycles share the same caller-thread user
group; kernel work and peer-thread events are excluded. All 1488 paired rows
have running/enabled = 1. This shows no observed multiplexing, not absence of
other disturbance. There were 81 endpoint CPU changes. No rows were discarded;
equal endpoint CPUs do not rule out intervening migration. Frequency, peer
placement and concurrent host load were not controlled.

## Standard-tool cross-check and retained failures

Both the focused preliminary campaign (`../pmu_crosscheck_v3/`) and this full
campaign used perf events disabled at startup, explicit enable command/ACK,
the same numeric n=65536, V0, eight-repetition batch, then disable command/ACK.
Both results have checksum 34359214080. In the full campaign:

| Measurement | Instructions | Cycles | Running time (ns) | Running/enabled |
| --- | ---: | ---: | ---: | ---: |
| Local group | 1311296 | 270792 | 74340 | 1 |
| Standard perf | 1312226 | 270513 | 80943 | 100.00% reported |

The instruction difference is about 0.071%; the cycle difference about -0.103%.
This is a useful independent acquisition cross-check with matching work and
positive counts/times. It is not exact equality: enable/disable control-return
instructions are measured, the mechanisms have different overhead, and these
are separate executions on observed CPUs 6 and 4. Perf reports rounded running
percentage; its raw output is retained in `logs/standard-perf.txt`. The workload's
`work` JSON deliberately has local status `empty` and raw local counts zero;
perf owns those counters and the valid values come from perf's own output.
The controlled pipe test also verifies bad ACK rejection without hardware.

The original campaign in `../pmu_hardware/` used prctl-only activation and perf
reported `<not counted>` despite exit zero. The subsequent failed v2 cross-check
in `../pmu_crosscheck_v2_failure/` exposed the real five-byte ACK, `ack\n\0`:
Linux perf includes the string terminator. Its four-byte reader left the NUL
for the next response. The regression test now sends those exact five bytes,
fails against the v2 binary and passes against v3. Neither earlier receipt was
rewritten into success. The adapter matches Linux perf's actual
[acknowledgement implementation](https://github.com/torvalds/linux/blob/v7.0/tools/perf/util/evlist.c)
and [ACK tag](https://github.com/torvalds/linux/blob/v7.0/tools/perf/util/evlist.h).

## Results and competing explanations

These largest-regime medians are per repetition, V0/V1 as defined below.
Search includes 256 queries; concurrency includes n updates per counter; service
includes the entire n-deadline lifecycle. CPI is the median of each observation's
cycles/instructions, not a quotient of medians. A lower CPI alone does not mean
less execution time. The final column counts paired V1 time wins, not a confidence
level. These descriptive results make no population significance or WCET claim.

| Family | n | V0/V1 time (µs) | V0/V1 instructions | V0/V1 CPI | V1 faster |
| --- | ---: | ---: | ---: | ---: | ---: |
| allocation | 4096 | 75.181 / 1.077 | 1485509 / 22852 | 0.215 / 0.190 | 31/31 |
| concurrency | 65536 | 1341.861 / 174.185 | 164654 / 164650 | 33.653 / 2.875 | 31/31 |
| dispatch | 262144 | 440.489 / 502.439 | 3670359 / 5243213 | 0.501 / 0.401 | 2/31 |
| layout | 262144 | 103.697 / 669.952 | 1048675 / 1048678 | 0.420 / 2.631 | 0/31 |
| numeric | 1048576 | 309.187 / 285.092 | 2621792 / 2097510 | 0.494 / 0.575 | 21/31 |
| search | 65536 | 2111.188 / 10.764 | 43494932 / 46417 | 0.207 / 0.962 | 31/31 |
| service | 1024 | 734.102 / 301.505 | 11903790 / 4519121 | 0.260 / 0.283 | 31/31 |
| sort | 1024 | 125.160 / 15.865 | 2277731 / 304450 | 0.234 / 0.220 | 31/31 |

**Search: algorithmic work versus branch/locality cost.** V0 linear and V1
binary preserve lower-bound results. Binary's instruction count falls greatly
at n=65536 despite higher CPI. The binary retains conditional search branches
(`execute`, linear loop at 0x3b8d and binary `jb` at 0x3f74). Reduced search work
is supported; a lower-CPI explanation is refuted. At n=16 binary wins 24/31 pairs
with a much smaller difference. No universal crossover or branch-miss cause
follows from these two events and this deterministic query corpus.

**Sorting: comparison work versus merge overhead.** Actual `algo_insertion`
and `algo_merge` calls remain at 0x3ee0/0x3d65. Merge wins every pair at n=128
and 1024 and executes far fewer instructions, supporting less algorithmic work
on this repeated-key corpus. n=16 descriptively favors insertion, but its batch
failed the calibration target and remains provisional. Scratch cost and other
input distributions were not independently varied.

**Layout: instruction count versus memory-access cost.** V0 packed 8-byte and
V1 wide 64-byte records retain scalar add loops with strides 8 and 64 at
0x3b1e/0x3f2b. At n=262144 the instruction counts differ by less than 0.001%,
yet wide has much greater cycles/CPI and loses all pairs. The 2-MiB versus
16-MiB representation intervention supports a memory-access-cost explanation
and rejects instruction-count-only. Cache misses, cache level, TLBs, prefetching
and bandwidth are unresolved because their events were not counted. n=256
remains provisional and its small ranking changes across the two campaigns.

**Allocation: allocator calls versus layout and compiler changes.** V0 uses
per-value allocations plus a pointer array; V1 uses one contiguous block.
Both include the complete acquire/initialize/sum/free lifecycle. Repeated
`malloc`/`free` calls remain in V0, while block loops are vectorized. At n=4096
V1 wins all pairs and retires much less work. Fewer allocator calls, indirections
and generated loop instructions jointly support this implementation contrast;
the experiment does not isolate allocator cost from the other changed causes.
n=16 and n=256 fail calibration and remain provisional.

**Numerical reduction: dependency structure versus vectorization.** Both
variants are vectorized: V0 has one vector accumulator (`paddq` at 0x3aaf),
V1 two vector chains (`paddq` at 0x3e65/0x3e69), despite four source accumulators.
Thus a scalar-versus-SIMD explanation is refuted. V1 executes fewer instructions
and wins 21/31 pairs at n=1048576 (22/31 at n=8192). The compiled work/dependency
change is supported; unique latency-versus-bandwidth attribution is unresolved.
The largest regime's higher V1 CPI coexists with lower time and fewer instructions.
n=256 is provisional. Variance makes an invariant speedup claim inappropriate.

**Dispatch: retained indirect work versus prediction.** V0 direct noinline
calls remain at 0x3a22/0x3da9; V1 has an indirect call at 0x3ce4. The compiler
has retained the intended control distinction. Values are even and repetition
index sets parity, so the target is constant within each repetition. This is
a predictable-target workload. V1 executes more instructions and loses 29/31
largest-regime pairs, supporting extra dispatch work. Branch-misprediction and
unpredictable-target conclusions are unsupported.

**Concurrent state: instruction work versus sharing interference.** Adjacent
V0 atomic slots and 64-byte-separated V1 slots have identical relaxed atomic
updates and thread lifecycle. The binary retains `lock addq` in the peer at
0x91a3 and caller at 0x92a8/0x92c0/0x92c5; its aligned layout separates the selected
addresses as intended. Almost identical caller instructions but much lower
padded cycles and time support layout-dependent sharing interference. This is
consistent with false sharing on this target. Coherence events and peer CPU
placement were not recorded, so the cause is not uniquely established. Wall
time includes thread creation/barrier/join; PMU counts cover only the caller.
No universal cache-line-size, padding ratio or whole-process count is claimed.

**Integrated service: deadline selection versus unchanged parsing work.** V0
scan and V1 heap dispatch identical live deadlines to the actual segmented
`EvParser`, copied `EvQueue` and independent `EvSum`; multiples of five are
cancelled first. Calls to `tc_begin`, `ev_feed` and `tc_finish` remain at
0x9594/0x965a/0x97a1 with the other queue/consumer calls in `pf_service_work`.
At n=1024 heap reduces instructions by about 62% and wins all pairs, supporting
reduced deadline-selection work in the same integrated lifecycle. At n=8 heap
wins only 13/31, with a 50.75-ns median paired disadvantage; there is no robust
small-regime advantage. This is in-process integration, not network throughput,
real-time scheduling assurance or an observed upper bound on all executions.

## Sampling and unresolved gates

The sample adapter executes numeric n=65536 for a fixed 256 repetitions,
independent of the count-mode repetition argument. It captured 88 samples,
zero reported loss/unknown records, and equal positive enabled/running times.
`analysis.json` translates recorded process mappings through the archived ELF
LOAD segments and resolves every address in `execute`: 86 at ELF 0x3ab3
(loop compare), one at 0x3ab6 (loop branch), and one at 0x3a61 (control branch).
`attribution-elf-loads.txt` and disassembly preserve the identity argument.
With `precise_ip=0`, skid prevents blaming the compare instruction for the
cycles. This locates a hot loop, not its exact cause. No loss in this bounded
run does not guarantee lossless collection at another period or workload.

Allocation n=16/256, layout n=256, numeric n=256 and sort n=16 reached the
bounded 1024-repetition cap below the timer-bracket target. Those five regimes
remain provisional; no hardware success flag erases that limitation. All other
regimes met calibration. PMU control overhead is not separately subtracted.
Advanced branch/cache/raw-event/IBS/top-down attribution remains open, as do
placement-controlled concurrent transfer and independent learner assessment.
Different calibration repetitions can change the query sequence across
campaigns; compare variants within each paired regime and do not pool campaigns.

`assessment.json` marks only these evidenced hypotheses as author-reviewed,
with their remaining alternatives. It does not award delayed transfer,
independent assessment or a complete causal decomposition. Reproduce derived
integrity and attribution checks without privileges:

```sh
python3 tools/analyze_native_pmu.py --archive reviews/v5/expansion/native_revision/pmu_hardware_v3
```
