# What does each useful key cost?

**Historical observation.** On CPU 2 of the Ryzen 3 7330U, the AoS64 loop took an additional 0.195 ns/key over SoA at 2,048 keys. At 2,097,152 keys the difference grew to 2.744 ns/key. These are paired medians for the five-counter fills group.

The variants sum identical unsigned keys. Element initialization, allocation and the warm-up scan take place outside the window. Each tested layout must match the oracle; every timed pair checks equivalent work. The SoA key array advances by eight bytes. AoS16, AoS64 and AoS128 place the key at the beginning of each wider element. A cache-line hypothesis predicts an increasing penalty as the working set crosses cache capacities. An instruction-count explanation predicts substantially different loops. GCC 15.2 at `-O2` emitted scalar `add (%rax),%rdx` and `add $0x8,%rax` in `rl_sum_soa`. The corresponding hot instructions in `rl_sum_aos64` are `add (%rdi),%rax` and `add $0x40,%rdi`. Inspect the recorded binary's loop bounds and scalar accumulator in [the disassembly](disassembly.txt).

| Keys | AoS64 minus SoA, ns/key | Sign-test interval | Largest additional fill source per key |
|---:|---:|---:|---|
| 2,048 | +0.195 | +0.142 to +0.199 | Local L2, +1.002 |
| 32,768 | +0.370 | +0.345 to +0.386 | L3 or another L2 in this CCX, +0.951 |
| 2,097,152 | +2.744 | +2.737 to +2.754 | Local memory or I/O, +0.890 |

All three comparisons passed acquisition, scheduling, timing, comparability and interpretation. In the smallest regime the instruction difference was −0.0015/key and the user cycle difference was +0.840/key. Sparse access incurred a cost even when the large memory source contributed almost no additional fills. Event 0x44 with source masks 0x01, 0x02 and 0x08 includes prefetch traffic. Only the measured user thread contributes counts, and a fill's reported source does not identify a stalling instruction. The source classifications and Linux PMU encodings are recorded for this processor in [events.json](events.json).

A counterexample to a simple line-count model is the large AoS64/AoS128 comparison. Both touch one useful key per visited line. AoS128 still added 1.760 ns/key and 0.01562 L2 DTLB misses/key in the separate translation group. That group and the prefetch group measure different repetitions. Page translations, prefetch coverage and address span co-vary, so the extra time cannot be apportioned to a particular event. Changing page size while holding the sequence of key addresses constant would test the translation explanation.

There were 31 alternating-order pairs per condition. For the largest SoA/AoS64 fills regime, excluding pairs with an in-window involuntary switch left 27 pairs and a median of +2.747 ns/key. Each group records load and calibration brackets; carry-over can compromise the independence assumption of the interval. The raw [1,510 rows](rows.jsonl), [source and executable hashes](build.json), [host and toolchain](environment.json), [predeclared controls](spec.json), [tests](tests.json) and [rederived analysis](analysis_rederived.json) describe this historical build.
