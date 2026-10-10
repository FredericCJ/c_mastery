# Offline learner workflow

Use one study profile for your progress and separate workspaces for implementations. All commands run locally. Python manages the learning records; execute the C laboratories in Linux or WSL with the tools described by their family.

The v4 records add working units to the existing projects. Existing `init`, `advance`, `check`, `report`, and schema-2 receipts remain usable. Practice and checks remain accessible when an acceptance requirement needs repair.

## Start a study session

From the curriculum directory, replace the example learner paths with your own separate directories:

```sh
python -m cmastery study init ../my-study
python -m cmastery study next ../my-study
python -m cmastery init PR-T01-01 ../prefix-lab --unit U-T01-01-01
python -m cmastery study sheet ../my-study ../session.md --unit U-T01-01-01 --assessment-template ../assessment.json
```

`study next` shows ready units and at most two due recall sets within a 15-minute review allowance. It does not mark work as learned. Read `UNIT.md`, make a prediction, implement or investigate the assigned mechanism, and retain the requested evidence. The generated assessment template starts with unaccepted values.

After studying a unit, explicitly record that event:

```sh
python -m cmastery study next ../my-study --studied U-T01-01-01
python -m cmastery check ../prefix-lab --unit U-T01-01-01
python -m cmastery study status ../my-study
```

Studying activates that unit's recall sets. It establishes neither implementation success nor mastery. A unit without an executable adapter uses its documented trace, analysis, observation, or design checks; the CLI reports an unavailable executable adapter rather than inventing a pass.

Use `advance WORKSPACE PROJECT --unit UNIT` to select another unit in the same family while preserving code. Advancing within the same project is allowed when its unit changes. The previous brief, source fingerprint, prediction record, and latest receipt are archived. A different family needs a separate workspace and explicit component imports.

## Attempt recall before feedback

```sh
python -m cmastery recall due ../my-study
python -m cmastery recall start ../my-study T01.01
python -m cmastery recall submit ../my-study ATTEMPT_ID ../original-answer.txt
python -m cmastery recall grade ../my-study ATTEMPT_ID ../grade.json
```

`start` returns an attempt ID, the selected prompt, and its objective IDs. Write the original answer, including an admission of uncertainty or inability to recall when appropriate. `submit` preserves it and then reveals the feedback checklist and resource references. An original submitted answer cannot be overwritten. Use `--assistance "consulted the old loop"` for disclosed help; multiple assistance entries are allowed.

A grade file uses the exact objective IDs returned for the attempt:

```json
{
  "scores": {"T01.01.a": 2, "T01.01.b": 1, "T01.01.c": 2},
  "feedback": "The loop invariant was correct. The recursive stack bound omitted one live frame; retained the corrected state table.",
  "assistance": []
}
```

Use the existing 0–3 rubric: absent/incorrect, nominal without sufficient explanation, correct under the declared contract, or independent transfer with a defended tradeoff. Score only what the attempt demonstrates. All prompted objectives receive their own score; an unattempted objective receives zero. Correcting a grade appends a superseding evaluation and keeps the earlier record.

The default successful-review gaps are 1, 3, 7, 14, and 30 days. The first review becomes due one day after study. An independent successful review advances the sequence only when its interval has elapsed. Early practice can satisfy an immediate preflight but cannot compress the spaced sequence. After the fifth spaced success, the set leaves the automatic queue; later consumption or cumulative work prompts another preflight as necessary.

An assisted or failed attempt schedules an independent retry for the following day. An immediate repaired answer remains corrective practice; it does not establish delayed retention. Missing a month produces one overdue opportunity per active set, not thirty daily entries. Reviews rotate among the authored variants. Already accepted historical implementation evidence is not revoked solely because recall becomes due.

The schedule is a curriculum policy, not a claim of a universal optimal interval. The profile stores configurable positive day gaps and the freshness window. Sessions remain bounded to at most two sets and 15 minutes; split larger preflights across sessions.

## Seal a consequential design choice

At a designated architecture boundary, the first unit tells you which final milestone's decision to seal. Comparison prototypes may precede the commitment; main implementation and its acceptance checks follow it.

```sh
python -m cmastery decision seal ../my-study ../component-lab FINAL_UNIT ../decision.json
```

The decision file contains:

```json
{
  "requirements": ["At most 64 queued records", "No allocation after startup"],
  "alternatives": ["caller-owned ring", "fixed linked pool"],
  "predictions": ["The ring uses less metadata; the pool makes cancellation cheaper"],
  "spike_evidence": ["evidence/ring-vs-pool.md"],
  "choice": "caller-owned ring",
  "rejected_reason": "Cancellation is rare and measured metadata cost exceeds the budget",
  "reversal_trigger": "If arbitrary cancellation dominates, repeat the comparison"
}
```

The CLI requires two distinct alternatives and local experiment evidence, pins the comparison files, and records the source state at commitment. Acceptance requires subsequent source changes and, where tests apply, a later check receipt. If `decision_before` names the first implementation unit, seal before accepting that unit. These local timestamps support an honest study process; they cannot independently prove when an idea was conceived.

## Accept the specific evidence demonstrated

Fill the generated assessment template using actual evidence. Each published check must be addressed in order. Each required claim names its scope: `host`, `model`, `emulator`, or `physical`.

```sh
python -m cmastery accept ../my-study ../prefix-lab --unit U-T01-01-01 --assessment ../assessment.json
```

The assessment has these fields:

| Field | Required content |
|---|---|
| `explanation` | Mechanism and invariants explained in your own words |
| `transfer` | What changed, your prediction, and the observed result |
| `independence` | `unaided: true` and disclosed assistance; assisted work remains practice |
| `evidence` | Relative paths to retained explanations, traces, or experimental results |
| `source_files` | Relevant source/header paths whose later changes reopen these component claims |
| `checks` | Published check text, in order, with a truthful `met` value |
| `claims` | Claim ID, scope, local evidence paths, and any required target, CPU counts, or interfaces |
| `dimensions` | Six scores, only for a milestone or exported component |

The six dimension keys are `mechanism`, `correctness`, `memory_ownership`, `resources_dependencies`, `failure_diagnosis`, and `communication_transfer`. A full milestone requires all six to score at least 2 and at least two to score 3. Ordinary working units do not require this full dossier.

Acceptance checks actual prerequisite records, required component imports, critical recall, the decision gate when designated, and current learner test results where the unit declares tests. Assessor runs never establish learner acceptance. A physical claim requires current registered physical captures; a target build or a host model does not become a physical observation. Explanations and trace interpretations remain explicit self-assessment rather than automatic certification.

Store evidence in distinct per-unit files instead of repeatedly overwriting one notebook. Each check retains an immutable receipt in `evidence/checks/` and updates the convenience `evidence/latest.json`. Acceptance copies source and evidence bytes into the profile's content-addressed object store. These accepted revisions preserve prerequisites when the next unit adds a function to the same C file. Current claim validity remains separate: changed claim files, listed component sources, or imports reopen the live claim and prevent reuse or export until revalidated. Changed unit contracts or damaged retained objects also invalidate historical prerequisites. Export additionally requires the entire producer workspace to match its accepted source fingerprint.

Required claim constraints cannot be weakened in the assessment. A `target` or `targets` declaration requires evidence for every listed target; `cpus: [1, 2, 4]` requires all three guest runs at each required target. An image build or audit is insufficient for emulator scope: the receipt must include an actual guest execution test. Unit-specific `target_tests` select the appropriate implemented guest stage without requiring later family mechanisms. Keep the ordinary host check and each required target/CPU run; their receipts can be recorded in any order.

## Retain physical interface and board evidence

Some units declare manual `capture_targets`, such as `interface-bench`, separately from executable family targets. These declarations authorize physical capture registration, not a simulated or automatic hardware test. Use `record-physical WORKSPACE MANIFEST` after performing and reviewing the actual measurement. The manifest identifies the target, observed interfaces, exact source hash, image files, raw captures, and measurement conditions:

```json
{
  "target": "interface-bench", "interfaces": ["UART"],
  "source_sha256": "SOURCE_HASH_FROM_CURRENT_REPORT",
  "images": ["uart-bench.elf"], "captures": ["uart-trace.csv"],
  "board_revision": "Recorded board and transceiver revision",
  "silicon_revision": "Recorded device revision", "clocks": "Measured clock configuration",
  "cache_mpu": "Recorded configuration or explicit not applicable",
  "instrument": "Instrument and probe configuration",
  "instrumentation_overhead": "Measured or bounded impact",
  "trace_overruns": "Observed loss or explicit none",
  "observations": "Interpret the raw capture against the required interface contract"
}
```

Image and capture paths are relative to the manifest directory. Only interfaces declared by the unit's manual target can be registered. Captures can be combined to cover the required interfaces. A build target named `stm32f439` cannot satisfy a required `stm32h745` observation, and a model cannot satisfy either physical claim.

Portfolio claims use canonical `source_units` to reuse accepted physical observations from earlier workspaces. They require a new interpretation dossier and verify each required target and interface against those retained observations. The portfolio pins the earlier acceptance identities; reassessing a source unit requires refreshing the live portfolio review. `study status` reports completed projects and core-route completion from accepted revisions, including every designated project completion gate. Missing physical or emulator obligations keep those gates incomplete. The report also lists current live acceptances separately, so historical course completion does not assert that subsequently edited code is still valid.

## Export your component and consume it elsewhere

An artifact is a portable implementation with a declared public contract, not a reference to an assessor solution. Finish and accept its producer unit before export. Keep application entry points outside a reusable library when the artifact contract describes a portable core.

An export declaration selects the actual relative files in your implementation:

```json
{
  "contract_version": "1",
  "cmake_target": "my_checked_bytes",
  "adapter_roles": ["foundations"],
  "profiles": ["C"],
  "targets": ["host"],
  "sources": ["src/checked_bytes.c"],
  "headers": ["include/checked_bytes.h"],
  "include_dirs": ["include"],
  "assets": [],
  "dependencies": []
}
```

Choose profiles and targets permitted by the catalog artifact. Declare every artifact imported by the producer in `dependencies`; the exporter copies their complete pinned closures. List required generated data or build-time inputs in `assets`. The exporter generates a relocatable CMake static-library target for the C/assembly sources and public include directories.

```sh
python -m cmastery artifact export ../my-study ../bytes-lab bytes.checked ../artifacts/bytes-v1 --manifest ../export.json
python -m cmastery artifact link ../my-study ../consumer-lab ../artifacts/bytes-v1 --profile C --target host
```

The bundle carries source, contract, acceptance and content identities, public headers, a CMake entry point, dependency closures, and the producer's scoped claims. Linking copies the pinned bundle into the consumer and generates `.cmastery/components.cmake`. Include it from the consumer's CMake file and link the returned target through your adapter:

```cmake
include(.cmastery/components.cmake)
target_link_libraries(my_service PRIVATE my_checked_bytes)
```

The learner owns that adapter and its integration tests. Exported profiles and targets describe declared compatibility; the retained evidence identifies which execution conditions were actually checked. Continue the required compiler, link-closure, freestanding, and hardware audits.

The supplied integration family also consumes these exports directly through `check --unit LAB-IN-01` (and its later integration units). Export `adapter_roles` defaults to the producer family; explicitly declare a compatible role when exporting a differently organized adapter. Its public headers must expose the integration API, and its target must contain the required implementation. For example, the supplied P1 integration expects the foundational cursor/parser interfaces and algorithm ring interface. Unfinished later functions may remain learner stubs in an early export. When a consumer combines multiple artifacts from one monolithic provider, revalidate and re-export the retained components from one coherent source revision, or split implementations into distinct source files with separate public symbol ownership. The runner rejects differing snapshots of the same source path within an adapter role before linking; it never silently chooses the first archive's stale stub.

For a controlled check, the runner verifies the pinned bundles, regenerates a CMake import file inside the fresh build directory, and passes its path plus validated `CMASTERY_COMPONENTS_FOUNDATIONS_TARGETS`, `CMASTERY_COMPONENTS_ALGORITHMS_TARGETS`, and other declared role targets. It does not execute the editable `.cmastery/components.cmake` convenience file. Artifact imports take precedence over manual source roots for their roles. Missing imports or tests cannot silently fall back to assessor code. `--all-tests` requires every test in the declared family or selected target scope; a P1-only dependency set cannot claim the firmware or VM/store integration scenarios.

The old explicit `--dependency FAMILY=WORKSPACE` route remains supported. Structured family declarations map implementation and include variables to the learner's actual `src` and `include` directories; existing runtime, storage, and kernel layouts retain their adapters. Every dependency fingerprint remains in the receipt.

Before accepting consumption, complete fresh independent recall for the artifact's critical objectives under its current contract:

```sh
python -m cmastery recall start ../my-study TOPIC_ID --contract bytes.checked@1
```

Use the artifact's actual critical-topic IDs, submit, and grade normally. Acceptance deduplicates objective requirements and defaults to a 14-day freshness window. A successful preflight under one artifact contract does not erase a different contract's evidence. Replacing a link or altering a pinned file makes dependent evidence stale; changing an unrelated component does not globally reset progress.

## Records, portability, and recovery

The profile is `PROFILE/.cmastery-study/profile.json`; the append-only ledger is `ledger.jsonl`; retained source and evidence objects live in `objects/` under their content hashes. Back up the whole profile and your workspaces. Corrections append records with `supersedes`; do not edit old ledger lines or retained objects. Hash chaining detects accidental edits, while clock checks reject future records or backwards appends. The profile is local self-reported evidence, not a secure examination system.

A short exclusive `ledger.lock` protects concurrent appends. After a crashed writer, inspect that no learning command is running before removing a leftover lock. The ledger is portable JSON, while its recorded workspace locations refer to the local evidence copies; preserve or deliberately restore those paths when moving a study installation.

Printable sheets use the same unit, set, variant, and objective identifiers. Write your answer on paper first, then enter it through the same start/submit/grade flow. Record the paper date in the answer if helpful; the CLI retains the actual entry time and does not backdate acceptance. Both workflows preserve the separation between practice, observed behavior, independent explanation, transfer, and physical evidence.
