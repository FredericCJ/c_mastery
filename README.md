# C Mastery 2

Learn C by building small components, from representation and assembly to firmware,
communication stacks, a bytecode runtime, a persistent store and an educational OS.

Start with [the second-edition guide](curriculum/guides/second-edition.md), then
[the project route](curriculum/guides/project-route.md). The
[complete Markdown curriculum](c_programming_mastery_curriculum.md) and generated
HTML handbook combine teaching, readings, project contracts and objective maps.

The v1 Git tag `curriculum-v1-2026-10-10` preserves the earlier edition.
`curriculum/migration.json` maps all 1,491 previous objectives and 297 projects.
T46–T48 add the runtime, store and kernel. Their final integration persists a
VM-generated value in a guest process, reboots the kernel and recovers it.

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

Full completion includes [physical evidence](curriculum/guides/physical-evidence.md)
for all ten named interfaces. Hardware observations, target timing, formal claims,
independent transfer and learner-time calibration remain separate from host tests.

## Build and verify the handbook

These publishing commands require the source checkout or `c_mastery_v2_sources.zip`.
The learner bundle supports the laboratory runner above, but excludes author review
records and assessor implementations, so it cannot rebuild or accept the handbook.
Use `c_mastery_v2_html.zip` for a ready-to-open handbook alongside learner work.

```powershell
python -m venv .venv
.venv\Scripts\python.exe -m pip install -r requirements-lock.txt
.\build.ps1
.\serve.ps1
```

On Linux, install the same locked publishing dependencies in a virtual environment
and run `python tools/build_v2.py`. Open the local handbook with
`python -m http.server 8766 --bind 127.0.0.1 --directory build/html`.

`tools/run_lab_matrix.py` runs actual assessor and unsolved-scaffold checks; on
Windows it dispatches to WSL. `reviews/v2/` contains current source-bound reports.
The older `reviews/final_acceptance.json` is historical and does not accept v2.
`tools/accept_v2.py` checks current receipts before `tools/package_v2.py` exports
source, learner, assessor and HTML archives and rebuilds extracted sources.

Canonical teaching lives in `curriculum/`, executable work in `labs/`, solutions in
`assessor/`, and supplied data in `fixtures/`. Bounded source inspections are in
`references/` and `research/`; research used the local library and seed graphs.
Rebuilding requires neither absolute library paths nor private source captures.
Archives exclude private book extracts and the publishing virtual environment.
