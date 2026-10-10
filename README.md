# C Mastery 3

Learn C by building small components, from representation and assembly to firmware,
communication stacks, a bytecode runtime, a persistent store and an educational OS.
The third edition adds original task runtimes, synchronization, real-time
scheduling, x86 SMP, physical M7/M4 AMP and concurrent OS services.

Start with [the third-edition guide](curriculum/guides/third-edition.md), then
[the project route](curriculum/guides/project-route.md). The
[complete Markdown curriculum](c_programming_mastery_curriculum.md) and generated
HTML handbook combine teaching, readings, project contracts and objective maps.

The curriculum now contains 56 domains, 429 projects and 1,881 objectives.
T49–T56 add 92 projects and 276 objectives, with 204 reference records across the
complete curriculum. Each project is budgeted at 4–12 hours; the new route includes
1,023 provisional hours with reading, assessment and consolidation.

The v1 Git tag `curriculum-v1-2026-10-10` and v2 commit `40bf1b2` preserve prior
editions. `curriculum/migration_v3.json` retains all 1,605 v2 objectives and 337
project identities. The earlier VM/store/kernel implementation remains available.

## Use the laboratories

Run C laboratories in Linux or WSL with Python 3.11+, CMake, Ninja and GCC/Clang.
The capability report lists optional tools and target requirements.

```sh
python3 -m cmastery doctor
python3 -m cmastery list --module T46
python3 -m cmastery init PR-T46-01 ../my-bytecode-lab
# Implement the assigned boundary in the new workspace.
python3 -m cmastery check ../my-bytecode-lab
python3 -m cmastery report ../my-bytecode-lab
# After completing the project dossier, preserve the code and select the next brief.
python3 -m cmastery advance ../my-bytecode-lab PR-T46-02
```

Learner scaffolds compile but leave semantic work unsolved. Ordinary initialization
copies no assessor implementation. `--assessor` is an explicit author/reviewer
option; `--all-tests` runs a complete family. `--dependency runtime=PATH` and
`--dependency storage=PATH` select completed learner components for integrations.
Each family manifest states executable scope and remaining project work.
A passed anchor test does not establish every linked objective.
`advance` selects another supported project in the same family, archives its prior
brief, predictions and latest receipt under `evidence/history/`, and starts fresh
evidence for the new project. It preserves learner code and existing fixtures.

New families are `os_observatory`, `execution` and `kernel_smp`. Select declared
targets with `--target`; x86 SMP supports `--cpus 1`, `2` and `4`. The original
freestanding images link no libc, libgcc, compiler-rt, libatomic or existing OS.
Linux and pinned FreeRTOS comparisons use separate builds. See
[concurrent-system contracts](curriculum/guides/concurrent-system-contracts.md)
for wait/wake, lifetime, memory-order and hardware responsibilities.

Full completion includes [physical evidence](curriculum/guides/physical-evidence.md)
for all ten named interfaces. Hardware observations, target timing, formal claims,
independent transfer and learner-time calibration remain separate from host tests.
F439 and H745 image builds do not claim board execution. `record-physical` imports
raw captures with source/image hashes and instrument metadata for later assessment.

## Build and verify the handbook

These publishing commands require the source checkout or `c_mastery_v3_sources.zip`.
The learner bundle supports the laboratory runner above, but excludes author review
records and assessor implementations, so it cannot rebuild or accept the handbook.
Use `c_mastery_v3_html.zip` for a ready-to-open handbook alongside learner work.

```powershell
python -m venv .venv
.venv\Scripts\python.exe -m pip install -r requirements-lock.txt
.\build.ps1
.\serve.ps1
```

On Linux, install the same locked publishing dependencies in a virtual environment
and run `python tools/build_v3.py`. Open the local handbook with
`python -m http.server 8766 --bind 127.0.0.1 --directory build/html`.

`tools/run_lab_matrix.py` runs actual assessor and unsolved-scaffold checks; on
Windows it dispatches to WSL. `reviews/v3/` contains current source-bound reports.
`tools/run_target_matrix.py` checks the Cortex-M target builds and 1/2/4-CPU guests;
`tools/run_os_comparison.py` checks the pinned Linux/FreeRTOS workload.
Earlier acceptance receipts are historical and do not accept this edition.
`tools/accept_v3.py` checks current receipts before `tools/package_v3.py` exports
source, learner, assessor and HTML archives and rebuilds extracted sources.

Canonical teaching lives in `curriculum/`, executable work in `labs/`, solutions in
`assessor/`, and supplied data in `fixtures/`. Bounded source inspections are in
`references/` and `research/`; research used the local library and seed graphs.
Rebuilding requires neither absolute library paths nor private source captures.
Archives exclude private book extracts and the publishing virtual environment.
