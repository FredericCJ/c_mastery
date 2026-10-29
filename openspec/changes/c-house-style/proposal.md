## Why

The course had no coding standard: much laboratory and assessor C is written as
dense one-line statements, which hides control flow and failure paths from the
learners who are meant to read it. The user mandated one house style blending
BARR-C:2018, SEI CERT C, MISRA C:2023 and the Linux kernel style, enforced with
clang-format. Canonical obligations: POLISH-STYLE and POLISH-MECHANICAL in
`research/v5/expansion/polish_revision/obligations.json`.

## What Changes

- New learner-facing guide `curriculum/guides/c-style.md` with three tiers
  (layout, naming/structure, correctness by context), provenance for each rule
  and explicit resolution of conflicts between the four sources.
- Pinned clang-format 21.1.8 configuration (`.clang-format`), a kernel-style
  configuration for the six genuine Linux kernel modules, `.clang-tidy`, the
  scope manifest `tools/c_style_manifest.json` and `tools/check_c_style.py`
  (added to public validation).
- **BREAKING** for learner component exports: public C type names become
  lowercase module-prefixed `struct`/`enum` tags (for example `PFPmu` becomes
  `struct pf_pmu`); component contracts move to version 2 with a recorded
  migration. Historical receipts are not edited.
- All active C is reformatted in separate, equivalence-checked steps: rename
  (object identity), whitespace formatting (token identity) and brace insertion
  (brace-only token diff).

## Capabilities

### New Capabilities

- `c-house-style`: the layout, naming and context-dependent correctness rules
  that every laboratory and assessor C file follows, and how they are checked.

### Modified Capabilities

None.

## Impact

`labs/`, `assessor/` C sources; mutation runners and source slicers in `tools/`,
`labs/` and `assessor/` (ported to token matching in `tools/c_source.py`);
generators that emit C; `curriculum/v5/exercises.json` bound-file pins and route
component-contract header pins (regenerated). Historical `reviews/` receipts are
preserved; evidence that cannot be re-run is carried forward only through the
recorded source-equivalence bridge.
