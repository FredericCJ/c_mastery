# C Mastery 5

Learn C by growing a shared component toolbox and three recurring systems: a Linux
telemetry/HTTP service, a firmware acquisition gateway, and a VM/store kernel
appliance. Representation, algorithms, assembly, architecture, debugging,
measurement, communications and concurrent execution recur as the systems grow.
Every domain names its mechanisms in precise, idiomatic vocabulary.

The required [exercise expansion](curriculum/v5/exercises/shared-contract.md)
adds mechanism-specific workspaces, fixtures, separate assessor material and
objective-specific evidence across retained and new domains. Required
[legacy assessment supplements](curriculum/v5/expansion/legacy_assessments/shared-contract.md)
deepen the original projects without changing their historical records, including
subjects historically marked elective. The [depth audit](curriculum/v5/expansion/audit.md),
[completion ledger](curriculum/v5/expansion/completion_ledger.json) and
[remaining-work manifest](curriculum/v5/expansion/remaining_work.json) distinguish
actual authoring, scoped execution and still-required work. Historical hours are
descriptive; additional exercise work has no learner-time ceiling.

The [reconciled obligation inventory](curriculum/v5/expansion/work_inventory.json)
connects each original criterion and changed condition to its implementation and
evidence. Read its six dimensions separately: teaching, implementation, author
validation, external review, independent learner performance and physical
observation. The [scope record](research/v5/expansion/followthrough/scope.md)
explains the source boundary and required extensions whose authoritative inputs
remain unavailable. An authored domain or passing reference suite is not release
acceptance or a learner mastery result.

The [finite corpus screen](research/v5/expansion/corpus_followthrough/README.md)
maps 84 actually read design descriptions to concrete implementations and
assessment criteria, including the completed parser, index, storage, Treiber
stack and textual escape companions. The other 999 SWE entries remain metadata
routing; this screen does not claim those works were read or replace their
separately tracked curriculum obligations.

```sh
python -m cmastery init PR-T04-01 ../extent-depth --exercise X-T04-001
python -m cmastery check ../extent-depth --exercise X-T04-001
```

Current integrated receipts live under `reviews/v5/expansion/integrated/`; earlier batch and pre-expansion
reviews are retained byte-for-byte. Author test results are distinct from
independent learner assessment and physical validation.

Start with [the learning guide](curriculum/v4/guide.md), then
[the offline workflow](curriculum/v4/workflow.md). The
[complete Markdown curriculum](c_programming_mastery_curriculum.md) and generated
HTML handbook combine teaching, source passages, project contracts and objective maps.

The fifth edition is an expansion on the accepted fourth edition (commit `be975ef`,
tag `curriculum-v4-2026-10-11`). `curriculum/edition.json` is the manifest: it
records the preserved fourth-edition counts, the domains and laboratory families
added so far, the receipts acceptance requires and the status of physical and
learner evidence. [The overhaul](planning/overhaul_v5.md) and
[modern-techniques plan](planning/modern_techniques_plan.md) now have authored
domains T57–T76; their current project and objective totals are derived in the
manifest. Their source registers, contracts, laboratories and scoped
campaigns retain explicit limits; authoring is not edition acceptance. The accepted
baseline retains 56 domains, 429 projects, 1,881
objectives, 1,357 working units and 58 artifact contracts. Its historical record
contains 627 recall sets, 13 laboratory families and 4,879 planned hours. The
current catalog and family counts are derived in the manifest and completion
ledger. Additions have separate evidence and do not redefine those historical
hours. Required F439/H745 and other physical observations, independent learner
assessments and any still-unexecuted target experiments remain distinct obligations.

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
bundle excludes author reviews, assessor implementations, and fifth-edition
authoring recipes that contain solution analyses. Inspected source registers
remain available in the learner bundle; complete audits and regeneration inputs
belong to the source bundle. Use
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

Current author checks and receipts live in `reviews/v5/expansion/integrated/`. All 242
pre-expansion tracked review files, including the old `reviews/v5/` records,
are pinned and preserved byte-for-byte after every build.
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
- `tools/validate_expansion_v5.py`: integrated author checks, including the
  Linux campaign guards that verify the recorded compiler and tool identities.
- `tools/check_candidate_exports_v5.py --work-directory PATH --report reviews/PATH.json`:
  build fresh source, learner, assessor and HTML review archives, then rebuild the
  extracted sources and compare all canonical inputs and published outputs. This
  requires a current successful local publication; it does not accept or promote
  a release. Use a fresh report path and a work directory with space for the full
  source evidence and its extracted copy. Some current evidence checks require
  private reference captures and retained experiment artifacts excluded from the
  archives. For a local author reproduction, first run
  `tools/candidate_author_environment.py --manifest reviews/ENVIRONMENT.json`,
  then supply `--author-environment-manifest reviews/ENVIRONMENT.json` to the
  candidate checker. This explicitly supplies only the enumerated, hash-verified
  local files to the extracted workspace, after archive creation. The report
  records these prerequisites; it does not claim that the ZIP alone reproduces
  checks that require them. Absolute external dependencies are not copied.
- `tools/record_review_v5.py INPUT.json`: an independent review receipt.
- `tools/accept_v5.py` and `tools/package_v5.py`: verify current source-bound
  receipts, then stage four archives and reproduce HTML, Markdown and every tracked
  generated file from extracted sources. If local author inputs are needed,
  acceptance must pass first; generate a fresh environment manifest afterwards,
  then run `tools/package_v5.py --author-environment-manifest reviews/ENVIRONMENT.json`.
  Packaging rechecks acceptance and the exact selected inputs before promotion.

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
seed graphs. Full author validation requires the exact local evidence inputs
described above; those inputs are not all included in the source archive.
Archives exclude private extracts and the publishing virtual environment.
