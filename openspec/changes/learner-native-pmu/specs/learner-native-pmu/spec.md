## ADDED Requirements

### Requirement: Learner-built native collector
A learner workspace SHALL build the native collector from the learner's own PMU
component and explicitly selected learner components, using only files shipped
in the learner archive; supplied adapters SHALL be identified as supplied.

#### Scenario: Unfinished scaffold
- **WHEN** the learner runs the protocol step on the unfinished `pmu.c`
- **THEN** the run reports a semantic rejection, distinct from a build error, crash or timeout

#### Scenario: Author checkout unavailable
- **WHEN** the workflow runs from an extracted learner archive with the author checkout hidden
- **THEN** initialization, protocol, export/link, native build, capability report and a supported experiment complete without reading assessor or author-only paths

### Requirement: Precise capability diagnostics
The runner SHALL report the PMU access state and name the exact missing
capability when hardware events cannot be opened, without changing host policy.

#### Scenario: Paranoid setting blocks access
- **WHEN** `perf_event_open` fails with EACCES
- **THEN** the report names the perf_event_paranoid value and the CAP_PERFMON alternative and marks the hardware gate blocked, not passed
