# C Mastery 2: the implementation route

This edition retains the original assessable coverage and deepens it through a
single growing collection of components. The archived first edition is identified
by Git tag `curriculum-v1-2026-10-10`; `curriculum/migration.json` maps each prior
objective and project into active teaching. Active sources may be corrected and
reorganized without rewriting that historical record.

## A project route through the domains

Use the [project route](project-route.md), not numerical domain order. Start with
the diagnostic and precise C foundations. Introduce warnings, same-failure
reduction, debugger observations, bounds experiments and simple work counts as
soon as their named prerequisites are ready. Later diagnosis and measurement
projects reuse those habits under optimization, concurrency, devices and resource
pressure.

The sequence develops portable components, assembly/ABI evidence, Linux and MCU
adapters, physical communication, complete protocol families and advanced
reasoning. T46 then builds a bytecode runtime, T47 a persistent store and T48 an
educational operating system. Their final integration runs a VM and store in one
user process on the learner's kernel. Previously accepted parsers, containers,
allocators, queues and tests move forward with their contracts.

## Working with the supplied laboratory

Run the laboratory inside Linux x86-64; Windows hosts may use WSL. The Python
runner uses the standard library and is separate from the Sphinx publication
environment. From the source checkout:

```sh
python3 -m cmastery doctor
python3 -m cmastery list --module T46
python3 -m cmastery init PR-T46-01 ../my-runtime-lab
python3 -m cmastery check ../my-runtime-lab --compiler gcc --profile debug
python3 -m cmastery check ../my-runtime-lab --compiler clang --profile asan
python3 -m cmastery report ../my-runtime-lab
# Once its project evidence is complete:
python3 -m cmastery advance ../my-runtime-lab PR-T46-02
```

The first check of an unsolved scaffold is expected to fail its implementation
checks. Read the contract, seal predictions and implement the named mechanism.
The runner records commands, results, environment and source identity under the
learner workspace. A later source edit makes old evidence stale. Missing tools or
an unavailable adapter produce an explicit unavailable outcome.

Family suites can exercise more than one milestone. Passing a suite does not
automatically complete every objective linked to the family. The project-specific
explanation, failure campaign, independent variation and physical claims remain
separate. Assessor implementations are a distinct validation bundle; they are not
copied by the learner initialization command. Deliberately consulting them changes
the independence claim and calls for a fresh transfer exercise.

The [executable evidence directory](executable-evidence.md) lists exact test names,
scope and remaining work. The runner selects the current project's checks; use
`--all-tests` for cumulative family regression. For a second listed family, use
`cmastery init PROJECT EMPTY_PATH --family NAME`. Explicit completed dependencies
are selected with `--dependency runtime=PATH` or `--dependency storage=PATH`.
Use `advance WORKSPACE PROJECT` for the next supported project in that family.
It preserves implementation files and existing fixtures, archives the previous
brief, predictions and latest result in `evidence/history/`, and selects fresh
project evidence. Selection does not itself accept a prerequisite or a completed
project. Keep each independent transfer attempt in a separate workspace.

## Comprehensive completion

Complete every required objective, integration and delayed-transfer gate, including
physical UART, USART, I2C, SPI, RS-232, RS-422, RS-485, CAN, CAN FD and Ethernet
observations. Partial portfolios remain useful records of completed capabilities.
Software, emulator and physical evidence retain separate identities.

The full program is intentionally extensive. Project hours count new work once;
directories and reused artifacts add no duplicate budget. Reading, implementation,
verification, assessment and consolidation are recorded separately. Trial timings
from authors are not learner-time calibration; workload estimates retain that
uncertainty until actual learner observations exist.

## Closing the final system

First execute the runtime and store independently on Linux. Then establish kernel
boot, exceptions, paging, process isolation, scheduling and block ownership. Port
through explicit allocation, console and block adapters. Run a bytecode workload
that commits data, restart the guest and recover the committed result. Inspect
failure and resource evidence at each boundary before defending the whole system.

The required kernel has a deliberately bounded process/device contract. A working
guest is evidence for its pinned emulator and configuration. It does not establish
physical disk power-failure behavior, general OS compatibility or the MCU's
electrical/timing properties.
