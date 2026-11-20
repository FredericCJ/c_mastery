## Why

A learner holding the exported course cannot build or run the native PMU
collector: the only builder is an author tool that compiles assessor sources,
`init` omits the supplied adapters, the PMU units have no checks (so `check`
reports unavailable and `accept` never binds a protocol run), and the
extracted-archive validation stops at `init`. Canonical obligation:
POLISH-LEARNER-PMU in `research/v5/expansion/polish_revision/obligations.json`.

## What Changes

- Learner runner `labs/performance_followthrough/native.py` (no dependency on
  `tools/` or `assessor/`): `doctor`, `protocol`, `build`, `run`, `sample`,
  `report`, with explicit inputs for learner components and labelled supplied
  adapters.
- Route data supplies the adapters and gives the PMU units real checks;
  a `measurement.pmu` component contract connects the PMU work to the deadline
  comparison and the representation dossier.
- One shared learner-archive predicate for every packager.
- An end-to-end test from an extracted learner archive with the author checkout
  hidden.

## Capabilities

### New Capabilities

- `learner-native-pmu`: the public, learner-run path from protocol tests to a
  supported native hardware experiment.

### Modified Capabilities

None.

## Impact

`labs/performance_followthrough/`, `cmastery/`, route sources, `tools/public_exports.py`,
`tools/package_v5.py`, `tools/public_validation.py`, `curriculum/guides/public-reproduction.md`.
