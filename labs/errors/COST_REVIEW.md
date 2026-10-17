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

## Required family comparison

Compare explicit tags versus niches, explicit propagation versus macros, callback composition versus direct calls, and setjmp cleanup versus result returns. Allocation accounting must be rerun through the T66 typed callback adapter before claiming that path-level absence of allocations covers composition. The jump boundary is hosted profile A; other components retain their exact profile declarations.

This is the learner evidence template, not a fabricated measurement report. Actual author observations remain in source-bound campaign logs.
