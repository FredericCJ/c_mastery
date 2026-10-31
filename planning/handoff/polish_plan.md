# C Mastery polish revision: execution plan

## Context

`/home/fred/dev/c_mastery_claude_opus_polishing_prompt.md` asks for an implemented and validated polish of C Mastery 5:
- correct measurement;
- a complete public learner-native PMU path;
- explicit learning transitions;
- readable code and teaching;
- honest evidence.

The user has added these mandates:
- **House style.** All active C is rewritten in a house style that blends BARR-C:2018, SEI CERT C, MISRA C and the Linux kernel style, enforced by clang-format.
  - BARR-C Allman braces, 4-space indent, 80 columns.
  - Public names become lowercase module-prefixed `struct`/`enum` tags.
  - The six real Linux kernel modules keep kernel style.
  - The goal is readability plus an introduction to industry standards; safety rules apply only where the context needs them.
- **Route.** A new `contextual-v2` route (episodes plus explicit encounter bindings) with a v1→v2 migration. v1 stays frozen.
- **Repo tools.** Use Graphify, OpenSpec, Spec Kit, Superpowers and Beads.
- **Orchestration.** A central orchestrator with multi-agent waves that always dispatch the cheapest capable model.
- **Claude Code setup.** `CLAUDE.md`, a `.claude/` directory, and project-scoped installs: the repo-tool plugins, local `beads-mcp`, and the `clangd-lsp` and `pyright-lsp` plugins. Nothing remote.
- **Commits.** Local commit per verified batch, on a local branch. Never push, tag or release.
- **MISRA.** Cite it from `/home/fred/library/MISRA/Electronics and embedded systems` by locator only; its text never enters the repo or any archive.
- **Board.** The connected **NUCLEO-F439ZI** is to be used for real physical work.

## Starting state (re-record in B0)

- `main` at `29b0d4108`, clean.
- Host software:
  - Ubuntu 26.04.1, kernel **7.0.0-42**, GCC 15.2.0, Clang 21.1.8, CMake 4.2.3, Ninja 1.13.2, Python 3.14.4.
  - perf 7.0.14, arm-none-eabi-gcc, qemu-system-arm/x86_64, strace, bwrap, valgrind, gdb.
  - **Missing:** clang-format, clang-tidy, clangd, cppcheck, OpenOCD, gdb-multiarch, stlink-tools, CBMC, Frama-C, KLEE, GenMC.
- PMU and CPU:
  - Ryzen 3 7330U (Zen 3).
  - The user set `perf_event_paranoid=-1` host-wide; lockdown is `none` and Secure Boot is off. Record this as an experiment condition and recommend restoring paranoid to 4 afterwards.
  - The kernel headers define `PERF_FORMAT_LOST`.
- Board:
  - ST-LINK/V2-1 0483:374b, firmware **V2J39M27**, serial `0669FF485753667187124334`.
  - Mass storage is mounted at `/run/media/fred/NOD_F439ZI`, so drag-and-drop flashing works now.
  - The virtual serial port `/dev/ttyACM0` is `root:dialout 660`, and fred is not in `dialout`. The USB node is not writable, so SWD needs a udev rule.
- Code: 3,913 C/H files (about 223k lines) in `labs/` and `assessor/`.
  - About 19k lines hold three or more statements; indentation is mixed 1, 2 and 4 spaces.
  - 2,405 mixed-case public typedef names with about 66k uses; 68 names are defined in more than one family.
  - `reviews/` holds 6,194 C files that are never touched.

## Confirmed findings (verified against source)

| ID | Finding | Evidence |
|---|---|---|
| POLISH-PARSE | Numbers are narrowed before the range check ("4294967297"→1); empty strings are accepted as 0; no errno check; stale `end`. About 69 unchecked argv sites; no shared parser | `labs/performance_followthrough/pmu_native.c:155-177`, `trace_main.c:5` |
| POLISH-LOSS | Final-drain-only with in-band LOST counting only, so trailing losses are invisible and `lost:0` is really unknown. `PERF_FORMAT_LOST` is unread; `execute(w,256)` ignores the requested repetitions; the ring keeps only the earliest samples | `pmu_native.c:110-150` |
| POLISH-CALIB | Exhausted calibration still produces rankings for 5 regimes; the deadline bench has no adequacy gate | `run_native_pmu.py:203`, `analyze_native_pmu.py:119-122`, `run_deadline_pilot.py` |
| POLISH-REGRESS | The sync fault-injection tests never ran under sanitizers, although the doc claims they did; the TSan profile is never built | `public_validation.py:271-273`, `check_native_build_profiles.py:50-52` |
| POLISH-LEARNER-PMU | No learner-runnable collector; `init` omits the adapters; PMU units have `tests: []`; the extracted-archive test stops at `init`; the two learner-archive predicates disagree; `doctor` is too thin | `run_native_pmu.py:82-154`, `generate_native_route.py:79-81`, `public_exports.py:34-50` vs `package_v5.py:28-38` |
| POLISH-ROUTE | Encounter roles come from list positions; least-recently-used context rotation (1,957 switches over 1,982 transitions); 1,976 boilerplate `why_now` strings; no episodes; no checker reads roles | `generate_native_route.py:127-174` |
| POLISH-THEORY | No exercise rubric covers invariants, amortized analysis, DP or exchange arguments | rubric scan |
| POLISH-SECURITY | 78 family-E exercises have templated security sections, with no structured target, adversary, property, boundary, impact or remediation | `curriculum/v5/exercises/*.md` |
| POLISH-STYLE | No style guide, format/lint config or shared parser | explorer scan |
| POLISH-PHYS | 1,847 obligations have `physical_observation: not-run`; the F439 board is now available | `work_inventory.json` |

The ledger also tracks POLISH-DOSSIERS, POLISH-ASSESS, POLISH-CARDS, POLISH-PROFILES and POLISH-TOOLING.

## Orchestration model

- **Orchestrator.** The main session (Opus 5.5) owns:
  - the plan, ledger, Beads graph and shared/generated files;
  - integration, every gate, commits and reports;
  - all acceptance decisions, which it never delegates.
- **Cheapest capable model.** Prices per million tokens in/out: Haiku 5.5 $0.10/$0.50 < Sonnet 5.5 $2/$10 < Opus 5.5 $4/$20 < Fable 5.1 $10/$50.
  - Each task starts at its role's tier.
  - After two failed verifications it escalates one tier.
  - Fable is a logged last resort only.
  - Every dispatch (task, model, effort, outcome) is logged in its Beads issue.
- **Roles.** Each role is defined in `.claude/agents/<role>.md` with `model:`; CLAUDE.md states the effort per role.

| Role | Model / effort | Work |
|---|---|---|
| `inventory-scout` | haiku / low | graphify and grep inventories, receipt summaries, log triage |
| `c-mechanic` | haiku / medium | run the rename/format/brace tools on one family, build, test, report |
| `campaign-runner` | haiku / medium | run existing producers and summarize receipts and failures |
| `board-operator` | haiku / medium | execute prepared, reviewed flash/capture scripts on the F439 (one holder at a time) |
| `c-rewriter` | sonnet / high | semantic readability rewrite of one family |
| `curriculum-editor` | sonnet / high | card and strand prose, security blocks, episode drafts, assessment packets |
| `reviewer` | sonnet / high (opus for security, concurrency, PMU and firmware) | independent review given code, not conclusions |
| `systems-engineer` | opus / xhigh | PMU and loss accounting, native runner, dossiers, F439 bring-up design |
| `pedagogy-architect` | opus / xhigh | episode model, encounter bindings, theory rubrics |

- **Waves.**
  - The Workflow tool runs waves of 10 or fewer agents (the session guideline; the user can raise it in /config). Each wave runs transform → verify → review.
  - Workers get disjoint write sets: one `labs/<family>` plus `assessor/<family>` and a private build directory.
  - Tools, generators, catalogs and the ledger are orchestrator-only.
  - The board is a single exclusive resource, held through a Beads lock issue.
- **Repo tools.**
  - Beads: one issue per batch and per family, referencing POLISH IDs; `bd ready` drives dispatch. Closure never closes an obligation.
  - Graphify: impact queries before edits, and re-extraction after mechanical steps.
  - OpenSpec: one change per area (`c-house-style`, `native-pmu-measurement`, `learner-native-pmu`, `route-contextual-v2`, `security-lab-structure`, `f439-physical`), with scenarios that cite POLISH IDs.
  - Spec Kit: specs plus `speckit-analyze` for the house style, the episode model and the learner runner.
  - Superpowers: `test-driven-development`, `systematic-debugging`, `verification-before-completion`, `requesting-code-review` and `subagent-driven-development`.

## Batches (dependency order; local commit per verified boundary on branch `polish/revision`)

### B0: Setup, ledger and tooling

1. **Branch, ledger and checkpoint.**
   - Create local branch `polish/revision`.
   - Write `reviews/v5/expansion/polish_revision/environment.json`.
   - Add the register `research/v5/expansion/polish_revision/obligations.json`, discovered by `tools/reconcile_implementation_v5.py`. Each item gets its source, intended change, completion criterion, tests and classification.
   - Keep a checkpoint at `planning/polish_revision.json`, updated at every batch end.
   - Dispositions go to `curriculum/v5/expansion/implementation_updates.json` in B11.
2. **Pinned tools, no sudo.**
   - clang-format, clang-tidy and clangd **21.1.x** PyPI wheels (released at least two weeks ago, matching Clang 21.1.8), with hashes in `tools/author_tools_requirements.lock`.
   - Add clang-format to `requirements-lock.txt` for public validation and CI.
   - Extend `install_author_tools.py`, `author_tools.lock.json`, `check_author_tools.py` and `planning/author_tools.md`.
3. **User-run privileged steps.** I hand these over as exact `! sudo …` commands; all are reversible, and none touches PMU or security policy.
   - `apt install openocd gdb-multiarch stlink-tools cbmc` (0.12.0 / 17.1 / 1.8.0 / 6.6.0).
   - A udev rule `/etc/udev/rules.d/70-cmastery-stlink.rules` giving `TAG+="uaccess"` for 0483:374b and its ttyACM, applied with reload and trigger. This grants seat-scoped access instead of a world-writable mode or group changes.
   - Frama-C, KLEE and GenMC are not packaged. Families that depend on them get only equivalence-proven transformations, and re-running them is listed as open (see B2 bridge).
4. **Claude Code setup.** Inspect every template before installing it.
   - `CLAUDE.md`: `@AGENTS.md`, the style summary, the orchestration and model policy, the verification commands, board safety rules and the never-do list.
   - `.claude/settings.json`: permission allowlists, `enabledPlugins`, local-directory `extraKnownMarketplaces` from the pinned sources, and a PostToolUse hook that checks edited `*.c`/`*.h` files with the pinned clang-format. The hook reports only and never rewrites. No git or commit hooks.
   - `.claude/agents/*.md`: the roles above.
   - Integrations:
     - Beads plugin and `beads-mcp` (stdio, `.mcp.json`) from `build/author-tools/sources/beads` v1.3.2, with telemetry and auto-push off.
     - Superpowers Claude plugin pinned to commit `2ce8aa15`.
     - `openspec init --tools claude` and `specify init --here --integration claude`, run in the integration stage directory; copy only the Claude files and preserve the constitution and `openspec/config.yaml`.
     - `graphify install --platform claude`, project-scoped; no `graphify hook install`.
     - Official `clangd-lsp` and `pyright-lsp` plugins.
   - Verify with `claude plugin list`, `claude mcp list` and `check_author_tools.py`.

### B1: House style specification

- **Source register** `research/v5/expansion/polish_revision/style_sources.json`, holding inspected locators for:
  - MISRA C:2023 (3rd ed., 2nd rev.), the MISRA C:2023 Addendum 3 (CERT C coverage) and MISRA Compliance:2020, all from the private library and cited only;
  - BARR-C:2018 (public), the SEI CERT C wiki and the kernel `coding-style.rst`.
- **Guide** `curriculum/guides/c-style.md`: each rule is tagged with its provenance; conflicts and their resolutions are taught explicitly; no formal compliance is claimed.
- **Tier L, layout** (clang-format):
  - Allman braces, 4 spaces, 80 columns.
  - `InsertBraces`; `PointerAlignment: Right`; `SortIncludes: Never`.
  - `CommentPragmas: '^@'`, so ACSL comments are never reflowed.
  - Kernel modules get a kernel `.clang-format` plus checkpatch, as a taught exception.
- **Tier N, naming and structure** (clang-tidy `readability-identifier-naming` and `readability-isolate-declaration`, plus a custom checker):
  - Lowercase module-prefixed functions and tags, with no struct/enum typedefs. This resolves the conflict between BARR-C's `_t` suffix and POSIX/CERT DCL37-C.
  - UPPER_SNAKE prefixed macros and constants; `_fn` function-pointer typedefs only.
  - File-local `static`, visible units, and contract comments on public APIs.
  - Function length and complexity are review triggers, not gates.
- **Tier C, correctness by context:**
  - CERT ERR34-C conversions everywhere.
  - A MISRA subset in freestanding and firmware code: no dynamic memory after init, no recursion, `-Wconversion -Wsign-conversion` in that profile.
  - Documented deviations: forward `goto` to a single cleanup label (MISRA 15.1) and early returns (15.5).
- **Exceptions:** `CM-SPECIMEN:` tags with manifest entries; scaffolds keep their stubs; vendored/config shims are excluded (FreeRTOSConfig.h, the HAL confs, `freestanding_include`).
- **Files:** `.clang-format`, `.clang-tidy`, `.editorconfig`, `tools/c_style_manifest.json`, and `tools/check_c_style.py`, which writes a receipt and joins the public profile and CI. Add a Spec Kit spec and constitution v1.1.0.

### B2: Mechanical transformation of all active C (separate local commits: 2c, 2d, 2e, 2f)

- **2a. Make the tooling layout-independent first**, using the layout inventory:
  - Add `tools/c_source.py`: a C tokenizer, a unique token-sequence matcher, and function-definition lookup with balanced braces.
  - Port to it every source consumer that depends on layout:
    - about 65 exact-anchor tools and their roughly 95 anchors;
    - the 6 v4 runners and the `v4_lab_assets.py` template, which has drifted;
    - the kernel `[^}]+}` regexes;
    - the `.index('sig(')` slicers (`assessor/{algorithm_heap,…}/generate.py`, `check_algorithm_*`, `run_dispatch_costs.py:49`);
    - the indentation-dependent `guest.py` anchors;
    - the single-line signature regexes (`run_metaprog_*_cost.py`, `author_ad_mp_support.py:124`);
    - `metrics.py`.
  - Silent no-op `.replace()` calls become hard failures: `mutate_store.py:13`, `author_storage_references.py:90-91`, `check_mmio_exports.py:25`, `run_build_followthrough.py:99,125`, `check_storage_async_guest.py:39`.
  - Make generators emit house style: b06 `author_domains.py`, `author_ad_mp_support.py`, `author_replicated_support.py`, `generate_registers.py`, `v4_lab_assets.py`, `author_storage_references.py`.
  - Replace the exact line pins with symbol anchors: `packet_analysis.c:10` (`suppression.json` plus `check_testing_followthrough.py:69`) and `publication_model.c:13`.
  - Run an anchor census (every anchor matches exactly once) before and after each step.
- **2b. Equivalence bridge.**
  - `tools/source_equivalence.py` and `curriculum/v5/style_migration.json` map each file's old→new sha256 with its proven class: object-identical rename, token-equivalent, brace-only or comment-only.
  - Checkers keep the historical receipts untouched. Through a shared helper, they accept an old pin only along a proven chain and label the evidence "carried forward, not re-executed".
  - The CLI uses the same bridge for existing learner workspaces and `bound_files`.
  - Frozen and hash-pinned providers (`assessor/runtime/cvm.c`, `assessor/storage/store.c`, `assessor/kernel_smp/*`, `legacy_smp/startup_guest.c`) and the 3 hard-coded pins go through the bridge.
- **2c. Rename.**
  - Per-family maps are generated from public headers; the 68 collisions are resolved explicitly.
  - The maps are applied at token level to C, anchors, Markdown teaching, fixtures and contracts; never to `reviews/`.
  - Proof: every TU in every profile and target is compiled at `-g0` before and after, and the `.text`, `.data` and `.rodata` sections must be byte-identical. Line numbers are unchanged.
- **2d. Whitespace-only clang-format.** Proof: per-file token identity, with comments normalized and ACSL excluded from reflow.
- **2e. InsertBraces.** Proof: the token diff is only `{`/`}`. Migrate the 11 brace-spanning v4 anchors explicitly and re-verify mutant rejection.
- **2f. Regenerate and revalidate.**
  - Regenerate `exercises.json` and the header pins; bump component contracts to v2 (renamed API); remove the ~20 `-Wno-misleading-indentation` workarounds.
  - `campaign-runner` re-runs every runnable producer: the legacy matrix, unit campaigns and families, targets including QEMU, workflow, release guards, `validate_expansion_v5`, and CBMC once installed.
  - New receipts go under `reviews/v5/expansion/polish_revision/`. Anything not runnable is listed with the bridge class it relies on.

### B3: Measurement defects A–D (systems-engineer/opus, TDD; reviewer/opus)

- **A, parsing.**
  - Supplied `labs/support/cm_parse.{h,c}`.
  - Grammar: ASCII decimal digits only. It uses `strtoumax` with errno and endptr and checks the range in `uintmax_t` before narrowing.
  - Tests derive bounds from `UINT_MAX`/`SIZE_MAX`: max, max+1, beyond `UINTMAX_MAX`, empty, trailing text and wrap-on-narrow.
  - Apply it to `pmu_native`, `trace_main` and every non-specimen site among the ~69.
- **B, loss accounting.**
  - Split `pmu_native.c` into argument parsing, workloads, sampling lifecycle, interpretation and report modules.
  - Read `PERF_FORMAT_LOST` behind a build-time `#ifdef`, with a runtime EINVAL fallback that records the count as unknown.
  - Report fields: in-band LOST, hardware LOST_SAMPLES, nullable `kernel_lost`, `unreported`, and `loss_accounting` (complete, partial-unknown or inconsistent). `lossless` is reported only when the accounting is complete and zero.
  - Drain modes: `final` and `concurrent` (drainer thread, stop flag, join, then a final drain). Ring size and period are bounded options; requested repetitions are honoured.
  - Tests:
    - a decoder test;
    - a reconciler table test;
    - a user-space model of the kernel ring producer showing that final drain misses trailing losses, run under ASan/UBSan and TSan;
    - wrap tests at every origin.
  - **Native saturation run:** a small ring comparing final and concurrent drain. If saturation is not reached, report it as not reached.
- **C, gates.**
  - Separate states for acquisition, scheduling, timing adequacy, comparability and interpretation (supported, inconclusive or refused).
  - Exhausted calibration keeps raw rows but is excluded from rankings and claims.
  - The same gates apply to the deadline bench; Python unit tests use synthetic rows.
- **D, regressions.**
  - Run sanitizers by default for the protocol and publication tests; build the TSan profile.
  - Re-run the mutation, strict-group, cleanup, zero-running and card checks.
  - Never substitute software events for hardware ones.

### B4: Public learner-native PMU path (systems-engineer/opus; curriculum-editor/sonnet)

- **Runner** `labs/performance_followthrough/native.py`, with no dependency on `tools/` or `assessor/`. Subcommands:
  - `doctor`: paranoid, CapEff, sysfs PMUs, strict-group and sampling probes, `PERF_FORMAT_LOST`, mlock, affinity and virtualization, naming the exact missing capability.
  - `protocol`: classifies semantic rejection vs build error vs crash vs timeout.
  - `build`: takes explicit learner component sources or linked bundles; supplied adapters are labelled.
  - `run`, `sample` and `report`.
- **Route and contracts.**
  - Supplied-artifact lists move into the canonical route source.
  - U-T43-09-* units get real checks, so `check` and `accept` bind to the protocol run.
  - New component contract `measurement.pmu`.
  - One shared learner-archive predicate is used by `public_exports.py` and `package_v5.py`.
- **Extracted-archive end-to-end test**, run under bwrap with the checkout hidden (strace audit as fallback):
  - init, then scaffold rejection;
  - a labelled TEST implementation injected by the harness;
  - export and link, native build and doctor;
  - real counting and sampling.

### B5: contextual-v2 route (pedagogy-architect/opus; curriculum-editor/sonnet; reviewer/opus)

- **Canonical source** `contextual-v2.source.json`:
  - `episodes[]`: entering state, motivating limitation, construction (node IDs), exiting artifact and transition reason.
  - `mechanisms[].encounters[]`: role, node, objective, deliverable, criterion, assumption change, prerequisites, produces and consumes.
  - Each mechanism declares its three forms (small model → scalable implementation → integrated consumer).
- **Generator.** Schedules episodes contiguously, subject to prerequisites, with no context-rotation balancing. Nodes not yet in an episode become visible `unauthored` project blocks; `why_now` comes from episodes or is flagged generic.
- **Authoring order:** the three representative paths, then all 13 mechanisms, then the rest of the route in route order. Coverage counts stay visible.
- **Validator** (`check_native_route.py`): roles are distinct and ordered, each objective belongs to its node, criteria exist, assumption changes are stated, transfer contexts consume the mechanism, and component contracts are consistent.
- **Theory strand.** Author criteria for invariants, termination, recurrences, amortized analysis, greedy/exchange arguments, DP, graph correctness and probabilistic assumptions. Each criterion names the plausible wrong answer it rejects. Later tasks state requirements without naming the structure.
- **Migration.** v1 stays frozen. `route.py` loads pinned historical routes, and `study route --adopt` moves a profile to v2 using the v1→v2 migration file. `active_route` becomes v2. Strand pages point to episodes, unit IDs and learner commands.

### B6: F439 physical work (systems-engineer/opus designs; board-operator/haiku executes; reviewer/opus)

- **Safety.** Only original curriculum images are flashed, to this dev board only.
  - No option-byte writes: RDP, WRP and BOR stay untouched, and RDP level 2 (permanent) is never used.
  - No ST-LINK firmware update.
  - Bounded run times.
  - No external wiring unless the user confirms the setup.
- **Identity and access.** Record the board identity: ST-LINK firmware, DBGMCU IDCODE/REV, UID, flash size, and the OpenOCD/gdb versions. Flash by mass-storage copy or SWD (OpenOCD `board/st_nucleo_f4.cfg`).
- **Feasible with the board alone (inventory scout classifies the not-run obligations first):**
  - reset-to-main bring-up and vector table;
  - clock tree readback cross-checked against DWT, SysTick and host timing;
  - USART3 over the ST-LINK virtual serial port: real bytes, with a parity/baud changed-condition;
  - ITM/SWO trace;
  - DWT cycle experiments (flash wait states and ART on/off), the MCU counterpart of the PMU dossiers;
  - faults, MPU and IWDG reset cause;
  - RNG/HASH/CRYP for security labs;
  - Ethernet only if the user supplies a cable and peer.
- **Imports.** Results are imported with `record-physical` as source-bound dossiers following `curriculum/guides/physical-evidence.md`.
- **Still open.** Interfaces needing instruments or peers (I2C/SPI waveforms, RS-232/422/485, CAN/CAN FD), the H745 and measured baud error stay open with exact reasons.

### B7: Teaching material and security (curriculum-editor/sonnet per domain; reviewer/opus for security)

- **Cards.** One effective spec per exercise; scripted checks catch example, hint, acceptance and solution mismatches and leaks.
- **Prose.** Worked derivations, traces and counterexamples in place of generic advice; verified citations.
- **Security labs.** The 78 family-E exercises and the T35/T27 projects get a structured `security_lab` header: target, adversary, violated property, boundary, expected evidence, impact class (crash, sanitizer finding, UB, demonstrated invariant violation or proved impact), remediation and limitations.
  - A validator checks the headers.
  - Specimens run only after inspection, contained by bwrap and rlimits.
  - Kernel and firmware specimens run under QEMU, or on the F439 only when no host effect is possible.

### B8: Diagnostic dossiers (systems-engineer/opus)

- Representation/locality, dispatch (with unpredictable targets) and concurrent state, the last with `pthread_setaffinity_np` placement within the allowed set, in-thread steady-state timing and per-thread peer groups.
- Each dossier holds:
  - hypotheses and equivalent contracts;
  - workload distributions;
  - `objdump` inspection;
  - event justification traced to the kernel's AMD event mapping;
  - raw evidence, alternative explanations and uncertainty.
- Receipts go under `reviews/v5/expansion/polish_revision/dossiers/`, plus a learner worked example.

### B9: Readability rewrite (c-rewriter/sonnet per family, waves of 10 or fewer; reviewer)

- **Order.** Route order, after the native and representative-path code.
- **Changes.** Restructure dense logic; document invariants, units and error contracts; remove one-use abstractions. Keep behaviour, specimen defects and scaffold stubs.
- **Gate:** the family campaign passes (assessor pass, learner rejection, mutation rejection), sanitizer and TSan profiles pass, clang-tidy is clean and a reviewer approves.
- **Families with unavailable re-run tools** (Frama-C, KLEE, GenMC) get only bridge-class transformations; the deeper rewrite is listed as open.
- **Tracking.** `c_style_manifest.json` records a per-family state as a visible ratchet, with a commit per wave.

### B10: Assessment packages (curriculum-editor/sonnet; reviewer/opus)

Refine `assessor/native_ubuntu_assessment/` for the three paths: withheld transfer tasks bound to route nodes, discriminating rubrics, observation forms, and checks for unstated prerequisites and imitation. No participants or delays are fabricated.

### B11: Final validation and report (orchestrator; campaign-runner/haiku)

- **Profiles.** Run the semantic, ASan/UBSan, TSan, optimized-native and target/ABI profiles, with effective flags verified from `compile_commands` and link lines.
- **Integrated checks:** `build_v5.py --profile public`, `check_candidate_exports_v5.py --profile public`, `check_c_style.py`, `check_native_route.py`, the extracted learner path, and the campaigns.
- **Ledger.** Reconcile it; dispositions use the six dimensions.
- **Docs.** Update README and guides.
- **Report:**
  - before/after examples;
  - evidence paths;
  - open items with reasons, including any security subtask left incomplete.
- If a limit interrupts the work, write a precise checkpoint and never label it complete.

## Verification

- **Mechanical:** object identity for the rename, token identity for formatting, a brace-only diff, the anchor census and bridge records. Commit only when all are green.
- **Semantic:** focused tests first (TDD where it fits), then family campaigns, sanitizer/TSan builds and mutation rejection.
- **Hardware (PMU and F439):** the environment and paranoid value, raw rows and captures, binary and source hashes, and gate states. Inconclusive results stay inconclusive.
- **End-to-end:** the extracted learner archive under bwrap, from init through real counting and sampling.
- **Final:** the public build, candidate exports, style, route and learner path, with implementation, author execution, hardware or physical observation and independent assessment reported as separate dimensions.
