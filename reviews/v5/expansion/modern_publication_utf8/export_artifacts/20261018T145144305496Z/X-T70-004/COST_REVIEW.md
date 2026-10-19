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

Compare AoS/SoA under equal numeric contracts, inline/fixed/heap byte storage, scalar/SWAR saturation, sparse/archetype updates, full copying/path sharing and rope/gap/piece edits. Count retained snapshots, callback ownership and invalidated borrows as well as bytes copied. Host measurements do not imply freestanding helper absence. Blank observations remain gaps.
