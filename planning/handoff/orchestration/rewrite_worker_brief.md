# Brief: semantic readability rewrite of lab families (POLISH-STYLE, B9)

Repository /home/fred/dev/c_mastery. Read AGENTS.md, CLAUDE.md and
curriculum/guides/c-style.md first. All C in labs/ and assessor/ has already been
renamed (struct/enum tags) and laid out by the pinned clang-format with braces;
layout is therefore not your job. Your job is readability that a formatter
cannot provide, in the families assigned to you: `labs/<FAMILY>/` and
`assessor/<FAMILY>/` for each.

## Working environment (important)

- You run in your OWN isolated git worktree of the repository (your current
  working directory). It is created from `main`, which predates this revision:
  FIRST run `git merge --ff-only polish/revision` in it (no commit is made) so
  you work on the renamed, formatted sources with tools/check_c_style.py and
  labs/support/ present. Edit and verify there; never touch
  /home/fred/dev/c_mastery itself. Do not commit; the orchestrator collects your
  `git diff` of `labs/<FAMILY>/`, `assessor/<FAMILY>/` and the family's own build
  glue.
- The host has ~6.6 GiB RAM shared with other agents: build ONE configuration
  at a time (no parallel make -j beyond 2, no parallel compiler/sanitizer runs).
- Set TMPDIR=/home/fred/.cache/cmastery-tmp/b9-<your first family> (on disk).
  /tmp is a small quota tmpfs; never put TMPDIR inside the worktree either (some
  producers treat files under the repository as inputs).
- The pinned clang-format lives in the main checkout:
  `mkdir -p build && ln -s /home/fred/dev/c_mastery/build/author-tools build/author-tools`
  inside your worktree so tools/check_c_style.py finds it.
- Campaign producers write receipts into tracked reviews/ paths inside your
  worktree; that is fine there, but exclude reviews/ from what you report as
  changed, and never edit reviews/ by hand.
- Do not rename public types or functions (names used outside the file); a
  separate mechanical batch owns naming. File-local (static) helpers you
  introduce follow the house naming.

## What to improve

- Split dense multi-purpose functions into named steps where it makes the
  algorithm easier to follow; remove accidental complexity and one-use
  indirection; keep substantive difficulty.
- One declarator per declaration; declare variables at first use in the
  smallest scope; `static` for file-local; `const` for what is not modified.
- Replace magic numbers with named constants where the meaning is not obvious;
  put units in names or comments (`_ns`, `_bytes`, `_cycles`).
- Comments state invariants, ownership, error contracts and why, never restate
  code. Public functions get a contract comment (preconditions, ownership
  before/after, return value on each failure, blocking).
- Command-line numeric parsing: use labs/support/cm_parse.h (checked decimal
  parser) for argv/env numbers, adding labs/support/cm_parse.c and the include
  path to the family's build (CMakeLists.txt, build.py or the runner tool's
  compile command - if the compile command lives in tools/, report the exact
  change instead of editing tools/). Do not change parsing in deliberate
  specimens.

## What you must preserve

- Behaviour and public contracts: the family's tests, expected outputs,
  mutation catalogs and anchors must still hold. Learner scaffolds stay
  unfinished (stubs return their sentinel); do not implement them.
- Deliberately defective specimens keep their defects (look for comments such
  as "intentionally flawed", "specimen", "deliberately"; add a
  `CM-SPECIMEN: <defect class>` comment at the top of such files and list them
  in your report so the orchestrator adds manifest entries).
- C string literals that tests or tools match as output (markers, labels).
- Kernel-style modules listed under `kernel_style` in tools/c_style_manifest.json.

## Anchors owned by tools

Tools and mutation catalogs locate code in your families by token sequences
(`git grep -ln "<FAMILY>" tools/ assessor/<FAMILY>/` and the producers'
`find_unique`/`replace_unique`/anchor strings). Code matched by an anchor must
keep matching exactly once: either leave that code's tokens intact, or, if a
restructuring is worth it, report the exact old and new anchor text and the
tool file so the orchestrator can migrate it. Do not edit tools/. The producer
run in step 2 proves anchors still hold.

## Producer notes (learned in wave 1)

- The shared harness (assessor/legacy_systems/toolchain/common.py) compares
  diagnostic markers ignoring whitespace, so the formatted, spaced CHECK text
  matches the compact markers in tools/run_*.py; do not write wrappers.
- On this loaded host set CMASTERY_TIMEOUT_SCALE=4 for producers built on that
  harness (some ASan compiles exceed the default 90 s bound).
- Producers that need private toolchains on another host (paths under
  /home/frede/ or ~/cmastery-*-tools, e.g. mpfr for float intervals) cannot
  fully pass here: report exactly which group fails and why.
- Python strings that generate C must use the current type names.

## Lessons from wave 2

- `CM-SPECIMEN:` tags in learner-visible files (labs/) stay neutral: say the
  file is defective by design and ask the learner to classify or predict
  first, pointing at the family's CONTRACTS.md (for example
  `/* CM-SPECIMEN: defective by design; classify the defect before running
  (see CONTRACTS.md). */`). Never point at tools/c_style_manifest.json
  (learners do not receive it) and never name the defect there; put the
  defect class in your report for the manifest.
- If a learner-visible program adopts cm_parse, every learner build of it
  needs labs/support/cm_parse.c and `-I labs/support`. List every compile
  command that builds it (CMake, build.py, tools/run_*.py, tools/check_*_exports.py)
  and the generator of the project card's `supplied_artifacts` (often a
  `public=` list in tools/close_*.py) as exact proposed edits in your report;
  do not edit tools/ or curriculum/ JSON yourself. If that widening is not
  worth it for a test-selector argument, leave the selector alone.
- Exercise briefs cite line numbers in lab files (for example
  `authorization.c:37-38`). Before reporting, run
  `git grep -nE "<basename>:[0-9]" -- curriculum/` for every file you changed
  and list each citation whose target line moved, with its new line numbers.
- If a producer needs a private toolchain that is absent, run it unchanged
  first and report the exact failing group. You may then run a patched
  temporary copy kept OUTSIDE the repository (in TMPDIR) to compare baseline
  and after; say exactly what you patched.

## Lessons from wave 3

- The earlier mechanical formatting and brace insertion can break a
  family at baseline:
  - braces that remove a probe's or specimen's intended defect;
  - a `__LINE__`-dependent pair split across lines;
  - stringified or preprocessed spellings that a tool matches exactly;
  - generated files whose bytes a generator or manifest checks.

  When a producer fails at baseline for such a reason, restore the original
  form inside a `// clang-format off: CM-STYLE: <reason>` ...
  `// clang-format on` region (check_c_style.py requires the CM-STYLE
  justification on the off line) and report it. For generated files, report
  instead of reformatting: the orchestrator restores generator output and
  excludes the directory from layout.
- Never run a producer with --help to learn its arguments: many
  tools/run_*.py have no argument parser and start a full run that rewrites
  receipts. Read the source (`grep -n "add_argument\|argv"`). Run producers
  only inside your worktree.
- Thin producers (one line that calls a shared runner, such as
  run_portability_scoped) do not mention the family name. Grep for the
  modules that import the producers you found, and run those too.

## Verification (must run, report exit codes)

1. `build/author-tools/python/bin/clang-format -i --style=file:.clang-format`
   on every file you changed, then `python3 tools/check_c_style.py --family <FAMILY>`.
2. Run the family's campaign producer (find it with
   `git grep -ln "<FAMILY>" tools/run_*.py`, or
   `python3 tools/run_unit_campaigns_v5.py family <FAMILY>` when the family has
   labs/<FAMILY>/family.json) with a fresh report path the producer accepts,
   once BEFORE your changes (baseline) and once after; both results go in the
   report. If a producer needs a private toolchain that is absent (paths under
   /home/frede/ or ~/cmastery-*-tools), say so and verify with the family's
   tests built directly instead. The assessor must pass, the learner scaffold
   must still be rejected, and every mutation must still be rejected.
3. Build once with `-fsanitize=address,undefined` where the family is hosted C.

## Report

Files changed; what was restructured and why (2-3 before/after examples);
baseline vs after producer results with exit codes; specimens found; anchors
that needed migration; anything you could not verify; the absolute path of your
worktree and `git -C <worktree> diff --stat`.
