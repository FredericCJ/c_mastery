# C Mastery polish revision: continuation on a new machine

Handoff written 2026-10-31 on the original host (Ubuntu 26.04.1, kernel
7.0.0-42, AMD Ryzen 3 7330U, 8 GB RAM). Branch `polish/revision`, 47
commits on top of `main` at `29b0d4108`. Nothing has been pushed by the
assistant; the user pushes.

Part A is the setup checklist for the user. Part B is the prompt to give
the new Claude Code session. Parts C-F are the reference that prompt
points at.

---------------------------------------------------------------------------

## A. Setup on the new machine (user)

1. **Clone and check out.** Clone the pushed repository and
   `git checkout polish/revision`. Confirm that `git log -1` is the
   handoff commit ("Halt the polish revision for handoff ...").
2. **Packages.** System packages used on the original host (Ubuntu
   names). Compiler majors should match, because receipts bind tool
   versions; record any difference.
   - compilers and build: gcc 15 (gcc-multilib for -m32), clang-21,
     lld-21, llvm-21, cmake (4.x), ninja-build, python3 (3.14), make;
   - debug and analysis tools: gdb, valgrind, strace, bubblewrap,
     linux-tools (perf);
   - embedded and emulation: gcc-arm-none-eabi, qemu-system-arm,
     qemu-system-x86, qemu-user (qemu-ppc64, qemu-x86_64), mtools,
     openocd, gdb-multiarch, stlink-tools, cbmc, zstd;
   - **also install libssl-dev.** It was missing on the original host, so
     run_checkpoint_transfer and the b06 signature producers could not
     run.

   The original host's versions are in
   `reviews/v5/expansion/polish_revision/environment.json`.
3. **Pinned author tools** (clang-format 21.1.8 and the others; never use
   another clang-format):

   ```
   python3 tools/install_author_tools.py --install --initialize-beads
   python3 tools/install_author_tools.py --user-lsp-shims   # optional, for the LSP plugins
   python3 tools/check_author_tools.py --report /tmp/author_tools_check.json
   ```

   See `planning/author_tools.md`. Then check
   `build/author-tools/python/bin/clang-format --version` reports 21.1.8.
4. **Claude Code project setup.** The project setup is tracked:
   `CLAUDE.md`, `AGENTS.md`, `.claude/settings.json`, `.claude/agents/*.md`,
   `.mcp.json` and `.claude/skills`. Re-enable the plugins listed in
   `planning/author_tools.md` ("Claude Code integration"): Beads,
   Superpowers, OpenSpec, Spec Kit, Graphify, clangd-lsp and pyright-lsp.
5. **Claude memory.** Copy `planning/handoff/claude-memory/*.md` into the
   new session's memory directory,
   `~/.claude/projects/<repo path with / replaced by ->/memory/`. For
   example, for `/home/me/dev/c_mastery` that is
   `~/.claude/projects/-home-me-dev-c-mastery/memory/`. Alternatively, the
   continuation prompt below tells the session to read them from the
   repository.
6. **Beads issues** (optional): run
   `python3 tools/author-tool bd import planning/handoff/beads_issues.jsonl`
   after initialisation. These are 12 coarse batch epics. The canonical
   ledger is the obligations file plus the checkpoint.
7. **Host policy for PMU work (B8).** The original host had
   `kernel.perf_event_paranoid = -1` and Secure Boot disabled, both set by
   the user. Tools must never change them. On a new machine, set them
   yourself if PMU measurements are to run. Hardware counters differ by
   CPU, so measurements from the two machines must never be mixed (see
   B8 below).
8. **NUCLEO-F439ZI board,** if it moves to the new machine:
   - Recreate the udev rule `/etc/udev/rules.d/70-cmastery-stlink.rules`,
     giving `TAG+="uaccess"` for 0483:374b and its ttyACM. Reload with
     `udevadm control --reload` and run `udevadm trigger`.
   - Copy the identity baseline to the tool's state directory:
     `mkdir -p build/f439 && cp reviews/v5/expansion/polish_revision/physical/phase2/probe-baseline.json build/f439/`.
   - The board was left running the park image bf-report-115200, with
     option bytes OPTCR 0x0FFFAAED and OPTCR1 0x0FFF0000 equal to the
     baseline.
9. **Raw evidence tarball** (optional, not in git). The untracked campaign
   receipts (about 906 MB) and the build-side logs were packed on the
   original host into `/home/fred/c_mastery_handoff_raw_20261031.tar.zst`:
   42.6 MB, sha256
   `24b14d8b2ab06029d25eb1cd9769888d3ba0d3b6bfaf48653e2b67621f435f07`. Copy
   it over by hand if you want the raw receipts, and extract it at the
   repository root:

   ```
   zstd -dc c_mastery_handoff_raw_20261031.tar.zst | tar -xf -
   ```

   This restores `reviews/v5/expansion/polish_revision/campaigns/`,
   `build/campaign-logs/`, `build/f439/runs/` and
   `build/f439/probe-baseline.json`. A compact index of all of it is
   tracked in `planning/handoff/campaign_index.json`. Nothing in the plan
   requires the tarball: B11 re-executes producers.
10. **Private toolchains of the first author host** (`/home/frede/...`,
    `~/cmastery-*-tools`, `build/b06-tools`, `build/posix-io-toolchain`,
    `build/checkpoint-tools`) were absent on the original polish host too.
    Producers that need them fail as "infra" (Part E).

---------------------------------------------------------------------------

## B. Prompt for the new Claude Code session

Paste this as the first message, from the repository root on `polish/revision`:

> Resume the C Mastery polish revision on this machine. You are the
> orchestrator. First read, in order:
>
> 1. `AGENTS.md` and `CLAUDE.md` (the working agreement and Claude rules;
>    they take precedence).
> 2. `planning/handoff/CONTINUATION_PROMPT.md`, Parts C-F (state,
>    procedures, open items, next steps).
> 3. `planning/polish_revision.json` (the checkpoint) and
>    `research/v5/expansion/polish_revision/obligations.json` (the
>    canonical POLISH-* ledger).
> 4. `planning/handoff/polish_plan.md` (the approved plan) and
>    `planning/handoff/original_polish_prompt.md` (the original
>    campaign).
> 5. `planning/handoff/claude-memory/*.md`. Treat these as your memory,
>    and install them into your memory directory if they are not there.
>
> Then verify the environment (Part A of the handoff, items 2-4). Run
> `python3 tools/check_c_style.py` and
> `python3 tools/public_validation.py --output <fresh path under build/>`,
> and report what differs from the original host. Then continue with Part
> F, "Next steps", in order. Keep every rule in CLAUDE.md:
> - cheapest capable model per role;
> - local commits per verified batch on polish/revision;
> - never push, tag or release;
> - never edit historical reviews/ receipts;
> - never weaken checks;
> - MISRA text never copied;
> - perf_event_paranoid recorded and never changed.
>
> Report honestly what is unavailable on this host.

---------------------------------------------------------------------------

## C. State at handoff (per batch)

| Batch | State | Key commits |
| --- | --- | --- |
| B0 setup, ledger, tools, Claude setup | complete | early commits; `planning/author_tools.md` |
| B1 house style spec | complete | `curriculum/guides/c-style.md`, `.clang-format`, `tools/check_c_style.py`, `tools/c_style_manifest.json` |
| B2 mechanical migration | committed and audited; revalidation in progress | 74a903922 (anchors), f6f8d8c07 (rename), 7004a7e3b (format), e70609563 + 6c5fc73ee + 3434dbdf5 (braces), ddd1bad6f/d4dc63dca (text rename), baca38839 (bridge in shared helpers), f1346f02c (2d/2e object audit), 488bf821e, a84fe2292, 39f67fe39, 9f351de15 (producer repairs found by the sweep) |
| B3 measurement defects | complete | e7a8fd9c0 |
| B4 learner-native PMU path | complete | e7a8fd9c0 |
| B5 contextual-v2 route | committed | e0ec0a5e6 |
| B6 F439 physical | tooling revision 4; phases 2-3 recorded | 7ce0251ba, 647feced2 (tool); 4126763f9, e0ff8dfa4 (phase 2); cbd064ea3 (phase 3) |
| B7 security labs | complete | 2d9a7d75f |
| B8 diagnostic dossiers | **evidence collected, write-ups missing, unreviewed** | this handoff commit (see below) |
| B9 readability rewrite | 27 lab families reviewed (waves 1-5) + map.c fix | fb4016ff2, de4f786f5, 12a7eaea0, b32007134, f7b79d464, 5fae180f8, 28138c9e2, e5d606493, 26ce2abee, e23fdf94a |
| B10 assessment packages | complete | 760171add |
| B11 final validation, receipts, ledger, report | pending | — |

**B8 detail.** All measurements ran on the original host (Ryzen 3 7330U,
Zen 3, kernel 7.0.0-42, perf_event_paranoid -1), in quiet windows with
load below 1.5.
- **Evidence (committed):**
  `reviews/v5/expansion/polish_revision/dossiers/`, with one directory each
  for `representation_locality`, `dispatch_predictability` and
  `concurrent_state`. Each holds:
  - `spec.json`, `tests.json` (equivalence tests), `rows.jsonl` (raw
    per-repetition rows), `analysis.json` and `analysis_rederived.json`;
  - `events.json` (sysfs and perf-list event definitions),
    `environment.json`, `build.json`, `build/` (the measured binaries) and
    `disassembly.txt`;
  - `logs/`.

  There are also `pilots/` and `sources.json` (the primary sources
  consulted, including the kernel AMD event mapping).
- **Sources (committed but staged, unreviewed):** the drivers and
  kernels sit in `planning/handoff/b8_wip/`, under
  `labs/performance_followthrough/dossiers/...` and
  `assessor/performance_followthrough/dossiers/...` (relative paths kept).
  They were moved out of `labs/` and `assessor/` so that unreviewed code
  is not scanned by the style check or public validation. The receipts'
  source pins refer to the original paths: move the trees back before any
  check.
- **Missing:**
  - the three `DOSSIER.md` write-ups, which must be written from the
    collected evidence;
  - the learner worked example;
  - the reviewer-deep review;
  - the commit.

  The agent was stopped just after it wrote: "All the evidence is in
  place. Now the three DOSSIER.md write-ups". Brief:
  `planning/handoff/orchestration/b8_dossier_brief.md`.
- **New machine:** writing the dossiers from this evidence needs no
  hardware. Any NEW measurement on a different CPU must go to separate,
  clearly labelled receipts and must never be mixed with the 7330U data.

**B9 detail.**
- Manifest states: 28 reviewed (27 lab families + board_f439), 117
  renamed (to rewrite), and 26 formatted. The formatted ones hold the
  typedef naming findings, so the naming batch comes first for them.
- The remaining families in route order, with sizes, are in
  `planning/handoff/b9_remaining.json`. Next in route order:
  - hosted_systems (12.8k lines; split by subdirectory);
  - advanced_concurrency_depth (4.8k);
  - kernel_lifecycle (1.4k);
  - bringup_depth (1.5k);
  - peripheral_depth (13.4k; split);
  - firmware_io, serial_depth, firmware_lifecycle, g474_uart_followthrough,
    diagnostic_ring, interrupt_depth, constrained_depth,
    embedded_debug_depth, testing_followthrough, security_depth, and so on.

**B2 sweep detail** (`planning/handoff/sweep/`):
- **sweep1:** all 617 `tools/run_*.py` with a `__main__` were run at
  4126763f9 (`sweep1_runs.tsv`, `sweep1_class.json`). Results:
  - 298 passed;
  - 200 needed arguments;
  - the rest were classified. Two read-only diagnoses (`diag1_ids.txt`,
    `diag2b_ids.txt`) led to the repair commits listed above.
- **sweep2:** the 203 argument-carrying runs (`sweep2.list`, built from
  each producer's argparse) were started at e23fdf94a and stopped at
  164/203 (`sweep2_runs.tsv`, `sweep2_class.json`; not yet diagnosed).
- **Remaining classes:**
  - infra (private toolchains);
  - stale nested evidence (B11);
  - the kernel_smp QEMU SMP flake (pre-existing at the base).

**B6 open items.**
- E3 `bu-watchdog-cross` did not arm its IWDG within 20 s (stage 3,
  IWDG_SR PVU|RVU pending). The hypothesis is untested: the halted attach
  with DBG_IWDG_STOP stalls the update. Testing it needs a reviewed
  decision.
- The D1 mechanism (running debugger reads unreliable while the bring-up
  images run) is not established.
- The power-on step (USB replug) needs the user.
- Evidence: `reviews/v5/expansion/polish_revision/physical/{PLAN.md,
  phase2/STATUS.md, phase3/STATUS.md}`.

---------------------------------------------------------------------------

## D. Working procedures that proved necessary

**Orchestration**
- The main session orchestrates. It owns the plan, ledger, tools,
  generators, catalogs, the manifest, acceptance and every commit.
- Workers get disjoint write sets.
- At most 3 build-heavy jobs at once (6.6 GiB RAM), plus at most one
  campaign driver.
- Roles and models are in `.claude/agents/`:
  - c-rewriter (sonnet) for rewrites and fix passes;
  - reviewer (sonnet) for ordinary review;
  - reviewer-deep (opus) for kernel, process, signal, concurrency,
    firmware and security code;
  - systems-engineer (opus) for PMU and F439 design;
  - board-operator (haiku) for exact board command lists;
  - inventory-scout and campaign-runner (haiku).
- Escalate one tier only after two failed verifications.

**B9 family rewrite cycle** (each step has a reusable brief in
`planning/handoff/orchestration/`; adjust absolute paths)

1. Dispatch c-rewriter with `isolation: worktree` and the brief
   `rewrite_worker_brief.md`. The worker does:
   - `git merge --ff-only polish/revision`;
   - symlinks `build/author-tools`;
   - runs every producer of the family before and after;
   - reports specimens, anchors, cm_parse wiring and line citations.

   Two or three families per worker, about 5k lines.
2. Review with `review_brief.md`, pointing at the worker's worktree diff.
   Use reviewer-deep for risky families. Insist on differential testing
   (HEAD against new), strace and fault injection for OS code, and token
   identity for concurrency code and generator-copied regions.
3. Fix pass: c-rewriter in the same worktree with a fix brief (examples:
   `example_*_fix_brief.md`). Comment-only fixes must prove identical
   `.text`.
4. Integrate:
   - `git -C <wt> diff HEAD -- labs/<f> assessor/<f> > patch` then
     `git apply`;
   - add specimen entries (`{path, defect}`) to `tools/c_style_manifest.json`,
     with learner-visible tags kept neutral and pointing at CONTRACTS.md;
   - set the family states to `reviewed`;
   - run `tools/check_c_style.py --family ...`.
5. Commit-view verification:
   - `git worktree add --detach build/frozen/<label> HEAD`;
   - symlink `build/author-tools`;
   - rsync exactly `git diff --name-only`;
   - run an adapted `commit_view_verify_template.sh`: regenerate the
     catalog (`tools/author_exercises_v5.py`) and the route
     (`tools/generate_native_route.py`); then `check_c_style`,
     `check_security_labs`, `check_native_route`, `check_exercises_v5`,
     `check_exercise_workflow` and `public_validation`; then every
     consuming producer (`git grep -l <family> -- 'tools/run_*.py'
     'tools/check_*export*.py'`).
6. Close out:
   - copy the regenerated `curriculum/v5/exercises.json` and
     `routes/contextual-v2.json` back to the main tree;
   - `cmp` every path between the view and main;
   - archive receipts under
     `reviews/v5/expansion/polish_revision/campaigns/<label>/` (untracked);
   - commit the explicit paths with
     `git -c user.name=FredericCJ -c user.email=frederic.ferragne@outlook.com commit ...`
     (no global identity was configured on the original host; use the
     user's own if configured), ending the message with
     `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`;
   - remove the view and the worker worktree and branch.

**Producer and tool gotchas** (each one cost a regression here)
- Many `tools/run_*.py` have no argparse: `--help` runs them and rewrites
  tracked receipts. Read the source for arguments, and run producers only
  in frozen or commit-view worktrees. Many require `--report` under a
  fixed base directory (`base=ROOT/'reviews/...'`); some need `--work`.
- Use `CMASTERY_TIMEOUT_SCALE=4` on a loaded host.
- TMPDIR goes on disk OUTSIDE the repository. On the original host `/tmp`
  was a small quota tmpfs, and producers treat files under the repo as
  inputs.
- Golfed one-line Python: never append inline `# comments` when patching,
  because they silently comment out the rest of the line. Patch exact
  substrings and preserve CRLF files byte for byte (`.gitattributes` has
  `* -text`). Many cards and JSON files are CRLF.
- Anchors: tools locate C by token sequence (`tools/c_source.py`
  `find_unique`/`replace_unique`, which tolerates inserted braces). Some
  tools still had exact-text anchors; several were found and ported by
  the sweep.
- Mutation markers are literal CHECK text. They are compared whitespace-
  insensitively (squash), so variable names inside CHECK lines must not
  change.
- Generators copy regions token-for-token:
  - `assessor/algorithm_invariants/instrument.py` copies sequence.c and
    transfer.c;
  - `assessor/algorithm_heap/generate.py` copies ordered.c;
  - `author_storage_references.py`;
  - `labs/language_portability/registry/generate.py`, whose output is
    excluded from layout.
- Exercise briefs cite line numbers in lab files (for example X-T35-001).
  Re-anchor them when a rewrite moves lines.
- Historical pins: accept them only through
  `tools/source_equivalence.carried_binding`/`carried_forward` (the style
  bridge `curriculum/v5/style_migration.json`), labelled "carried
  forward, not re-executed". Tools are never carried; changed tools mean
  re-execution.
- `source_equivalence.py objects --neutral-line --squash-strings` is an
  audit mode. Its reports are refused by `record`.

**F439 board** (`tools/f439_board.py` only)
- Original images only.
- No option-byte, OTP or ST-LINK writes.
- One holder at a time.
- Identity is read inside a checked halt window.
- `recover` after any refusal, and `analyze probing` before the next
  flash.
- Exact command lists in PLAN.md §6.
- Use board-operator with strict stop rules and quote-only reports.

**Quiet-window protocol for PMU measurement**
- Create `<orch dir>/QUIET`, wait until the 1-minute load is below 1.5,
  measure, then remove the marker (on error too).
- The drivers in `planning/handoff/orchestration/frozen_driver2.sh` and
  the verify template pause while the marker exists.

---------------------------------------------------------------------------

## E. Open items and known defects

All are in `planning/polish_revision.json` `known_defects`. The main
ones:

- **Infra on the original polish host** (private toolchains of the first
  author host):
  - object_{alias,atlas,buffer,cursor} (i386 sysroot, b06 lld);
  - effects_followthrough (strace, musl, glibc 2.41, picolibc);
  - foundation_followthrough's ppc64 group;
  - portability facilities and schema companions;
  - architecture_followthrough (STM32 HAL);
  - numeric_followthrough and checkpoint_entry (qemu and strace trees);
  - posix_context_followthrough (glibc source);
  - thread_atomics (i386);
  - debugging_followthrough, b06_* and security_extensions (lld,
    monocypher, OpenSSL, rustc, z3, cbmc);
  - capstone_firmware and serial_spi (newlib);
  - network_http_variants (AVR);
  - performance_delegated_exports (kernel and busybox);
  - checkpoint_transfer (libssl-dev).

  Workers' patched-copy comparisons are not receipts.
- **Stale tracked receipts.** Tracked `research/` receipts were not
  refreshed by the commit-view runs (those are archived under
  `campaigns/`). Downstream checks of nested evidence fail until B11
  re-executes producers at the final source state:
  - run_posix_retry/workload;
  - run_security_worker;
  - run_metaprog/adt measurement closures;
  - run_replicated_build_cost.
- **kernel_smp.** The QEMU SMP guest tests are flaky under load, at the
  base and at the revision. This affects run_legacy_matrix_v4/v5,
  run_target_matrix, run_targets_v4/v5 and run_storage_async_guest.
- **Text-rename leftovers.** The B06 proof-pinned documents keep old type
  names (a successor proof is needed). The close_object_{cursor,alias}
  --integrate inputs keep pre-rename names.
- **Briefs and routes.** 24 family-E briefs still say "compiled semantic
  controls". There are four route gaps from B10.
- **Naming batch.** 99 findings (84 aggregate typedefs, 13 `_fn`, 2
  typedef-case) via `tools/c_rename.py` plus object proof. The 9
  common-word typedefs (queue, store, ring, shared, split, packet,
  medium, fifo, Context) need type-aware tooling.
- **F439:** E3 watchdog cross-check, the D1 mechanism, the power-on
  observation.
- **Rejected settings.** Proposed `.claude/settings.json`
  permission/hook entries were not applied, because the auto-mode
  classifier treated them as self-modification (`planning/author_tools.md`).

---------------------------------------------------------------------------

## F. Next steps, in order

1. **Environment check** (Part A). Record differences from
   `environment.json` as a new receipt under
   `reviews/v5/expansion/polish_revision/` (fresh path).
2. **Finish B8.** Move `planning/handoff/b8_wip/*` back to `labs/` and
   `assessor/`. Then:
   - format with the pinned clang-format and pass
     `tools/check_c_style.py`;
   - re-run each dossier's equivalence tests (offline);
   - write the three `DOSSIER.md` from the committed evidence, plus a
     learner worked example;
   - get a reviewer-deep review that checks every claim against
     `rows.jsonl`, `analysis*.json`, `events.json` and `disassembly.txt`;
   - commit.

   Do not re-measure unless the review finds the evidence inadequate. On
   a different CPU, re-measurement means new, separately labelled
   receipts.
3. **Finish the B2 sweep.**
   - Resume sweep2 (39 runs left) at the current HEAD with
     `frozen_driver2.sh` (adjust its absolute paths). Its `SKIP_EXISTING`
     needs the local campaign dirs from the tarball; without them it
     simply re-runs.
   - Classify with `classify_sweep.py`.
   - Diagnose new failures read-only, with a reviewer and the
     A/B/C/D/E/F taxonomy in Part C, then repair tools centrally.
4. **Continue B9** in route order (`planning/handoff/b9_remaining.json`),
   two or three workers per wave.
5. **Naming batch** for the 26 formatted families, then their B9 rewrite.
6. **B6 follow-ups** when the board is attached: decide on and test the E3
   hypothesis; the power-on step needs the user's replug.
7. **B11:**
   - re-execute every runnable producer at the final source state and
     commit the refreshed tracked receipts;
   - historical receipt audit through the bridge;
   - `tools/build_v5.py --profile public`, the author-evidence profile and
     `check_candidate_exports_v5.py`;
   - ledger reconciliation (`curriculum/v5/expansion/implementation_updates.json`);
   - final report.

   Report implementation, author execution, hardware or physical
   observation, and independent assessment as separate dimensions. Never
   claim learner or independent-review evidence that was not produced.
