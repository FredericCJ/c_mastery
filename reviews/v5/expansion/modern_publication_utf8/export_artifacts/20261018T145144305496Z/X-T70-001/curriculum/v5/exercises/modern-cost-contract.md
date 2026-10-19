# Modern abstraction cost and enforcement contract

Apply this contract to T65–T76 in addition to each mechanism's exact API.
Submit one evidence row per dimension below, with the relevant operation,
assumptions, bound or measurement, command/source identity, and failure policy.
An inapplicable row needs a mechanism-specific reason. An unmeasured row stays
open; a zero observed count is not a universal guarantee.

| Dimension | Required evidence |
| --- | --- |
| Representation | Handle size/alignment and valid-state encoding |
| Reachable storage | Backing objects, metadata and retention graph |
| Lifetime | Creation, invalidation and destruction events |
| Mutation/aliasing | Permitted simultaneous accesses and rejected traces |
| Allocation | Normal and failure paths, including callbacks |
| Movement/copying | Payload copies, address changes and invalidated views |
| Destruction | Maximum work, recursion and blocking assumptions |
| Dispatch | Direct/indirect calls and inspected generated code |
| Concurrency | Publication, cancellation and reclamation ordering |
| Failure | Surviving state after each partial-progress boundary |
| Compilation | Clean/incremental costs and diagnostic limitations |
| Binary/stack | Link-map sizes and frame/stack bounds |
| Boundary | ABI, process, serialization and version contracts |

Compare mechanisms under equal correctness, ownership and failure contracts.
Use operation counts before timing; report input distribution, warmup, repeated
samples, observer overhead and uncertainty for timings. Do not subtract required
checks or cleanup to make an abstraction appear cheaper. Hosted instrumentation
is a separate build from a strict target image. Inspect implicit compiler calls
and transitive dependencies before claiming a runtime restriction.

For each claimed guarantee label its enforcer: ISO C type/semantic rule,
implementation diagnostic or extension, external proof with stated model,
runtime check, or caller discipline. Provide a counterexample to one stronger
claim. A passing test, a quiet sanitizer, an opaque handle, and an atomic count
each establish different, bounded facts.

This original assessment contract adapts the inspected cost-review dimensions
in research/v5/modern/census/modules/07_engineering_selection.md. That discovery
document does not substitute for the primary references attached to an exercise.
