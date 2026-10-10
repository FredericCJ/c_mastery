# Integration through learner-owned components

These eight units add adapter and failure behavior to components already built.
Implement `src/integration.c`; preserve the imported implementations.

The first useful path is:

1. Use the bounded command-line tool to produce decimal records.
2. Export the earlier foundation cursor/parser and algorithm ring.
3. In `LAB-IN-01`, combine them into a bounded length-delimited record service.
4. In `LAB-IN-02`, attach a writer that returns partial progress or a retry.
5. In `LAB-IN-03`, feed records one byte at a time and inject malformed lengths.

`LAB-IN-01` accepts either immediate cursor advancement with accepted-prefix
tracking or a lookahead cursor committed after queue admission. The private
reference builds both against the same observable contract. Preserve the
decision rationale and later change queue capacity to test it.

## Import interfaces

The learner workflow can supply verified CMake targets through
`CMASTERY_COMPONENTS_FILE` and role lists such as
`CMASTERY_COMPONENTS_FOUNDATIONS_TARGETS` and
`CMASTERY_COMPONENTS_ALGORITHMS_TARGETS`. Targets export their public includes.
The component file is generated from verified export manifests by the learner
workflow; it is not an instruction to locate private reference code.

For manual experiments, provide absolute source and include directories:

```sh
cmake -S labs/integration -B build/integration-practice \
  -DCMASTERY_FOUNDATIONS_ROOT=/work/foundation/src \
  -DCMASTERY_FOUNDATIONS_INCLUDE=/work/foundation/include \
  -DCMASTERY_ALGORITHMS_ROOT=/work/algorithms/src \
  -DCMASTERY_ALGORITHMS_INCLUDE=/work/algorithms/include
cmake --build build/integration-practice
ctest --test-dir build/integration-practice -R integration.p1 --output-on-failure
```

Foundations exports `foundation.c` and `v4_foundation.c`; algorithms exports
`algo.c` and `v4_algo.c`. Unimplemented later functions may remain buildable
stubs while early units use only their already-accepted interfaces.

The optional `FIRMWARE_IO`, `RUNTIME` and `STORAGE` roles use the same
`CMASTERY_<ROLE>_ROOT` / `_INCLUDE` convention or corresponding component target
list. Firmware adds P4 and the MCU integration; runtime plus storage adds the
Linux appliance integration. Missing roles leave those checks unavailable.
There is no automatic source discovery or assessor substitution.

The `CMASTERY_ASSESSOR_ROOT` option selects only this family's private
orchestrator. Its dependencies must still be explicitly supplied. Mutation
controls require explicit dependency sources; exported target checks assess
the linked contracts and record their own dependency identities.
