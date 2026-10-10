# C Mastery 4

Learn C by growing a shared component toolbox and three recurring systems: a Linux
telemetry/HTTP service, a firmware acquisition gateway, and a VM/store kernel
appliance. Representation, algorithms, assembly, architecture, debugging,
measurement, communications and concurrent execution recur as the systems grow.

Start with [the v4 learning guide](curriculum/v4/guide.md), then
[the offline workflow](curriculum/v4/workflow.md). The
[complete Markdown curriculum](c_programming_mastery_curriculum.md) and generated
HTML handbook combine teaching, source passages, project contracts and objective maps.

The catalog retains **56 domains, 429 projects and 1,881 objectives**. Its learner
route now contains **1,357 working units**, including **56 new executable units**,
**627 recall sets** with 1,881 variants, and **58 named artifact contracts** across
13 laboratory families. Projects remain budget envelopes; a working unit usually
fits a one-to-three-hour session. Integration, design decisions and changed
requirements repeatedly bring earlier components back into use.

The workload remains **4,879 planned hours** for the full catalog, **4,799 core
hours**, or **4,777 hours** when the entry diagnostic removes its remaining
remediation. The new units subdivide or strengthen existing work. Reading,
verification, design, assessment and retrieval are accounted for once; estimates
remain provisional. See [the workload ledger](curriculum/v4/budgets.md).

## Start studying and implementing

Run C laboratories in Linux or WSL with Python 3.11+, CMake, Ninja and GCC/Clang.
The offline study commands also run on Windows. Keep the study profile and learner
workspaces outside the source checkout.

```sh
python3 -m cmastery doctor
python3 -m cmastery study init ../my-study
python3 -m cmastery study next ../my-study
python3 -m cmastery init PR-T01-01 ../prefix-lab --unit U-T01-01-01
python3 -m cmastery study sheet ../my-study ../session.md --unit U-T01-01-01 --assessment-template ../assessment.json
# Read UNIT.md, predict a boundary case, and implement the assigned increment.
python3 -m cmastery check ../prefix-lab --unit U-T01-01-01
python3 -m cmastery study next ../my-study --studied U-T01-01-01
python3 -m cmastery recall due ../my-study
python3 -m cmastery study status ../my-study
```

`study next` recommends ready work and at most two due recall sets in a 15-minute
allowance. A recall attempt is submitted before feedback; assisted work and
same-session repair remain distinct from delayed independent success. Printable
sheets use the same identifiers as the CLI.

At designated boundaries, use `decision seal` to record requirements, alternatives,
a discriminating experiment and a reversal condition before implementation. Use
`artifact export` and `artifact link` to carry pinned learner components into new
consumers, then `accept` to record the specific evidence demonstrated. The
[workflow guide](curriculum/v4/workflow.md) supplies exact commands and JSON formats;
[architecture](curriculum/v4/architecture.md), [reuse](curriculum/v4/reuse.md) and
[retrieval](curriculum/v4/retrieval.md) explain the progression rules.

Existing `init`, `advance`, `check`, `report` and schema-2 receipts remain usable.
`advance WORKSPACE PROJECT --unit UNIT` preserves the implementation while archiving
its prior brief and evidence. A different family uses a separate workspace and
explicit imports. Learner scaffolds compile with semantic work left unsolved.
Ordinary initialization copies no assessor implementation; `--assessor` is an
explicit author/reviewer option. Passing visible tests does not establish every
linked objective or independent learner mastery.

The new families are `machine_image`, `hosted_systems`, `firmware_io` and
`integration`; earlier families retain their roles. Target selection uses
`--target`; x86 SMP supports `--cpus 1`, `2` and `4`. Original freestanding images
retain the strict exclusion of libc, libgcc, compiler-rt, libatomic and an existing
OS. Hosted Linux and pinned FreeRTOS comparisons use separate builds.

Full completion retains [physical evidence](curriculum/guides/physical-evidence.md)
for all ten named interfaces and the required F439/H745 work. Image compilation,
host tests, emulator execution and recall cannot establish board timing, cache/DMA
behavior or physical communication. `record-physical` imports source-bound captures
and instrument metadata for later assessment.

## Build, verify and export

Publishing requires the source checkout or `c_mastery_v4_sources.zip`. The learner
bundle excludes author reviews and assessor implementations. Use
`c_mastery_v4_html.zip` for a ready-to-open handbook.

```powershell
python -m venv .venv
.venv\Scripts\python.exe -m pip install -r requirements-lock.txt
.\build.ps1
.\serve.ps1
```

On Linux, use the same locked publishing dependencies and run
`python tools/build_v4.py`. Serve the result with
`python -m http.server 8766 --bind 127.0.0.1 --directory build/html`.

Current author checks and receipts live in `reviews/v4/`. The build checks
curriculum structure, the local learner workflow, release guards, references and
published outputs. Executable and target campaigns are separate:

- `tools/run_lab_matrix.py`: retained laboratory families.
- `tools/check_machine_hosted_v4.py`: the 24 new machine/hosted units and mutations.
- `tools/verify_v4_lab_units.py`: the other 32 new units and their declared target checks.
- `tools/run_targets_v4.py`: six retained target configurations plus the pinned
  Linux/FreeRTOS comparison, preserving historical v3 receipt bytes.
- `tools/accept_v4.py` and `tools/package_v4.py`: verify current source-bound
  receipts, then stage four archives and reproduce HTML/Markdown from extracted sources.

Run execution campaigns after source changes are complete. A changed input or
failed current receipt requires revalidation; old acceptance is historical.

## Earlier editions and source organization

The v1 tag `curriculum-v1-2026-10-10`, v2 commit `40bf1b2`, and v3 commit `4f3e3a2`
preserve earlier editions. The [third-edition guide](curriculum/guides/third-edition.md)
and `curriculum/migration_v3.json` remain historical references. V4 preserves all
429 project IDs and 1,881 objectives through `curriculum/v4/migration.json`.

Canonical teaching lives in `curriculum/`, executable work in `labs/`, solutions in
`assessor/`, and supplied data in `fixtures/`. References and bounded source
inspections are in `references/` and `research/`; they use the local library and
seed graphs. Rebuilding requires neither absolute library paths nor private book
captures. Archives exclude private extracts and the publishing virtual environment.
