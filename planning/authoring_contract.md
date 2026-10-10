# Authoring contract

Teaching authorship begins only after `reviews/research_gate.json` reports accepted. The three original specifications remain unchanged. Each author owns disjoint module, project and fixture paths assigned by the integrator. Source selections must use IDs and locator scopes from the accepted registry; newly assigned passages need additional inspections.

## Canonical module Markdown

Each `curriculum/modules/Txx.md` begins with a fenced JSON object with these fields:

`id` (Txx), `title`, `track` (shared/linux/bare-metal/elective/cross-track), `status` (core/elective/diagnostic), `prerequisites` (domain IDs), `prelude_objectives` (leaf IDs required before projects rather than all of that domain), `workload` (reading, implementation, verification, assessment, consolidation; integer hours), `readings` (resource_id, locator, purpose, hours), `projects` (project IDs), `profile` (A–F or several), `equipment` (array).

The visible prose has: `## Purpose and outcomes`, `## Entry check and remediation`, `## Study sequence and teaching`, `## Reading guidance`, `## Implementation projects`, `## Mastery gate and feedback`, `## Consolidation and transfer`, `## Environment and limits`. All subtopics are taught under `### Txx.nn — Title` headings inside the study sequence. Give every subtopic an original explanation of the mechanism, a concrete investigation or implementation task, a misconception and observable evidence. Do not merely paraphrase the taxonomy or generate uniform filler. Include short original worked reasoning where useful, especially numerical/semantic traps. The public site will append the current assessable objective set and cross-links at each subtopic; the author should not duplicate their IDs throughout prose.

Core/elective dispositions follow the taxonomy; T26 is core awareness with advanced implementations elective. At least one detailed project covers every subtopic: topic-to-project links may share a project where its contract explicitly exercises those concepts. Within a shared project the learner supplies an evidence item for each linked objective. Source inspection and design-analysis projects are acceptable for awareness/history objectives; they do not replace required mechanism implementations. Domain prerequisites may be refined using justified prelude subsets.

## Canonical project Markdown

Each `curriculum/projects/PR-Txx-nn.md` begins with fenced JSON: `id`, `title`, `module` (Txx), `topics` (subtopic IDs), `status` (core/elective/diagnostic/awareness), `hours` (4–12), `profile` (A–F), `fixtures` (relative fixture paths). Project prose has `## Contract`, `## Inputs and expected results`, `## Steps`, `## Failure cases`, `## Acceptance and evidence`, `## Hints and recovery`, `## Transfer challenge`. State exact contracts, outputs, constraints, independent oracle, appropriate edge cases and failure response, and the connection to prior work. Supply compiling unsolved learner scaffolds with staged hints. Keep complete reference implementations in assessor/ and a separate assessor export; ordinary learner initialization must not copy them. Disclose emulator/hardware distinctions and any environment-sensitive expected observations.

Choose coherent small milestones (usually 3–6 per module; more when breadth requires). Reading, assessment and delayed consolidation have separate budgets. Sum project hours to module implementation+verification budget; do not add project totals again to route totals. Project status for elective topics is elective; if mixed, make the core subset and optional extensions explicit.

Supply at least one concrete fixture per project in `fixtures/<author>/`. Prefer JSON with `id`, `description`, `input`, `expected`, `verification_method`; supply CSV/binary/text where the contract needs it. Put an independent saved Python `verify_<author>.py` in `tools/` which recomputes every expected fixture value in that author's files. It must verify the stated mathematical/semantic oracle, not implement the learner's full target program. Command output lists counts and failures. For document/binary-inspection projects use declared static synthetic data with deterministic expectations, while actual tool behavior remains separately observed by learners.

## Review

All phases must contain substantive prose, bounded essential readings, explicit assessment/remediation and original project contracts. Source IDs/locators and numerical expectations are checked independently. A machine coverage pass alone does not establish that the teaching supports every objective. Report gaps instead of concealing them with boilerplate.


## Edition 2 migration and execution

The v1 tag preserves the historical edition. Active prose and project structure may change; curriculum/migration.json maps every previous objective/project to retained or explicitly replaced teaching. Objective cardinality follows the concepts rather than a fixed count of three. Project-level dependencies govern the route; selected-objective preludes must name evidence producers and remain distinct from complete-domain requirements.

New family sources need bounded inspection records under research/v2 and explicit reading eligibility. Reference identities distinguish work, edition and inspection. Do not turn bibliographic discovery into inspected evidence. Run actual learner-stub rejection and assessor C tests; fixture Python alone establishes only the supplied data. Include incorrect implementations that meaningful checks reject.

Each test result records input hashes, environment, commands, output and scope. A stale, unavailable or unrun result cannot pass. Separate host, emulator and physical claims. Full program completion requires all ten physical interfaces. Learner time calibration is pending until learners perform the work; author execution times are not calibration.

The fixed first-edition identity snapshot is `curriculum/baseline_v1.json`, checked
against its pinned SHA-256. Migration rows must preserve the complete old ID set,
original objective text and original project fingerprints. Dispositions are
`retained`, `revised`, `split` or `merged`; changing a count cannot hide a substitution.

A project may declare `prelude_override` to replace inherited module preludes for
an earlier, narrower experiment. Its `prelude_objectives` and visible brief must
agree with that selection, and `prelude_policy` explains the dependency decision.
Later projects retain the applicable advanced objective cards. Every v2 source
addition must be pinned in `reviews/v2/research_gate.json` before publication.
