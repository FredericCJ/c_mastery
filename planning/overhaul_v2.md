# C Mastery 2 implementation record

## Authorized outcome

Implement the approved comprehensive second-edition overhaul. Retain the existing
coverage and expand technical depth; supply unsolved scaffolds and staged hints;
keep x86-64 and Thumb as implementation targets; require physical evidence for
UART, USART, I2C, SPI, RS-232, RS-422, RS-485, CAN, CAN FD and Ethernet.
Add a portable bytecode runtime, persistent key-value store and bounded educational
OS, culminating in the VM and store running in one user process on the new kernel.

Baseline: `6750f52`, archived by tag `curriculum-v1-2026-10-10`.
Working branch: `overhaul/v2`. The original specifications and v1 evidence remain
historical records; active v2 acceptance requires current-source evidence.

## Delivery sequence

1. Edition manifest, objective/project migration, source research and contracts.
2. Learner runner, capability doctor, scaffolds, assessor checks and evidence model.
3. Existing-domain teaching/sequence overhaul and runnable project support.
4. Runtime, storage and OS families; portable interfaces and final integration.
5. Validation, independent review, publication and reproducible exports.

## Work ownership

- Root: edition migration, source registry integration, existing-domain overhaul,
  project graph, lab runner, validation/publication, final integration and review.
- Runtime agent: T46, runtime scaffolds/reference/tests and scoped research.
- Storage agent: T47, storage scaffolds/reference/tests and scoped research.
- Kernel agent: T48, kernel scaffolds/reference/tests and scoped research; owns
  Ubuntu package installation to prevent concurrent package-manager writes.

## Acceptance discipline

Each executable project needs meaningful checks, a correct assessor implementation
and rejected incorrect variants. Compilation, model execution, host execution,
emulator execution and physical observations are separate evidence kinds.
Unperformed hardware measurements and learner-time calibration remain pending.
Author trial times never become learner calibration by relabeling.

All existing objective identities must map to actual active teaching. Changed IDs
need explicit replacement mappings. No fixed three-objective cardinality applies
to new teaching. Reference assignments require bounded inspected passages and
explicit eligibility. Every current receipt binds to its input hashes.

## Current status

Authoring and independent review are complete. The edition contains 48 domains,
337 projects and 1,605 objectives, preserving all prior identities. Six laboratory
families supply unsolved scaffolds, staged hints, original assessor implementations
and explicitly scoped executable support for 167 projects. The remaining project
and objective obligations are stated in their briefs and evidence directory.

Review corrected migration identity validation, early prerequisite selection,
workspace advancement, dependency-cache isolation, staged test dependencies,
runtime parsing, heap repair and several target/protocol boundary defects. The
current review records preserve their findings and exact inspected source hashes.

This file is an implementation record. Final release authority is
`reviews/v2/acceptance.json`; current execution and export reproduction are in
`lab_matrix.json` and `package_report.json`. Earlier first-edition acceptance
remains historical. Physical observations and learner-time calibration remain
open learner evidence, and browser rendering review was unavailable here.
