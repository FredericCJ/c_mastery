# C Mastery — substantive polishing, learner-path validation, and technical consolidation

You are the curriculum editor, C systems engineer, and verification engineer for
my `c_mastery` repository:

https://github.com/FredericCJ/c_mastery.git

Work in the available repository checkout on native Ubuntu. Implement the changes
and run the available checks. An assessment, proposal, or list of suggested edits
alone does not fulfill this assignment.

## 1. Objective and limits of this revision

Polish the existing curriculum into a coherent, technically accurate, readable,
and reproducible learning experience. “Polish” includes correcting code and
measurement defects, improving explanations and exercises, strengthening learning
transitions, and completing public learner workflows. It does not mean cosmetic
rewording, another indiscriminate expansion, or replacing the current architecture.

Preserve the breadth and depth of low-level, hosted, and freestanding C; computer
architecture; algorithms and data structures; first-principles abstractions; and
embedded cybersecurity. The intended level is advanced systems engineering and
research. There is no learner-time ceiling or exercise-count target. Remove
accidental complexity, redundant exposition, and misleading instructions rather
than substantive difficulty.

Preserve the contextual route: after a compact foundation, introduce algorithms
and data structures where a system need motivates them. Revisit them through
changed contracts, failure modes, measurement, and independent transfer. Keep the
subject catalog useful for reference without reverting to a large isolated
algorithms block as the default learning sequence.

Prioritize:
1. Correctness and trustworthy measurement.
2. Complete learner-facing execution paths.
3. Explicitly authored learning transitions and assessments.
4. Readable implementations and teaching material.
5. Reproducibility and accurate completion claims.

Retain stable identifiers, working interfaces, and historical evidence where
possible. Introduce migrations only when justified. Do not create a new edition,
tracking framework, or parallel curriculum merely to organize this assignment.

## 2. Authorization, security teaching, and execution boundaries

This is my curriculum repository. I authorize changes to this checkout and
bounded local validation of its original teaching programs and test fixtures,
subject to the workspace's actual permissions. This authorization does not extend
to third-party systems or other data present on the machine.

Cybersecurity is a substantive part of the curriculum. Preserve rigorous teaching
of vulnerability mechanisms, exploitability conditions, adversarial inputs,
security invariants, mitigations, and regression testing. Do not automatically
replace an advanced exercise with a generic secure-coding checklist because it
contains a security-related term.

For executable demonstrations, use original, intentionally vulnerable curriculum
programs, learner implementations, or explicitly designated laboratory targets.
Use synthetic inputs and data, bounded resource consumption, and harmless success
conditions confined to the teaching target. Examples include demonstrating an
out-of-bounds access in a local specimen, exposing an integer-conversion defect,
or violating a modeled authorization invariant with synthetic identities.

Classify artifacts before modifying them:
- Production-quality course infrastructure and reference components need repairs.
- Deliberately defective specimens retain their pedagogically necessary defects.
- Corrected counterparts demonstrate the repair under the relevant contract.
- Incomplete learner scaffolds remain distinct from assessor implementations.

For each security exercise, make the target, adversary assumptions, violated
property, experimental boundary, expected evidence, remediation, and limitations
explicit. Distinguish a crash, sanitizer finding, or undefined behavior from a
proved security impact. Retain technically meaningful bounded demonstrations
where supported, not just warnings that code might be unsafe.

Run vulnerable user-space specimens only after inspecting their effects and
establishing appropriate containment. Use disposable virtual machines or suitable
emulators for kernel/firmware experiments that could affect the host. A directory
or a container label alone does not establish sufficient isolation. Do not execute
unsafe specimens merely because they are present in the repository.

No third-party exploitation, production-system interaction, real credential use,
host persistence, or blanket disabling of host security is authorized. Public
source research and legitimate dependency retrieval are separate from executable
attack targets. Respect the harness's approval requirements for privileged or
external actions; continue independent work when an approval is unavailable.

If a particular subtask is outside what you can support, identify the specific
action, briefly explain the boundary, leave unrelated content intact, and continue
independent permitted work. Record the exact unresolved learning objective and
which parts were completed. Do not silently remove the topic, claim a weaker
exercise teaches an equivalent mechanism, or treat the entire curriculum as
blocked. These scope statements are not a request to override your safeguards.

## 3. Establish the actual state before editing

Read applicable `AGENTS.md` and `CLAUDE.md` instructions, the README, active edition
manifest, coding conventions, route schemas, canonical authoring inputs,
generators, laboratory contracts, assessor boundaries, and validation producers.

Record the starting commit, branch, pre-existing working-tree changes, and relevant
Ubuntu, kernel, compiler, build-tool, and CPU information. Preserve unrelated local
work. Do not push, publish, tag, reset destructively, or commit unless separately
requested or explicitly required by the applicable workspace instructions.

Earlier review findings below are leads to verify against the current checkout,
not proof of present defects. The previous review was not a commit-pinned full
build or independent hardware audit. Reproduce problems against actual sources;
do not substitute a transcription and call it repository validation.

Inspect current equivalents of these entry points:
- `labs/performance_followthrough/pmu_native.c`;
- `labs/performance_followthrough/PMU_PROTOCOL.md`;
- `tools/run_native_pmu.py`;
- `tools/generate_native_route.py`;
- `tools/public_exports.py`;
- `tools/check_native_build_profiles.py`;
- `curriculum/v5/routes/`;
- `labs/hosted_systems/timer_depth/`;
- related source, fixture, assessor, evidence, and completion records.

Create or update a small revision ledger using existing conventions. Link each
item to its source, intended change, completion criterion, tests, and status.
Distinguish confirmed defects, already-fixed findings, pedagogical weaknesses,
and external-evidence requirements. Keep the initial audit bounded and begin
implementation rather than spending the whole run elaborating a plan.

## 4. Verify and repair measurement and interface weaknesses

### A. Checked numeric parsing

Inspect whether CLI arguments are narrowed before range validation or accepted
without a successful lexical conversion. Implement a shared checked parser where
appropriate. Specify the accepted grammar, including signs, whitespace, base,
empty input, and trailing text. Validate conversion status and the range in the
wide conversion type before narrowing to the destination type.

Test accepted boundaries, maximum-plus-one, conversion overflow, empty strings,
trailing characters, and values that would wrap when narrowed. Derive expectations
from the actual type widths; avoid tests that assume every target is LP64. Invalid
precision or workload requests must not silently become weaker valid requests.

### B. Sampling loss accounting

Inspect the sampling lifecycle, ring draining, loss records, and event read format.
Determine whether a final-drain-only experiment can miss losses accumulated after
the last successfully published loss notification.

Where supported, obtain the kernel event loss count through the applicable Linux
interface and decode its format correctly. Verify both build-time definitions and
runtime support. Retain separate fields for in-stream loss notifications, kernel
loss counts, unavailable information, and the completeness of loss accounting.
Avoid double-counting overlapping reports. An unsupported loss counter is unknown,
not a zero. Do not label an observation lossless without sufficient evidence.

Add deterministic tests and a bounded native saturation experiment that fills a
small sampling ring and stops without relying on a later successful publication.
Compare final-drain and concurrent-drain behavior where implemented. Establish
memory-ordering, wraparound, parsing, shutdown, and resource-cleanup correctness.

### C. Calibration and interpretation gates

Inspect whether exhausted calibration still permits a timing comparison to pass.
Separate acquisition validity, event scheduling, timing adequacy, comparability,
and support for a causal interpretation. Preserve useful raw data from an
inadequately timed run, but keep it out of unsupported rankings and corresponding
competence claims. Use explicit inconclusive states instead of false precision.

### D. Regression protection

Confirm the earlier repairs for strict PMU group construction, partial acquisition
cleanup, zero running time, actual synchronization fault injection, sanitizer
coverage, and mismatched exercise cards remain protected. Repair regressions but
avoid duplicating already-correct implementations.

Use current primary Linux and processor documentation for interface semantics.
Keep raw observations separate from derived estimates. Never replace unavailable
hardware events with software events while claiming hardware PMU validation.

## 5. Complete the public learner-native PMU path

Verify the entire learner journey, not merely the existence of an author campaign.
A learner with the exported course and required component artifacts must be able
to build the native collector, inspect capabilities, execute supported experiments,
and produce the specified evidence without private or assessor-only source paths.

Implement or complete a public runner with explicit inputs for learner components.
Identify any supplied adapters as supplied adapters. Preserve separation between
protocol exercises, actual hardware acquisition, and microarchitectural diagnosis.
Correct unsupported-event handling cannot satisfy a hardware measurement gate.

Test from an extracted learner archive with the author checkout and assessor paths
unavailable to the invoked workflow. Check initialization, incomplete-scaffold
rejection, component export/import, native build, capability reporting, and a
supported experiment. Use clearly labeled test implementations to exercise the
infrastructure; they are not evidence of independent learner performance.

Provide reproducible setup instructions and precise capability diagnostics.
Respect available permissions; do not silently change `perf_event_paranoid`,
capabilities, security mitigations, global power policy, or privileged host settings.
Run actual bounded hardware experiments when accessible. When blocked, complete
portable tests and report the exact missing capability rather than claiming the
hardware path passed.

## 6. Replace inferred pedagogy with explicit learning transitions

Inspect whether encounter roles such as introduction, derivation, changed
assumptions, and transfer are assigned from list positions rather than actual
exercise requirements. Replace such inference with explicit canonical bindings.

For each major mechanism and encounter, identify:
- the learning objective and encounter role;
- the exact exercise and learner deliverable;
- the discriminating assessment criterion;
- the introduced or changed assumption;
- the required prior knowledge and produced/consumed component.

A topic mention is not coverage. An implementation task does not automatically
assess its proof, cost argument, or independent selection. Missing bindings remain
visible until substantive work and assessment exist.

Introduce or refine authored learning episodes above individual units. Each episode
has an entering system state, a motivating limitation, a coherent construction or
investigation, and an exiting artifact or demonstrated competence. Interleave
complete episodes deliberately instead of alternating ready units merely to
balance context labels. Explain why the learner changes context at that point.

Review the complete active route for coherence, preserving the subject catalog
and typed prerequisites. Trace producer/consumer contracts and assess transitions,
not only individual cards. Retain representative end-to-end paths through:
- streaming input, rings, parsing, publication, and sampled-record consumption;
- timer scans, heaps, cancellation, alternative deadline representations, and
  hardware-backed comparison;
- representation choice, invariants, abstract cost, locality, and transfer.

Preserve the theory strand: loop invariants, termination, recurrences, amortized
analysis, greedy arguments, dynamic-programming recurrence design, graph
correctness, and relevant probabilistic assumptions. Map each to actual reasoning
and a rubric capable of rejecting a plausible incorrect answer.

Maintain three forms where appropriate: a small inspectable model, a scalable
implementation, and an integrated consumer. Assess what changes between them.
Later assessments should increasingly supply requirements without naming the
structure, requiring selection, rejected alternatives, and revision under changed
assumptions. Do not require artificial use of every structure in every system.

## 7. Polish teaching material and implement readable code

Present one effective active specification per exercise. Resolve superseded cards
and overlays through canonical generation; keep history accessible but separate.
Check that examples, hints, acceptance criteria, and solutions describe the same
problem and do not leak withheld answers.

Make explanations technically self-contained at their declared prerequisite level.
State assumptions, units, boundaries, invariants, and failure behavior. Prefer
worked derivations, counterexamples, and concrete traces over repeated generic
advice. Distinguish ISO C guarantees, extensions, ABI conventions, OS contracts,
and hardware observations. Verify reference editions and source passages instead
of inventing citations or claiming inaccessible sources were inspected.

Follow the repository's established coding standard. Where gaps remain, implement
a concise project-specific readability profile informed by BARR-C and selected
CERT C guidance, without claiming formal compliance with either.

Favor four-space indentation, a roughly 100-column target, mandatory braces,
one statement per line, meaningful module-prefixed public names, visible units,
explicit ownership and error contracts, and straightforward control flow.
Use comments for reasoning, invariants, and constraints rather than restating code.
Allow early returns and structured cleanup when they make resource effects clearer.
Avoid hidden control flow, accidental complexity, and one-use abstractions that
make an algorithm harder to follow. Numerical notation remains appropriate when
its meaning is explicit.

Prioritize the native-performance code: separate checked parsing, workload logic,
measurement lifecycle, result interpretation, and serialization. Make failure
transitions and cleanup reviewable. Keep formatting-only and semantic changes
separable. Configure pinned, compatible formatting and selected C-applicable
analysis checks; thresholds are review triggers, not a substitute for judgment.

Preserve intentional defects and learner scaffolds with narrowly scoped exceptions.
Do not weaken course-wide checks or exclude whole subject areas to accommodate
one specimen. Keep upstream vendored and archival code outside unrelated rewrites.

## 8. Produce complete diagnostic examples, not just counter tables

Complete three representative dossiers using existing course components:
representation/locality, control flow or dispatch, and concurrent-state behavior.
Select actual mechanisms and supported events rather than imposing a fixed
processor-specific catalog.

Each dossier needs competing hypotheses, equivalent correctness contracts,
controlled workload distributions, generated-code inspection, event justification,
placement and measured-thread scope, raw evidence, analysis, alternative
explanations, and explicit uncertainty. Add controlled placement within the allowed
CPU set where feasible, without changing global scheduling policy.

Choose conditions capable of challenging the first explanation. Separate setup
and thread-start costs from steady-state work when that distinction matters.
Avoid universal performance winners, golden cycle counts, and causal claims based
only on correlated counters. An honestly inconclusive result is acceptable.

Bind claims to the exact source, binary, toolchain, machine, event definitions, and
measurement conditions. Keep finite observations distinct from timing guarantees.
Where hardware is unavailable, provide runnable experiments and separate example
or simulated data clearly; never manufacture measurements.

## 9. Validate the actual deliverable

Maintain separate profiles for semantic correctness, instrumented defect
detection, optimized native performance, and target/ABI checks. Verify effective
flags and dependencies. Instrumented timing is not native performance evidence.

Run focused regressions during implementation and applicable integrated checks
against the final regenerated source state. Test intended rejection as well as
success. Distinguish semantic rejection from unrelated crashes, timeouts, build
errors, and infrastructure failures. Use independent oracles where appropriate
and detect vacuous assertions and unexercised failure paths.

Validate route integrity, objective preservation, prerequisite compatibility,
generated learner pages, CLI workflows, packaging, and assessor exclusion. Run
full public learner paths beyond workspace initialization. Keep private-reference
audits, physical measurements, and independent human assessments separate from
self-contained public checks.

Prepare or refine independent-learner assessment packages for the representative
paths: withheld transfer tasks, rubrics, observation forms, and checks for unstated
prerequisites or imitation. Authoring those packages is part of this assignment;
actual independent participants and delayed assessments must not be fabricated.

Record exact commands, environment, exit status, bounds/seeds where applicable,
source identity, results, and supported claims. Preserve historical receipts;
generate new evidence for new work rather than editing old results to match.

## 10. Execution, completion, and reporting

Implement in coherent dependency-ordered batches. Resolve concrete defects early,
complete the learner-native workflow, refine episode and encounter bindings, and
carry the readability and pedagogical pass across the active curriculum. Use
focused diagnostic dossiers and end-to-end paths to validate the architecture,
not as a reason to leave the remaining in-scope polish untouched.

Continue ready work without routine confirmation between batches. Use concise
progress updates that report substantive changes, findings, and remaining work.
Apply proportionate validation: avoid endlessly rerunning unrelated campaigns
without a changed dependency or another concrete reason.

Reuse existing trackers and producers. Do not substitute a large volume of new
metadata, reports, or generated prose for substantive implementation. Every closed
item needs a changed artifact or an evidence-based demonstration that no change
was necessary. Keep blocked items and unsatisfied dimensions visible.

Completion requires corrected verified defects, a usable public learner workflow,
explicit learning-role bindings and coherent episodes, readable in-scope code and
teaching material, and executed relevant validation. Report implementation,
author execution, native hardware observation, and independent assessment as
separate dimensions. Hardware or policy limitations do not excuse unrelated work,
but must not be hidden to produce a clean summary.

If an actual execution or context limit interrupts work, save a precise repository
checkpoint containing the starting/current state, completed and partial items,
commands/results, known defects, remaining dependencies, and next concrete action.
Do not promise background continuation or label the checkpoint complete.

Finish with a concise substantive change summary, exact evidence paths, before/after
examples for important teaching or readability changes, validation outcomes, and
remaining obligations with their reasons. Clearly identify any cybersecurity
subtask left unchanged or incomplete and its effect on the learning objective.

Begin by inspecting the current checkout and verifying the review leads. Then
implement and validate this polishing revision rather than stopping at another
assessment.
