# C Mastery 5

Learn C by growing a shared component toolbox and three recurring systems: a Linux
telemetry/HTTP service, a firmware acquisition gateway, and a VM/store kernel
appliance. Representation, algorithms, assembly, architecture, debugging,
measurement, communications and concurrent execution recur as the systems grow.
Every domain names its mechanisms in precise, idiomatic vocabulary.

Start with [the learning guide](curriculum/v4/guide.md), then
[the offline workflow](curriculum/v4/workflow.md). The
[complete Markdown curriculum](c_programming_mastery_curriculum.md) and generated
HTML handbook combine teaching, source passages, project contracts and objective maps.

The fifth edition is an expansion on the accepted fourth edition (commit `be975ef`,
tag `curriculum-v4-2026-10-11`). `curriculum/edition.json` is the manifest: it
records the preserved fourth-edition counts, the domains and laboratory families
added so far, the receipts acceptance requires and the status of physical and
learner evidence. `planning/overhaul_v5.md` states the authorized scope: eight new
core domains (T57–T64), a physical-evidence campaign on the NUCLEO-F439ZI and
NUCLEO-H745ZI-Q boards, and a vocabulary system for all domains. Until a domain is
authored and accepted, the catalog is the fourth-edition catalog: 56 domains, 429
projects, 1,881 objectives, 1,357 working units, 627 recall sets and 58 artifact
contracts across 13 laboratory families, within 4,879 planned hours.

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

A laboratory family that ships its own `CONTRACTS.md` keeps it in the learner
workspace; the generic component-contracts guide then lands beside it as
`COMPONENT-CONTRACTS.md`. Ordinary initialization copies no assessor
implementation; `--assessor` is an explicit author/reviewer option. Passing visible
tests does not establish every linked objective or independent learner mastery.

Full completion retains [physical evidence](curriculum/guides/physical-evidence.md)
for all ten named interfaces and the required F439/H745 work. Image compilation,
host tests, emulator execution and recall cannot establish board timing, cache/DMA
behavior or physical communication. `record-physical` imports source-bound captures
and instrument metadata for later assessment.

## Build, verify and export

Publishing requires the source checkout or `c_mastery_v5_sources.zip`. The learner
bundle excludes author reviews and assessor implementations. Use
`c_mastery_v5_html.zip` for a ready-to-open handbook.

```powershell
python -m venv .venv
.venv\Scripts\python.exe -m pip install -r requirements-lock.txt
.\build.ps1
.\serve.ps1
```

On Linux, use the same locked publishing dependencies and run
`python tools/build_v5.py`. Serve the result with
`python -m http.server 8766 --bind 127.0.0.1 --directory build/html`.

Current author checks and receipts live in `reviews/v5/`; `reviews/v2`, `reviews/v3`
and `reviews/v4` are historical and are restored byte-for-byte after every build.
The build checks curriculum structure, preserved fourth-edition identities
(`curriculum/v5/baseline.json`), manifest arithmetic, the local learner workflow,
release guards, references and published outputs, including the prose normalizer
and the lints against doubled list markers and glued word/number forms.
Executable and target campaigns are separate, and every receipt has a tracked producer:

- `tools/run_legacy_matrix_v5.py`: the nine retained laboratory families.
- `tools/run_unit_campaigns_v5.py machine-hosted | firmware-integration | family NAME`:
  the v4 executable units and any fifth-edition family, with mutation rejection.
- `tools/run_targets_v5.py`: six retained target configurations plus the pinned
  Linux/FreeRTOS comparison.
- `tools/run_workflow_v5.py` and `tools/run_release_guards_v5.py`: the learner
  workflow contracts and the isolated release guards on both host platforms.
- `tools/record_review_v5.py INPUT.json`: an independent review receipt.
- `tools/accept_v5.py` and `tools/package_v5.py`: verify current source-bound
  receipts, then stage four archives and reproduce HTML, Markdown and every tracked
  generated file from extracted sources.

Run execution campaigns after source changes are complete. A changed input or
failed current receipt requires revalidation; old acceptance is historical.

## Earlier editions and source organization

The v1 tag `curriculum-v1-2026-10-10`, v2 commit `40bf1b2`, v3 commit `4f3e3a2`
and v4 commit `be975ef` (tag `curriculum-v4-2026-10-11`) preserve earlier editions.
The [third-edition guide](curriculum/guides/third-edition.md), `curriculum/migration_v3.json`
and `curriculum/v4/migration.json` remain historical references. The fifth edition
preserves all fourth-edition project IDs, objectives, unit contracts, recall sets and
artifact contracts through `curriculum/v5/baseline.json` and `curriculum/v5/migration.json`.

Canonical teaching lives in `curriculum/`, executable work in `labs/`, solutions in
`assessor/`, and supplied data in `fixtures/`. References and bounded source
inspections are in `references/` and `research/`; they use the local library and
seed graphs. Rebuilding requires neither absolute library paths nor private book
captures. Archives exclude private extracts and the publishing virtual environment.
