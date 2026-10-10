# Reproducible laboratory and repository

Use Linux x86-64 for hosted experiments, GCC and Clang as complementary compilers, and C17 as the default dialect. WSL is a suitable host arrangement. Record distribution, compiler, libc, linker and kernel versions. The curriculum's publishing environment is separate from the learner's C toolchain.

## Learner repository

```text
c-lab/
  contracts/           # dependency and interface contracts
  include/             # self-contained public headers
  src/core/            # portable mechanisms written in C
  src/linux/           # syscall/POSIX adapters and application
  src/firmware/        # MMIO adapters, vectors, startup and application
  tests/unit/
  tests/integration/
  tests/negative/      # expected compile/link failures
  fixtures/            # inputs plus separately stored oracle results
  platform/            # linker scripts, startup assembly, target descriptions
  evidence/PROJECT-ID/ # commands, versions, reports, review and limitations
  cmake/               # optional toolchain files and presets
  CMakeLists.txt
  Makefile
```

Choose Make or CMake for the first implementation, then reproduce one component with the other to investigate dependency semantics. The build files are learner work. Keep target objects in separate output directories; never reuse a hosted object in a cross build. Header dependencies must rebuild affected objects. Generated headers need explicit producing rules and dependencies.

## Build/test profiles

| Profile | Suggested investigation | Evidence |
|---|---|---|
| Debug hosted | C17, `-O0 -g -Wall -Wextra -Wpedantic`; add focused warnings as supported | Command, diagnostics, debugger record, tests |
| Optimized hosted | `-O2 -g`, then selected `-O3`/LTO experiments | Results, assembly differences, unchanged contracts |
| Sanitized hosted | Address/undefined sanitizers together where supported; thread sanitizer separately | Instrumented paths, tool limitations and minimized defects |
| Portable component | Two compilers, no external library calls in the component | Object symbol audit, harness tests, target comparison |
| Custom Linux entry | Own entry/adapter and explicitly selected link suppression flags | ELF and syscall dossier; no accidental CRT/libc assumptions |
| Cortex-M | `arm-none-eabi` toolchain, exact core/FPU/float ABI flags and linker script | Map, symbol/section/segment audit and reset trace |
| C23 companion | Explicit supported C23 dialect and feature-specific probes | Compile results separated from library/runtime availability |

Suggested warning flags are starting points, not portable assertions. Do not impose `-Werror` on third-party headers indiscriminately. Negative tests should check that a diagnostic occurs for the intended constraint; exact wording changes by version. Preserve literal commands and full exit status.

CMake presets should name a generator, output directory and toolchain, with separate debug/release/sanitized/cross configurations. Export compile commands for inspection. Investigate installation/exported usage requirements with a second consuming repository: an in-tree successful build is not proof that a library package is usable. Do not copy the curriculum's Python publishing dependencies into a learner C project.

## Continuous verification

Run a small matrix on every meaningful change: GCC/Clang C17 unit tests, one sanitizer job, fixture regression, header self-containment and component symbol allowlists. Run cross compilation without hardware. Run actual-board tests separately when connected. Cache build tools deliberately and record resolved versions. Retain a failing case and logs as artifacts. A clean rebuild is necessary after changing toolchains, ABI or compile-time configuration.

Add Valgrind where the chosen host and program are supported; explain its overlap with and differences from compiler sanitizers. Use GDB or LLDB for minimized failures. Collect coverage and targeted mutation evidence after defining useful tests, rather than optimizing a percentage alone. Concurrency tests must identify synchronization assumptions and instrumentation effects.

## Firmware and emulation

Record the board marking/revision, schematic revision, MCU marking, flash/SRAM placement, ST-LINK firmware and bridge configuration. The default USART3 virtual-COM and ST-LINK MCO arrangements are document-revision-dependent; verify the actual board. Keep DMA buffers out of F439 CCM: the CPU-only memory path is unsuitable for the selected DMA contracts.

Use QEMU MPS2 AN386 for a separate reset/vector/memory experiment with its own linker map and UART model. Query the installed QEMU machine list and record the version before using it. The research inspected the published MPS2 documentation, not an installed emulator. Do not reuse a STM32 register driver against an MPS2 peripheral.

For first power-up and flashing, preserve the known image and record reset/boot assumptions. Host tests establish portable logic; emulation establishes only modeled behavior; real-board traces establish observations for the tested setup. Timing and electrical conclusions require suitable physical measurements.

## Rebuilding this handbook

The extension adds handwritten x86-64/Thumb assembly, small architecture models, serial/electrical measurements and two physical stack routes. Follow {ref}`guide-enhancement` and the equipment table in {ref}`guide-communication-stacks` before acquiring parts. Record exact interface voltage, connector, transceiver supply and timing contracts. F439 classic bxCAN and an external analog transceiver do not supply a CAN FD controller; the required physical FD route requires its own supported controller and additional inspected setup.

The editable sources are under `curriculum/`, `references/`, `research/`, `fixtures/`, `site/` and `tools/`. Create a Python virtual environment and install `requirements-lock.txt`, then run `build.ps1`. This validates canonical metadata, recomputes fixture expectations, builds Sphinx with warnings as errors and checks local links and source fidelity. `serve.ps1` serves the generated HTML on loopback port 8766. Normal rebuilding does not query the Knowledge library or resource graphs and needs none of their document payloads.

<!-- performance-debugging guide extension -->

## Measurement and fault-experiment builds

Keep native, debug, optimized, sanitized, coverage, profiling and firmware outputs in distinct build directories. A diagnostic runtime or compiler hook changes the binary and its dependencies; compare against the matching uninstrumented artifact. Record source/image hashes, compiler/linker/tool versions, input and clock/counter identities, actual returned statuses and raw reports.

Store controlled fault cases under a separate learner test target, with finite capacities/timeouts and paired repaired controls. Native exhaustion runs in a learner-owned child process with verified child-only limits and supervisor evidence. Undefined behavior has no promised native crash value or diagnostic string. ASan/UBSan and unsanitized Memcheck runs are separate; conditional MSan needs its own supported instrumented environment.

Use permitted per-process profiling and read-only cgroup observations first. A prepared delegated subtree supports the optional quota experiment. Hardware profiling and fault work reuse the recorded target map, drivers, clock and trace ownership. Static/captured-data analysis is available when a capability is missing, but it leaves the corresponding runtime or physical claim open. Follow {ref}`guide-performance-debugging` and the exact project limits.


Edition 2 requires physical closure for every named interface; see {ref}`guide-physical-evidence`. Run the learner CLI and retain its scoped receipts as described in {ref}`guide-second-edition`.
