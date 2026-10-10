# Independent review: embedded and integration teaching

Status: accepted after fresh review. The review covers ten modules, 59 module projects, all twenty integration briefs, the profiles/inventory/integration guides and their saved oracles. All four findings are resolved; no blocking finding remains. Exact source-locator, workload and prerequisite checks have no local errors.

## EIR-01 — resolved (medium)

Paths: `curriculum/projects/PR-T35-02.md`, `curriculum/modules/T35.md`

Original finding: Actual denied-effect acceptance is required, but the concrete activity specifies only a private working directory, descriptor inheritance and logical capability membership. Those do not define which attempted OS effect is enforced as denied or how. The prose correctly denies that an ordinary subprocess is automatically a sandbox, but the required activity still lacks the enforcement step.

Verified resolution: Freshly read PR-T35-02 and T35.05/.08: actual inherited O_RDONLY descriptor read, write=-1/EBADF and unchanged digest; separate logical write-config denial and explicit residual filesystem/network/process authority table. No complete sandbox claim.

## EIR-02 — resolved (medium)

Paths: `curriculum/projects/PR-T34-06.md`

Original finding: The whole project is marked Profile E while its steps ask for -fprofile-generate/-fprofile-use and physical target deployment without identifying where profile collection runs or which instrumentation/runtime/transport services exist. This conflates a hosted PGO comparison with the firmware dependency boundary.

Verified resolution: Freshly read PR-T34-06: hosted Profile A libgcov/instrumentation/training and held-out comparison explicitly separated from plain/LTO uninstrumented Profile E firmware; independent physical timing and dependency evidence remain required.

## EIR-03 — resolved (low)

Paths: `curriculum/projects/PR-T35-01.md`, `curriculum/projects/PR-T35-03.md`, `curriculum/projects/PR-T32-06.md`, `curriculum/modules/T17.md`

Original finding: Several project sentences concatenate words and numbers, such as opcode/count descriptions, work-quota contracts and MC/DC pair hints. The T17 project-list preview truncates the FIR/IIR contract at y[n]=0. The detailed project supplies the full recurrence, so this is presentation rather than a numerical failure.

Verified resolution: Freshly read corrected command/quota contracts, spaced MC/DC pair hint and complete FIR/IIR project-list summary. No arithmetic change hidden in the prose repairs.

## EIR-04 — resolved (low)

Paths: `curriculum/guides/integrations.md`

Original finding: The P4 activity explicitly consumes P3 output framing, but the integration-family foundation table omits P3 from P4 foundations. The project contract makes the dependency discoverable, yet the table can mislead route planning.

Verified resolution: Freshly read integration guide: P4 foundations now explicitly include P3, matching the existing framing-consuming activity.

## Verification and limits

The saved embedded oracle recomputed 59 fixtures with zero failures; the integration oracle recomputed seven fixtures with zero failures. Seven independent boundary probes checked rounding/saturation, finite publication schedules, duplicate DMA completion, MC/DC pairs, modular vector arithmetic, roofline arithmetic and rejection of an invalid initialization extent. The refreshed inventory comparison has nonnegative absolute/relative tolerances, correct output units and zero/far-reference examples; it separates reference uncertainty from conditioning/stability.

These checks certify the declared synthetic/model scopes. Physical firmware, kernel loading, sanitizer campaigns and external proof-tool execution remain learner evidence obligations. No other author’s files were edited by this review. The JSON receipt preserves original findings, resolution evidence and fresh SHA-256 snapshots.
