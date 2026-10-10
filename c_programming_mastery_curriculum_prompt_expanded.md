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

This inventory preserves the 35 domain IDs T01–T35 and adds **395 subtopics** and **1,185 leaf-level learning objectives**. It is a proposed knowledge and competence taxonomy, not a schedule or a claim that 1,185 separate lessons are necessary.

The learner enters with basic algorithmics and basic C syntax. T01 is diagnostic; missing prerequisites receive targeted remediation. The Linux x86-64 and Cortex-M tracks share the portable language, library, design, verification, and runtime foundations. The intended primary Cortex-M board remains NUCLEO-F439ZI; target-specific capabilities shall be verified when constructing actual labs.

**Hierarchy:** `T16` identifies a domain; `T16.03` identifies a subtopic; `T16.03.b` identifies one assessable objective. IDs are traceability keys, not a mandatory chronological order. A subtopic may need several exercises, and one project may satisfy objectives from several domains.

**Depth:** domain-level depth distinguishes core mastery, working proficiency, diagnostic remediation, and electives. `[E]` marks elective subtopics; T26 is awareness-level core material with advanced implementations elective. C23 is a parallel standards track, not an excuse to omit the portability-oriented baseline. A curriculum author shall assign any more granular depth exceptions explicitly.

**Prerequisites:** listed domain references indicate the relevant foundations, not compulsory completion of every advanced objective in those domains. They are a coarse, acyclic entry map. The eventual curriculum shall refine them into leaf-level dependencies. In particular, bare-metal use of T25 means the relevant C memory-model and atomic foundations, not a requirement to finish all POSIX-thread exercises first.

**Source discipline:** the catalog states things to investigate, implement, and verify. It is not a clause-by-clause technical reference or a compiler-support certification. Source anchors identify primary authorities for later instruction; URLs were checked on **2026-10-04**. Language-edition details, actual compiler/library support, exact ABIs, peripheral features, and tool compatibility shall be checked for the selected lab versions. An implementation-specific observation shall retain its configuration and version qualifiers.

**Evidence:** explain the contract; construct or inspect the mechanism; exercise boundaries and failure cases; inspect binaries where relevant; and state what the evidence establishes. Requirements, implementation mechanisms, and empirical behavior shall remain distinguishable. Foundation-level assessment can be small; capstone-level assessment requires integrated evidence.

**Cross-cutting practice:** introduce small contracts, tests, debugging, source inspection, and resource reasoning in the first phase. Revisit T18, T21, T32, T33, and T35 throughout rather than postponing all verification and architecture until their numerical position.

**Core implementation obligation:** required algorithm, container, allocator, abstraction, and runtime exercises shall be implemented in C. Their reusable core shall use caller-provided storage or an explicit allocator interface and a declared dependency profile. The hosted test harness may use independent libraries as oracles. Implementing the algorithm under study by forwarding to an existing equivalent library routine shall not satisfy the implementation objective.

**Scope boundary:** kernel development, advanced memory reclamation, specialized numerical algorithms, advanced peripheral protocols, and novel language proposals are explicitly separable extensions. Full production cryptography, a complete operating system, every allocator family, and every balanced-tree family are not prerequisites for completing the core mastery path.

### Domain index

| Domain | Track | Subtopics | Leaf objectives |
|---|---|---:|---:|
| [T01 — Entry diagnostic and foundational consolidation](#t01) | Shared | 6 | 18 |
| [T02 — Declarations, types, expressions, and translation units](#t02) | Shared | 12 | 36 |
| [T03 — The object model, pointers, and memory correctness](#t03) | Shared | 14 | 42 |
| [T04 — Integer, bit-level, and floating-point semantics](#t04) | Shared | 12 | 36 |
| [T05 — Standards evolution, dialects, and portability](#t05) | Shared | 10 | 30 |
| [T06 — Preprocessing and metaprogramming](#t06) | Shared | 9 | 27 |
| [T07 — Compilation and build engineering](#t07) | Shared | 12 | 36 |
| [T08 — Machine architecture, assembly, and ABIs](#t08) | Shared | 10 | 30 |
| [T09 — Linkers, ELF, and binary inspection](#t09) | Shared | 12 | 36 |
| [T10 — Hosted startup and minimal Linux executables](#t10) | Linux | 10 | 30 |
| [T11 — Freestanding and minimal-runtime construction](#t11) | Cross-track | 10 | 30 |
| [T12 — Standard-library semantics and implementation literacy](#t12) | Shared | 16 | 48 |
| [T13 — Implicit resource use and hidden costs](#t13) | Cross-track | 10 | 30 |
| [T14 — Data structures implemented from first principles](#t14) | Shared | 15 | 45 |
| [T15 — Algorithms, byte processing, and parsers](#t15) | Shared | 15 | 45 |
| [T16 — Allocation strategies and lifetime-oriented design](#t16) | Cross-track | 14 | 42 |
| [T17 — Numerical and data-oriented C](#t17) | Cross-track | 10 | 30 |
| [T18 — Contracts, ownership, and API design](#t18) | Shared | 10 | 30 |
| [T19 — Higher-level abstractions implemented in C](#t19) | Shared | 10 | 30 |
| [T20 — Modular and system architecture](#t20) | Cross-track | 10 | 30 |
| [T21 — Error handling, resource management, and observability](#t21) | Cross-track | 10 | 30 |
| [T22 — Production-quality libraries and repositories](#t22) | Shared | 10 | 30 |
| [T23 — Linux processes, memory, files, and POSIX interfaces](#t23) | Linux | 14 | 42 |
| [T24 — Signals, IPC, networking, and event-driven systems](#t24) | Linux | 12 | 36 |
| [T25 — Threads and the C memory model](#t25) | Cross-track | 12 | 36 |
| [T26 — Advanced concurrency and optional kernel work](#t26) | Advanced elective | 10 | 30 |
| [T27 — Reset-to-main and board bring-up](#t27) | Bare metal | 10 | 30 |
| [T28 — MMIO, peripherals, and driver construction](#t28) | Bare metal | 12 | 36 |
| [T29 — Interrupts, DMA, and hardware/software ordering](#t29) | Bare metal | 10 | 30 |
| [T30 — Scheduling, timing, and constrained execution](#t30) | Bare metal | 10 | 30 |
| [T31 — Embedded debugging, fault handling, and deployment](#t31) | Bare metal | 10 | 30 |
| [T32 — Systematic testing and debugging](#t32) | Cross-track | 14 | 42 |
| [T33 — Formal reasoning and verification](#t33) | Cross-track | 10 | 30 |
| [T34 — Optimization, vectorization, and measurement](#t34) | Cross-track | 12 | 36 |
| [T35 — Secure and resilient C](#t35) | Cross-track | 12 | 36 |
| **Total** | **35 domains** | **395** | **1,185** |

### Phase I — Precise command of the language

<a id="t01"></a>

#### T01 — Entry diagnostic and foundational consolidation

**Track:** Shared. **Depth:** Diagnostic; targeted remediation. **Entry foundations:** Entry diagnostic; no additional prerequisite domain.

**Primary-source anchors:** [^S02], [^S05].

##### T01.01 — Algorithmic reasoning

- **T01.01.a** Trace loops and recursion using explicit state tables.
- **T01.01.b** Derive time and auxiliary-space costs for simple implementations.
- **T01.01.c** State loop invariants and termination conditions before coding.

##### T01.02 — Representation prerequisites

- **T01.02.a** Convert between binary, hexadecimal, and decimal representations.
- **T01.02.b** Relate Boolean expressions to masks and truth tables.
- **T01.02.c** Distinguish mathematical integers from finite machine representations.

##### T01.03 — Basic C diagnostic

- **T01.03.a** Predict array, pointer, function, and control-flow behavior.
- **T01.03.b** Locate uninitialized values and off-by-one errors in small examples.
- **T01.03.c** Replace guesses about evaluation order with explicit reasoning.

##### T01.04 — Execution model primer

- **T01.04.a** Distinguish source files, objects, executables, and running processes.
- **T01.04.b** Sketch registers, memory, stack frames, and address spaces.
- **T01.04.c** Separate compile-time, link-time, startup, and runtime failures.

##### T01.05 — Development workflow

- **T01.05.a** Build and run a small program from the shell.
- **T01.05.b** Preserve reproducible changes and regressions with version control.
- **T01.05.c** Record compiler versions, commands, inputs, and observed outputs.

##### T01.06 — Specification and test habits

- **T01.06.a** Convert a prose requirement into inputs, outputs, and limits.
- **T01.06.b** Select normal, boundary, invalid, and empty-input cases.
- **T01.06.c** Diagnose failures before changing code and retain regression tests.

**Domain mastery checkpoint:** Repair and test a small multi-file program; explain its algorithm, memory accesses, build steps, and boundary cases.

<a id="t02"></a>

#### T02 — Declarations, types, expressions, and translation units

**Track:** Shared. **Depth:** Core mastery. **Entry foundations:** T01.

**Primary-source anchors:** [^S01], [^S02], [^S05].

##### T02.01 — Declaration grammar

- **T02.01.a** Parse pointer, array, and function declarators systematically.
- **T02.01.b** Distinguish declarations, definitions, typedef names, and type specifiers.
- **T02.01.c** Rewrite intricate declarations using readable intermediate typedefs.

##### T02.02 — Type categories

- **T02.02.a** Distinguish object, function, incomplete, scalar, and aggregate types.
- **T02.02.b** Identify compatible types across separate declarations.
- **T02.02.c** Explain the limits of typedef-based semantic type separation.

##### T02.03 — Scope and identifier namespaces

- **T02.03.a** Trace block, file, function, and prototype scopes.
- **T02.03.b** Distinguish tag, label, member, and ordinary identifier namespaces.
- **T02.03.c** Diagnose shadowing and accidental name collisions.

##### T02.04 — Linkage and storage classes

- **T02.04.a** Compare internal linkage, external linkage, and no linkage.
- **T02.04.b** Analyze extern, static, register, and thread-storage declarations by edition.
- **T02.04.c** Diagnose inconsistent declarations across translation units.

##### T02.05 — Functions and calls

- **T02.05.a** Specify parameter and return types with complete prototypes.
- **T02.05.b** Compare array-parameter adjustment with actual array objects.
- **T02.05.c** Audit callback signatures, function-pointer conversions, and call compatibility.

##### T02.06 — Variadic interfaces

- **T02.06.a** Apply default argument promotions at variadic call sites.
- **T02.06.b** Use va_start, va_arg, va_copy, and va_end correctly.
- **T02.06.c** Specify a reliable argument-type and argument-count convention.

##### T02.07 — Expressions and conversions

- **T02.07.a** Separate precedence, associativity, and evaluation order.
- **T02.07.b** Trace integer promotions and usual arithmetic conversions.
- **T02.07.c** Analyze conditional expressions, assignments, and explicit casts.

##### T02.08 — Sequencing and side effects

- **T02.08.a** Distinguish sequenced, unsequenced, and indeterminately sequenced evaluations.
- **T02.08.b** Classify conflicting reads and writes under the selected edition.
- **T02.08.c** Refactor expressions into explicit, reviewable steps.

##### T02.09 — Initialization

- **T02.09.a** Compare initialization, assignment, and implicit zero initialization.
- **T02.09.b** Use aggregate, designated, and compound-literal initialization appropriately.
- **T02.09.c** Analyze initializer lifetime and partially initialized aggregates.

##### T02.10 — Qualifiers and interfaces

- **T02.10.a** Distinguish pointer qualification from pointed-to qualification.
- **T02.10.b** Design const-correct APIs and inspect qualifier-loss diagnostics.
- **T02.10.c** Introduce volatile and restrict without treating them as synchronization.

##### T02.11 — Inline and multi-file definitions

- **T02.11.a** Compare static inline, external definitions, and inline declarations.
- **T02.11.b** Investigate ISO versus GNU inline conventions with symbol inspection.
- **T02.11.c** Design self-contained headers with a predictable definition strategy.

##### T02.12 — Constraints and diagnostics

- **T02.12.a** Separate syntax errors, constraint violations, and runtime undefined behavior.
- **T02.12.b** Construct negative compilation tests with expected diagnostics.
- **T02.12.c** Explain why accepted source is not automatically conforming source.

**Domain mastery checkpoint:** Explain a deliberately difficult multi-file program without executing it, then verify each prediction using the appropriate evidence.

<a id="t03"></a>

#### T03 — The object model, pointers, and memory correctness

**Track:** Shared. **Depth:** Core mastery. **Entry foundations:** T02.

**Primary-source anchors:** [^S01], [^S02], [^S05].

##### T03.01 — Storage duration and lifetime

- **T03.01.a** Distinguish automatic, static, thread, and allocated storage duration.
- **T03.01.b** Track when objects begin and cease to exist.
- **T03.01.c** Find escaped pointers to expired local and temporary objects.

##### T03.02 — Object and value representations

- **T03.02.a** Inspect bytes without assuming every bit is value-bearing.
- **T03.02.b** Analyze padding, representation-dependent values, and edition-sensitive rules.
- **T03.02.c** Separate equality of values from equality of object representations.

##### T03.03 — Alignment

- **T03.03.a** Derive alignment requirements for objects and array elements.
- **T03.03.b** Detect invalid casts from insufficiently aligned storage.
- **T03.03.c** Design portable alignment checks and aligned-storage interfaces.

##### T03.04 — Arrays and subobjects

- **T03.04.a** Model nested arrays as arrays of array objects.
- **T03.04.b** Distinguish contiguous storage from unrestricted pointer-arithmetic permission.
- **T03.04.c** Diagnose incorrect pointer-to-pointer substitutes for multidimensional arrays.

##### T03.05 — Pointer arithmetic

- **T03.05.a** Prove valid indexing and one-past formation within arrays.
- **T03.05.b** Distinguish forming a pointer from dereferencing that pointer.
- **T03.05.c** Analyze pointer subtraction, representability, and relational-comparison preconditions.

##### T03.06 — Null and special pointer values

- **T03.06.a** Distinguish null pointer constants from pointer representations.
- **T03.06.b** Analyze void-pointer conversions and pointer/integer round trips.
- **T03.06.c** Separate object pointers from function pointers and platform extensions.

##### T03.07 — Allocated storage and effective type

- **T03.07.a** Trace how allocated storage is initialized and accessed.
- **T03.07.b** Compare typed writes, character access, and copying representations.
- **T03.07.c** Audit reusable raw-storage containers against the selected C rules.

##### T03.08 — Aliasing and type punning

- **T03.08.a** Identify potentially incompatible lvalue accesses.
- **T03.08.b** Compare union-based, byte-copy, and cast-based representation inspection.
- **T03.08.c** Separate ISO guarantees from compiler-specific aliasing conventions.

##### T03.09 — restrict contracts

- **T03.09.a** Identify the objects and executions covered by restrict associations.
- **T03.09.b** Construct valid and invalid overlapping-access examples.
- **T03.09.c** Compare optimized code only after establishing the aliasing preconditions.

##### T03.10 — Provenance and address identity

- **T03.10.a** Separate numeric address equality from valid object access.
- **T03.10.b** Compare base-standard rules, provenance specifications, and implementation models.
- **T03.10.c** Document assumptions behind low-level pointer reconstruction techniques.

##### T03.11 — Indeterminate state

- **T03.11.a** Distinguish initialization defects from legitimate unspecified representation bytes.
- **T03.11.b** Audit uninitialized scalar, pointer, aggregate, and padding observations.
- **T03.11.c** Compare diagnostics and dynamic checks with actual language guarantees.

##### T03.12 — Flexible and variable-sized objects

- **T03.12.a** Calculate checked allocation sizes for flexible array members.
- **T03.12.b** Track VLA lifetime, bounds evaluation, and edition-dependent availability.
- **T03.12.c** Compare flexible arrays with explicit pointer/length representations.

##### T03.13 — Invalidation and relocation

- **T03.13.a** Model realloc success and failure as distinct lifetime transitions.
- **T03.13.b** Specify iterator, slice, and pointer validity across container mutations.
- **T03.13.c** Choose stable handles where direct pointers cannot remain valid.

##### T03.14 — Memory correctness case studies

- **T03.14.a** Repair use-after-free, double-free, overlap, and lifetime-escape defects.
- **T03.14.b** Audit offsetof and container-recovery idioms for required assumptions.
- **T03.14.c** Explain why a successful test cannot validate every undefined execution.

**Domain mastery checkpoint:** Audit a container implementation for lifetime, bounds, aliasing, alignment, and invalidation; justify each access against its object model.

<a id="t04"></a>

#### T04 — Integer, bit-level, and floating-point semantics

**Track:** Shared. **Depth:** Core mastery. **Entry foundations:** T02.

**Primary-source anchors:** [^S01], [^S02], [^S05].

##### T04.01 — Integer type selection

- **T04.01.a** Use limits instead of assuming widths for fundamental types.
- **T04.01.b** Compare size_t, ptrdiff_t, intmax_t, and fixed-width families.
- **T04.01.c** Account for optional exact-width types and non-eight-bit bytes.

##### T04.02 — Promotions and mixed arithmetic

- **T04.02.a** Trace small-integer promotion before bitwise and arithmetic operations.
- **T04.02.b** Diagnose signed/unsigned comparison surprises.
- **T04.02.c** Design conversion boundaries that reject out-of-range values.

##### T04.03 — Overflow and checked arithmetic

- **T04.03.a** Separate mathematical overflow from the selected language semantics.
- **T04.03.b** Implement checked addition, subtraction, multiplication, and size calculations.
- **T04.03.c** Compare manual checks, compiler builtins, and C23 checked arithmetic.

##### T04.04 — Division and remainder

- **T04.04.a** Analyze quotient and remainder signs for supported editions.
- **T04.04.b** Handle zero divisors and minimum-integer edge cases.
- **T04.04.c** Distinguish truncating division from floor division and modular arithmetic.

##### T04.05 — Shifts and masks

- **T04.05.a** Validate shift counts and operand types before shifting.
- **T04.05.b** Construct field insertion, extraction, and rotation operations.
- **T04.05.c** Compare population count, leading zeros, and bit-width calculations.

##### T04.06 — Byte order and serialization

- **T04.06.a** Encode integers independently of native byte order.
- **T04.06.b** Avoid alignment and aliasing assumptions when decoding byte buffers.
- **T04.06.c** Separate wire formats from packed in-memory structure layouts.

##### T04.07 — Bit-fields and representation control

- **T04.07.a** Evaluate bit-field allocation and ordering assumptions.
- **T04.07.b** Compare language bit-fields with explicit masks for register access.
- **T04.07.c** Document ABI dependence in externally shared binary layouts.

##### T04.08 — Fixed-point arithmetic

- **T04.08.a** Choose scale, signedness, range, and precision explicitly.
- **T04.08.b** Implement widening intermediates, rounding, and saturation policies.
- **T04.08.c** Derive overflow and quantization-error bounds for a calculation.

##### T04.09 — Floating-point representations

- **T04.09.a** Identify target formats and relevant implementation characteristics.
- **T04.09.b** Examine NaNs, infinities, signed zero, and subnormal handling.
- **T04.09.c** Compare floating-point representation with real-number arithmetic.

##### T04.10 — Floating-point computation

- **T04.10.a** Quantify rounding, cancellation, and accumulation error.
- **T04.10.b** Compare fused operations and separated arithmetic under stated assumptions.
- **T04.10.c** Choose absolute, relative, and ULP-oriented test criteria.

##### T04.11 — Environment and compiler modes

- **T04.11.a** Inspect rounding modes and floating-point exception behavior.
- **T04.11.b** Analyze evaluation precision and floating-point contraction policies.
- **T04.11.c** Compare strict and fast-math builds without silently changing the specification.

##### T04.12 — Numerical boundary testing

- **T04.12.a** Generate exceptional, adjacent, extreme, and cancellation-prone inputs.
- **T04.12.b** Use higher-precision or independently derived reference results.
- **T04.12.c** Separate tolerance selection from unexplained test relaxation.

**Domain mastery checkpoint:** Implement checked integer primitives and a numerically specified fixed-point or floating-point calculation with boundary-focused tests.

<a id="t05"></a>

#### T05 — Standards evolution, dialects, and portability

**Track:** Shared. **Depth:** Core literacy; mastery through comparisons. **Entry foundations:** T02, T03, T04.

**Primary-source anchors:** [^S01], [^S02], [^S03], [^S05].

##### T05.01 — Standards and authority

- **T05.01.a** Distinguish published editions, drafts, corrigenda, and committee proposals.
- **T05.01.b** Read normative requirements separately from notes and rationale.
- **T05.01.c** Trace a claim to its applicable edition and implementation documentation.

##### T05.02 — C89 and C90 baseline

- **T05.02.a** Identify declaration, prototype, and preprocessing conventions of older code.
- **T05.02.b** Analyze historical implicit declarations and old-style function definitions.
- **T05.02.c** Port legacy constructs while preserving defined intended behavior.

##### T05.03 — C95 and internationalization

- **T05.03.a** Locate amendment-era library and source-language changes.
- **T05.03.b** Study wide-character and multibyte facilities in their actual context.
- **T05.03.c** Separate historical compatibility from modern Unicode requirements.

##### T05.04 — C99 language and library changes

- **T05.04.a** Exercise designated initializers, compound literals, restrict, and variadic macros.
- **T05.04.b** Compare VLA, inline, integer-type, and formatting facilities.
- **T05.04.c** Build explicit C90-to-C99 migration and regression cases.

##### T05.05 — C11 changes

- **T05.05.a** Study atomics, thread facilities, alignment, generic selection, and assertions.
- **T05.05.b** Identify optional facilities and implementation support boundaries.
- **T05.05.c** Distinguish language acceptance from linked-library availability.

##### T05.06 — C17 and corrections

- **T05.06.a** Identify corrections relevant to selected components.
- **T05.06.b** Compare C11 and C17 behavior using authoritative change records.
- **T05.06.c** Avoid inventing a large new feature set for a corrective edition.

##### T05.07 — C23 declaration and expression facilities

- **T05.07.a** Investigate nullptr, typeof forms, inference, and constexpr object declarations.
- **T05.07.b** Analyze _BitInt, enum changes, and updated constant-expression facilities.
- **T05.07.c** Contrast C semantics with similarly named C++ constructs.

##### T05.08 — C23 preprocessing and library facilities

- **T05.08.a** Investigate #embed, __VA_OPT__, attributes, and updated keyword spellings.
- **T05.08.b** Exercise checked arithmetic and bit-operation headers where implemented.
- **T05.08.c** Track expanded freestanding requirements separately from host library support.

##### T05.09 — Feature detection and migration

- **T05.09.a** Use edition macros and documented capability tests.
- **T05.09.b** Test strict and extension dialects in both compiler families.
- **T05.09.c** Keep unsupported facilities behind explicit portability boundaries.

##### T05.10 — Beyond-base-standard mechanisms [E]

- **T05.10.a** Survey provenance specifications and relevant technical reports.
- **T05.10.b** Track proposals such as cleanup mechanisms without treating them as adopted C23.
- **T05.10.c** Record extensions with replacement strategies and versioned evidence.

**Domain mastery checkpoint:** Produce a source-level migration dossier with edition-specific tests, compiler/library support evidence, and justified portability decisions.

### Phase II — Toolchains, binaries, execution, and runtimes

<a id="t06"></a>

#### T06 — Preprocessing and metaprogramming

**Track:** Shared. **Depth:** Core mastery. **Entry foundations:** T02, T05.

**Primary-source anchors:** [^S02], [^S03], [^S05].

##### T06.01 — Translation phases

- **T06.01.a** Trace source-character handling, line splicing, tokenization, and preprocessing.
- **T06.01.b** Analyze comments and literals at the correct translation stage.
- **T06.01.c** Inspect preprocessed output to explain unexpected compilation results.

##### T06.02 — Macro expansion

- **T06.02.a** Follow argument prescan, substitution, rescanning, and suppression rules.
- **T06.02.b** Distinguish object-like and function-like macro invocation.
- **T06.02.c** Diagnose surprising expansion caused by nested replacement lists.

##### T06.03 — Stringification and token construction

- **T06.03.a** Apply stringification and token pasting with controlled expansion.
- **T06.03.b** Generate identifiers without accidentally creating invalid tokens.
- **T06.03.c** Separate textual assembly from semantic type checking.

##### T06.04 — Variadic preprocessing

- **T06.04.a** Handle empty and nonempty variadic argument cases deliberately.
- **T06.04.b** Compare C23 __VA_OPT__ with documented older extensions.
- **T06.04.c** Design macros that preserve argument evaluation contracts.

##### T06.05 — Macro safety

- **T06.05.a** Prevent unintended repeated evaluation and precedence changes.
- **T06.05.b** Analyze statement macros, dangling else, and scope interactions.
- **T06.05.c** Avoid reserved names and collisions with caller identifiers.

##### T06.06 — X-macros and registries

- **T06.06.a** Generate enums, names, dispatch tables, and validation tables.
- **T06.06.b** Maintain one authoritative input representation for related artifacts.
- **T06.06.c** Check completeness and consistency with compilation tests.

##### T06.07 — Type-directed interfaces

- **T06.07.a** Implement _Generic dispatch with documented compatible-type rules.
- **T06.07.b** Compare generic selection with static inline specialization.
- **T06.07.c** Investigate diagnostics from unselected associations rather than assuming lazy parsing.

##### T06.08 — Configuration architecture

- **T06.08.a** Separate build configuration from public API contracts.
- **T06.08.b** Validate incompatible feature combinations at compilation time.
- **T06.08.c** Preserve generated configuration provenance and reproducibility.

##### T06.09 — External code generation

- **T06.09.a** Define schemas for repetitive code and register or protocol descriptions.
- **T06.09.b** Test generated artifacts against the generator inputs.
- **T06.09.c** Evaluate macros versus generation using debugging and maintenance evidence.

**Domain mastery checkpoint:** Develop a small type-specialized component generator and compare its safety, generated code, diagnostics, and maintenance costs with alternatives.

<a id="t07"></a>

#### T07 — Compilation and build engineering

**Track:** Shared. **Depth:** Core mastery. **Entry foundations:** T01, T02.

**Primary-source anchors:** [^S02], [^S04], [^S05], [^S10].

##### T07.01 — Build pipeline

- **T07.01.a** Invoke preprocessing, compilation, assembly, and linking independently.
- **T07.01.b** Explain each intermediate file and its consumers.
- **T07.01.c** Reproduce a compiler-driver command using explicit stages.

##### T07.02 — Compiler driver and target selection

- **T07.02.a** Distinguish target triple, CPU, ISA features, and ABI.
- **T07.02.b** Inspect implicit include paths, libraries, and startup objects.
- **T07.02.c** Separate host tools from target executables during cross-compilation.

##### T07.03 — Diagnostics policy

- **T07.03.a** Select warnings according to target, edition, and project boundaries.
- **T07.03.b** Test expected warnings and expected compilation failures.
- **T07.03.c** Scope suppressions narrowly and document their justification.

##### T07.04 — Optimization and debug profiles

- **T07.04.a** Compare optimization levels through behavior and generated code.
- **T07.04.b** Configure separate debug, sanitizer, release, and size-oriented builds.
- **T07.04.c** Avoid assuming debug and release profiles have identical dependencies.

##### T07.05 — Make fundamentals

- **T07.05.a** Model explicit targets, prerequisites, recipes, and generated dependencies.
- **T07.05.b** Distinguish ordinary files, phony targets, and order-only prerequisites.
- **T07.05.c** Diagnose stale, missing, and unnecessarily rebuilt outputs.

##### T07.06 — CMake target model

- **T07.06.a** Distinguish sources, usage requirements, and transitive link requirements.
- **T07.06.b** Apply PRIVATE, PUBLIC, and INTERFACE at correct boundaries.
- **T07.06.c** Export and consume a library without leaking build-tree assumptions.

##### T07.07 — Generated inputs and configuration

- **T07.07.a** Sequence code generation and configuration checks correctly.
- **T07.07.b** Validate generated headers and embedded resource dependencies.
- **T07.07.c** Avoid target-code execution during unsupported cross-configuration checks.

##### T07.08 — Ninja and compilation databases

- **T07.08.a** Inspect incremental-build decisions and dependency graphs.
- **T07.08.b** Generate compilation databases for analysis and editor tooling.
- **T07.08.c** Reproduce individual translation-unit commands outside the build front end.

##### T07.09 — Cross-toolchain configuration

- **T07.09.a** Set compiler, sysroot, linker script, and target libraries explicitly.
- **T07.09.b** Control find-root behavior and host utility discovery.
- **T07.09.c** Audit multilib and floating-point ABI selection.

##### T07.10 — Dependency acquisition

- **T07.10.a** Pin source versions and record integrity identifiers.
- **T07.10.b** Compare system packages, vendored source, and fetched dependencies.
- **T07.10.c** Define offline and clean-room build prerequisites.

##### T07.11 — Testing and CI integration

- **T07.11.a** Register unit, integration, negative-build, and on-target tests.
- **T07.11.b** Test representative compiler, architecture, and configuration combinations.
- **T07.11.c** Preserve logs and failing binaries for reproducible diagnosis.

##### T07.12 — Reproducible artifacts

- **T07.12.a** Control timestamps, paths, generated identifiers, and archive ordering.
- **T07.12.b** Compare output hashes and explain unavoidable variability.
- **T07.12.c** Record compiler, linker, source, configuration, and dependency provenance.

**Domain mastery checkpoint:** Deliver GCC and Clang builds, hosted tests, a cross-build, and a reproducibility manifest with target-scoped dependencies.

<a id="t08"></a>

#### T08 — Machine architecture, assembly, and ABIs

**Track:** Shared. **Depth:** Core working proficiency; advanced mastery. **Entry foundations:** T03, T04, T07.

**Primary-source anchors:** [^S05], [^S06].

##### T08.01 — Instruction-set fundamentals

- **T08.01.a** Read arithmetic, branches, calls, loads, and stores in disassembly.
- **T08.01.b** Relate addressing modes to C array and structure accesses.
- **T08.01.c** Distinguish instruction-set guarantees from microarchitectural behavior.

##### T08.02 — Registers and flags

- **T08.02.a** Track general, stack, status, and floating-point registers.
- **T08.02.b** Identify caller-saved and callee-saved responsibilities.
- **T08.02.c** Explain condition flags without assuming portable access from C.

##### T08.03 — Call boundaries

- **T08.03.a** Map parameters and return values to the chosen ABI.
- **T08.03.b** Analyze aggregate passing and hidden return-storage conventions.
- **T08.03.c** Inspect stack alignment and argument spill areas.

##### T08.04 — Stack frames

- **T08.04.a** Identify frame setup, teardown, locals, spills, and saved registers.
- **T08.04.b** Analyze recursion, tail calls, and omitted frame pointers.
- **T08.04.c** Apply red-zone assumptions only to environments that support them.

##### T08.05 — Data layout and ABI compatibility

- **T08.05.a** Compare ILP32, LP64, and other relevant data models.
- **T08.05.b** Inspect structure padding, alignment, and exported layout.
- **T08.05.c** Detect mixed-object ABI mismatches before deployment.

##### T08.06 — Floating-point ABI

- **T08.06.a** Distinguish arithmetic instruction availability from calling convention.
- **T08.06.b** Compare soft, softfp, and hard-float configurations where applicable.
- **T08.06.c** Audit library compatibility and floating-point register preservation.

##### T08.07 — Standalone assembly

- **T08.07.a** Write callable routines with explicit symbol and section declarations.
- **T08.07.b** Generate useful unwind/debug annotations where supported.
- **T08.07.c** Test register preservation and stack alignment mechanically.

##### T08.08 — Inline assembly

- **T08.08.a** Specify input, output, tied, and early-clobber operands.
- **T08.08.b** Explain register and memory clobbers and their limits.
- **T08.08.c** Compare compiler barriers with actual processor-ordering instructions.

##### T08.09 — Exceptions and privileged execution

- **T08.09.a** Distinguish ordinary calls, traps, interrupts, and exceptions.
- **T08.09.b** Identify privilege restrictions on system instructions.
- **T08.09.c** Separate architectural exception state from language-level control flow.

##### T08.10 — Compiler-to-machine reasoning

- **T08.10.a** Compare source, intermediate representation, and final disassembly.
- **T08.10.b** Explain branch elimination, inlining, strength reduction, and spills.
- **T08.10.c** Identify undefined assumptions before blaming an optimizer transformation.

**Domain mastery checkpoint:** Write and inspect interoperable C/assembly routines whose register, stack, argument, return, and clobber behavior matches a selected ABI.

<a id="t09"></a>

#### T09 — Linkers, ELF, and binary inspection

**Track:** Shared. **Depth:** Core mastery. **Entry foundations:** T07, T08.

**Primary-source anchors:** [^S04], [^S06], [^S11].

##### T09.01 — ELF structure

- **T09.01.a** Parse headers, section tables, program headers, and symbol tables.
- **T09.01.b** Distinguish link-editor organization from loader-visible mappings.
- **T09.01.c** Inspect relocatable objects, executables, and shared objects.

##### T09.02 — Symbols and binding

- **T09.02.a** Explain local, global, weak, undefined, and common symbols.
- **T09.02.b** Trace symbol resolution across objects and archives.
- **T09.02.c** Diagnose duplicate definitions and accidentally exported internals.

##### T09.03 — Static archives

- **T09.03.a** Build and inspect archive indexes and members.
- **T09.03.b** Explain demand-driven extraction and link-order sensitivity.
- **T09.03.c** Use grouping or whole-archive behavior only with explicit justification.

##### T09.04 — Relocations

- **T09.04.a** Relate relocation entries to unresolved address computations.
- **T09.04.b** Compare absolute, PC-relative, and selected architecture relocations.
- **T09.04.c** Diagnose relocation overflow and incompatible relocation models.

##### T09.05 — Position-independent code

- **T09.05.a** Compare PIC, PIE, non-PIE, and static relocation assumptions.
- **T09.05.b** Inspect address formation and indirection in generated code.
- **T09.05.c** Identify relocation costs and restrictions in the selected environment.

##### T09.06 — Dynamic linking

- **T09.06.a** Trace interpreter selection, dependency discovery, and symbol lookup.
- **T09.06.b** Inspect GOT, PLT, binding behavior, and symbol interposition.
- **T09.06.c** Compare startup and lazy-resolution behavior without assuming universality.

##### T09.07 — TLS and initialization metadata

- **T09.07.a** Inspect thread-local sections and access models.
- **T09.07.b** Locate initialization/finalization arrays and their execution mechanisms.
- **T09.07.c** Separate linker metadata from the code that actually processes it.

##### T09.08 — Linker-script fundamentals

- **T09.08.a** Define memory regions, output sections, and placement constraints.
- **T09.08.b** Distinguish load addresses from execution addresses.
- **T09.08.c** Export linker symbols through documented C/assembly conventions.

##### T09.09 — Retention and garbage collection

- **T09.09.a** Control section collection, KEEP, and archive extraction.
- **T09.09.b** Account for interrupt tables and registration entries with indirect references.
- **T09.09.c** Diagnose missing code caused by unintended section elimination.

##### T09.10 — Memory and image budgets

- **T09.10.a** Use map files and linker assertions for memory limits.
- **T09.10.b** Quantify text, read-only data, initialized data, and zero-fill storage.
- **T09.10.c** Compare file size, load image size, and runtime RAM use.

##### T09.11 — ABI evolution

- **T09.11.a** Control visibility and stable exported interfaces.
- **T09.11.b** Study symbol versioning and compatibility testing for shared libraries.
- **T09.11.c** Compare source compatibility with binary compatibility.

##### T09.12 — Binary transformations

- **T09.12.a** Convert ELF to raw binary and Intel HEX deliberately.
- **T09.12.b** Preserve debug artifacts, build identifiers, and address correlation.
- **T09.12.c** Audit stripping, post-link patching, and checksum insertion steps.

**Domain mastery checkpoint:** Explain every significant section, segment, relocation, dependency, and startup symbol in a hosted executable and a firmware image.

<a id="t10"></a>

#### T10 — Hosted startup and minimal Linux executables

**Track:** Linux. **Depth:** Core working proficiency; advanced runtime mastery. **Entry foundations:** T08, T09.

**Primary-source anchors:** [^S04], [^S09], [^S12].

##### T10.01 — Kernel-to-userspace handoff

- **T10.01.a** Trace exec loading, mappings, and initial entry selection.
- **T10.01.b** Identify which responsibilities belong to kernel and dynamic loader.
- **T10.01.c** Distinguish an executable entry point from a C function call.

##### T10.02 — Initial process stack

- **T10.02.a** Inspect argc, argument strings, environment, and auxiliary vectors.
- **T10.02.b** Decode addresses with the selected process-entry ABI.
- **T10.02.c** Respect alignment and object-lifetime assumptions during startup.

##### T10.03 — Startup object families

- **T10.03.a** Inventory crt0, crt1, crti/crtn, and compiler CRT objects for one toolchain.
- **T10.03.b** Explain target-specific naming and division of responsibility.
- **T10.03.c** Distinguish startup assembly filenames from standardized interfaces.

##### T10.04 — C runtime initialization

- **T10.04.a** Trace TLS, stream state, initialization hooks, and runtime setup.
- **T10.04.b** Identify which services custom startup chooses to omit.
- **T10.04.c** Audit assumptions made by any retained library calls.

##### T10.05 — Process termination

- **T10.05.a** Compare returning from main, exit, _Exit, and Linux exit syscalls.
- **T10.05.b** Account for flushing, registered handlers, and process/thread distinctions.
- **T10.05.c** Define behavior when a custom entry routine unexpectedly returns.

##### T10.06 — Syscall ABI

- **T10.06.a** Map syscall numbers, arguments, clobbers, and results to registers.
- **T10.06.b** Compare syscall ABI with ordinary C-call ABI.
- **T10.06.c** Separate raw kernel error returns from libc errno conventions.

##### T10.07 — Minimal output and input

- **T10.07.a** Implement partial-transfer handling and interruption policies.
- **T10.07.b** Handle EOF, zero progress, and operation-specific error conditions.
- **T10.07.c** Keep syscall wrappers separate from higher-level stream abstractions.

##### T10.08 — Minimal executable variants

- **T10.08.a** Compare dynamic, static, and static-PIE startup requirements.
- **T10.08.b** Build a deliberately small executable with stated retained support.
- **T10.08.c** Audit loader metadata rather than relying on file-size comparisons.

##### T10.09 — Runtime expansion

- **T10.09.a** Add argument parsing, controlled storage, and error formatting incrementally.
- **T10.09.b** Measure every new dependency introduced by compiler and code changes.
- **T10.09.c** Maintain an explicit supported-service contract.

##### T10.10 — Startup failure diagnosis

- **T10.10.a** Debug before main using entry breakpoints and disassembly.
- **T10.10.b** Repair stack alignment, relocation, and initialization failures.
- **T10.10.c** Reproduce faults with minimized entry and link configurations.

**Domain mastery checkpoint:** Build a custom-entry Linux program, identify all dependencies, and explain initialization, syscall behavior, and termination without attributing startup to main.

<a id="t11"></a>

#### T11 — Freestanding and minimal-runtime construction

**Track:** Cross-track. **Depth:** Core mastery. **Entry foundations:** T03, T04, T07, T08, T09.

**Primary-source anchors:** [^S02], [^S03], [^S04], [^S05], [^S06].

##### T11.01 — Dependency profiles

- **T11.01.a** Separate hosted, freestanding, libc-free, allocation-free, and custom-startup requirements.
- **T11.01.b** State whether restrictions apply to component or entire executable.
- **T11.01.c** Enumerate headers, libraries, runtime helpers, syscalls, and allocation sources.

##### T11.02 — Compiler and linker controls

- **T11.02.a** Compare -ffreestanding and -fno-builtin with link-stage exclusions.
- **T11.02.b** Distinguish -nostdlib, -nodefaultlibs, and -nostartfiles experimentally.
- **T11.02.c** Treat header-search restrictions as a separate build dimension.

##### T11.03 — Compiler-generated memory operations

- **T11.03.a** Detect emitted memcpy, memmove, memset, and comparison operations.
- **T11.03.b** Exercise aggregate copies and initialization at different optimization levels.
- **T11.03.c** Supply documented support routines where required by the toolchain.

##### T11.04 — Arithmetic support

- **T11.04.a** Identify division, multiplication, conversion, and soft-float helpers.
- **T11.04.b** Inspect libgcc or compiler-rt symbol contracts for the target.
- **T11.04.c** Decide whether to link, implement, or avoid each helper.

##### T11.05 — Atomic and instrumentation support

- **T11.05.a** Detect out-of-line atomic operations and support-library calls.
- **T11.05.b** Audit stack-protection, sanitizer, profiling, and coverage runtime requirements.
- **T11.05.c** Keep verification-only dependencies separate from production dependency claims.

##### T11.06 — Minimal memory primitives

- **T11.06.a** Implement byte-copy, overlap-safe move, set, and comparison operations.
- **T11.06.b** Test boundaries, overlap contracts, alignment, and zero-length conditions.
- **T11.06.c** Prevent optimizer transformations from creating unintended self-recursive support calls.

##### T11.07 — Environment adaptation

- **T11.07.a** Specify the services required by a selected embedded C library.
- **T11.07.b** Examine syscall retargeting, heap extension, and stream backends.
- **T11.07.c** Distinguish semihosting dependencies from standalone board operation.

##### T11.08 — Runtime entry contracts

- **T11.08.a** Specify initialized RAM, stack, exception state, and entry arguments.
- **T11.08.b** Define responsibility for TLS and initialization hooks when retained.
- **T11.08.c** Make unsupported hosted services fail visibly rather than accidentally work.

##### T11.09 — Automated dependency audit

- **T11.09.a** Check undefined symbols, linked objects, and dynamic dependencies.
- **T11.09.b** Inspect final disassembly for compiler helpers and instrumentation.
- **T11.09.c** Compare audits across optimization, compiler, and configuration changes.

##### T11.10 — Runtime validation

- **T11.10.a** Differential-test primitives against an independent hosted reference.
- **T11.10.b** Run failure and boundary tests without relying on the forbidden runtime.
- **T11.10.c** Document supported targets, assumptions, and excluded execution contexts.

**Domain mastery checkpoint:** Deliver a runtime support package whose allowed external symbols, startup behavior, allocation sources, and compiler-support obligations are automatically audited.

### Phase III — Libraries, algorithms, representations, and memory management

<a id="t12"></a>

#### T12 — Standard-library semantics and implementation literacy

**Track:** Shared. **Depth:** Core mastery. **Entry foundations:** T02, T03, T04.

**Primary-source anchors:** [^S01], [^S02], [^S09].

##### T12.01 — Library and header taxonomy

- **T12.01.a** Separate the complete standard library from the single stdlib.h header.
- **T12.01.b** Inventory headers and function families by language edition.
- **T12.01.c** Distinguish ISO, POSIX, optional annex, and implementation-specific interfaces.

##### T12.02 — Types, limits, and feature interfaces

- **T12.02.a** Use standard limit, type, format, and feature macros correctly.
- **T12.02.b** Audit fixed-width integer formatting and size-dependent conversions.
- **T12.02.c** Separate compiler-provided headers from external linked facilities.

##### T12.03 — Memory operations

- **T12.03.a** Specify overlap, object validity, size, and return-value contracts.
- **T12.03.b** Compare copying representations with constructing typed values.
- **T12.03.c** Test boundary cases without inventing universally valid null-pointer exceptions.

##### T12.04 — Byte strings and bounded buffers

- **T12.04.a** Distinguish byte arrays, terminated strings, and pointer/length views.
- **T12.04.b** Track capacity, logical length, termination, and truncation policies.
- **T12.04.c** Compare strcpy, strncpy, snprintf, and explicit-length interfaces by contract.

##### T12.05 — Character and locale handling

- **T12.05.a** Check valid domains for ctype functions and macros.
- **T12.05.b** Distinguish byte classification from Unicode character properties.
- **T12.05.c** Identify global and per-operation locale assumptions.

##### T12.06 — Multibyte and wide-character conversion

- **T12.06.a** Track conversion state and incomplete input across calls.
- **T12.06.b** Handle invalid sequences and output-capacity limits.
- **T12.06.c** Separate wchar_t representation from a portable Unicode interchange format.

##### T12.07 — Allocation API semantics

- **T12.07.a** Compare malloc, calloc, realloc, free, and aligned allocation.
- **T12.07.b** Analyze failure, zero-size, alignment, and lifetime rules by edition.
- **T12.07.c** Preserve ownership correctly when resizing operations fail.

##### T12.08 — Integer and floating conversion

- **T12.08.a** Compare strto-family APIs with weaker conversion interfaces.
- **T12.08.b** Validate end pointers, range errors, signs, bases, and trailing data.
- **T12.08.c** Specify locale and rounding assumptions for numeric conversions.

##### T12.09 — Formatted output

- **T12.09.a** Match variadic argument types to format specifications.
- **T12.09.b** Handle precision, width, length modifiers, and required output size.
- **T12.09.c** Design bounded formatting with explicit truncation and error reporting.

##### T12.10 — Formatted input

- **T12.10.a** Analyze conversion counts, field widths, and destination storage.
- **T12.10.b** Compare scanf-family parsing with explicit tokenization and conversion.
- **T12.10.c** Handle ambiguous input, leftover characters, and EOF distinctly.

##### T12.11 — Streams and buffering

- **T12.11.a** Model FILE ownership, orientation, buffering, and error indicators.
- **T12.11.b** Distinguish EOF, failure, flushing, positioning, and update-stream transitions.
- **T12.11.c** Relate standard streams to lower-level OS descriptors without conflating them.

##### T12.12 — Searching, sorting, and random numbers

- **T12.12.a** Specify comparator consistency and avoid arithmetic-overflow comparisons.
- **T12.12.b** Separate mandated semantics from implementation algorithm choices.
- **T12.12.c** Compare reproducible pseudorandom use with security-sensitive randomness requirements.

##### T12.13 — Mathematics and floating environment

- **T12.13.a** Map scalar, complex, type-generic, and environment facilities.
- **T12.13.b** Examine domain/range errors and target-dependent floating behavior.
- **T12.13.c** Compare mathematical accuracy specifications with observed numerical results.

##### T12.14 — Time and execution environment

- **T12.14.a** Distinguish calendar time, CPU time, and elapsed-time measurements.
- **T12.14.b** Audit environment variables, termination handlers, and system interaction.
- **T12.14.c** Identify implementation-dependent time representations and concurrency hazards.

##### T12.15 — Signals, nonlocal jumps, and thread facilities

- **T12.15.a** Separate ISO signals from richer operating-system signal interfaces.
- **T12.15.b** Analyze setjmp/longjmp lifetime, local-state, and cleanup consequences.
- **T12.15.c** Locate thread and atomic contracts before applying platform assumptions.

##### T12.16 — Implementation source literacy

- **T12.16.a** Navigate pinned glibc and musl implementations of selected routines.
- **T12.16.b** Compare an embedded-oriented library's configuration and backend requirements.
- **T12.16.c** Separate public contracts, internal optimization, and application-visible extensions.

**Domain mastery checkpoint:** Build an edition-aware library map and audited wrappers for representative routines, with explicit ownership, errors, side effects, and portability limits.

<a id="t13"></a>

#### T13 — Implicit resource use and hidden costs

**Track:** Cross-track. **Depth:** Advanced core mastery. **Entry foundations:** T08, T09, T12.

**Primary-source anchors:** [^S04], [^S09], [^S12].

##### T13.01 — Effects contract

- **T13.01.a** Enumerate allocation, stack, locks, syscalls, state changes, and blocking.
- **T13.01.b** Separate caller-visible outputs from indirect environmental effects.
- **T13.01.c** State component-local and transitive resource requirements independently.

##### T13.02 — Heap allocation census

- **T13.02.a** Trace direct, indirect, lazy, and allocator-internal memory activity.
- **T13.02.b** Compare allocator interposition, wrappers, source inspection, and profiling.
- **T13.02.c** Identify paths that bypass the chosen interception technique.

##### T13.03 — Stack consumption

- **T13.03.a** Inspect static frames, recursion, VLAs, and dynamic stack use.
- **T13.03.b** Compare compiler estimates with observed high-water marks.
- **T13.03.c** Include interrupt nesting and library calls in whole-system budgets.

##### T13.04 — Locking and contention

- **T13.04.a** Identify stream, allocator, locale, and initialization synchronization.
- **T13.04.b** Distinguish lock acquisition from observed contention and blocking.
- **T13.04.c** Test uncontended, contended, and reentrant-call conditions separately.

##### T13.05 — Lazy initialization

- **T13.05.a** Compare first-call and steady-state execution paths.
- **T13.05.b** Identify initialization triggered by locale, formatting, or runtime services.
- **T13.05.c** Reproduce results in genuinely fresh processes or reset states.

##### T13.06 — System interaction

- **T13.06.a** Trace filesystem, terminal, loader, memory, and scheduling interactions.
- **T13.06.b** Explain why syscall absence does not prove allocation absence.
- **T13.06.c** Attribute user-space work separately from kernel execution.

##### T13.07 — Global and thread-local state

- **T13.07.a** Identify errno, locale, stream, random, and cached state.
- **T13.07.b** Assess interference between calls and threads.
- **T13.07.c** Define reset, reproducibility, and independent-instance requirements.

##### T13.08 — Execution-context suitability

- **T13.08.a** Assess thread safety, reentrancy, and async-signal safety independently.
- **T13.08.b** Evaluate ISR suitability against platform and runtime contracts.
- **T13.08.c** Separate bounded storage from bounded execution and blocking.

##### T13.09 — Implementation comparison

- **T13.09.a** Audit the same API across pinned library versions.
- **T13.09.b** Test configuration-dependent paths such as floating formatting or locale support.
- **T13.09.c** Present observed behavior separately from portable guarantees.

##### T13.10 — Replacement decisions

- **T13.10.a** Define a restricted alternative only after characterizing the actual need.
- **T13.10.b** Compare correctness, size, latency, and maintenance with the original routine.
- **T13.10.c** Validate that replacement dependencies satisfy the intended effects contract.

**Domain mastery checkpoint:** Produce a routine-by-routine effects ledger with source evidence, instrumentation results, failure cases, blind spots, and version-specific conclusions.

<a id="t14"></a>

#### T14 — Data structures implemented from first principles

**Track:** Shared. **Depth:** Core implementation mastery; selected electives. **Entry foundations:** T03, T04, T18.

##### T14.01 — Representation contracts

- **T14.01.a** Define logical contents separately from concrete storage layout.
- **T14.01.b** Specify empty, full, invalid, and destroyed states.
- **T14.01.c** Establish ownership and mutation rules before implementation.

##### T14.02 — Fixed arrays and views

- **T14.02.a** Implement bounded access, insertion, erasure, and traversal.
- **T14.02.b** Compare owning storage with non-owning slices and strided views.
- **T14.02.c** Prove capacity and overlap requirements for each operation.

##### T14.03 — Dynamic arrays

- **T14.03.a** Implement growth, reserve, shrink, append, and insertion policies.
- **T14.03.b** Analyze amortized costs and reallocation-induced invalidation.
- **T14.03.c** Preserve defined state under allocation failure.

##### T14.04 — Linked structures

- **T14.04.a** Implement singly and doubly linked lists with sentinel alternatives.
- **T14.04.b** Verify link consistency, removal, and iterator behavior.
- **T14.04.c** Compare intrusive nodes with separately allocated wrapper nodes.

##### T14.05 — Stacks, queues, and deques

- **T14.05.a** Implement bounded and dynamically backed variants.
- **T14.05.b** Compare array and linked representations by operation costs.
- **T14.05.c** Specify overflow, underflow, and batch-operation behavior.

##### T14.06 — Ring buffers

- **T14.06.a** Define head/tail conventions and full/empty disambiguation.
- **T14.06.b** Implement wraparound, split spans, and bulk transfers.
- **T14.06.c** Keep single-thread correctness separate from later concurrent publication rules.

##### T14.07 — Hash tables

- **T14.07.a** Implement at least one chained and one open-addressed design.
- **T14.07.b** Analyze load factors, collision handling, tombstones, and resizing.
- **T14.07.c** Test adversarial keys and unstable-key ownership defects.

##### T14.08 — Heaps and priority queues

- **T14.08.a** Implement binary-heap insertion, extraction, and heap construction.
- **T14.08.b** Support or explicitly exclude update-key and stable-priority semantics.
- **T14.08.c** Prove heap invariants and index-arithmetic safety.

##### T14.09 — Search trees

- **T14.09.a** Implement ordered search, insertion, deletion, and traversal.
- **T14.09.b** Compare recursive and explicit-stack operations.
- **T14.09.c** Track parent links, ownership, and mutation invalidation.

##### T14.10 — Balanced trees

- **T14.10.a** Implement one AVL or red-black family completely.
- **T14.10.b** Test rotations, rebalancing, and deletion corner cases.
- **T14.10.c** Compare alternative balancing policies without shallow duplicate implementations.

##### T14.11 — Tries and prefix indexes

- **T14.11.a** Implement one byte-oriented prefix representation.
- **T14.11.b** Compare memory cost with compressed or radix variants.
- **T14.11.c** Specify key ownership and termination handling.

##### T14.12 — Bitsets and sparse sets

- **T14.12.a** Implement bit operations, iteration, and set algebra.
- **T14.12.b** Compare dense, sparse, and generation-based representations.
- **T14.12.c** Bound storage and clearing costs for constrained systems.

##### T14.13 — Disjoint sets and graphs

- **T14.13.a** Implement union-find with documented optimization invariants.
- **T14.13.b** Build adjacency-list, matrix, and compressed graph representations.
- **T14.13.c** Separate representation operations from graph algorithms in T15.

##### T14.14 — Advanced structures [E]

- **T14.14.a** Explore B-trees, skip lists, and probabilistic membership filters.
- **T14.14.b** Implement a selected structure for a concrete workload.
- **T14.14.c** Account for persistence or probabilistic guarantees where applicable.

##### T14.15 — Container verification

- **T14.15.a** Generate mutation sequences against a simple reference model.
- **T14.15.b** Check invariants after every operation and injected failure.
- **T14.15.c** Benchmark representative workloads without replacing asymptotic reasoning.

**Domain mastery checkpoint:** Deliver an enumerated container library with caller-storage or allocator injection, invariants, invalidation rules, model-based tests, and complexity arguments.

<a id="t15"></a>

#### T15 — Algorithms, byte processing, and parsers

**Track:** Shared. **Depth:** Core mastery; deeper methods elective. **Entry foundations:** T04, T14.

##### T15.01 — Search and partition

- **T15.01.a** Implement linear search, binary search, and lower/upper bounds.
- **T15.01.b** Prove loop invariants and empty-range behavior.
- **T15.01.c** Implement partitioning without overflow-prone midpoint or index calculations.

##### T15.02 — Elementary and comparison sorting

- **T15.02.a** Implement insertion sort as a small-input reference.
- **T15.02.b** Implement merge sort and heap sort with explicit storage contracts.
- **T15.02.c** Compare stability, comparisons, writes, and worst-case guarantees.

##### T15.03 — Quicksort and hybrid sorting

- **T15.03.a** Implement partition schemes and duplicate-key handling.
- **T15.03.b** Analyze recursion depth and adversarial pivots.
- **T15.03.c** Add a justified fallback rather than relying on average-case behavior.

##### T15.04 — Counting and radix methods

- **T15.04.a** Implement counting sort and one radix-sort variant.
- **T15.04.b** Specify key domains, signedness, and temporary-storage costs.
- **T15.04.c** Compare suitability with comparison-based sorting.

##### T15.05 — Selection and order statistics

- **T15.05.a** Implement quickselect or another selected rank algorithm.
- **T15.05.b** Define behavior with duplicates and invalid ranks.
- **T15.05.c** Compare expected and worst-case complexity honestly.

##### T15.06 — String matching

- **T15.06.a** Implement naive matching and one linear-time algorithm.
- **T15.06.b** Preserve boundaries for non-terminated byte sequences.
- **T15.06.c** Compare prefix-function, failure-table, or rolling-hash trade-offs.

##### T15.07 — Graph traversal

- **T15.07.a** Implement breadth-first and depth-first traversal.
- **T15.07.b** Track visited state, disconnected graphs, and cycles.
- **T15.07.c** Replace recursion with bounded explicit work storage where needed.

##### T15.08 — Graph ordering and paths

- **T15.08.a** Implement topological ordering and cycle detection.
- **T15.08.b** Implement shortest paths under explicit edge-weight assumptions.
- **T15.08.c** Compare Dijkstra, Bellman-Ford, and unweighted traversal appropriately.

##### T15.09 — Greedy and dynamic programming

- **T15.09.a** Derive one greedy algorithm with an exchange argument.
- **T15.09.b** Implement dynamic programming with state and dependency definitions.
- **T15.09.c** Analyze memory reduction without invalidating update dependencies.

##### T15.10 — Byte primitives and spans

- **T15.10.a** Implement bounded copy, compare, search, split, and trim operations.
- **T15.10.b** Define aliasing, termination, and empty-input contracts.
- **T15.10.c** Share primitives with the runtime without creating dependency cycles.

##### T15.11 — Numeric parsing and formatting

- **T15.11.a** Implement checked integer parsing and bounded integer formatting.
- **T15.11.b** Specify bases, prefixes, signs, overflow, and partial consumption.
- **T15.11.c** Treat full floating conversion as a separate advanced implementation problem.

##### T15.12 — Serialization and integrity

- **T15.12.a** Implement endian-independent field encoding and decoding.
- **T15.12.b** Specify field widths, versioning, reserved bits, and unknown extensions.
- **T15.12.c** Implement checksums and CRCs with independently validated test vectors.

##### T15.13 — Framing and incremental parsing

- **T15.13.a** Build delimiter, length-prefixed, or escaped frame parsers.
- **T15.13.b** Support arbitrary input chunk boundaries and backpressure.
- **T15.13.c** Bound buffering, nesting, recovery work, and malformed-input cost.

##### T15.14 — Lexing and grammar processing

- **T15.14.a** Implement a tokenizer with source-location tracking.
- **T15.14.b** Build a recursive-descent or precedence-based expression parser.
- **T15.14.c** Separate syntax trees, semantic validation, and diagnostics.

##### T15.15 — Advanced algorithm engineering [E]

- **T15.15.a** Explore suffix indexes, compression, or specialized numerical conversion.
- **T15.15.b** Select algorithms against measured workload and resource constraints.
- **T15.15.c** Preserve a simple reference implementation as a correctness oracle.

**Domain mastery checkpoint:** Implement a documented core algorithm set and a streaming binary parser with malformed-input tests, complexity bounds, and no hidden algorithm-library shortcuts.

<a id="t16"></a>

#### T16 — Allocation strategies and lifetime-oriented design

**Track:** Cross-track. **Depth:** Core mastery; specialized allocators elective. **Entry foundations:** T03, T04, T14, T18.

##### T16.01 — Allocator contracts

- **T16.01.a** Specify allocate, deallocate, resize, and alignment semantics.
- **T16.01.b** Define zero-size requests, exhaustion, ownership, and supported lifetimes.
- **T16.01.c** Distinguish storage acquisition from object initialization.

##### T16.02 — Layout and checked arithmetic

- **T16.02.a** Calculate aligned offsets without overflowing intermediate arithmetic.
- **T16.02.b** Separate requested payload from headers, padding, and usable capacity.
- **T16.02.c** Verify metadata alignment and minimum block sizes.

##### T16.03 — Bump and arena allocation

- **T16.03.a** Implement allocation from caller-owned contiguous storage.
- **T16.03.b** Support explicit reset with invalidation of all affected objects.
- **T16.03.c** Compare high-water usage with total requested payload.

##### T16.04 — Checkpoints and temporary regions

- **T16.04.a** Implement marks, rollback, and nested temporary scopes.
- **T16.04.b** Define which references survive each rollback boundary.
- **T16.04.c** Detect escaped pointers and out-of-order checkpoint misuse.

##### T16.05 — Stack allocation

- **T16.05.a** Implement strict last-in-first-out release using explicit metadata.
- **T16.05.b** Compare stack allocators with language automatic storage.
- **T16.05.c** Specify destructor or cleanup responsibilities separately from byte reclamation.

##### T16.06 — Fixed-block pools

- **T16.06.a** Implement free-slot tracking with bounded storage.
- **T16.06.b** Compare embedded free links, bitmaps, and generation-counted handles.
- **T16.06.c** Detect stale handles, double release, and pool exhaustion.

##### T16.07 — Variable-size free lists

- **T16.07.a** Implement first-fit or best-fit selection with split policies.
- **T16.07.b** Maintain minimum-sized fragments and free-list invariants.
- **T16.07.c** Compare search cost and fragmentation under adversarial workloads.

##### T16.08 — Boundary tags and coalescing

- **T16.08.a** Encode neighboring-block information safely.
- **T16.08.b** Merge adjacent free blocks while preserving list consistency.
- **T16.08.c** Verify heap traversal and metadata after every mutation.

##### T16.09 — Segregated classes and slabs

- **T16.09.a** Group requests by size class and object lifetime.
- **T16.09.b** Quantify internal fragmentation and partially occupied slabs.
- **T16.09.c** Separate cache reuse from object reinitialization requirements.

##### T16.10 — Buddy and real-time-oriented allocation [E]

- **T16.10.a** Implement a selected buddy policy or study TLSF internals.
- **T16.10.b** Analyze splitting, merging, indexing, and metadata costs.
- **T16.10.c** Distinguish algorithmic operation bounds from complete hardware timing bounds.

##### T16.11 — Resize and migration

- **T16.11.a** Handle in-place growth, shrinkage, and moving resize.
- **T16.11.b** Preserve original ownership and contents on failure.
- **T16.11.c** Specify alias and iterator invalidation after successful movement.

##### T16.12 — Backing storage and concurrency

- **T16.12.a** Adapt static RAM, caller buffers, and Linux mappings.
- **T16.12.b** Compare per-thread arenas, shared heaps, and synchronization costs.
- **T16.12.c** Separate allocator synchronization from application-object synchronization.

##### T16.13 — Lifetime architecture

- **T16.13.a** Choose region boundaries from actual ownership and use intervals.
- **T16.13.b** Compare per-message, per-frame, per-request, and persistent allocation.
- **T16.13.c** Avoid imposing arena lifetimes on objects that need independent release.

##### T16.14 — Allocator verification

- **T16.14.a** Generate allocate/free/resize sequences with a reference ownership model.
- **T16.14.b** Check overlap, alignment, leaks, corruption, and exhaustion invariants.
- **T16.14.c** Measure peak usage, fragmentation, latency distribution, and instrumentation overhead.

**Domain mastery checkpoint:** Implement an arena, a fixed-block pool, and one variable-size allocator; verify invariants, exhaustion handling, alignment, and workload-dependent costs.

<a id="t17"></a>

#### T17 — Numerical and data-oriented C

**Track:** Cross-track. **Depth:** Core working proficiency; specialization elective. **Entry foundations:** T04, T14.

##### T17.01 — Numerical problem formulation

- **T17.01.a** State units, domains, tolerances, and conditioning assumptions.
- **T17.01.b** Separate physical-model error from numerical and representation errors.
- **T17.01.c** Define independently checkable invariants and reference cases.

##### T17.02 — Stable accumulation

- **T17.02.a** Compare naive, pairwise, and compensated summation.
- **T17.02.b** Test cancellation and changing accumulation order.
- **T17.02.c** Relate reproducibility requirements to parallel and vectorized implementations.

##### T17.03 — Fixed-point kernels

- **T17.03.a** Implement scaled multiply-accumulate with explicit widening.
- **T17.03.b** Derive saturation and rounding behavior at every boundary.
- **T17.03.c** Validate representative sensor and control signal ranges.

##### T17.04 — Vector and matrix representations

- **T17.04.a** Define dimensions, strides, layout, and aliasing contracts.
- **T17.04.b** Implement core vector operations and small matrix products.
- **T17.04.c** Compare row-major, transposed, and blocked traversal.

##### T17.05 — Interpolation and approximation

- **T17.05.a** Implement linear interpolation with boundary policies.
- **T17.05.b** Compare lookup tables and polynomial approximations.
- **T17.05.c** Quantify approximation error and computational cost.

##### T17.06 — Digital filters

- **T17.06.a** Implement FIR and a selected IIR structure.
- **T17.06.b** Specify state initialization, coefficient representation, and stability checks.
- **T17.06.c** Compare sample-wise and block-oriented interfaces.

##### T17.07 — Linear systems and iterative methods

- **T17.07.a** Implement a small solver with pivoting where needed.
- **T17.07.b** Define convergence, failure, and ill-conditioning diagnostics.
- **T17.07.c** Compare mathematical operation counts with actual storage traffic.

##### T17.08 — Signal transforms and simulation [E]

- **T17.08.a** Implement a small FFT or an ODE integration method.
- **T17.08.b** Validate scaling, normalization, conservation, or convergence properties.
- **T17.08.c** Separate mathematical model validation from implementation validation.

##### T17.09 — Data-oriented representations

- **T17.09.a** Compare structures of arrays with arrays of structures.
- **T17.09.b** Organize hot and cold fields by access patterns.
- **T17.09.c** Explain padding, stride, locality, and SIMD implications.

##### T17.10 — Reproducibility and reference testing

- **T17.10.a** Use higher-precision or analytically derived reference values.
- **T17.10.b** Control floating environment, compiler assumptions, and input generation.
- **T17.10.c** Report error distributions rather than only one passing tolerance.

**Domain mastery checkpoint:** Deliver a scalar numerical component with a mathematical specification, error budget, independent reference, and a layout suitable for later optimization.

### Phase IV — Interfaces, abstractions, architecture, and maintainability

<a id="t18"></a>

#### T18 — Contracts, ownership, and API design

**Track:** Shared. **Depth:** Core mastery. **Entry foundations:** T02, T03.

##### T18.01 — Domain and representation contracts

- **T18.01.a** Specify valid values, ranges, units, and sentinel meanings.
- **T18.01.b** Distinguish representable states from permitted states.
- **T18.01.c** Make assumptions about target representation explicit.

##### T18.02 — Function contracts

- **T18.02.a** Define preconditions, postconditions, and preserved invariants.
- **T18.02.b** Specify capacity, aliasing, mutation, and failure effects.
- **T18.02.c** Separate caller obligations from implementation checks.

##### T18.03 — Ownership vocabulary

- **T18.03.a** Distinguish owned, borrowed, shared, transferred, and weak references.
- **T18.03.b** Document which operation creates and releases each resource.
- **T18.03.c** Model ownership consistently for memory, descriptors, locks, and devices.

##### T18.04 — Lifetime relationships

- **T18.04.a** Express parent/child, view/owner, and callback/context lifetimes.
- **T18.04.b** Identify references invalidated by mutation or reset.
- **T18.04.c** Replace ambiguous lifetime rules with handles or explicit protocols.

##### T18.05 — Object lifecycle

- **T18.05.a** Specify initialization, active, failed, stopped, and destroyed states.
- **T18.05.b** Design safe partial initialization and repeated shutdown behavior.
- **T18.05.c** Identify operations legal in each state.

##### T18.06 — Opaque and exposed representations

- **T18.06.a** Compare incomplete types, exposed structs, and caller-owned opaque storage.
- **T18.06.b** Assess allocation, ABI, debugging, and embedding consequences.
- **T18.06.c** Avoid hiding layout through unsupported size/alignment guesses.

##### T18.07 — Errors and result types

- **T18.07.a** Compare status returns, tagged results, and output parameters.
- **T18.07.b** Define success and failure initialization of output values.
- **T18.07.c** Prevent ambiguous sentinel collisions with valid results.

##### T18.08 — Execution-context contracts

- **T18.08.a** Declare allocation, blocking, reentrancy, thread, and ISR properties.
- **T18.08.b** Identify whether callbacks execute synchronously or asynchronously.
- **T18.08.c** Specify cancellation and timeout behavior where relevant.

##### T18.09 — Contract enforcement

- **T18.09.a** Use compile-time checks, runtime validation, assertions, and tests appropriately.
- **T18.09.b** Define behavior when assertions or diagnostics are disabled.
- **T18.09.c** Keep contract violations distinguishable from ordinary environmental failures.

##### T18.10 — API evolution

- **T18.10.a** Version interfaces without silently changing ownership or layout.
- **T18.10.b** Test compatibility from an independent consumer project.
- **T18.10.c** Provide migration paths for deprecation and changed constraints.

**Domain mastery checkpoint:** Design an interface whose preconditions, ownership, state transitions, failure guarantees, and execution-context constraints can be tested independently of implementation.

<a id="t19"></a>

#### T19 — Higher-level abstractions implemented in C

**Track:** Shared. **Depth:** Core mastery. **Entry foundations:** T06, T18.

##### T19.01 — Objects as state and operations

- **T19.01.a** Represent object state explicitly and define valid transitions.
- **T19.01.b** Separate construction, operation, and destruction responsibilities.
- **T19.01.c** Compare plain structs with opaque object interfaces.

##### T19.02 — Function-table interfaces

- **T19.02.a** Define operation tables with consistent signatures and context ownership.
- **T19.02.b** Separate shared immutable tables from per-instance state.
- **T19.02.c** Version interfaces and handle optional operations deliberately.

##### T19.03 — Closures and callbacks

- **T19.03.a** Pair function pointers with explicit context pointers.
- **T19.03.b** Specify callback registration, lifetime, reentrancy, and removal.
- **T19.03.c** Handle callbacks occurring during destruction or cancellation.

##### T19.04 — Generic components

- **T19.04.a** Compare void-pointer, macro-generated, and _Generic interfaces.
- **T19.04.b** Maintain element size, alignment, copy, and destruction contracts.
- **T19.04.c** Evaluate compile-time diagnostics and runtime indirection costs.

##### T19.05 — Tagged unions and variants

- **T19.05.a** Represent alternatives with an explicit discriminant.
- **T19.05.b** Validate tag/payload consistency and exhaustive handling.
- **T19.05.c** Implement conversion and error variants without unsafe reinterpretation.

##### T19.06 — State and strategy patterns

- **T19.06.a** Separate transition logic from effectful operations.
- **T19.06.b** Compare tables, switches, and function-based dispatch.
- **T19.06.c** Justify dynamic dispatch against simpler static alternatives.

##### T19.07 — Iterators and ranges

- **T19.07.a** Specify traversal order, mutation, and invalidation behavior.
- **T19.07.b** Implement cursor-based and callback-based traversal.
- **T19.07.c** Compare pull and push iteration under bounded-storage requirements.

##### T19.08 — Streams and composable I/O

- **T19.08.a** Define read/write contracts including partial progress and errors.
- **T19.08.b** Compose buffering, framing, checksumming, and transport adapters.
- **T19.08.c** Preserve backpressure and ownership across abstraction boundaries.

##### T19.09 — Reference and handle abstractions

- **T19.09.a** Implement reference counting or generation-counted handles with explicit limits.
- **T19.09.b** Analyze cycles, overflow, stale access, and reclamation.
- **T19.09.c** Separate shared ownership from concurrent access safety.

##### T19.10 — Abstraction cost analysis

- **T19.10.a** Inspect generated code for each dispatch and representation strategy.
- **T19.10.b** Measure size, allocations, branches, and cache access where relevant.
- **T19.10.c** Remove abstraction only when evidence and maintainability justify the change.

**Domain mastery checkpoint:** Implement one component with static dispatch, callbacks, and a function-table interface; compare memory, lifetime, code-size, and testability costs.

<a id="t20"></a>

#### T20 — Modular and system architecture

**Track:** Cross-track. **Depth:** Core mastery. **Entry foundations:** T18, T19.

##### T20.01 — Module boundaries

- **T20.01.a** Group responsibilities by cohesion and ownership.
- **T20.01.b** Separate public interfaces from implementation and platform detail.
- **T20.01.c** Identify cycles and unstable dependency directions.

##### T20.02 — Mechanism and policy

- **T20.02.a** Distinguish reusable capability from deployment-specific choice.
- **T20.02.b** Inject allocation, timing, transport, and scheduling policies explicitly.
- **T20.02.c** Compare compile-time and runtime policy selection.

##### T20.03 — Functional core and effects

- **T20.03.a** Move deterministic transformations behind explicit inputs and outputs.
- **T20.03.b** Keep hardware and OS effects at identifiable boundaries.
- **T20.03.c** Design tests that exercise core logic without platform emulation.

##### T20.04 — Ports and adapters

- **T20.04.a** Define contracts for files, clocks, storage, and devices.
- **T20.04.b** Implement Linux, hardware, and test adapters independently.
- **T20.04.c** Preserve semantics across adapters rather than only matching signatures.

##### T20.05 — HAL, BSP, driver, and service layers

- **T20.05.a** Allocate responsibility for registers, board wiring, devices, and application services.
- **T20.05.b** Compare vendor HAL use with a purpose-built minimal layer.
- **T20.05.c** Avoid leaking register assumptions into portable logic.

##### T20.06 — Execution architecture

- **T20.06.a** Compare superloops, event loops, tasks, and thread pools.
- **T20.06.b** Model queues, ownership transfer, latency, and overload.
- **T20.06.c** Choose a design against actual timing and resource constraints.

##### T20.07 — State-machine composition

- **T20.07.a** Define hierarchical or cooperating machines where justified.
- **T20.07.b** Specify event ordering, deferred events, and cancellation.
- **T20.07.c** Prevent accidental dependence on callback or scheduler ordering.

##### T20.08 — Configuration architecture

- **T20.08.a** Separate build-time features from runtime parameters.
- **T20.08.b** Validate combinations and generate documented configuration identities.
- **T20.08.c** Keep configuration changes traceable to tests and binary artifacts.

##### T20.09 — Fault containment

- **T20.09.a** Distinguish logical boundaries from memory and privilege boundaries.
- **T20.09.b** Specify failure propagation, degradation, restart, and supervision.
- **T20.09.c** Test adapters that violate contracts without overstating isolation guarantees.

##### T20.10 — Architecture evaluation

- **T20.10.a** Record alternatives, assumptions, and decision criteria.
- **T20.10.b** Trace requirements to boundaries and observable behavior.
- **T20.10.c** Revisit decisions when workload, hardware, or safety assumptions change.

**Domain mastery checkpoint:** Deliver one shared C core with Linux and firmware adapters, explicit dependency direction, architectural decisions, and substitution tests.

<a id="t21"></a>

#### T21 — Error handling, resource management, and observability

**Track:** Cross-track. **Depth:** Core mastery. **Entry foundations:** T18.

##### T21.01 — Error taxonomy

- **T21.01.a** Separate invalid input, contract violations, resource failures, and hardware faults.
- **T21.01.b** Define recoverable, retryable, terminal, and degraded outcomes.
- **T21.01.c** Preserve enough context to identify the original cause.

##### T21.02 — Propagation and translation

- **T21.02.a** Map backend errors into stable component-level results.
- **T21.02.b** Retain causal context without exposing unstable implementation details.
- **T21.02.c** Avoid silent information loss and ambiguous success codes.

##### T21.03 — Partial initialization

- **T21.03.a** Track resource acquisition order and completed initialization stages.
- **T21.03.b** Roll back safely after every possible failure point.
- **T21.03.c** Test repeated initialization and cleanup under injected failures.

##### T21.04 — Cleanup control flow

- **T21.04.a** Compare early returns, nested cleanup, and narrowly scoped cleanup labels.
- **T21.04.b** Establish exactly-once release for each acquired resource.
- **T21.04.c** Choose structure by correctness and readability rather than blanket style rules.

##### T21.05 — Transactional mutation

- **T21.05.a** Define strong, basic, and explicitly weaker failure guarantees.
- **T21.05.b** Stage changes before committing externally visible state.
- **T21.05.c** Verify rollback and idempotent retries where promised.

##### T21.06 — Cancellation and timeouts

- **T21.06.a** Distinguish timeout expiry from successful cancellation.
- **T21.06.b** Specify ownership of operations still in flight.
- **T21.06.c** Prevent stale callbacks and duplicate completion reporting.

##### T21.07 — Assertions and fault policy

- **T21.07.a** Separate development assertions from required runtime validation.
- **T21.07.b** Define panic, safe-state, reset, or error-return policies explicitly.
- **T21.07.c** Audit behavior when assertion code is absent.

##### T21.08 — Logging and trace design

- **T21.08.a** Use bounded records with severity, timestamps, and context.
- **T21.08.b** Specify overflow, rate limiting, and transport backpressure.
- **T21.08.c** Avoid recursive failures inside error-reporting code.

##### T21.09 — Developer and integrator diagnostics

- **T21.09.a** Separate white-box introspection from stable black-box diagnostics.
- **T21.09.b** Define independently togglable interfaces and supported contracts.
- **T21.09.c** Verify disabled instrumentation removes promised evaluation and storage costs.

##### T21.10 — Crash evidence and recovery

- **T21.10.a** Preserve minimal fault context in suitable storage.
- **T21.10.b** Correlate records with build identifiers and symbol files.
- **T21.10.c** Distinguish symptom collection from validated fault recovery.

**Domain mastery checkpoint:** Demonstrate correct cleanup at every failure point and two controlled diagnostic interfaces, including a build with instrumentation removed.

<a id="t22"></a>

#### T22 — Production-quality libraries and repositories

**Track:** Shared. **Depth:** Core mastery. **Entry foundations:** T07, T18, T20, T21.

**Primary-source anchors:** [^S10].

##### T22.01 — Repository organization

- **T22.01.a** Separate public headers, private code, tests, examples, and tools.
- **T22.01.b** Keep target-specific backends identifiable.
- **T22.01.c** Make the minimal supported build easy to reproduce.

##### T22.02 — Naming and readability

- **T22.02.a** Encode role, units, ownership, and domain meaning in names.
- **T22.02.b** Reduce unnecessary branching and hidden state.
- **T22.02.c** Use complexity metrics to investigate code rather than replace judgment.

##### T22.03 — Documentation

- **T22.03.a** Write Doxygen-compatible contracts and usage examples.
- **T22.03.b** Explain rationale and non-obvious invariants in comments.
- **T22.03.c** Keep generated API references distinct from architecture narratives.

##### T22.04 — Requirements and traceability

- **T22.04.a** Give requirements and tests stable identifiers.
- **T22.04.b** Link API behavior, implementation decisions, and verification evidence.
- **T22.04.c** Detect requirements without tests and tests without stated purpose.

##### T22.05 — Library packaging

- **T22.05.a** Export build targets and install headers and artifacts coherently.
- **T22.05.b** Test consumption outside the source and build trees.
- **T22.05.c** Document static/shared and platform-specific packaging choices.

##### T22.06 — Compatibility and FFI

- **T22.06.a** Preserve ABI layout and calling-convention requirements at language boundaries.
- **T22.06.b** Test independent consumers and version skew.
- **T22.06.c** Define string, buffer, allocation, and ownership transfer across FFI.

##### T22.07 — Dependency and licensing records

- **T22.07.a** Inventory direct and transitive dependencies with pinned provenance.
- **T22.07.b** Separate source reuse from independent educational reimplementation.
- **T22.07.c** Track license obligations without assuming compatibility from popularity.

##### T22.08 — Code review and source reading

- **T22.08.a** Review invariants, ownership, failure paths, and generated-code risks.
- **T22.08.b** Navigate unfamiliar code using call and data-flow evidence.
- **T22.08.c** Form hypotheses before modifying complex legacy behavior.

##### T22.09 — Refactoring and regression analysis

- **T22.09.a** Establish characterization tests before structural changes.
- **T22.09.b** Use bisection and minimized failures to identify regressions.
- **T22.09.c** Preserve public behavior while making intended contract changes explicit.

##### T22.10 — Release engineering

- **T22.10.a** Produce versioned binaries, debug symbols, and reproducibility records.
- **T22.10.b** Document changes, known limitations, and migration instructions.
- **T22.10.c** Test clean installation and rollback or replacement procedures.

**Domain mastery checkpoint:** Release a reusable C library with standalone consumer tests, documented interfaces, versioned artifacts, traceability, and a reproducible maintenance workflow.

### Phase V — Linux systems and concurrency

<a id="t23"></a>

#### T23 — Linux processes, memory, files, and POSIX interfaces

**Track:** Linux. **Depth:** Core mastery. **Entry foundations:** T03, T07, T12, T18.

**Primary-source anchors:** [^S07], [^S12].

##### T23.01 — Process model

- **T23.01.a** Distinguish process identity, address space, threads, and credentials.
- **T23.01.b** Inspect arguments, environment, working directory, and resource limits.
- **T23.01.c** Separate POSIX guarantees from Linux implementation mechanisms.

##### T23.02 — Process lifecycle

- **T23.02.a** Use fork, exec-family operations, wait, and termination correctly.
- **T23.02.b** Handle failed exec and child error reporting.
- **T23.02.c** Account for inherited resources and post-fork restrictions in threaded programs.

##### T23.03 — Descriptors and open-file descriptions

- **T23.03.a** Track descriptor ownership separately from shared file offsets and status.
- **T23.03.b** Use duplication, redirection, and close-on-exec deliberately.
- **T23.03.c** Diagnose descriptor leaks and unintended inheritance.

##### T23.04 — Robust byte I/O

- **T23.04.a** Handle partial reads and writes, EOF, and zero progress.
- **T23.04.b** Define operation-specific interruption and retry policies.
- **T23.04.c** Avoid treating every failing operation as safely repeatable.

##### T23.05 — Paths and filesystem objects

- **T23.05.a** Distinguish pathnames, directory entries, inodes, and open handles.
- **T23.05.b** Compare relative, absolute, and directory-relative operations.
- **T23.05.c** Handle symbolic links and race-prone pathname checks deliberately.

##### T23.06 — Metadata and permissions

- **T23.06.a** Inspect file type, size, mode, owner, and timestamps.
- **T23.06.b** Account for creation masks and permission failure.
- **T23.06.c** Separate authorization assumptions from successful pathname lookup.

##### T23.07 — File positioning and representation

- **T23.07.a** Use explicit offsets with checked type conversions.
- **T23.07.b** Compare sequential access, positioned I/O, and sparse files.
- **T23.07.c** Handle large files and concurrent offset use.

##### T23.08 — Virtual memory

- **T23.08.a** Distinguish mappings, pages, faults, address space, and physical residency.
- **T23.08.b** Analyze anonymous mappings, copy-on-write, and protection changes.
- **T23.08.c** Separate allocated virtual space from committed or resident memory.

##### T23.09 — Memory-mapped files

- **T23.09.a** Define mapping ownership, bounds, resizing, and cleanup.
- **T23.09.b** Compare mapped access with explicit read/write behavior.
- **T23.09.c** Analyze truncation, synchronization, and fault cases.

##### T23.10 — Shared memory

- **T23.10.a** Establish layout, initialization, and interprocess synchronization contracts.
- **T23.10.b** Avoid process-local pointers in portable shared representations.
- **T23.10.c** Define ownership and cleanup after participant failure.

##### T23.11 — Pipes and terminal I/O

- **T23.11.a** Build pipelines with correct descriptor closure and backpressure.
- **T23.11.b** Configure raw or canonical terminal behavior when needed.
- **T23.11.c** Handle broken pipes and device-disconnect conditions.

##### T23.12 — Persistence and crash consistency

- **T23.12.a** Distinguish buffering, visibility, and durable storage claims.
- **T23.12.b** Design update protocols using explicit synchronization and replacement steps.
- **T23.12.c** Test interrupted updates without assuming universal filesystem guarantees.

##### T23.13 — Time, resources, and scheduling interfaces

- **T23.13.a** Choose clock domains appropriate to deadlines and measurements.
- **T23.13.b** Inspect limits, affinity, and scheduling parameters with permission checks.
- **T23.13.c** Separate userspace timing observations from scheduler guarantees.

##### T23.14 — POSIX/Linux boundary

- **T23.14.a** Tag interfaces by standard, optional profile, or Linux specificity.
- **T23.14.b** Use feature-test macros and runtime capability checks appropriately.
- **T23.14.c** Provide documented fallback or explicit unsupported behavior.

**Domain mastery checkpoint:** Implement a robust file/process utility with descriptor ownership, partial-I/O handling, memory mapping, and verified failure cleanup.

<a id="t24"></a>

#### T24 — Signals, IPC, networking, and event-driven systems

**Track:** Linux. **Depth:** Core mastery; advanced I/O elective. **Entry foundations:** T23, T15, T21.

**Primary-source anchors:** [^S07], [^S12].

##### T24.01 — Signal model

- **T24.01.a** Distinguish generation, pending state, masking, and delivery.
- **T24.01.b** Install handlers with explicit disposition and restart policies.
- **T24.01.c** Limit handler operations to the applicable safety contract.

##### T24.02 — Race-free signal integration

- **T24.02.a** Compare atomic waiting, self-pipe, and signalfd designs.
- **T24.02.b** Coordinate signal masks across threads.
- **T24.02.c** Eliminate lost-wakeup windows during shutdown and child handling.

##### T24.03 — IPC selection

- **T24.03.a** Compare pipes, local sockets, shared memory, and queues.
- **T24.03.b** Define message boundaries, ownership, and failure notification.
- **T24.03.c** Select mechanisms against latency, isolation, and backpressure needs.

##### T24.04 — Socket lifecycle

- **T24.04.a** Manage address resolution, creation, bind, listen, accept, and connect.
- **T24.04.b** Track every descriptor through failure and shutdown paths.
- **T24.04.c** Compare IPv4, IPv6, and local-domain addressing assumptions.

##### T24.05 — TCP stream protocols

- **T24.05.a** Separate byte-stream transport from application messages.
- **T24.05.b** Implement framing, partial transfer, half-close, and reconnect policy.
- **T24.05.c** Bound queued data and stalled-peer resource use.

##### T24.06 — UDP and datagrams

- **T24.06.a** Preserve message boundaries and handle truncation explicitly.
- **T24.06.b** Design sequencing, duplicate handling, and loss policies where required.
- **T24.06.c** Separate local send success from remote delivery evidence.

##### T24.07 — Nonblocking execution

- **T24.07.a** Model readiness, would-block, partial progress, and completion.
- **T24.07.b** Maintain operation state across event-loop iterations.
- **T24.07.c** Prevent busy loops and unfair starvation of other connections.

##### T24.08 — poll and epoll

- **T24.08.a** Compare level-triggered and edge-triggered operation.
- **T24.08.b** Handle registration changes, descriptor reuse, and stale events.
- **T24.08.c** Design bounded work per event-loop turn.

##### T24.09 — Timers and notifications

- **T24.09.a** Integrate timerfd, eventfd, and monotonic deadlines where appropriate.
- **T24.09.b** Distinguish timer expiry from actual handler execution time.
- **T24.09.c** Handle cancellation, rescheduling, and counter accumulation.

##### T24.10 — Serial and embedded transports

- **T24.10.a** Configure a serial transport with explicit framing and timeout behavior.
- **T24.10.b** Reuse portable parsers between serial, network, and test backends.
- **T24.10.c** Explore SocketCAN using a controlled virtual or bench network.

##### T24.11 — Graceful shutdown and overload

- **T24.11.a** Define admission limits, queue bounds, and load-shedding policy.
- **T24.11.b** Drain or cancel work with unambiguous ownership.
- **T24.11.c** Test blocked consumers, disappearing peers, and repeated shutdown requests.

##### T24.12 — Advanced I/O [E]

- **T24.12.a** Investigate io_uring with version-specific operation and lifetime rules.
- **T24.12.b** Compare copy avoidance with ownership and pinning costs.
- **T24.12.c** Measure complete workloads before choosing a more complex interface.

**Domain mastery checkpoint:** Build an event-driven framed-data service that survives partial input, overload, interrupted operations, disconnects, and orderly shutdown.

<a id="t25"></a>

#### T25 — Threads and the C memory model

**Track:** Cross-track. **Depth:** Core mastery. **Entry foundations:** T03, T04, T23.

**Primary-source anchors:** [^S01], [^S07], [^S12].

##### T25.01 — Thread lifecycle

- **T25.01.a** Create, join, and manage thread resources explicitly.
- **T25.01.b** Compare detached and joinable lifetime strategies.
- **T25.01.c** Separate C thread facilities from POSIX-specific APIs.

##### T25.02 — Shared-state design

- **T25.02.a** Identify shared, immutable, thread-owned, and transferred state.
- **T25.02.b** Define ownership of work items and shutdown messages.
- **T25.02.c** Prefer reduced sharing before introducing fine-grained synchronization.

##### T25.03 — Mutexes and invariants

- **T25.03.a** Protect complete invariants rather than isolated assignments.
- **T25.03.b** Establish lock ordering and scope.
- **T25.03.c** Handle initialization, destruction, and failure without racing users.

##### T25.04 — Condition variables

- **T25.04.a** Associate waits with predicates under the required mutex.
- **T25.04.b** Handle spurious wakeups and state changes before waiting.
- **T25.04.c** Define shutdown and timeout behavior without lost notifications.

##### T25.05 — Other synchronization primitives

- **T25.05.a** Compare semaphores, barriers, read/write locks, and one-time initialization.
- **T25.05.b** Analyze fairness and starvation assumptions.
- **T25.05.c** Select primitives according to operation semantics rather than convenience.

##### T25.06 — Data races and happens-before

- **T25.06.a** Construct sequenced-before and synchronizes-with relations.
- **T25.06.b** Identify conflicting accesses and missing ordering.
- **T25.06.c** Separate race freedom from higher-level algorithm correctness.

##### T25.07 — Atomic objects and operations

- **T25.07.a** Use atomic initialization, loads, stores, and read-modify-write operations.
- **T25.07.b** Check operation support and lock-free properties on the actual target.
- **T25.07.c** Account for alignment and out-of-line helper dependencies.

##### T25.08 — Memory-order reasoning

- **T25.08.a** Compare relaxed, acquire, release, acquire-release, and sequentially consistent operations.
- **T25.08.b** Explain publication and observation with explicit execution diagrams.
- **T25.08.c** Treat consume and fence rules as edition- and implementation-sensitive topics.

##### T25.09 — Compare-exchange

- **T25.09.a** Handle success and failure memory orders correctly.
- **T25.09.b** Distinguish weak and strong operations and retry requirements.
- **T25.09.c** Preserve expected-value and ownership semantics on failed exchanges.

##### T25.10 — Progress and scheduling hazards

- **T25.10.a** Analyze deadlock, livelock, starvation, and priority inversion.
- **T25.10.b** Distinguish mutual exclusion from fairness or bounded response.
- **T25.10.c** Avoid assuming short critical sections imply real-time safety.

##### T25.11 — Cache and locality effects

- **T25.11.a** Identify false sharing and shared-cache-line contention.
- **T25.11.b** Compare per-thread state, batching, and partitioned ownership.
- **T25.11.c** Measure synchronization cost independently from useful computation.

##### T25.12 — Concurrency validation

- **T25.12.a** Use race detection, deterministic schedules, and stress tests where supported.
- **T25.12.b** Explain what each technique can and cannot establish.
- **T25.12.c** Preserve a written ordering and lifetime argument alongside test evidence.

**Domain mastery checkpoint:** Implement a bounded threaded pipeline and a correctly justified atomic publication example, with explicit lifetime and happens-before reasoning.

<a id="t26"></a>

#### T26 — Advanced concurrency and optional kernel work

**Track:** Advanced elective. **Depth:** Awareness core; advanced implementations elective. **Entry foundations:** T25, T14, T16.

**Primary-source anchors:** [^S08], [^S12].

##### T26.01 — Concurrent-object specifications

- **T26.01.a** Define permitted operation histories and completion behavior.
- **T26.01.b** Distinguish linearizability from memory-model consistency.
- **T26.01.c** Identify candidate linearization points in a concrete structure.

##### T26.02 — Progress guarantees

- **T26.02.a** Compare blocking, obstruction-free, lock-free, and wait-free claims.
- **T26.02.b** State scheduler, allocation, and atomic-operation assumptions.
- **T26.02.c** Analyze per-operation progress separately from whole-system throughput.

##### T26.03 — SPSC structures

- **T26.03.a** Implement an SPSC queue with separate producer/consumer ownership.
- **T26.03.b** Justify publication and reclamation of occupied slots.
- **T26.03.c** Address counter wraparound and cache-line placement.

##### T26.04 — MPSC and MPMC structures

- **T26.04.a** Study one complete multi-producer algorithm with explicit slot states.
- **T26.04.b** Analyze contention, reservation, publication, and cancellation.
- **T26.04.c** Avoid extrapolating SPSC ordering rules to additional participants.

##### T26.05 — ABA and identity

- **T26.05.a** Construct an ABA execution with object reuse.
- **T26.05.b** Compare tagged values, generations, and deferred reuse.
- **T26.05.c** Account for tag wraparound and target atomic-width limitations.

##### T26.06 — Hazard pointers

- **T26.06.a** Define pointer protection, validation, retirement, and reclamation.
- **T26.06.b** Bound or document retained-memory growth.
- **T26.06.c** Analyze thread departure and stalled-participant behavior.

##### T26.07 — Epoch and quiescence schemes

- **T26.07.a** Track read-side participation and retirement epochs.
- **T26.07.b** Identify conditions required before storage reuse.
- **T26.07.c** Compare reclamation delay under stalled or failed participants.

##### T26.08 — RCU concepts

- **T26.08.a** Separate publication, read-side access, and grace-period completion.
- **T26.08.b** Contrast userspace libraries with Linux kernel mechanisms.
- **T26.08.c** State reclamation and lifetime responsibilities precisely.

##### T26.09 — Kernel programming boundaries

- **T26.09.a** Study kernel build interfaces and version-matched execution contexts.
- **T26.09.b** Separate user pointers, kernel allocation, and sleeping restrictions.
- **T26.09.c** Use isolated test systems and documented kernel APIs.

##### T26.10 — Kernel memory and lifetime models

- **T26.10.a** Compare kernel ordering primitives with ISO C userspace atomics.
- **T26.10.b** Analyze module unloading, callback lifetime, and deferred work.
- **T26.10.c** Keep model-specific correctness arguments explicit and separate.

**Domain mastery checkpoint:** For a chosen advanced structure, provide a linearization argument, safe reclamation scheme, explicit progress assumptions, and implementation-specific validation.

### Phase VI — Bare-metal and real-time engineering

<a id="t27"></a>

#### T27 — Reset-to-main and board bring-up

**Track:** Bare metal. **Depth:** Core mastery. **Entry foundations:** T08, T09, T11.

**Primary-source anchors:** [^S06], [^S11].

##### T27.01 — Hardware evidence and target selection

- **T27.01.a** Identify exact MCU, board revision, memory map, and boot configuration.
- **T27.01.b** Separate board documentation from device and core documentation.
- **T27.01.c** Verify emulator coverage before assigning emulator-dependent labs.

##### T27.02 — Reset and boot ownership

- **T27.02.a** Trace ROM, boot pins, bootloader, and application handoff.
- **T27.02.b** Distinguish cold, warm, watchdog, and debugger-triggered resets.
- **T27.02.c** Record reset-state assumptions used by startup code.

##### T27.03 — Vector table

- **T27.03.a** Define initial stack and exception entries for the selected architecture.
- **T27.03.b** Place and retain the table in the correct image region.
- **T27.03.c** Analyze relocation and bootloader-offset configurations where supported.

##### T27.04 — Stack initialization

- **T27.04.a** Select stack location, alignment, bounds, and fault policy.
- **T27.04.b** Reserve capacity for nested calls and interrupts.
- **T27.04.c** Ensure startup establishes the ABI before calling compiled C.

##### T27.05 — Initialized and zeroed data

- **T27.05.a** Copy initialized sections from load storage to execution storage.
- **T27.05.b** Clear intended zero-initialized regions without corrupting retained data.
- **T27.05.c** Validate multiple RAM banks and custom section placement.

##### T27.06 — Early machine setup

- **T27.06.a** Configure only required clocks, memory access, and core state.
- **T27.06.b** Account for floating-point and privilege configuration where needed.
- **T27.06.c** Respect device-specific ordering and readiness conditions.

##### T27.07 — Runtime hooks and entry

- **T27.07.a** Decide which initialization arrays and runtime hooks are supported.
- **T27.07.b** Establish a documented application-entry signature and return policy.
- **T27.07.c** Avoid relying on services not initialized by the chosen startup.

##### T27.08 — Independent startup implementation

- **T27.08.a** Read and annotate vendor startup before replacing selected parts.
- **T27.08.b** Implement a minimal startup with explicit linker symbols.
- **T27.08.c** Compare symbols, image layout, and observable reset behavior.

##### T27.09 — Retained state and boot handoff

- **T27.09.a** Define retained-memory validity markers and initialization rules.
- **T27.09.b** Specify bootloader/application responsibilities at transfer.
- **T27.09.c** Reject stale or incompatible retained state safely.

##### T27.10 — Bring-up verification

- **T27.10.a** Observe early startup through debugger and minimal hardware output.
- **T27.10.b** Diagnose bad vectors, wrong addresses, and missing initialization.
- **T27.10.c** Test power-cycle operation separately from debugger-assisted execution.

**Domain mastery checkpoint:** Boot a selected Cortex-M target from a self-explained startup and linker configuration, with proven memory placement and no unintended debugger dependency.

<a id="t28"></a>

#### T28 — MMIO, peripherals, and driver construction

**Track:** Bare metal. **Depth:** Core mastery; complex protocols elective. **Entry foundations:** T04, T18, T20, T27.

##### T28.01 — Register documentation

- **T28.01.a** Interpret access widths, offsets, reset values, and reserved fields.
- **T28.01.b** Identify read side effects and write-one-to-clear semantics.
- **T28.01.c** Translate peripheral sequences into explicit driver state transitions.

##### T28.02 — MMIO access rules

- **T28.02.a** Form device-register accesses using documented compiler and target conventions.
- **T28.02.b** Separate volatile access from atomicity, synchronization, and ordering.
- **T28.02.c** Analyze read-modify-write hazards and dedicated set/clear registers.

##### T28.03 — Clock and reset control

- **T28.03.a** Sequence peripheral clocks, reset release, and readiness checks.
- **T28.03.b** Compute clock-tree assumptions relevant to peripheral timing.
- **T28.03.c** Handle clock changes without silently invalidating driver configuration.

##### T28.04 — GPIO

- **T28.04.a** Configure direction, alternate function, pull state, and output mode.
- **T28.04.b** Implement atomic pin updates where hardware supports them.
- **T28.04.c** Compare input sampling, debouncing, and interrupt-driven detection.

##### T28.05 — UART

- **T28.05.a** Configure baud rate, frame format, and status handling.
- **T28.05.b** Implement bounded transmit/receive buffers and error reporting.
- **T28.05.c** Compare polling, interrupt, and DMA transfer contracts.

##### T28.06 — Timers and PWM

- **T28.06.a** Derive prescaler, reload, compare, and capture values.
- **T28.06.b** Handle rollover, update events, and synchronization.
- **T28.06.c** Validate frequency, duty cycle, and timestamp precision on hardware.

##### T28.07 — SPI

- **T28.07.a** Specify mode, word size, chip-select timing, and transfer ownership.
- **T28.07.b** Handle full-duplex data flow and partial transactions.
- **T28.07.c** Separate peripheral register operation from device protocol semantics.

##### T28.08 — I2C

- **T28.08.a** Model address, start, repeated-start, transfer, acknowledgement, and stop phases.
- **T28.08.b** Handle timeouts, arbitration outcomes, and device-specific recovery.
- **T28.08.c** Keep bus-level error handling separate from application retry policy.

##### T28.09 — ADC and analog interfaces

- **T28.09.a** Configure sampling, conversion, triggering, and result transfer.
- **T28.09.b** Account for calibration and analog-source constraints from device documentation.
- **T28.09.c** Add DAC or comparator work only on verified supported targets.

##### T28.10 — DMA driver interface

- **T28.10.a** Define buffer ownership, alignment, completion, and cancellation.
- **T28.10.b** Handle partial completion and transfer errors.
- **T28.10.c** Keep peripheral and memory-coherency requirements visible in the API.

##### T28.11 — Peripheral abstraction progression

- **T28.11.a** Implement register access, a minimal driver, and a portable service.
- **T28.11.b** Use mockable boundaries without pretending ordinary RAM fully models MMIO.
- **T28.11.c** Compare generated headers, vendor HAL, and direct-register maintenance costs.

##### T28.12 — CAN, USB, and Ethernet [E]

- **T28.12.a** Select one protocol extension with explicit hardware prerequisites.
- **T28.12.b** Reuse portable framing and state-machine components where appropriate.
- **T28.12.c** Separate controller operation, physical layer, protocol stack, and conformance testing.

**Domain mastery checkpoint:** Deliver polling and asynchronous driver variants with register-level rationale, portable service interfaces, and hardware-validatable error behavior.

<a id="t29"></a>

#### T29 — Interrupts, DMA, and hardware/software ordering

**Track:** Bare metal. **Depth:** Core mastery. **Entry foundations:** T25, T27, T28.

**Primary-source anchors:** [^S06], [^S08].

##### T29.01 — Exception entry and return

- **T29.01.a** Inspect saved architectural state and exception-return mechanisms.
- **T29.01.b** Distinguish automatic hardware stacking from compiler-generated prologues.
- **T29.01.c** Decode nested exceptions using the selected core documentation.

##### T29.02 — Interrupt routing and priorities

- **T29.02.a** Configure enable, pending, priority, and masking behavior.
- **T29.02.b** Analyze nesting and priority grouping on the chosen target.
- **T29.02.c** Avoid assuming numeric priority conventions are portable.

##### T29.03 — ISR design

- **T29.03.a** Bound work and delegate noncritical processing.
- **T29.03.b** Account for reentrancy and shared peripheral state.
- **T29.03.c** Define behavior under bursty or repeated interrupt arrival.

##### T29.04 — Critical sections

- **T29.04.a** Compare interrupt masking with atomic access and locking.
- **T29.04.b** Restore prior masking state correctly.
- **T29.04.c** Analyze interference from higher-priority interrupts and DMA separately.

##### T29.05 — Shared foreground/ISR data

- **T29.05.a** Design ownership and publication between execution contexts.
- **T29.05.b** Verify atomic-width and compiler-runtime assumptions on target.
- **T29.05.c** Avoid equating volatile visibility with a complete communication protocol.

##### T29.06 — ISR-safe queues

- **T29.06.a** Implement bounded event or sample transfer with explicit overflow policy.
- **T29.06.b** Handle producer/consumer wraparound and partial publication.
- **T29.06.c** Measure latency while retaining a correctness argument.

##### T29.07 — DMA ownership

- **T29.07.a** Model CPU-owned, device-owned, complete, and cancelled buffer states.
- **T29.07.b** Specify when data and descriptors become visible to each participant.
- **T29.07.c** Prevent reuse before transfer and reclamation conditions are satisfied.

##### T29.08 — Ordering and coherency

- **T29.08.a** Distinguish compiler ordering, CPU ordering, cache maintenance, and mutual exclusion.
- **T29.08.b** Study barriers and cache operations only for applicable hardware.
- **T29.08.c** Compare cache-equipped and cacheless target assumptions explicitly.

##### T29.09 — Timestamp and event consistency

- **T29.09.a** Correlate counter reads with interrupt and DMA events.
- **T29.09.b** Detect missed events, rollover, and torn multiword observations.
- **T29.09.c** Define timestamp uncertainty and event-ordering semantics.

##### T29.10 — Race and latency validation

- **T29.10.a** Inject delayed servicing, buffer exhaustion, and cancellation races.
- **T29.10.b** Correlate software traces with logic-analyzer or scope observations.
- **T29.10.c** Separate logical simulation evidence from real bus and interrupt timing.

**Domain mastery checkpoint:** Implement bounded ISR-to-foreground transfer and a DMA ownership protocol, with justified ordering and tests for overflow, cancellation, and missed events.

<a id="t30"></a>

#### T30 — Scheduling, timing, and constrained execution

**Track:** Bare metal. **Depth:** Core mastery; RTOS implementation elective. **Entry foundations:** T20, T21, T29.

##### T30.01 — Superloop design

- **T30.01.a** Bound each foreground activity and service interval.
- **T30.01.b** Separate periodic work from event-driven work.
- **T30.01.c** Identify starvation caused by long-running operations.

##### T30.02 — Cooperative scheduling

- **T30.02.a** Represent runnable tasks and explicit yield points.
- **T30.02.b** Implement timer-driven release and bounded work queues.
- **T30.02.c** Define fairness, priorities, and overload policy.

##### T30.03 — Preemption and RTOS integration

- **T30.03.a** Map application ownership and blocking rules to RTOS services.
- **T30.03.b** Understand context switching before relying on task abstractions.
- **T30.03.c** Treat writing a scheduler kernel as a separate elective project.

##### T30.04 — Clock and deadline arithmetic

- **T30.04.a** Select time units and a monotonic counter representation.
- **T30.04.b** Prove rollover-safe comparison under explicit interval bounds.
- **T30.04.c** Distinguish elapsed intervals from absolute timestamps.

##### T30.05 — Latency and jitter

- **T30.05.a** Define release latency, execution time, response time, and jitter.
- **T30.05.b** Budget interrupt interference and queueing delays.
- **T30.05.c** Measure distributions without labeling observed maxima as proven bounds.

##### T30.06 — Scheduling analysis

- **T30.06.a** Derive utilization and response-time checks for stated task models.
- **T30.06.b** Identify blocking and priority-inversion contributions.
- **T30.06.c** Document assumptions violated by asynchronous hardware or unbounded services.

##### T30.07 — Memory and resource budgets

- **T30.07.a** Bound queue capacity, stack depth, and persistent storage.
- **T30.07.b** Include interrupt nesting and library/runtime contributions.
- **T30.07.c** Define predictable behavior on exhaustion.

##### T30.08 — Watchdogs and progress

- **T30.08.a** Define meaningful progress conditions rather than periodic blind refresh.
- **T30.08.b** Coordinate supervision across tasks and execution contexts.
- **T30.08.c** Test hangs, overload, and recovery loops.

##### T30.09 — Low-power execution

- **T30.09.a** Design sleep entry and wakeup without lost-event windows.
- **T30.09.b** Account for clock restoration and peripheral state.
- **T30.09.c** Measure energy and timing on actual hardware when making physical claims.

##### T30.10 — Real-time evidence

- **T30.10.a** Separate average timing, measured maxima, analytical bounds, and WCET claims.
- **T30.10.b** Inspect critical-path generated code and runtime dependencies.
- **T30.10.c** Preserve measurement configuration and uncertainty with each timing result.

**Domain mastery checkpoint:** Build a bounded event-driven firmware application with explicit deadlines, stack/queue budgets, overload behavior, and measured timing uncertainty.

<a id="t31"></a>

#### T31 — Embedded debugging, fault handling, and deployment

**Track:** Bare metal. **Depth:** Core mastery. **Entry foundations:** T27, T28, T29.

**Primary-source anchors:** [^S06], [^S11].

##### T31.01 — Debug connection

- **T31.01.a** Configure SWD/JTAG, reset strategy, and supported debug server.
- **T31.01.b** Distinguish connection, flash, reset, and application failures.
- **T31.01.c** Record probe firmware and target/debug configuration.

##### T31.02 — Source and machine debugging

- **T31.02.a** Correlate source, disassembly, registers, and memory.
- **T31.02.b** Use hardware breakpoints and watchpoints within target limits.
- **T31.02.c** Diagnose optimized-out variables and misleading source stepping.

##### T31.03 — Fault decoding

- **T31.03.a** Read exception status and saved execution context.
- **T31.03.b** Reconstruct the failing instruction and access.
- **T31.03.c** Separate primary faults from secondary faults in the handler.

##### T31.04 — Stack integrity

- **T31.04.a** Detect overflow, bad alignment, corrupted returns, and nested-handler growth.
- **T31.04.b** Compare canaries, watermarks, guards, and MPU protection where supported.
- **T31.04.c** Avoid treating one observed watermark as a universal bound.

##### T31.05 — Peripheral and bus faults

- **T31.05.a** Debug wrong clocks, wrong modes, and erroneous register sequencing.
- **T31.05.b** Use device documentation to interpret status and error flags.
- **T31.05.c** Correlate software access with observable external signals.

##### T31.06 — DMA and concurrency diagnosis

- **T31.06.a** Trace buffer ownership and completion events.
- **T31.06.b** Reproduce stale data, premature reuse, and missing coherency operations.
- **T31.06.c** Account for debugger effects on timing and peripheral progress.

##### T31.07 — Trace and timing tools

- **T31.07.a** Use cycle counters and trace facilities verified for the exact target.
- **T31.07.b** Design bounded trace output and timestamp correlation.
- **T31.07.c** Quantify measurement disturbance and missing-event behavior.

##### T31.08 — Image deployment

- **T31.08.a** Produce, identify, flash, and verify release images.
- **T31.08.b** Preserve matching ELF symbols and map files.
- **T31.08.c** Test standalone power-on startup after programming.

##### T31.09 — Boot and update interfaces

- **T31.09.a** Define image metadata, compatibility, and transfer-of-control contracts.
- **T31.09.b** Model interrupted update and invalid-image cases.
- **T31.09.c** Separate integrity checking from cryptographic authenticity.

##### T31.10 — Fault injection and recovery

- **T31.10.a** Inject invalid state, timeouts, corrupted input, and simulated hardware failure.
- **T31.10.b** Verify crash records and watchdog recovery behavior.
- **T31.10.c** State the difference between detecting a fault and containing its consequences.

**Domain mastery checkpoint:** Diagnose injected startup, stack, peripheral, and DMA faults; deploy an independently bootable image with reproducible fault evidence.

### Phase VII — Verification, security, and performance mastery

<a id="t32"></a>

#### T32 — Systematic testing and debugging

**Track:** Cross-track. **Depth:** Core mastery; introduced from the first phase. **Entry foundations:** T07, T12, T18.

**Primary-source anchors:** [^S05], [^S13].

##### T32.01 — Test architecture

- **T32.01.a** Separate production components from test harness and platform adapters.
- **T32.01.b** Define fixtures, deterministic inputs, and independent oracles.
- **T32.01.c** Audit dependencies introduced solely by verification tooling.

##### T32.02 — Unit testing

- **T32.02.a** Test public contracts and critical internal invariants.
- **T32.02.b** Cover boundaries, invalid input, empty states, and failure returns.
- **T32.02.c** Avoid tests coupled unnecessarily to one implementation layout.

##### T32.03 — Integration and system testing

- **T32.03.a** Verify contracts across real component boundaries.
- **T32.03.b** Exercise backend substitution and complete operational workflows.
- **T32.03.c** Distinguish mocked behavior from target or OS behavior.

##### T32.04 — Property and model-based testing

- **T32.04.a** Generate operation sequences against a simpler reference model.
- **T32.04.b** State invariants independently of implementation code.
- **T32.04.c** Preserve seeds and minimize failing sequences.

##### T32.05 — Differential and metamorphic testing

- **T32.05.a** Compare independent implementations where valid references exist.
- **T32.05.b** Define transformations with predictable relationships between outputs.
- **T32.05.c** Detect shared assumptions that invalidate apparent oracle independence.

##### T32.06 — Fuzzing

- **T32.06.a** Build bounded, deterministic parser or component harnesses.
- **T32.06.b** Seed corpora with meaningful valid and malformed cases.
- **T32.06.c** Triage crashes and maintain minimized regression inputs.

##### T32.07 — Fault injection

- **T32.07.a** Inject allocation, I/O, timeout, and partial-progress failures.
- **T32.07.b** Exercise every acquisition and rollback boundary.
- **T32.07.c** Test misbehaving adapters while stating actual isolation limits.

##### T32.08 — Dynamic instrumentation

- **T32.08.a** Apply address, undefined-behavior, thread, and memory-initialization checks where supported.
- **T32.08.b** Understand added runtime, memory, and execution costs.
- **T32.08.c** Document unsupported targets and incompatible instrumentation combinations.

##### T32.09 — Static analysis

- **T32.09.a** Combine compiler warnings with path-sensitive or abstract analysis.
- **T32.09.b** Review findings against contracts and configuration assumptions.
- **T32.09.c** Track justified suppressions and analysis regressions.

##### T32.10 — Coverage

- **T32.10.a** Distinguish statement, branch, condition, path, and MC/DC objectives.
- **T32.10.b** Verify tool support and explain unreachable or excluded code.
- **T32.10.c** Avoid equating executed code with tested correctness.

##### T32.11 — Mutation testing

- **T32.11.a** Select meaningful fault models and surviving mutations.
- **T32.11.b** Diagnose weak oracles and untested error paths.
- **T32.11.c** Separate equivalent mutations from actual testing deficiencies.

##### T32.12 — Interactive debugging

- **T32.12.a** Use debugger state, watchpoints, and backtraces to test hypotheses.
- **T32.12.b** Minimize examples before changing multiple variables.
- **T32.12.c** Diagnose optimized and concurrent failures with appropriate caution.

##### T32.13 — Host, simulator, and target evidence

- **T32.13.a** Allocate tests according to the behavior each environment models.
- **T32.13.b** Keep hardware timing and analog claims outside unsupported simulations.
- **T32.13.c** Reuse vectors while preserving target-specific expectations.

##### T32.14 — Regression and reproducibility

- **T32.14.a** Retain failing inputs, binaries, logs, and configuration manifests.
- **T32.14.b** Automate representative build and execution matrices.
- **T32.14.c** Make every claimed property traceable to evidence and known limitations.

**Domain mastery checkpoint:** Produce a verification portfolio with unit, model-based, fuzz, fault-injection, coverage, and regression evidence, each tied to a stated property and limitation.

<a id="t33"></a>

#### T33 — Formal reasoning and verification

**Track:** Cross-track. **Depth:** Core reasoning; selected tool mastery. **Entry foundations:** T03, T04, T14, T18, T32.

**Primary-source anchors:** [^S14], [^S15].

##### T33.01 — Specifications and invariants

- **T33.01.a** Express preconditions, postconditions, and representation invariants precisely.
- **T33.01.b** Separate safety, functional correctness, and liveness properties.
- **T33.01.c** Match mathematical state to concrete C representations.

##### T33.02 — Loop reasoning

- **T33.02.a** Derive invariants strong enough to establish postconditions.
- **T33.02.b** Prove initialization, preservation, and exit consequences.
- **T33.02.c** State termination variants and resource bounds.

##### T33.03 — Arithmetic and memory models

- **T33.03.a** Account for machine integers, pointer validity, and aliasing.
- **T33.03.b** Distinguish mathematical abstractions from executable C semantics.
- **T33.03.c** Prevent proofs from silently assuming away implementation defects.

##### T33.04 — Contracts and annotations

- **T33.04.a** Write selected ACSL or tool-specific contracts.
- **T33.04.b** Specify frame conditions and allowed side effects.
- **T33.04.c** Keep annotations consistent with public API documentation.

##### T33.05 — Bounded model checking

- **T33.05.a** Construct harnesses with nondeterministic but valid inputs.
- **T33.05.b** Bound loops and environment behavior explicitly.
- **T33.05.c** Interpret unwinding checks and counterexamples correctly.

##### T33.06 — Deductive verification

- **T33.06.a** Generate and discharge proof obligations for selected routines.
- **T33.06.b** Separate axioms, assumptions, and proved lemmas.
- **T33.06.c** Investigate unproved goals instead of reporting blanket success.

##### T33.07 — Abstract interpretation

- **T33.07.a** Interpret over-approximations and alarm explanations.
- **T33.07.b** Tune precision with justified invariants and partitioning.
- **T33.07.c** Distinguish soundness assumptions from false-positive reduction.

##### T33.08 — Environment models

- **T33.08.a** Model allocators, I/O, hardware registers, and callbacks at appropriate fidelity.
- **T33.08.b** Document behavior excluded by stubs and assumptions.
- **T33.08.c** Check that the environment model is not easier than the real contract.

##### T33.09 — Concurrency reasoning [E]

- **T33.09.a** Model a small publication or queue protocol.
- **T33.09.b** Distinguish linearizability properties from memory-order validity.
- **T33.09.c** Use model-specific tools without generalizing beyond supported semantics.

##### T33.10 — Evidence composition

- **T33.10.a** Combine proofs, static analysis, tests, and measurements by property.
- **T33.10.b** Track trusted compiler, library, hardware, and tool assumptions.
- **T33.10.c** State precisely what remains unproved or outside the model.

**Domain mastery checkpoint:** Verify a bounded container or parser property with a formal tool, document all assumptions, and distinguish the result from tests and unrestricted correctness claims.

<a id="t34"></a>

#### T34 — Optimization, vectorization, and measurement

**Track:** Cross-track. **Depth:** Core mastery; ISA specialization elective. **Entry foundations:** T08, T17, T25, T32.

**Primary-source anchors:** [^S05], [^S06].

##### T34.01 — Optimization specification

- **T34.01.a** Define workload, latency, throughput, memory, and energy objectives.
- **T34.01.b** Preserve correctness and numerical constraints as acceptance gates.
- **T34.01.c** Select representative inputs rather than only favorable microbenchmarks.

##### T34.02 — Cost models

- **T34.02.a** Separate asymptotic work from instructions and memory traffic.
- **T34.02.b** Identify compute-bound, bandwidth-bound, and latency-bound regions.
- **T34.02.c** Use measurements to revise initial bottleneck hypotheses.

##### T34.03 — Memory hierarchy

- **T34.03.a** Analyze cache locality, working set, TLB effects, and access stride.
- **T34.03.b** Compare data layout, blocking, and hot/cold field separation.
- **T34.03.c** Account for alignment without assuming alignment alone improves speed.

##### T34.04 — Control and instruction effects

- **T34.04.a** Examine branches, prediction, dependencies, and instruction-level parallelism.
- **T34.04.b** Compare branchless and branch-based implementations empirically.
- **T34.04.c** Inspect spills, code size, and instruction-cache effects.

##### T34.05 — Compiler optimization evidence

- **T34.05.a** Read vectorization and optimization reports.
- **T34.05.b** Compare source, intermediate representation, and final machine code.
- **T34.05.c** Distinguish missed optimization from invalid programmer assumptions.

##### T34.06 — Auto-vectorization

- **T34.06.a** Identify dependence, aliasing, reduction, and control-flow barriers.
- **T34.06.b** Use restrict and alignment information only when contracts justify them.
- **T34.06.c** Test whether transformed loops preserve required numerical semantics.

##### T34.07 — Explicit SIMD

- **T34.07.a** Implement one target-appropriate intrinsic kernel.
- **T34.07.b** Handle lane operations, masks, alignment, and partial tails.
- **T34.07.c** Preserve a portable scalar fallback and reference tests.

##### T34.08 — ISA dispatch

- **T34.08.a** Detect required CPU and operating-system support before dispatch.
- **T34.08.b** Isolate specialized code from baseline execution paths.
- **T34.08.c** Verify build and linker behavior for multiversioned functions.

##### T34.09 — Benchmark construction

- **T34.09.a** Prevent dead-code elimination and unrelated setup contamination.
- **T34.09.b** Control warmup, cache state, affinity, and clock variation where possible.
- **T34.09.c** Report distributions, uncertainty, and environmental conditions.

##### T34.10 — Profiling and counters

- **T34.10.a** Use sampling and counters to locate actual hot paths.
- **T34.10.b** Interpret counter multiplexing and measurement limitations.
- **T34.10.c** Correlate observations with generated code and memory access.

##### T34.11 — Whole-program optimization

- **T34.11.a** Evaluate LTO, PGO, inlining, and section layout experimentally.
- **T34.11.b** Account for build cost, code growth, and profile representativeness.
- **T34.11.c** Recheck runtime dependencies and diagnostics after optimization.

##### T34.12 — Embedded optimization

- **T34.12.a** Compare speed, size, stack, and energy trade-offs.
- **T34.12.b** Measure on real target hardware for physical performance claims.
- **T34.12.c** Retain correctness, timing assumptions, and regression thresholds together.

**Domain mastery checkpoint:** Optimize a correct scalar kernel with justified layout and SIMD choices, preserving contracts and reporting reproducible speed, size, and error measurements.

<a id="t35"></a>

#### T35 — Secure and resilient C

**Track:** Cross-track. **Depth:** Core mastery; specialist security elective. **Entry foundations:** T18, T21, T23, T32.

**Primary-source anchors:** [^S02], [^S04], [^S05], [^S07], [^S12].

##### T35.01 — Threat and failure models

- **T35.01.a** Identify assets, trust boundaries, attacker-controlled input, and environmental faults.
- **T35.01.b** Distinguish malicious behavior from accidental failure.
- **T35.01.c** Choose security properties appropriate to the actual deployment.

##### T35.02 — Memory safety

- **T35.02.a** Trace bounds, lifetime, initialization, and ownership defects.
- **T35.02.b** Analyze how small arithmetic mistakes become invalid memory access.
- **T35.02.c** Prefer representations that make dangerous states difficult to construct.

##### T35.03 — Input validation

- **T35.03.a** Validate lengths, counts, nesting, and cross-field consistency.
- **T35.03.b** Separate syntactic validity from semantic authorization.
- **T35.03.c** Bound work and storage before processing untrusted input.

##### T35.04 — Integer and format vulnerabilities

- **T35.04.a** Detect signedness, truncation, multiplication overflow, and format mismatches.
- **T35.04.b** Use checked size calculations and trusted formatting specifications.
- **T35.04.c** Test extreme values and ambiguous textual encodings.

##### T35.05 — Filesystem and process boundaries

- **T35.05.a** Avoid race-prone check-then-use assumptions.
- **T35.05.b** Handle descriptor inheritance, temporary files, and process execution safely.
- **T35.05.c** Prefer explicit argument vectors over untrusted shell-string construction.

##### T35.06 — Resource-exhaustion resistance

- **T35.06.a** Bound queues, allocations, recursion, retries, and parser work.
- **T35.06.b** Define overload and admission-control behavior.
- **T35.06.c** Test pathological but syntactically valid inputs.

##### T35.07 — Binary hardening

- **T35.07.a** Investigate stack protection, fortification, PIE, RELRO, and executable-memory policy.
- **T35.07.b** Verify target and toolchain support rather than assuming flags suffice.
- **T35.07.c** Audit added runtime dependencies in minimal or freestanding builds.

##### T35.08 — Privilege and isolation

- **T35.08.a** Compare process isolation, sandboxing, capabilities, and hardware protection.
- **T35.08.b** Define permitted effects and least-privilege execution.
- **T35.08.c** Separate logical modules from enforceable security boundaries.

##### T35.09 — Sensitive data and timing

- **T35.09.a** Analyze secret lifetime, copies, clearing, and diagnostic exposure.
- **T35.09.b** Study constant-time concerns and compiler transformation risks.
- **T35.09.c** Keep educational cryptographic implementation separate from production deployment.

##### T35.10 — Concurrency and resilience

- **T35.10.a** Examine races that bypass validation or violate ownership.
- **T35.10.b** Define fail-safe state transitions and recovery limits.
- **T35.10.c** Test adversarial interleavings and repeated recovery attempts.

##### T35.11 — Supply chain and artifact integrity

- **T35.11.a** Record dependency provenance and verify release artifact identity.
- **T35.11.b** Distinguish checksums, signatures, authenticity, and rollback policy.
- **T35.11.c** Preserve build evidence without claiming reproducibility proves trustworthiness.

##### T35.12 — Security review and limits

- **T35.12.a** Review components against a concrete vulnerability and failure checklist.
- **T35.12.b** Document residual risks and unsupported threat assumptions.
- **T35.12.c** Integrate security regression tests with ordinary correctness verification.

**Domain mastery checkpoint:** Threat-model a selected component, repair representative vulnerability classes, and demonstrate hardening and recovery without overstating what those mechanisms guarantee.


### Cross-domain learning paths and recurring strands

These paths identify coherent progressions, not mandatory full-domain serial schedules. Teach the indicated foundations before their applications and let hosted tests support bare-metal work early.

| Strand | Progression and reason |
|---|---|
| Memory-safe reusable components | T02/T03/T04 → T18 → T14 → T16, with T32 tests from the first component. Establish objects and contracts before complex allocation policies. |
| Custom Linux execution | T07/T08/T09 → T10 → T11, with the selected T23 process/syscall background supplied before startup exercises. |
| Linux applications | T12/T18/T21/T23 → T24; introduce T25 when concurrency serves a defined workload. |
| Bare-metal bring-up | T07/T08/T09/T11 → T27 → T28, then relevant T25.06–T25.10 foundations → T29 → T30/T31. |
| Library effects audit | T12 → T13, supported by binary inspection from T09 and testing/instrumentation from T32. Compare alternatives in T16/T19 only after defining the needed contract. |
| Abstractions and architecture | T18 → T19 → T20, with lifecycle/error work from T21 and packaging/traceability from T22. |
| Performance | T04/T08/T17 → T34, with independent correctness oracles and regression tests from T32 before optimization. |
| Formal evidence | Invariants and simple arguments begin in T01/T14/T18; tool-assisted work in T33 follows mature specifications and executable tests. |
| Security and resilience | T35 contributes threat, failure, and resource-exhaustion cases to T15/T18/T21/T23/T24, not only a final security review. |

### Turning the taxonomy into a curriculum

The curriculum author shall retain the IDs while grouping related objectives into teachable modules. Each leaf shall map to a module, depth, assessment or explicit awareness activity, and core/elective disposition. A traceability entry may refer to a shared assessment; duplication of the same project effort shall not inflate workload totals.

The author shall use named boundaries for units that are often conflated: language semantics versus ABI; allocation-free code versus a freestanding environment; public API guarantees versus implementation effects; storage reclamation versus object cleanup; C atomics versus device ordering; race freedom versus algorithm correctness; and test execution versus proof.

A module shall contain enough context to teach its objectives, but the taxonomy shall remain an index rather than becoming a second, redundant prose textbook. The detailed curriculum shall expand concepts, readings, exercises, and rubrics once at their teaching locations and cross-reference them elsewhere.

The coarse dependency references require further refinement when selecting lessons. This inventory has not established exercise durations, verified every downstream tool version, implemented the projects, or experimentally validated hardware behavior. Those are deliverables of the subsequent curriculum-design and learning work, not implied achievements of this decomposition.

### Primary-source anchors

These are navigation anchors for technical authority, not a completed reading syllabus or evidence that every version-sensitive objective has been experimentally validated. Check date: 2026-10-04.

[^S01]: WG14 project status, editions, technical specifications, and change records. https://www.open-std.org/jtc1/sc22/wg14/www/projects

[^S02]: GCC manual: language standards and hosted/freestanding implementation responsibilities. https://gcc.gnu.org/onlinedocs/gcc/Standards.html

[^S03]: GCC manual: C dialect options. https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html

[^S04]: GCC manual: link options and excluded runtime/startup dependencies. https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html

[^S05]: Clang Compiler User Manual. https://clang.llvm.org/docs/UsersManual.html

[^S06]: Arm-maintained ABI specifications: procedure calls, ELF, runtime support, and related interfaces. https://github.com/ARM-software/abi-aa

[^S07]: The Open Group Base Specifications, Issue 8: POSIX.1-2024. https://pubs.opengroup.org/onlinepubs/9799919799/

[^S08]: Linux kernel documentation: memory barriers and their kernel-specific context. https://docs.kernel.org/core-api/wrappers/memory-barriers.html

[^S09]: GNU C Library manual: function and macro index. https://sourceware.org/glibc/manual/latest/html_node/Function-Index.html

[^S10]: CMake documentation: the buildsystem and target model. https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html

[^S11]: GNU ld documentation. https://sourceware.org/binutils/docs/ld/

[^S12]: Linux man-pages project: operating-system and library interface documentation. https://man7.org/linux/man-pages/index.html

[^S13]: Clang AddressSanitizer documentation: instrumentation, runtime, and limitations. https://clang.llvm.org/docs/AddressSanitizer.html

[^S14]: Frama-C documentation and verification manuals. https://www.frama-c.com/html/documentation.html

[^S15]: CBMC documentation. https://diffblue.github.io/cbmc/

## 4. Required structure of each curriculum module

The fine-grained inventory is the coverage specification, not the module count.
Group related objectives into pedagogically coherent modules and retain every
leaf ID in a coverage ledger. Each leaf shall map to its teaching module,
required depth, core/elective disposition, and assessment or explicit awareness
activity. Shared exercises may satisfy several leaves; workload shall be counted
once. Refine the coarse prerequisite references into a justified lesson-level
graph, and explain necessary introductions or revisits.

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
12. Traceability from every T01–T35 domain, subtopic, and leaf objective to
    modules, required depth, core/elective status, exercises, and assessment gates.
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
