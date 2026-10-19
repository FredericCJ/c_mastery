# Hosted C foundations, debugging and measurement anchors

Copy this directory into an empty learner workspace. It supplies buildable **unsolved** C mechanisms, public finite tests, explicit fault/repair observations, tool recipes and evidence templates for selected existing projects. `family.json` maps each project to its exact covered anchor and remaining work. No test result marks an entire project or objective set mastered.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

The eleven library-mechanism tests initially fail with NOT_IMPLEMENTED. Supplied compiler/language/instrumentation and fault-observation demonstrations can already pass; those passes establish that the observation harness works. Implement `src/foundation.c` independently, keeping `include/foundation.h` compatible. The assessor directory is separate and excluded from learner exports; an assessor must explicitly supply `-DCMASTERY_ASSESSOR_ROOT=/absolute/assessor/foundations` to select it.

Use focused tests in the sequence below and link the new evidence to the original project brief. Reading and delayed assessment retain their existing budgets; this package is a reusable executable support layer, not another complete set of project hours.

| Mechanism | CTest anchor | Contract and distinguishing case |
|---|---|---|
| Prefix sums | foundations.prefix | At most 16 int32 inputs; reject an unrepresentable intermediate before evaluation and publish no partial output. INT32_MAX,1,-1 must fail although its final mathematical sum fits. |
| Size calculation | foundations.size | Check count×width+extra before evaluation and preserve the old result on failure. Zero-width policy is explicit. |
| Bounded cursor | foundations.cursor | Validate remaining extent before advancing; an empty span never dereferences a null pointer. |
| Owner and view | foundations.owner | Fixed 32-byte owner; append stages bytes and permits overlap. Mutation increments generation; old borrowed views become stale. This chosen overlap policy must be reconciled with a project's selected alternative. |
| Strict decimal | foundations.parse | Optional minus, one or more ASCII digits, full-span consumption, no plus/whitespace; checked int32 and unchanged result on failure. Tokenization, application range and allocation adapter remain separate. |
| Stable sorting | foundations.sort / properties | Stable insertion sort, explicit comparison/shift/final-placement counts; duplicate-key order and an independent inversion-pair certificate. |
| Partial output | foundations.io | Positive accepted prefix, bounded retry, zero progress, terminal error and impossible over-report; callback never gains ownership of source bytes. |
| Cancellation | foundations.async | Request identity and in-flight ownership; cancel request keeps the operation pending until confirmed completion, exactly one release. |
| Construction | foundations.cleanup | Acquire three named callback resources; every failing prefix rolls back in reverse order. This anchor assumes infallible release; fallible cleanup/cause precedence remain project work. |
| Authorized packet | foundations.packet | Opcode/count plus big-endian uint16 values; at most four values, opcode 2 requires authorization, reject before publishing output. |

`foundation-language` is a supplied observation of rescanning/stringification, typed single-evaluation wrappers, `_Generic`, layout and initialized objects. Predict its output first. `foundations.compile` checks required C17 diagnostic cases without executing undefined expressions. `python3 capabilities.py --cc clang` reports bounded installed C17/C23 feature probes; unavailable features remain unavailable and do not silently select a new language baseline.

## Debugging and memory experiments

`drills/admission.c` is a defined-state fault: an admission mutates count beyond capacity before a clamp hides it. The repaired build checks before mutation. Both finish with count 4; only the event history distinguishes them. `drills/reduce.py build/admission-faulty NNANNAN` executes the real child with PASS/same-FAIL/UNRESOLVED classification and produces a deletion-1-minimal trigger. Reduction does not itself prove causality.

Run `python3 observe.py --build build --report evidence/observations.json` to collect actual bounded GDB, timer, I/O, compiler-hook, section and capability observations. GDB stops at the first watched count change. These visible defects are guided practice; use `templates/diagnosis.md` and a genuinely withheld new variant for independent assessment.

For isolated memory observations, configure a separate Clang build:

```sh
cmake -S . -B build-memory -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug -DCMASTERY_MEMORY_DRILLS=ON
cmake --build build-memory
ctest --test-dir build-memory -R foundations.memory --output-on-failure
```

Each faulty child has a paired repaired child and a five-second bound. Cases exercise heap-buffer overflow, use-after-free, stack-local buffer overflow and a lost-owner leak. Undefined behavior has no promised native symptom. The leak case records the default conservative result, then makes a **separately labeled reduced-root comparison** with `use_stacks=0:use_registers=0`; stale stack/register bits can otherwise retain the lost allocation. This setting is meaningful only for the closed child with its intentionally discarded owner and cannot establish correct ownership in a general application. Detector silence remains evidence to explain. Stack-buffer corruption is not stack exhaustion; these tests do not claim to demonstrate `.bss` capacity failure, physical stack high water or every lifetime defect.

## Measurement and package boundaries

`foundation-benchmark` reports five separate concerns: input/repetition count, abstract comparisons/shifts, monotonic elapsed time, process CPU time and a consumed result. Its timing boundary includes setup and instrumentation; a single batch cannot prove asymptotic complexity or a tail bound. `observe.py` retains five raw samples, rather than asserting a platform-independent speed threshold. Compare alternative builds only with their configuration and ordering disclosed.

`foundation-file-io` writes a private temporary file, records positive accepted bytes, requests fsync, reads the same bytes back and cleans up. It claims byte equality and a successful synchronization request on that environment, not power-loss durability. PMU and cgroup observations are separate. The cgroup route reads the current process membership and visible accounting files and never writes controller settings; PMU permissions may leave that claim open.

The compiler-hook demonstration uses `-finstrument-functions` on a bounded toy program and accounts for entering main before its return hook. It is a changed program, not transparent timing. A meaningful overhead comparison, stack-path bound, cache/traffic model and every physical MCU cycle claim still require the original project experiment and `templates/experiment.md`.

```sh
cmake --install build --prefix /absolute/private-prefix
cc -std=c17 consumer.c -I/absolute/private-prefix/include -L/absolute/private-prefix/lib -lfoundation -o consumer
```

Compile a separate consuming directory; do not count an in-tree link as installation evidence. The header also offers C linkage for a C++ consumer. `templates/proof.md` supplies an invariant/termination/frame record and the bounded CBMC harness command. A harness is not an executed proof; absent CBMC/Frama-C capabilities remain open. Record every bound and unresolved proof obligation.

The source-reading authority remains the exact bounded passages assigned by the existing modules. These original harnesses do not expand those reading scopes or replace the project-specific implementation, manual defense, independent test, target experiment or delayed retrieval.
