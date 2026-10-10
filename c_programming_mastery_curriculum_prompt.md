# C Programming Mastery: Language, Linux Systems, and Bare-Metal Engineering

## 1. Mission and expected result

Act as a curriculum architect with expertise in ISO C, compiler toolchains,
operating systems, embedded firmware, software architecture, algorithms,
performance engineering, and software verification.

Design a thorough, technically rigorous, implementation-heavy curriculum for
mastering C programming, Linux systems programming in C, and bare-metal
programming in C.

Produce a curriculum that can actually be followed: prerequisite relationships,
sequenced modules, learning resources, exercises, projects, assessment criteria,
and workload estimates. A topic inventory or collection of book recommendations
alone is insufficient.

The learner already understands basic algorithmics and basic C syntax. Treat
this as the entry level, not as the final content of a beginner phase. Use a
short diagnostic and targeted remediation instead of an extended introduction
to programming.

Define mastery operationally. The learner shall be able to explain a mechanism,
implement it, justify correctness and resource bounds, inspect its generated
code and dependencies, diagnose failures, measure its behavior, and defend its
architecture and trade-offs.

Design the curriculum now. Exercise briefs shall require complete learner
implementations where specified; the present deliverable is the curriculum,
not the completed source code for every exercise.

## 2. Scope, environments, and teaching principles

### 2.1 Learning tracks

Organize one connected curriculum with a shared foundation and these tracks:

- Portable C: language semantics, algorithms, libraries, architecture, and tests.
- Linux userspace: hosted programs, POSIX interfaces, Linux-specific facilities,
  and selected libc-free executables.
- Bare metal: startup, runtime construction, peripheral access, interrupts,
  resource constraints, and real-time behavior.
- Advanced electives: kernel programming, additional architectures, advanced
  lock-free algorithms, numerical kernels, and deeper toolchain internals.

Use Linux x86-64 with GCC and Clang as the primary hosted environment. Use an
Arm Cortex-M target as the primary bare-metal environment; prefer the
STM32 NUCLEO-F439ZI for concrete hardware exercises. An RP2040 or a documented
RISC-V target may serve as a later portability exercise.

Choose a specifically supported emulated target for hardware-independent labs.
Verify its machine model and peripheral support rather than assuming it can
emulate an arbitrary development board. Separate emulator-validatable claims
from measurements requiring real hardware.

Use C17 as a portability-oriented teaching baseline, with a parallel C23 track.
Validate this choice against actual compiler and library support at research
time. Study older editions for semantic differences, maintenance, and porting,
without requiring every modern project to remain C89-compatible.

C shall remain the implementation language. Assembly shall support startup,
ABI, interrupt, and machine-level lessons. Build scripts and test automation
may use suitable auxiliary languages; they shall not replace C implementations
of the mechanisms being studied.

### 2.2 Precision requirements

Every platform-sensitive claim shall distinguish among:

1. ISO C guarantees and requirements, identified by edition.
2. Implementation-defined, unspecified, and undefined behavior.
3. Compiler extensions and optimization assumptions.
4. ABI and instruction-set conventions.
5. C-library implementation behavior.
6. POSIX requirements and Linux-specific behavior.
7. MCU, board, peripheral, and execution-context constraints.

Separate language features from library availability. Distinguish normative
standards, drafts, defect reports, proposals, and implementation practice.
Experimental results demonstrate behavior for the tested configuration; they
shall not be presented as proofs of universal language guarantees.

### 2.3 Dependency and allocation profiles

Give every project an explicit dependency contract. Distinguish at least:

A. Hosted C with permitted standard-library use.
B. Allocation-free application/component code with explicitly permitted library
   calls and separately audited transitive behavior.
C. A portable component without external C-library calls, tested through a
   hosted harness that may use the library.
D. A Linux executable with a custom entry point and explicitly declared syscall
   and compiler-runtime dependencies.
E. Bare-metal firmware with custom startup, linker configuration, and an
   enumerated minimal runtime.

For each profile, specify permitted headers, linked libraries, compiler support
routines, startup objects, allocation sources, OS services, and external symbols.
Clarify whether restrictions apply to one component, its transitive dependencies,
or the entire executable.

Treat "without the standard library," "without <stdlib.h>," "without malloc,"
"freestanding," "without startup files," and "without compiler-runtime support"
as separate requirements. Header use alone shall not be equated with a linked
runtime dependency. Verify the actual freestanding requirements by C edition.

Educational reimplementation shall reveal mechanisms and trade-offs. It shall
not imply that replacing mature production libraries is automatically safer,
faster, smaller, or more maintainable.

## 3. Required topic coverage, from foundational to expert

The following is a minimum coverage inventory, not a rigid prerequisite order.
Reorder and subdivide it into teachable modules. Introduce testing, debugging,
contracts, and measurement from the first phase and revisit them throughout.
Label depth as awareness, working proficiency, or mastery; distinguish required
core material from advanced electives.

### Phase I — Precise command of the language

#### T01. Entry diagnostic and foundational consolidation

Assess control flow, decomposition, functions, arrays, basic pointer use,
recursion, algorithmic complexity, integer arithmetic, bit operations, and
compilation. Cover missing prerequisites in Boolean algebra, binary/hexadecimal
representation, basic architecture, command-line work, and version control.

Establish a workflow of small specifications, readable implementations,
automated tests, compiler diagnostics, and reproducible execution. Require
prediction and explanation of program behavior rather than syntax quizzes alone.

#### T02. Declarations, types, expressions, and translation units

Cover declaration versus definition; declarator reading; scope, linkage, storage
duration, and lifetime; identifier namespaces; qualifiers; typedefs; enums;
function prototypes; array parameters; function pointers; variadic functions;
default argument promotions; header boundaries; tentative definitions; and
inline semantics across relevant dialects.

Study precedence versus evaluation order, sequencing, short-circuit evaluation,
side effects, conversions, compatible types, and diagnostic requirements.
Include difficult but realistic declarations and multi-file failures.

#### T03. The object model, pointers, and memory correctness

Cover object representation, alignment, padding, arrays versus pointers,
multidimensional arrays, pointer arithmetic, one-past pointers, object lifetime,
effective type, aliasing, restrict, const-correctness, unions, flexible array
members, VLAs, indeterminate values, and version-dependent representation rules.

Examine dangling pointers, lifetime escape, pointer invalidation, overlapping
regions, struct comparison/serialization hazards, and allocation-size overflow.
Treat pointer provenance carefully, separating adopted rules from proposals and
compiler interpretation. Analyze container_of-style techniques with explicit
portability assumptions rather than declaring every common idiom ISO-portable.

#### T04. Integer, bit-level, and floating-point semantics

Cover integer ranks and promotions; signed/unsigned interactions; narrowing;
overflow; shifts; masks; rotations; bit-fields; byte order; CHAR_BIT; size_t;
ptrdiff_t; fixed-, minimum-, and fast-width types; and optional type availability.

Study checked and saturating arithmetic, fixed-point scaling, IEEE 754 where
applicable, rounding, cancellation, NaNs, infinities, signed zero, subnormals,
fused operations, floating-point environments, and fast-math trade-offs.
Distinguish numerical error, representation error, and language-level defects.

#### T05. Standards evolution and portability

Compare ANSI C89/ISO C90, the C95 amendment, C99, C11, C17/C18, and C23.
Build a feature-and-semantics matrix rather than a chronology of release dates.

Address additions, removals, corrections, optional facilities, feature-test
macros, required diagnostics, and migration hazards. Include designated
initializers, compound literals, variadic macros, inline, restrict, VLAs,
_Generic, atomics, threads, alignment, and static assertions.

For C23, investigate attributes, nullptr, typeof/typeof_unqual, type inference,
constexpr, _BitInt, #embed, __VA_OPT__, checked arithmetic, and bit operations.
Verify spelling, semantics, availability, and actual compiler/library support.
Contrast C meanings with similarly named C++ features where confusion is likely.

### Phase II — Toolchains, binaries, execution, and runtimes

#### T06. Preprocessing and metaprogramming

Cover translation phases, macro expansion/rescanning, stringification, token
pasting, variadic macros, conditional compilation, include guards, reserved
identifiers, and macro hygiene.

Study multiple evaluation, precedence hazards, statement macros, X-macros,
type-specific generation, _Generic dispatch, compile-time checks, configuration
headers, and external code generation. Compare macros, static inline functions,
callbacks, and generated source for type safety, debugging, code size, and
maintenance. Label GNU extensions and newer standard features precisely.

#### T07. Compilation and build engineering

Trace preprocessing, compilation, assembly, archiving, and linking. Introduce
compiler drivers, intermediate representations, optimizer passes, debug info,
LTO, and differences between language mode and optimization level.

Teach command-line builds first, then Make and target-oriented CMake with Ninja.
Cover dependency tracking, generated files, include visibility, compile options,
link requirements, out-of-source builds, compile_commands.json, cross-toolchain
files, sysroots, presets, installation/export, and library consumption.

Specify reproducible GCC/Clang builds, version manifests, checksums, CI matrices,
and justified warning policies. Avoid relying on one universal flags string for
all targets and project types. Survey alternatives only when educationally useful.

#### T08. Machine architecture, assembly, and ABIs

Cover registers, instruction encoding at a useful level, addressing modes,
load/store behavior, stack frames, calling conventions, register preservation,
stack alignment, aggregate arguments/returns, variadic calling, floating-point
ABIs, and endianness.

Read compiler-generated x86-64 and Cortex-M assembly; write small interoperable
assembly routines. Study inline-assembly constraints, clobbers, compiler versus
hardware barriers, and why an assembly instruction's existence does not make
its use portable or correct in every execution context.

#### T09. Linkers, ELF, and binary inspection

Cover ELF sections versus segments, symbol binding/visibility, relocations,
archive extraction and link order, weak symbols, static/shared libraries,
PIC/PIE, GOT/PLT, dynamic loading, TLS, symbol versioning, and ABI compatibility.

Teach linker scripts, memory regions, VMA/LMA, section placement, alignment,
KEEP, section garbage collection, linker assertions, map files, and orphan
sections. Inspect outputs with readelf, objdump, nm, size, and LLVM equivalents.
Explain which mechanisms are ELF/toolchain conventions rather than ISO C.

#### T10. Hosted startup and minimal Linux executables

Trace executable loading, entry point, initial stack, argc/argv/environment,
auxiliary vectors, loader work, libc initialization, main, termination, flushing,
and exit status.

Investigate crt0, crt1, crti/crtn, crtbegin/crtend, _start, and startup source
files such as start.s/start.S for selected toolchains. Treat names and division
of responsibility as platform-specific conventions, not universal C requirements.

Build progressively smaller Linux programs using a standard runtime, custom
startup, and direct syscalls. Cover syscall versus function-call ABIs, negative
kernel error returns versus libc errno conventions, partial writes, EINTR,
process exit, and the difference between process and thread termination.

#### T11. Freestanding and minimal-runtime construction

Explain and experimentally compare -ffreestanding, -fno-builtin, -nostdlib,
-nodefaultlibs, -nostartfiles, and relevant header-search choices.

Audit compiler-emitted memory operations, integer-division helpers, soft-float
routines, atomic helpers, stack-protection dependencies, instrumentation, and
libgcc/compiler-rt requirements.

Implement a documented minimal support layer. Address the risk of compiler
transformations reintroducing calls into the very routines being implemented.
Demonstrate dependency audits through symbols, disassembly, link maps, loader
metadata, and controlled build variants. A successful link alone is insufficient
as evidence of the intended runtime contract.

### Phase III — Libraries, algorithms, representations, and memory management

#### T12. Standard-library semantics and implementation literacy

Create an edition-aware map of standard headers and function families, including
memory/string operations, allocation, stdio, conversions, mathematics, time,
locale, character handling, assertions, variadic access, signals, nonlocal jumps,
atomics, threads, complex arithmetic, and newer facilities where supported.

Study contracts, error reporting, ownership, lifetime, overlap restrictions,
format-string correctness, buffering, statefulness, locale dependence, thread
safety, and implementation latitude. Distinguish ISO facilities from POSIX and
implementation extensions; identify optional or inconsistently available APIs.

Compare selected internals of glibc and musl, then an embedded-oriented library
such as newlib or picolibc. Pin source versions. Teach source navigation and
reasoning about implementation constraints rather than memorizing internals.

#### T13. Implicit resource use and hidden costs

Turn informal concerns about "ghost allocations" into a systematic audit of
heap activity, stack use, locking, TLS, lazy initialization, buffering, syscalls,
locale processing, blocking, and runtime helper calls.

Investigate representative formatting, stream, sorting, conversion, allocation,
and mathematical routines without assuming that all libraries implement them
identically. Classify findings as guaranteed, implementation-dependent, or
observed only for a tested configuration.

For each audited routine, record its contract, implementation/version, allocation
behavior, stack use, locks, blocking, global/TLS effects, failure modes, and
supporting evidence.

Use source inspection, allocation instrumentation, symbol inspection, tracing,
and cold/warm experiments. Explain blind spots: no observed syscall does not
establish no allocation; wrapping public malloc may miss internal paths.
Compare thread safety, reentrancy, async-signal safety, ISR suitability, and
bounded execution time as separate properties.

#### T14. Data structures implemented from first principles

Include arrays, dynamic arrays, intrusive/non-intrusive lists, stacks, queues,
deques, ring buffers, bitsets, sparse representations, hash tables, heaps,
priority queues, search trees, balanced trees, tries, disjoint sets, and graph
representations. Place specialized structures such as B-trees in justified
advanced modules rather than omitting them silently.

Require complete C implementations of an explicitly enumerated core set.
Compare fixed-capacity, caller-storage, and allocator-backed variants where
useful. Define invariants, ownership, iterator invalidation, failure behavior,
capacity limits, and asymptotic plus machine-level costs.

Required implementations shall provide a core without external C-library calls,
using caller storage or an explicit allocator interface. Hosted adapters and
test oracles may use libraries, with their dependencies kept separate. Enumerate
any compiler-support exceptions rather than concealing them.

Implement at least one balanced-tree family deeply; compare alternatives at a
clearly declared depth. Prefer deliberate coverage over shallow implementation
of every named structure.

#### T15. Algorithms, byte processing, and parsers

Cover searching, comparison and non-comparison sorting, selection, traversal,
shortest paths, topological ordering, representative greedy/dynamic-programming
algorithms, and string matching. Include recursion elimination and explicit
work stacks for constrained environments.

Implement bounded byte/string operations, spans/views, numeric parsing and
formatting, checksums/CRCs, endianness conversion, serialization, framing, and
incremental parsers. Include malformed input, truncation, integer overflow,
partial input/output, and Unicode-versus-byte-string boundaries.

Define the exact required algorithm inventory and map each item to exercises.
Algorithms under study shall be implemented by the learner rather than hidden
behind qsort, bsearch, external containers, or equivalent library shortcuts.

#### T16. Allocation strategies and lifetime-oriented design

Implement bump/arena allocation, checkpoints and rollback, stack allocators,
fixed-block pools, free lists, and at least one variable-size allocator.
Compare slab, segregated-fit, buddy, and real-time-oriented approaches at
appropriate depth.

Cover alignment, metadata, fragmentation, coalescing, allocator invariants,
reallocation, zeroing, overflow checks, exhaustion, debugging, and corruption
detection. Separate allocation policy from backing storage such as static RAM,
caller buffers, mmap regions, or OS heaps.

Study allocator injection, lifetime grouping, reference counting, region-based
ownership, explicit capacity budgets, and allocation-failure testing. Distinguish
bounded algorithmic work from a demonstrated hard real-time execution bound.

#### T17. Numerical and data-oriented C

Implement selected scalar kernels: fixed-point conversion, stable accumulation,
vector/matrix operations, interpolation, FIR/IIR filtering, and a small numerical
solver. Make FFTs and larger numerical methods explicit electives if needed.

Study numerical stability, error budgets, reproducibility, physical units,
saturation policies, data layout, and array-of-structures versus
structure-of-arrays representations. Connect mathematical correctness to
floating-point behavior, memory access, and later vectorization work.

### Phase IV — Interfaces, abstractions, architecture, and maintainability

#### T18. Contracts, ownership, and API design

Cover preconditions, postconditions, invariants, units, ranges, capacities,
nullability, aliasing, ownership transfer, borrowing, lifetimes, and invalidation.
Design initialization/operation/destruction protocols and failure guarantees.

Compare opaque handles, exposed structs, caller-owned storage, incomplete types,
context objects, generation-counted handles, and explicit allocator interfaces.
Document thread, interrupt, allocation, and blocking properties as part of APIs.
Use compile-time and runtime checks with defined release-build behavior.

#### T19. Higher-level abstractions implemented in C

Build objects as explicit state plus operations; interfaces as function tables;
closures as function/context pairs; and generic components through selected
macro, _Generic, callback, or generated-code mechanisms.

Compare static versus dynamic dispatch, composition, tagged unions, visitors,
strategy/state patterns, dependency injection, iterators, streams, and
serialization interfaces. Account for layout, indirection, lifetime, code size,
testability, and timing costs. Explain when a plain function or struct is better
than an elaborate abstraction.

#### T20. Modular and system architecture

Cover cohesive modules, dependency direction, layering, information hiding,
functional core/imperative shell, ports and adapters, explicit side effects,
HAL/BSP/driver/service boundaries, and platform-independent application cores.

Design reusable components that run in Linux and firmware with different
backends. Compare polling, event-driven, callback, message-passing, and
state-machine designs. Separate mechanism from policy and define configuration,
versioning, and compatibility contracts.

Use architecture decision records and competing designs. Distinguish logical
module isolation from actual memory/fault containment requiring processes,
privilege separation, or hardware protection.

#### T21. Error handling, resource management, and observability

Compare status codes, result structs, out parameters, error contexts, errno,
assertions, and fatal-error policies. Study cleanup, partial initialization,
rollback, idempotence, cancellation, and transaction boundaries.

Evaluate structured cleanup alternatives, including carefully scoped cleanup
labels, on clarity and correctness rather than stylistic dogma.

Design bounded and optionally removable logging, tracing, crash records,
developer white-box interfaces, and integrator black-box diagnostics. Verify
that disabled instrumentation has the promised evaluation, size, and timing
behavior. Specify fault propagation, degradation, and recovery explicitly.

#### T22. Production-quality libraries and repositories

Cover naming, readability, cognitive/cyclomatic complexity, justified
encapsulation, duplication versus coupling, source/ABI compatibility, versioning,
C interfaces for FFI, packaging, and dependency/license management.

Require useful Doxygen-compatible API documentation, rationale comments,
examples, changelogs, reproducible builds, and requirement-to-test traceability.
Teach source reading, review, refactoring under tests, regression analysis,
bisection, and maintenance of unfamiliar C codebases.

### Phase V — Linux systems and concurrency

#### T23. Processes, memory, files, and POSIX interfaces

Cover process creation, exec, wait, exit, environments, file descriptors,
open-file descriptions, descriptor inheritance, close-on-exec, pipes, redirection,
permissions, and resource limits.

Study read/write loops, file offsets, buffered versus descriptor I/O, mmap,
page faults, copy-on-write, shared memory, file locking, persistence, fsync,
rename-based updates, and crash consistency. Explain syscalls versus libc
wrappers and virtual memory versus physical memory/resource commitment.

#### T24. Signals, IPC, networking, and event-driven systems

Cover signal delivery/masking, async-signal safety, race-free waiting,
self-pipe/signalfd patterns, sockets, TCP/UDP, local sockets, DNS interfaces,
framing, timeouts, backpressure, and graceful shutdown.

Implement blocking and nonblocking designs using poll/epoll, timerfd/eventfd,
monotonic time, and bounded queues. Include partial I/O, disconnects, overload,
and interrupted operations. Treat io_uring, zero-copy mechanisms, and advanced
networking as later extensions with explicit kernel-version dependencies.

#### T25. Threads and the C memory model

Study POSIX threads and C thread facilities where available; mutexes,
condition variables, semaphores, read/write locks, thread-local state, thread
pools, and producer/consumer designs.

Cover data races, atomicity, happens-before, modification order, memory orders,
fences, compare-exchange, spurious wakeups, deadlock, livelock, fairness,
priority inversion, and false sharing.

Separate volatile, compiler barriers, C atomics, hardware barriers, and OS
synchronization. Explain why atomic operations need not be lock-free, and why
lock-free does not automatically mean safe in every ISR or real-time context.

#### T26. Advanced concurrency and optional kernel work

Progress from correct mutex-based designs to SPSC queues, selected MPSC/MPMC
structures, progress guarantees, ABA, safe memory reclamation, hazard pointers,
epoch techniques, and RCU concepts.

Require reasoning about publication, reclamation, lifetime, and ordering;
stress tests alone shall not be accepted as a correctness proof.

Offer Linux kernel C as a distinct elective: kernel build conventions,
execution contexts, allocation restrictions, user/kernel interfaces, driver
lifetimes, and the Linux kernel memory model. Use isolated test systems and
version-matched documentation. Kernel synchronization rules shall not be
substituted casually for ISO C userspace rules.

### Phase VI — Bare-metal and real-time engineering

#### T27. Reset-to-main and board bring-up

Implement and explain reset vectors, stack initialization, .data relocation,
.bss clearing, retained/uninitialized sections, vector tables, runtime hooks,
clock initialization, and entry into application code.

Write and audit startup assembly and linker scripts; establish stack/heap
budgets; inspect the complete image; handle unexpected return from the
application entry point. Compare cold reset, warm reset, bootloader handoff,
and debugger-assisted loading.

Progress from an explained vendor startup to an independently constructed
minimal startup. Document which pre-main responsibilities belong to ROM,
bootloader, startup, compiler runtime, board code, or application code.

#### T28. MMIO, peripherals, and driver construction

Cover address maps, register width/alignment, volatile access, reserved bits,
write-one-to-clear behavior, read side effects, atomic set/clear registers,
read-modify-write hazards, and peripheral sequencing.

Implement GPIO, UART, timers, SPI, and I2C drivers; add ADC and DMA where the
chosen target supports them. Treat CAN, USB, and Ethernet as explicit extensions
with realistic hardware and protocol prerequisites.

Compare polling, interrupt, and DMA implementations. Develop register-level
code, a minimal HAL, and a higher-level portable service for the same device,
with tests and explicit timing/error contracts at each boundary.

#### T29. Interrupts, DMA, and hardware/software ordering

Study exception entry/return, vector dispatch, nesting, priorities, critical
sections, shared foreground/ISR data, latency, reentrancy, and stack use.

Address DMA buffer ownership, bus-master concurrency, cache maintenance where
applicable, compiler ordering, device-memory ordering, and architecture-specific
barriers. Separate cache coherency, memory ordering, and mutual exclusion.

Implement bounded ISR-to-thread/foreground communication and reason about
wraparound, queue overflow, missed events, and timestamp consistency.

#### T30. Scheduling, timing, and constrained execution

Build a superloop, cooperative scheduler, and event/state-machine system.
Introduce preemption and RTOS integration only after execution contexts and
shared-state rules are understood; a small scheduler/context-switch exercise
may be an advanced project.

Cover periodic work, deadlines, response time, jitter, timer wraparound,
interrupt budgets, watchdogs, stack-depth analysis, fixed-capacity design,
power modes, wakeup races, and resource exhaustion.

Separate average performance, measured maxima, analytical bounds, and WCET
claims. Inspect generated code and runtime dependencies for critical paths.
Compare allocation policies and synchronization choices against declared
latency and memory budgets.

#### T31. Embedded debugging, fault handling, and deployment

Use SWD/JTAG, GDB, OpenOCD or supported alternatives, breakpoints, watchpoints,
register inspection, fault-status decoding, stack frames, and trace/cycle
facilities available on the actual target.

Teach optimized-code debugging, stack corruption, bad vectors, startup faults,
clock/peripheral mistakes, and DMA failures. Add host tests, simulator tests,
and on-target tests with explicit differences in what each can establish.

Cover image formats, flashing, bootloader interfaces, robust update concepts,
crash records, watchdog recovery, MPU protection where available, and selected
fault-injection experiments. Declare equipment needs without turning the
curriculum into a purchasing guide.

### Phase VII — Verification, security, and performance mastery

#### T32. Systematic testing and debugging

Develop unit, integration, system, property-based, differential, fuzz, and
metamorphic tests. Include allocator exhaustion, I/O failure, truncation,
corrupted state, timing perturbation, and adversarial backend substitution.

Use GCC/Clang diagnostics, static analysis, GDB/LLDB, sanitizers, Valgrind where
supported, coverage, mutation testing, and targeted MC/DC exercises. State
platform/tool limitations, instrumentation effects, and incompatibilities.

Teach failure minimization, deterministic replay where practical, test oracles,
and regression preservation. Separate component dependencies from dependencies
introduced only by the test harness. Passing tests and achieving coverage
shall not be described as proof of general correctness.

#### T33. Formal reasoning and verification

Use invariants, pre/postconditions, loop variants, state machines, abstract
models, and correctness/complexity arguments for real C implementations.

Introduce tools such as CBMC and Frama-C/ACSL through small complete examples:
array bounds, arithmetic safety, parser properties, allocator invariants, and
selected concurrency properties where supported.

State assumptions, environmental models, boundedness, tool support, and proof
obligations. Distinguish a bounded model-checking result, deductive proof,
static-analysis finding, stress test, and benchmark as different evidence types.

#### T34. Optimization, vectorization, and measurement

Cover workload modeling, asymptotic versus constant-factor costs, cache
hierarchies, locality, branch prediction, instruction-level parallelism, memory
bandwidth, alignment, data layout, and code-size/instruction-cache trade-offs.

Study auto-vectorization prerequisites, alias analysis, restrict, reductions,
loop transformations, dependency analysis, compiler optimization reports, SIMD
intrinsics, tail handling, scalar fallbacks, and runtime ISA dispatch with
required OS support.

Use optimized assembly inspection, perf and appropriate target counters,
benchmark controls, warm/cold runs, statistical variability, and regression
budgets. Prevent dead-code elimination and distinguish actual work from
measurement overhead. Validate correctness and numerical behavior after every
optimization. Real-hardware performance claims shall require real measurements.

#### T35. Secure and resilient C

Cover bounds/lifetime errors, integer-overflow chains, format strings, unsafe
parsing, resource-exhaustion inputs, uninitialized data, information leakage,
and concurrency-induced defects.

Explain stack protection, fortified interfaces, ASLR, PIE, RELRO, executable
memory policy, privilege separation, sandboxing, and MPU-based isolation where
applicable. Audit their availability and runtime requirements in minimal builds.

Teach constant-time concerns and compiler transformations at an appropriate
level; educational cryptographic code shall remain distinct from production
cryptography. Connect hardening, fault containment, error handling, and secure
update design to explicit threat and failure models.

## 4. Required structure of each curriculum module

For every module, provide:

1. Stable ID, title, track, depth, and core/elective status.
2. Prerequisites and a short explanation of why this position is appropriate.
3. Observable learning outcomes, phrased as demonstrable abilities.
4. Detailed topics and the misconceptions the module corrects.
5. Primary reading plus a limited set of targeted supplementary resources.
6. Exercises progressing through prediction, implementation, inspection,
   debugging, testing, and design comparison, as appropriate.
7. At least one concrete practical deliverable with inputs, outputs, constraints,
   failure cases, and acceptance criteria.
8. Toolchain, standard, target, dependency profile, and equipment requirements.
9. Assessment rubric, mastery gate, and remediation path.
10. Estimated reading, implementation, testing, and consolidation effort.

Define increasingly demanding gates. Early modules need small, focused evidence;
later modules shall require production-style reasoning and verification.

For substantial components, require a dossier containing problem specification,
public API, representation, invariants, ownership/resource model, correctness
argument, complexity analysis, test strategy, measurements, and limitations.
Documentation shall explain decisions, not merely restate source code.

## 5. Integrating projects and capstones

Build the curriculum around a reusable C component library developed gradually.
Its algorithms and representations shall support both hosted testing and
freestanding integration through explicit platform interfaces.

Include at least these integrating projects, sequenced by prerequisites:

P1. A bounded container/allocator library with failure injection, invariants,
    tests, and host/freestanding dependency audits.
P2. A small Linux executable with custom startup and a deliberately specified
    minimal runtime, accompanied by an ELF and syscall analysis.
P3. A binary protocol/framing/parser library with robust malformed-input
    handling and reuse in Linux and firmware.
P4. A Cortex-M firmware application with explained startup/linker configuration,
    register-level drivers, interrupt or DMA acquisition, and bounded buffering.
P5. A concurrent Linux telemetry or data-processing application with backpressure,
    graceful shutdown, explicit ownership, and measured resource limits.
P6. A scalar-to-vectorized computational kernel with a numerical/correctness
    oracle and a reproducible performance investigation.

Projects may evolve into one another to avoid duplicated effort. Identify which
are exercises, phase projects, and final capstones rather than treating all as
independent large projects.

Require a Linux systems capstone and a bare-metal capstone with a shared portable
core or protocol. Each shall include architecture alternatives, explicit
contracts, failure handling, traceability, tests, resource budgets, dependency
inspection, documentation, and a defended review.

Include perturbation assignments: change allocator policy, constrain memory,
replace a backend, alter the target ABI, inject failures, change concurrency
strategy, or remove a runtime dependency. The learner shall explain what changes,
what remains invariant, and which evidence needs to be regenerated.

Mark optional deeper projects separately: a small shell, bytecode interpreter,
ELF inspection/loading experiment, storage engine, RTOS kernel exercise,
advanced allocator, or kernel driver. Preserve a feasible core path.

## 6. Resources, research, and evidence requirements

Research authoritative resources in English and verify their existence,
editions, scope, and current availability. Prefer ISO/WG14 material, compiler
and linker manuals, architecture/ABI specifications, POSIX/Linux documentation,
library source, respected books, and substantive university materials.

For each recommended resource, provide title, author/maintainer, edition or
version, URL, free/paid status, relevant sections, prerequisites, curriculum
mapping, and a specific reason for inclusion. Recommend a small primary spine
plus focused supplements instead of making every book mandatory.

Distinguish published standards from accessible committee drafts. Give document
identifiers where known. Cite precise sections only when verified; otherwise
state the uncertainty instead of inventing chapter or clause numbers.

For version-sensitive tools, standards support, libraries, and emulator models,
record a check date and distinguish verified capabilities from assumptions.
Identify platform limitations and licensing/access restrictions. Paid resources
may be included when justified, with an accessible alternative where practical.

Reading source code shall be a recurring activity. Choose version-pinned,
manageable excerpts that expose specific mechanisms, and separate educational
inspection from permission to copy code into a new project.

## 7. Required output

Deliver a self-contained Markdown curriculum with these sections:

1. Mastery outcomes, assumptions, scope boundaries, and entry diagnostic.
2. Competency map and prerequisite graph.
3. Phase roadmap showing the shared core and Linux/bare-metal branches.
4. Detailed module specifications using the required module structure.
5. Standards comparison and compiler/library support matrix.
6. Explicit no-library/no-allocation/runtime dependency-profile matrix.
7. Required implementation inventory for algorithms, structures, allocators,
   abstractions, and runtime components.
8. Integrating projects, capstones, rubrics, and evidence requirements.
9. Resource bibliography mapped to modules and prioritized by role.
10. Reproducible laboratory/repository structure and build/test profiles.
11. Effort estimates and illustrative schedules at 10 and 20 hours/week.
12. Traceability from every T01–T35 topic group to modules, exercises, and gates.
13. Coverage audit, unresolved questions, assumptions, and elective extensions.

Compute effort totals consistently and avoid double-counting projects embedded
in modules. Treat schedule examples as planning estimates, not promises of
mastery within a fixed time. Prefer evidence-based progression over an arbitrary
number of weeks.

The module catalog shall be substantively developed throughout, including the
advanced phases. Brevity shall come from removing repetition and sharing common
rubrics, not from replacing later phases with headings or "study this next."

Finish with a curriculum audit: identify missing prerequisites, overloaded
modules, unsupported claims, redundant work, hardware bottlenecks, and objectives
without assessments. Explain the revisions made to resolve them.

The result shall be an actionable mastery curriculum, not a motivational essay,
a generic C tutorial, an unprioritized topic dump, or a bibliography-only answer.
