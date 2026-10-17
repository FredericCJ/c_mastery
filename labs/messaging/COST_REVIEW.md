# Abstraction cost and enforcement review

Artifact/revision: ___. Compiler/flags/ABI: ___. Workload and contract: ___.
Comparison alternative with equivalent correctness/failure behavior: ___.
For each row state a bound, an observation or a justified non-applicability;
link the exact evidence and record any remaining gap.

| Dimension | Mechanism/operation | Assumptions and bound | Observation and provenance | Failure policy / limit |
| --- | --- | --- | --- | --- |
| Object representation | | | | |
| Reachable storage | | | | |
| Lifetime | | | | |
| Mutation and aliasing | | | | |
| Allocation | | | | |
| Movement and copying | | | | |
| Destruction | | | | |
| Dispatch | | | | |
| Concurrency | | | | |
| Failure | | | | |
| Compilation | | | | |
| Binary and stack | | | | |
| Boundary | | | | |

For each guarantee name its enforcer: ISO rule, diagnostic/extension, external
verification, runtime check or caller discipline. Give a counterexample to a
stronger guarantee. Record measurement units, repetitions, uncertainty and
observer cost. A blank or unmeasured row is open evidence, not zero cost.

## Required comparisons

Compare a shared select lock with a correct registration/recheck alternative; SC deque/sequence ordering with a separately justified weaker implementation; transactions with a coarse mutex under equal atomicity; snapshot pins with copied snapshots; and request-per-item with batched demand. Report fixed struct sizes, retained versions, retry/conflict distribution, scheduling assumptions and tail latency. The stack/context retention during cancellation belongs in lifetime and reachable-storage rows. Count sanitizer and logging overhead separately. Unmeasured rows remain open evidence, not zero overhead.
