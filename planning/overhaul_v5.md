# C Mastery 5 implementation contract

## Authorized outcome

The accepted fourth edition (commit `be975ef`, tag `curriculum-v4-2026-10-11`,
fast-forwarded to `main` before this work) retains 56 domains, 429 project IDs,
1,881 objectives, 1,357 working units, 627 recall sets, 58 artifact contracts and
13 laboratory families within 4,879 catalog / 4,799 core / 3,599
implementation-verification hours. C Mastery 5 is an expansion: every one of
those identities, texts, unit contracts and budgets is preserved, and new
material is appended. Nothing is re-tiered; the comprehensive completion policy
continues.

The owner selected eight new core domains: T57 Linux kernel programming, T58
RISC-V and AArch64 ports, T59 firmware lifecycle on the NUCLEO-F439ZI and
NUCLEO-H745ZI-Q boards, T60 a C-subset compiler to x86-64 and Thumb, T61 formal
verification and fuzzing, T62 advanced Linux I/O, T63 memory-safety offense and
defense on the learner's own programs only, and T64 numerical and SIMD depth.
Each delivers a module, 8–12 projects, fixtures with an independent oracle, a
laboratory family with an assessor implementation and mutation controls,
hand-authored executable units and recall sets, a glossary, and a domain entry in
`curriculum/v5/domains.json`. The owner has both boards, an SWD probe, a logic
analyzer, RS-232/422/485 and CAN/CAN FD transceivers with an MCP2518FD, and the
LAN8742A Ethernet route; a first physical-evidence campaign records captures for
all ten required interfaces through `cmastery record-physical` and an on-target
runner. It establishes only what its receipts show.

The overhaul shall make the curriculum convey precise and idiomatic vocabulary
for all domains: a glossary source of truth per domain with normative
authorities, published glossary pages with term links, a vocabulary lint,
vocabulary recall sets, a terminology criterion in the rubric, and a reviewed
terminology pass over the existing 56 domains recorded in
`curriculum/v5/terminology_ledger.json`. Objective texts, project identities and
budgets stay immutable; prose differs from its accepted bytes only through
logged, reviewed hunks.

## Mechanics

`curriculum/edition.json` (schema 5) is the single manifest: expected
preserved/added/total counts, targets, hardware, status fields and the required
receipt list. `curriculum/v5/baseline.json` pins the fourth-edition identities at
record level; `tools/check_v5.py` rejects any changed accepted unit, recall set,
artifact contract, graph row, resource record or objective. Thin `*_v5.py` entry
points reuse the parameterized v4 functions. Every acceptance receipt has a
tracked producer (`run_unit_campaigns_v5`, `run_workflow_v5`, `record_review_v5`,
`run_release_guards_v5`, `run_legacy_matrix_v5`, `run_targets_v5`,
`run_physical_v5`). Published prose passes through `cmastery.text.normalize_prose`;
`check_markdown` rejects doubled list markers and glued word/number forms. The
learner CLI reads the catalog named by the manifest and no longer overwrites a
family's own CONTRACTS.md.

## Work ownership

- v5_mechanics: manifest, baseline, entry points, producers, lints, packaging.
- v5_vocabulary: glossary schema, lint, publication, recall, rubric, contract
  amendment, terminology ledger and the pass over T01–T56.
- One author per domain (v5_kernel_linux, v5_ports, v5_firmware_lifecycle,
  v5_compiler, v5_verification_fuzzing, v5_linux_io, v5_memory_safety,
  v5_numeric_simd) owning disjoint paths: `research/v5/<author>_*`,
  `curriculum/modules/Txx.md`, `curriculum/projects/PR-Txx-*.md`,
  `fixtures/<author>/`, `tools/verify_<author>_fixtures.py`, `labs/<family>/`,
  `assessor/<family>/`, `curriculum/v5/recall/Txx.json`,
  `curriculum/vocabulary/Txx.json` and the domain block of `domains.json`.
- v5_physical_campaign: bench tooling, board runner, dossier validation,
  `reviews/v5/physical_matrix.json`.
- Rotated reviewers (never the author of the reviewed scope) record findings
  through `tools/record_review_v5.py`.

## Evidence discipline

Research precedes authorship: `reviews/v5/research_gate.json` must accept every
`research/v5/*resources.json` before a module is written. Author tests,
campaigns, emulator runs and physical captures are separate claims with separate
receipts; a receipt is current only while every pinned path hashes identically.
Status fields in the manifest may never exceed the receipts present. Learner
mastery, time calibration and learning effectiveness remain not-assessed,
not-run and not-measured unless measured. Author bench runs are never learner
calibration; emulator receipts are never relabelled as physical.

## Implementation order

1. Git fast-forward and branch (done: `main` at `be975ef`, branch `overhaul/v5`).
2. Mechanics refactor with zero diff in tracked generated outputs (done).
3. Prose normalizer, publication lints and the CONTRACTS.md fix (done).
4. Manifest, baseline, v5 entry points and receipt producers verified with no
   new domains.
5. Vocabulary infrastructure on the existing 56 domains.
6. Research gates and taxonomy per domain.
7. Briefs, fixtures, labs, assessors, units, recall and glossaries per domain;
   terminology pass with ledger.
8. Physical campaign, execution campaigns, browser review, independent reviews.
9. `accept_v5`, `package_v5`, four bundles with a fresh extracted rebuild.

## Current status

Mechanics stages complete through the fifth-edition scaffold (accepted and
packaged with no new domain). The vocabulary system is in place: a 120-term
core glossary, domain glossaries for T09, T23, T25, T29, T30, T44, T45 and T47,
published glossary pages with term links, `tools/check_vocabulary.py` with a
recorded ratchet baseline over accepted prose, vocabulary recall sets, the rubric
criterion and the contract amendment. Authority locators marked pending (126)
await the fifth-edition research gate; the glossaries for the remaining domains
and the terminology pass over accepted prose are stage 6 work. The catalog is
otherwise identical to the fourth edition until domain authoring lands. See
`reviews/v5/` for current receipts.
