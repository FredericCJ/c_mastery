# Service mechanism depth

Implement the four mechanisms in src/service_depth.c against include/service_depth.h and CONTRACTS.md. The starter compiles but returns SD_UNIMPLEMENTED. Tests exercise owned synthetic data. Assessor implementations are separate and must not be used as learner dependencies.

```
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Anchors are service_depth.memory, .channel, .files and .journal. The repository author campaign also compiles two semantic mutants per anchor, compares actual C against independent Python state representations, checks cross-core composition, and builds a separate strict Cortex-M4 artifact. No board run or full kernel integration is inferred from that artifact. Match each result to its exact source hash and declared fault model.
