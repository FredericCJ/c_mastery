```json
{
  "schema_version": 2,
  "kind": "assessor-protocol",
  "route": "contextual-v2",
  "route_file": "curriculum/v5/routes/contextual-v2.json",
  "observation_form": "assessor/native_ubuntu_assessment/observation-form.json",
  "dimensions": {
    "mechanism": "Mechanism",
    "correctness": "Correctness",
    "memory_ownership": "Memory/ownership",
    "resources_dependencies": "Resources/dependencies",
    "failure_diagnosis": "Failure diagnosis",
    "communication_transfer": "Communication/transfer"
  },
  "pass_rule": {
    "every_dimension_at_least": 2,
    "dimensions_at_three_at_least": 2,
    "dimension_score": "minimum of the administered criteria mapped to that dimension; never a mean"
  },
  "minimum_delay_days": 7,
  "packets": [
    {"id": "streaming", "file": "streaming.md", "path": "streaming-to-perf"},
    {"id": "deadlines", "file": "deadlines.md", "path": "deadline-structures"},
    {"id": "representation-performance", "file": "representation-performance.md", "path": "representation-to-transfer"}
  ],
  "entry_check_results": ["pass", "fail-unstated-prerequisite", "fail-learner-gap", "fail-infrastructure"],
  "imitation_checks": [
    {"id": "IM-VERBATIM", "detects": "reproduced a public reference, public test or worked example (design text or code) without adapting it"},
    {"id": "IM-ADAPTER", "detects": "substituted an author adapter (assessor/ or labs/support reference code, or a shipped file) for a learner-built component"},
    {"id": "IM-REJECTED", "detects": "cannot explain, with a counterexample, an alternative the learner rejected"},
    {"id": "IM-PREDICT", "detects": "cannot predict the result under the changed condition before running it"}
  ],
  "imitation_outcomes": ["not-detected", "detected", "inconclusive", "not-applied"]
}
```

# Independent learner assessment protocol

These packets are assessor material. Do not include them in learner archives,
generated active cards, public fixtures or hints. Release only the chosen task
section at the assessment sitting. Keep the later changed-condition section
withheld until the learner seals the initial choice and prediction.

They follow the active route, `contextual-v2` (`curriculum/v5/routes/contextual-v2.json`,
generated from `contextual-v2.source.json`; its README and `semantic-review.md`
explain the episodes, encounters and theory bindings). Each task names the exact
v2 node and episode IDs it follows. The packets are instruments. No participant
has used them: they are not evidence of learner performance, delayed retention,
independent review or educational effectiveness, and nothing here may be cited
as such until real attempts are recorded on `observation-form.json`.

| Packet | Route path | Primary task | Gap task(s) the route review lists as unassessed |
| --- | --- | --- | --- |
| `streaming.md` | `streaming-to-perf` | T-STR-1 fragmented acquisition log | T-STR-2 foreign sample region (`pf_ring_drain`, U-T43-09-03) |
| `deadlines.md` | `deadline-structures` | T-DL-1 cancellable service appointments | T-DL-2 lateness summary (summaries), T-DL-3 payload reclamation (allocation, reclamation) |
| `representation-performance.md` | `representation-to-transfer` | T-REP-1 lookup service | T-REP-2 page-resident index (trees/pages), T-REP-3 bounded object store (allocation), T-REP-4 index selection under a budget (dynamic programming), T-REP-5 latency statistics (numerics) |

A path result is reported only for a path whose primary task and at least one
gap task were administered. Other tasks are alternates, each with its own entry
check. Several tasks need accepted work that the path itself does not teach (a
packet's `also_required_nodes`); a learner who lacks it fails the entry check as
a route gap, not as a learner failure.

## Sitting procedure

1. **Entry check** (below). Nothing is released until it is recorded.
2. **Release the initial section** of one task, and only that section. State the
   assistance policy and record it on the form before the clock starts.
3. **Seal the initial choice.** Before running anything, the learner writes the
   requirements analysis, the selected representation or mechanism, at least two
   rejected alternatives each with a reason and the smallest input on which it
   fails, and one predicted discriminating experiment with its expected result.
   The assessor records the wall-clock time (`date -u +%Y-%m-%dT%H:%M:%SZ`) and
   the SHA-256 of that file on the form (`sealed_choice`). Later edits are new
   artifacts; the sealed file is never replaced.
4. **Work.** Ordinary compiler and documentation use only as the declared policy
   permits. Every hint, prompt or looked-up solution goes into `assistance_observed`
   with its timestamp. Record what the learner did, not what the assessor thinks
   they intended.
5. **Release the changed-condition section** only after step 3 is recorded. For a
   delayed result, release it at least seven days after the last supported
   practice of that mechanism (see Delay). Before running, the learner seals a
   prediction of the result under the changed condition (`sealed_prediction`,
   with a second timestamp and hash), then revises the design, then runs.
6. **Imitation and substitution checks** (below), the same day.
7. **Score** each rubric criterion with its evidence, then the six dimensions.

## Entry check: unstated prerequisites

Each packet lists, in its manifest, the accepted learner artifacts the task
assumes, the source identities to capture, the tools, and a set of probes. The
assessor records all of the following on the form before release.

- **Accepted artifacts.** For each component the task assumes (for example
  `containers.ring`, `io.deadline`, `sync.spsc`), the producing node, the
  learner's recorded acceptance ID, and the SHA-256 of the learner's live
  imported bundle. A typed route prerequisite can be conceptual only (U-T43-09-03
  gates on `U-T55-01-03` as a concept), so the check must still ask for the
  learner's own source for any component the task reuses.
- **Source identities.** `contextual-v2.json` SHA-256, repository commit, the
  SHA-256 of every learner source file in the workspace, and compiler, Python
  and sanitizer versions. The assessor never recomputes an identity from the
  author's checkout in place of the learner's.
- **Tools.** The compiler and flags, whether sanitizers and ThreadSanitizer run,
  `perf_event_paranoid` and whether a hardware group opens, and for the PMU tasks
  the learner's own classification of a zero count with positive running time, a
  zero-running group and a denied event. Lack of hardware access does not waive
  the protocol probe; it only makes the hardware observation `not-observed`.
- **Probes.** The oral probes in the packet (each names the node it tests and the
  remediation). They are answered without opening old code.

Result values: `pass`, `fail-unstated-prerequisite` (the task needs a contract or
tool that the accepted path does not provide; revise the route), `fail-learner-gap`
(the path provides it and the learner lacks it) and `fail-infrastructure` (a tool,
runtime or permission problem). A failed check records which probe failed and the
node it maps to, ends that attempt, and produces remediation. The next sitting is
a **new attempt**: increment `attempt.attempt_number`, give it a new
`attempt_id`, list the failed attempts in `previous_attempt_ids` and never relabel
an assisted or remediated attempt as independent. A learner who has seen a task
has been exposed to it, so the new attempt uses an assessor-prepared variant
with different constants (record `task_variant`), and a delayed claim restarts
from the new last supported practice.

## Imitation and substitution checks

Apply all four on every attempt and record each outcome in `imitation_checks`
(`not-detected`, `detected`, `inconclusive`, `not-applied`) with the evidence.
Detection is a flag for oral follow-up. Hash equality is proof of a copy; textual
similarity and an unexplained rejection are not, and are recorded as such.

**IM-VERBATIM: public reference or worked example reproduced.**

1. *Canaries.* Every task changes constants and conventions from the public
   examples it follows (listed per task as `canaries`). An answer whose traces,
   field widths or limits are the public ones instead of the task's is evidence.
2. *Exact copy.* Hash every learner source file and every file the bound nodes
   supply (`supplied_artifacts` in the node, public tests, teaching pages under
   `curriculum/v5/routes/v2/` and the labs named in the task) at the recorded
   commit, and compare.
3. *Near copy.* Save the comparison once as `"$TMPDIR/ratio.py"`, then normalise both
   files with the pinned formatter (`build/author-tools/python/bin/clang-format
   --style=file`, 21.1.8, run from the repository root so `.clang-format` applies)
   and compare comment-free token streams with identifiers abstracted:

```sh
cat > "$TMPDIR/ratio.py" <<'PY'
import re, sys, difflib
def toks(p):
    s = re.sub(r'/\*.*?\*/|//[^\n]*', '', open(p, encoding='utf-8').read(), flags=re.S)
    t = re.findall(r'[A-Za-z_]\w*|0[xX][0-9a-fA-F]+|\d+|\S', s)
    kw = set('if else for while do switch case default break continue return goto sizeof struct union enum typedef static const volatile unsigned signed char short int long void uint8_t uint16_t uint32_t uint64_t size_t int32_t int64_t'.split())
    return [x if x in kw or not re.match(r'[A-Za-z_]', x) else 'ID' for x in t]
print(round(difflib.SequenceMatcher(None, toks(sys.argv[1]), toks(sys.argv[2]), autojunk=False).ratio(), 3))
PY
python3 -I "$TMPDIR/ratio.py" LEARNER.c PUBLIC.c
```

   A ratio of 0.80 or more on a function the learner was asked to design is
   recorded as `detected` pending the probe in step 4; a lower ratio is not
   clearance. Comparing a learner's file with the learner's own earlier accepted
   submission is expected and is not imitation. Run the same comparison against
   the assessor reference sources (see IM-ADAPTER), since a renamed copy of one is
   the likelier substitution.
4. *Adaptation probe.* Ask the learner to change one clause of the code (a width,
   a limit, a tie rule) live and say beforehand which lines change. Copied code
   that cannot be edited without breaking is `detected`.

**IM-ADAPTER: author adapter substituted for a learner-built component.**

`labs/performance_followthrough/native.py` refuses an assessor source that is not
labelled `--test-implementation`; its rule is the one used here. Apply it to every
file of the attempt, including components imported from earlier increments:

```sh
# 1. byte-identical copies; -L follows symlinks, and links are listed separately
find assessor -type f \( -name '*.c' -o -name '*.h' -o -name '*.S' \) \
  -exec sha256sum {} + | awk '{print $1}' | sort -u > "$TMPDIR/assessor.sha"
find -L LEARNER_DIR -type f \( -name '*.c' -o -name '*.h' -o -name '*.S' \) \
  -exec sha256sum {} + | sort > "$TMPDIR/learner.sha"
awk 'NR==FNR { a[$1]; next } ($1 in a)' "$TMPDIR/assessor.sha" "$TMPDIR/learner.sha"
find LEARNER_DIR -type l -exec ls -l {} +

# 2. renamed or reformatted copies: compare every learner source with every assessor
#    source (same ratio.py as IM-VERBATIM step 3) and list pairs at 0.80 or more
for l in $(find -L LEARNER_DIR -type f \( -name '*.c' -o -name '*.h' \)); do
  for a in $(find assessor -type f \( -name '*.c' -o -name '*.h' \)); do
    r=$(python3 -I "$TMPDIR/ratio.py" "$l" "$a"); \
    awk -v r="$r" 'BEGIN { exit !(r >= 0.80) }' && echo "$r $l $a"
  done
done
```

Any output of step 1 is a byte-identical assessor file, or a link into one. Step 2
catches a copy whose identifiers were renamed or whose layout changed, which hash
equality misses; it is slow, so restrict the assessor side to the directories of the
bound labs, and treat a hit as `detected` pending the adaptation probe, not as proof.
Also hash against `labs/support/` and against the unmodified scaffolds of the bound
nodes (a stub that still returns its scaffold status is not a learner component).
Inspect the build record for `TEST IMPLEMENTATION`: a labelled run is a legitimate
author run, never learner evidence. Provenance labels are self-asserted, so an
unlabelled run with a matching hash is `detected`, and a labelled run is `detected`
only if it is offered as the learner's. Compare each imported bundle hash with the
hash recorded at acceptance; a bundle that changed since is an unexplained
substitution until the learner explains it.

**IM-REJECTED: cannot explain a rejected alternative.**

For each alternative on the sealed list, ask for the smallest input or schedule
on which it violates the contract and the exact clause it violates. Then run the
assessor's independent model of that alternative (kept in the packet's traces, not
in learner reach) on that input. `detected` if the learner names no failing input,
names one on which the alternative is correct, or cites the alternative's general
reputation instead of a clause.

**IM-PREDICT: cannot predict the changed-condition result.**

The sealed prediction is compared with the assessor's worked answer in the packet
before the run. Also take one learner-written line, mutate it offline (flip a
comparison, weaken an ordering, change a width) and ask which listed trace fails
first and with what value. `detected` if the sealed prediction contradicts its own
stated model, or is only a restatement of the task, or the mutation prediction is
wrong without a stated assumption that explains it.

Consequences. A `detected` IM-VERBATIM or IM-ADAPTER caps every criterion that
cites the copied artifact at 1 and bars an `independent` result; record the
failure category *Imitation*. A `detected` IM-REJECTED or IM-PREDICT caps at 1 the
criteria that rest on the rejection or prediction (by default every criterion
mapped to mechanism or communication/transfer) and is imitation *evidence*, not
proof of copying. Neither is averaged against other scores.

## Delay

The changed condition is administered at least seven days after the last
supported practice of that mechanism. Record `last_supported_practice_at`, the
instant the changed section was released (`delay.changed_condition_released_at`)
and the list of intervening exposure (any lesson, review, discussion or tool use
that touched the mechanism, with its timestamp). The elapsed time is computed from
the two recorded instants, never typed. An earlier attempt may be useful formative
evidence but cannot count as delayed independent transfer. Do not fabricate
waiting periods, participants or reviews.

## Scoring

Score six dimensions separately from 0 to 3: mechanism, correctness,
memory/ownership, resources/dependencies, failure diagnosis and
communication/transfer. Zero means absent/unsafe; one means a correct fragment
requiring substantial prompts; two means independently adequate under the declared
contract; three means a defended design, independent falsification and transfer.
Every dimension must reach two and at least two must reach three. A safety or
correctness failure is never averaged away.

Each packet carries criteria. A criterion maps to one or more dimensions, states
the evidence that earns 2 and the additional evidence that earns 3, and names at
least one plausible wrong answer or wrong design it rejects. Score a criterion
from what the learner produced, with a pointer to that evidence on the form. A
dimension's score is the minimum over the administered criteria mapped to it, so
a safety defect cannot be offset elsewhere. A criterion not administered is
listed in `limitations` by id, not scored. Not every gap task has a criterion for every
dimension (the validator summary lists the gaps); a dimension's score for a path then
comes from the primary task and the criteria of the gap task, and a path result needs
both. Evidence for 3 requires independent
falsification, for example an oracle the learner built separately from the
implementation or an experiment that could have reversed the design.

Analysis uses the failure categories below. Preserve the first occurrence,
assistance and source revision; do not count multiple downstream failures as
independent misconceptions.

- Unstated prerequisite: the task needs a contract or tool not present in the
  accepted path. Revise the route and rerun on a new attempt.
- Mistaken contract: identify the exact misunderstood lifetime, timing, ordering
  or failure clause and the counterexample that exposed it.
- Imitation: a familiar API was reproduced but could not be justified or adapted.
  Separate this from unaided implementation success.
- Independent diagnosis: a hypothesis predicted a distinguishing observation,
  the learner obtained it and revised the model without a solution hint.
- Infrastructure limitation: tool/runtime/permission failure prevented an
  observation. Do not turn it into semantic rejection or a failing learner score.

## Claim boundaries

Keep ISO C, ABI, operating-system and hardware claims separate in every score
and note. A perf_event denial or a group that never schedules is a recorded state,
not a failing learner score and not a measurement. `perf_event_paranoid` is
recorded and never changed. Report individual paths and dimensions. These prepared
packets and author test results are not evidence of educational effectiveness, an
independent review, participant performance or delayed retention. Such evidence
starts only when real independent attempts are collected.

## Checking the package

`python3 tools/check_assessment_packages.py` verifies that every bound node and
episode exists in `contextual-v2.json` and that each node sits in the construction of
an episode of its own block (initial or changed) on the packet's path; that every
criterion names a rejected wrong answer and every dimension and task is covered; that
each packet's `worked_numbers` expressions evaluate to the stated values and that the
numbers quoted in the prose appear the stated number of times (so an edited figure
is caught); that release text does not name a structure (a keyword heuristic, not a
proof that nothing hints at the answer); that the form parses, carries the required
fields and is an empty template; that no packet path is a learner file; and that no
task identifier or `leak_fingerprints` trace string appears in learner-facing trees.
It checks structure and arithmetic consistency only; it does not assess the quality of
the tasks. `--form FILE` checks a completed form for timestamp order, the seven-day
rule, rubric arithmetic, the pass rule, the route hash, test-fixture hashes and, for an
accepted path result, that a primary and a gap task were administered. Edit a quoted
number and its `worked_numbers` entry together.

Assessor-built fixtures (for example the T-STR-2 writer) are recorded in `test_fixtures`
with their SHA-256 before the sitting and are never learner work.
