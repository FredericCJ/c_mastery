# Start here: evidence of mastery

This is an independent-study curriculum for someone who already writes basic C and understands elementary algorithms. The goal is to build small, explainable components, then carry them into a Linux process and a Cortex-M firmware image. Progress depends on evidence, rather than finishing a prescribed number of weeks.

A completed component has a contract, a working C implementation, a correctness argument, a resource model, adversarial tests, and a record of its dependencies. You must be able to explain a failed run and defend a design change. A passing test suite alone does not establish all of these abilities.

## Begin with the diagnostic

Complete the three-hour entry check in {ref}`T01` before choosing its remedial projects. Work without a solution or assistant-generated implementation. You may consult compiler and debugger manuals. Predict results before running them and save the prediction with the actual output. Identify which errors occur during compilation, linking, startup, or execution.

If the diagnostic passes its mandatory checks, retain its evidence and skip its remedial implementation work. Otherwise repair the specific gap and retest on changed inputs. There is no extended introductory course before the shared core.

Numerical work has additional entry checks. Before {ref}`T17`, explain absolute versus relative error, powers of two, a finite geometric sum, vectors and matrix multiplication, and the meaning of a residual. Before filtering, distinguish sample rate from signal frequency and explain convolution of two short sequences. Before scheduling analysis, add rational utilizations and solve a small fixed-point recurrence. Use paper calculations, then independently check them in a hosted harness. Missing mathematics calls for targeted remediation before the relevant project; it does not require postponing all language study.

## Use a project as the unit of study

1. Read the module entry check and the project's contract. Record the selected standard, platform, dependency profile, capacity and failure policy.
2. Predict two normal cases and two boundary cases before opening the supplied expected results.
3. Study only the assigned passages. Write a short explanation in your own words. Mark any claim that depends on a compiler, libc, ABI, OS, or peripheral.
4. Implement the mechanism in C. Use the supplied fixtures and add at least one independent test that could reject your implementation.
5. Inspect failures and generated artifacts. Submit the evidence named in the project, including limitations.
6. Apply the mastery gate. When it fails, reduce the failing case, repair the underlying model, and repeat the gate on a new case.
7. After a delay, rebuild or adapt the component without consulting the prior implementation. Keep the new result and comparison.

Most implementation milestones fit a 4–12 hour working session budget, often spread over several sessions. Reading, assessment and later retrieval have separate budgets. An estimate is permission to narrow scope and diagnose an obstacle; it is not a time limit on learning.

## Independent study without a required mentor

Self-review is structured. Keep predictions dated, conceal fixture answers until the first attempt, and use the rubric in {ref}`guide-assessment`. A week later, select a changed capacity, representation or failure point and defend the result from first principles. Record a short spoken explanation or a written review as if another programmer had to maintain the component.

An external reviewer is useful but optional. If a tool or assistant supplies a complete implementation, that implementation is study material. Reimplement the mechanism independently on a changed contract before claiming its objectives. Assistance with diagnosis is legitimate when the evidence records what was supplied and what you subsequently demonstrated.

## Equipment and boundaries

The shared core needs a Linux x86-64 environment, GCC, Clang and ordinary development tools. A Windows host may use WSL. The firmware branch uses a NUCLEO-F439ZI, its debugger connection, a serial connection and documented safe loopback or test devices. Analog and bus timing experiments need appropriate real instruments; they can be postponed while host and emulated work continues.

QEMU's MPS2 AN386 target is a separate Cortex-M learning platform. It cannot validate STM32F439 registers, pin routing, ADC behavior or board timing. A successful host, model or emulator test supports only its own contract. Hardware mastery gates remain pending until the physical evidence exists.

Advanced kernel work, modern lock-free reclamation, production cryptography, every balanced-tree family and every allocator family are extensions. Core awareness still requires explaining why their assumptions matter.
