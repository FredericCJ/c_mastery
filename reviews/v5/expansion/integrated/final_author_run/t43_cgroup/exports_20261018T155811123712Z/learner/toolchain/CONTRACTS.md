# T07 build engineering followthrough

This is required executable support for all36 original T07 supplemental criteria
in `curriculum/v5/expansion/legacy_assessments/PR-T07-*.json`. Their original
objective IDs, tasks, hints, per-leaf rubrics and seven-day transfer requirements
remain authoritative. This contract adds concrete inputs and investigation
procedures; it does not replace the accepted projects or award learner credit
from an author reference run.

Work in an owned scratch directory. Use C17 for the application code; Make,
Ninja, CMake, ELF, SysV/Linux, Arm PCS and compiler options are separate contracts.
GCC/Clang are host tools, not libraries admitted to a freestanding image. Cross
objects are never launched as native executables. Record missing tools as
infrastructure failures or unavailable execution, according to the actual case.
Do not turn off host networking or change security settings for the offline task.
All files, dependency packages, malformed specimens and firmware are original
synthetic teaching artifacts. No production service or real credential is used.

## Mechanism and progression

Begin with a graph of byte producers and consumers. `step.c` consumes `cfg.h`
while preprocessing; a frozen `step.i` does not. Assembly produces an object,
and the driver normally adds runtime/startup inputs when linking. At each edge,
name the machine that executes the producer and the machine described by its
output. A host generator may produce bytes for an Arm compilation without ever
executing an Arm program. Neither `-march` nor a successful link proves the
selected operating-system ABI, sysroot or library compatibility.

Build correctness has three separate parts: each individual command has the
right inputs; the dependency graph schedules commands after those inputs exist;
and the freshness policy invalidates outputs when relevant inputs change. For
example, `build/` is an order-only prerequisite because creating the directory
must precede writing an object, but unrelated directory-entry changes should
not invalidate every object. A configuration header is an ordinary prerequisite:
its bytes affect translation. A missing ordinary edge can yield a successful
stale executable. A timestamp-only cache cannot detect changed bytes whose
timestamp has been preserved; a content-identity policy adds hashing and manifest
maintenance costs. State the policy instead of treating either as infallible.

A static archive defers symbol resolution. Tiny can keep Support's headers
private while still requiring Support at the final consumer link. A type or
layout macro in Tiny's public API changes the consumer compile contract. Exported
targets must carry those requirements; an installed package must rediscover
dependencies at their installed locations rather than encode private producer
paths. Keep source identity, package label, trusted origin and runtime behavior
as distinct claims. A content hash answers only the first of these by itself.

Proceed through explicit stages and diagnostics (01–04), build graphs and usage
requirements (05–08), target/dependency policy (09–10), then matrix verdicts and
provenance (11–12). Supply prerequisite build artifacts at each step. For delayed
assessment, retain only the public contract and the learner's own prior work;
the assessor implementation is withheld.

## Concrete exercise inputs and outputs

| Original objectives | Required artifact and changed condition |
|---|---|
| T07.01.a–c | Use `int step(int x){return 3*x+BIAS;}` with BIAS7 and a caller expecting19 for input4. Keep `.i`, `.s`, `.o`, executable, exact driver argv and stage exits. Missing step fails at link. Change BIAS9 after freezing `.i`; explain19 versus21. Transfer to an assembly `_start` that owns alignment and process exit, and reconstruct PIE/static driver link inputs. |
| T07.02.a–c | Native x86-64 and compile-only Arm manifests include ELF class/data/machine, target triple, CPU/float ABI, selected paths and hashes. A host C generator reads ASCII `VALUE=7`, range0–100, and writes one macro. Reject missing generator/library and a deliberately mislabeled foreign object before execution. Compare selected startup/search inputs under the changed compiler/root. |
| T07.03.a–c | Isolated valid unsigned addition, unused parameter and const assignment TUs; a fourth TU refers to an undefined symbol only at link. Record valid-control success, intended diagnostic, missing include and nonexistent compiler as different outcomes. Scope one warning suppression to a shim; another TU/outer function must still warn. Port diagnostic-class matching to both compilers without demanding identical prose. Analyze signed overflow without running it as a valid golden-output case. |
| T07.04.a–c | Sum prefixes0–16 of `UINT32_MAX-i`, reduce an independent mathematical sum modulo2^32, and inspect a1024-byte aggregate copy. Separate debug/release/size/sanitizer/LTO/coverage output directories, object undefined symbols, final dynamic dependencies, sizes and disassembly. Keep a failing check active under NDEBUG. Transfer to disjoint `restrict` input/output, host-only coverage, and a custom-entry stack-protected object with missing guard support. |
| T07.05.a–c | `schema -> cfg.h + mirror.h -> step.o -> program`, with independent `main.o`. Before each run predict exact rebuilt target sets for no-op, directory touch, schema/header/generator changes, deletion, unrelated files and failed generation. Give both generated headers one grouped producer and require matching generation values. Repair an intentionally order-only header edge. Demonstrate same-timestamp changed bytes and a hash-based invalidation policy. |
| T07.06.a–c | Tiny returns `Support(x)+3`, Support returns `x+5`; public `struct tiny` has two integers via a propagated layout macro. Two consumers have different private helper values17/23. Install, relocate and consume only installed targets. Compare static/shared Support and make `support_value` public in a transfer. Relocate Support to a second prefix and discover it through the package dependency contract. A leaked private header must cause an observable failed consumer check. |
| T07.07.a–c | Compare configure-time and build-time ownership of generated headers, including failure before publication. Encode bytes00 7F80 FF, truncation, empty input with storage extent1/logical extent0, and ordered fragments. A changed resource byte must rebuild its header and consumer; a missing fragment rejects. Cross-configure with explicit target compile probes and a host generator. Name an emulator explicitly for the transfer and report its execution separately from physical observations. |
| T07.08.a–c | Ninja rebuild explanations for schema/header/source/flag changes; retain graph and discovered dependencies. Keep both debug/release compilation-database entries for one TU. Replay exact argv from the recorded cwd after supplying generated inputs; a spaced macro is one argv element. Compare absolute/relative names for one header. Relocate sources and regenerate the database rather than guessing path substitutions. |
| T07.09.a–c | Link an Arm object against an original target-only archive and explicit memory map; missing archive must reject. Change only placement and inspect new linked addresses while retaining unchanged object identity. Same-named host/target headers and libraries plus a host generator expose root-policy mistakes, including nested package discovery. Inspect softfp/hard objects for the same Cortex-M4 ISA, compatible/incompatible links and a variadic boundary. |
| T07.10.a–c | Two-source original Tiny/Support package with version and exact file hashes. Reject corruption, omitted transitive input and same-label patch. Separately record label-only change. Add generator/config/output identities. Build through local prefix, vendored sources and prepopulated FetchContent source with fetch/update disabled. A clean build with a deleted cache source must visibly fail; no network fallback or host networking changes. |
| T07.11.a–c | Register unit, integration, expected translation diagnostic, foreign compilation, emulator and physical cells with PASS/FAIL/UNAVAILABLE and exact scope. Execute GCC/Clang O0/O2 native cells; inspect actual Arm GCC/Clang objects. Reproduce a marked failure from source/header/argv/binary/stderr in a fresh directory without copying unrelated environment variables. Reject reuse after a header-only edit. Revalidate final closure for LTO. |
| T07.12.a–c | Build two roots with debug info, `__FILE__` and explicit generated timestamps. Compare whole binaries, `.text`, debug sections, build IDs and declared computation separately. Show why path mapping alone cannot normalize generated time. Then apply one explicit identical-input policy and test whole-file identity. Compare archive member orders, including duplicate-symbol semantic effects; a first-match generated table is a counterexample to treating all ordering differences as noise. Bind full provenance and reject changed inputs. |

The public `seed/` supplies only the first stage's C inputs and deliberately
incomplete build file. Build-system construction remains the learner's work.
Do not copy the author scripts into the submission. The author reference creates
inspectable original specimens for every row, including incorrect graphs and
local dependencies; it is packaged separately under `assessor/`.

## Verification, reasoning and assessment

Predict before running. Preserve each command's argv, cwd, narrowly relevant
environment, source/generated-input identity, stdout/stderr and exit status.
Report exact expected rebuild sets, not merely successful exit. A missing compiler
is not an intended language diagnostic. A negative link case that only compiled
has not been assessed. Unsupported execution must not be counted as passing.
Use a mathematical integer oracle for the sum and a separate byte/length oracle
for resources; neither expected result comes from the C implementation under test.

State ownership of generated outputs, the failure publication boundary and
cleanup policy. Grouped Make targets protect consumers scheduled by that graph;
they are not an atomic transaction for arbitrary outside readers or sudden power
loss. State diagnostic enforcement versus compiler extensions, ABI/platform
premises and programmer obligations. Analyze the invalid overlapping `restrict`
caller; do not give its undefined behavior a portable expected output.

The original per-leaf0–3 rubric applies separately. Level1 reproduces output;
level2 explains the causal boundary, completes the implementation/analysis and
rejects the relevant incorrect control; level3 transfers independently under the
changed assumption after the original delay. Passing one library test does not
substantiate every build or ABI objective. Assess exact rebuilt targets, selected
paths and dependency origins even when two executables return the same value.
Finite author tests and same-author analysis establish readiness only.

Hints in increasing specificity: first identify the last producer that consumes
the changed bytes; next separate ordering from invalidation; finally compare the
first differing artifact or selected dependency before interpreting the final
program. Preserve unaided evidence before releasing each hint.

Author validation uses `tools/run_systems_toolchain.py` with explicit LLVM linker
and bitcode-reader paths when needed. The receipt retains all commands, generated
text inputs, checks, tool identities and per-objective observations. Use
`tools/check_systems_toolchain.py` to reject stale/incomplete receipts. These tools
never submit learner scores. Physical boards, independent review and delayed
learner performance are separate evidence dimensions.

## Inspected source boundary

Original source locators remain in `research/v5/expansion/legacy_systems/resources.json`.
Additional bounded inspection: GNU Make4.4.1 manual source, “Multiple Targets”
(grouped `&:`); CMake3.31.12 `cmake-packages(7)`, “Creating Relocatable Packages”
and `find_dependency`; Clang compilation-database specification, format entries;
Ninja manual, command invalidation, discovered dependencies and path identity.
These are tool contracts, not ISO C requirements. The active C17 baseline stays
unchanged; no published C17/C23 text is claimed from an older committee draft.
