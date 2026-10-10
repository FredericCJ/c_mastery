# Build a bounded language runtime

This directory is the learner workspace for PR-T46-01 through PR-T46-12. Copy it into a new directory outside the curriculum checkout. `include/cvm.h` is the shared interface; `src/cvm.c` and `src/compiler.c` are compiling, unsolved stubs. The visible tests exercise finite contract anchors. Passing them does not complete the reasoning, independent cases, target observations or delayed transfer in the project briefs.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

The initial tests fail with NOT_IMPLEMENTED. Implement the VM one checkpoint at a time, then the compiler. Run `ctest --test-dir build -R runtime.wire --output-on-failure` for a narrow checkpoint. The wire gate calls only the raw structural codec; verified image wrappers arrive in checkpoint 02, dispatch in 03, call frames in 04, expressions/direct calls in 05 and source control flow in 06. The arena gate does not call collection. `runtime.portability` and `runtime.integration` are cumulative late regressions: run all tests once their mechanisms exist, not as the first checkpoint. `family.json` separates `tests` from `deferred_tests`. Never mark an unsolved test as expected success. The source and bytecode contracts are in `SPEC.md`; staged prompts are in `HINTS.md`; exact project-to-test scopes are in `family.json`.

Once implemented:

```sh
build/cvm-run '(fn main () (+ 20 22))'
build/cvm-compile examples/counter.cvm-src counter.cvm
```

The compiler creates a new output file and refuses to replace an existing one. The CLI error path is separate from compiler/VM semantics. `cvm_load` accepts the emitted file as a byte span. Compiler code stays on the host; only the core and independently audited support helpers enter freestanding targets.

Use a separate build for compiler/sanitizer comparison:

```sh
cmake -S . -B build-clang -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug -DCMASTERY_SANITIZERS=ON
cmake --build build-clang
ctest --test-dir build-clang --output-on-failure
```

`python3 thumb/run.py` builds and runs the learner core on QEMU MPS2 AN386 Cortex-M4 if `arm-none-eabi-gcc` and `qemu-system-arm` are available. It reports actual emulated load/verification/execution and rooted-pair collection. It makes no claim about a physical STM32 board, peripherals or timing. The kernel family supplies the independent ring-3 user-program integration. Capability-unavailable is a pending target claim.

Assessors explicitly select their separate implementation with `-DCMASTERY_ASSESSOR_ROOT=/absolute/path/to/assessor/runtime`. This option is empty by default; the ordinary learner build never discovers solutions automatically. The assessor directory is omitted from learner exports. Viewing it changes the independence of subsequent assessment and must be recorded.

For store integration, compile `adapters/store_host.c` against the accepted store API. Service 1 commits a signed integer under `vm-result`; service 2 reads it. An error after commit can mean uncertain persistence. Recover the store before retrying; the VM cannot infer rollback from HOST_ERROR. Host calls require a separate finite callback policy and may not reenter the VM.

Preserve the input, predictions, build identity, actual tool output, first invalid transition, regression and next retrieval date in your dossier. A returned object reference is external to the VM roots: push it before any allocation that may collect. Reset invalidates every old reference.
