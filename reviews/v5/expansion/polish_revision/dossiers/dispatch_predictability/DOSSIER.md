# When can a processor learn a dispatch sequence?

The historical Ryzen 3 7330U campaign compared direct conditional calls, a function-pointer table, a history-hinted indirect call and branch-free arithmetic. Constant, alternating and deterministic pseudo-random selectors distinguish target regularity from call mechanics. The kernels process the same selector state and return the oracle's checksum. Warm-up, table setup, input checks and process creation are outside the measured batch. The fixed CPU 2 user thread counted each event group separately.

An indirect call was slightly faster than the conditional dispatch under constant targets. The ranking reversed under alternating targets. This challenges the prediction that a period-two pattern is easy for every kind of predictor.

| Distribution and comparison | Median change in ns/call | Sign-test interval | Median change in the corresponding branch event per call |
|---|---:|---:|---|
| Constant, indirect minus conditional | −0.059 | −0.059 to −0.058 | One fewer retired branch, almost unchanged mispredictions |
| Alternating, indirect minus conditional | +6.983 | +6.947 to +7.194 | About one additional mispredicted indirect branch |
| Random, branch-free minus conditional | −2.824 | −2.838 to −2.813 | 0.491 fewer mispredicted retired branches |

All five measurement gates passed for these 31-pair comparisons. The timing row and corresponding branch-group deltas come from the same opens. Different event groups provide separate observations. The observed retirement events count events over the entire batch; attributing each event to a specific instruction would require additional evidence. See [events.json](events.json) and [raw rows](rows.jsonl).

In the recorded disassembly, `dp_kernel_branch` selects two direct calls using `jne`. The plain indirect site at 0x2020 uses `call *(%rcx,%rax,8)`. The hinted kernel introduces `je` before its indirect call at 0x213f. The branch-free kernel still has a conditional loop back edge. Examine the instructions and control-flow edges themselves in [disassembly.txt](disassembly.txt).

For alternating data the hinted kernel reduced mispredicted indirect branches by 0.99985/call relative to the plain table, despite adding a retired branch. That fits a correlation with recent control-flow history. Code alignment, predictor indexing, address aliasing and machine-specific mitigation state are alternative explanations. Another binary with padded or relocated call sites could distinguish some of them. The raw data cannot reveal proprietary predictor state. User-mode elapsed time also responds to load and frequency. Among the 14 undisturbed alternating indirect/conditional pairs the median remained +6.945 ns/call. The rank interval assumes independent pairs; inspect the prior-variant strata before generalizing.

The [predeclared hypotheses](spec.json), [test results](tests.json), [build and source hashes](build.json), [machine and toolchain facts](environment.json) and [analysis of 1,728 raw rows](analysis_rederived.json) bind these statements to the archived author campaign.
