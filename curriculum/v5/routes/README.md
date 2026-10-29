# The active contextual route

The subject catalog answers what is covered. The typed prerequisite graph answers
what must already be assessed or available. `contextual-v2.json` answers what to
study next and why. Its canonical source, `contextual-v2.source.json`, authors
learning **episodes** above individual units and exercises: each has an entering
state (accepted components and concepts), a motivating limitation, an ordered
construction of unit and exercise IDs, an exiting artifact or competence, the
reason the learner changes context afterwards, and the components it produces
and consumes. Episodes are scheduled contiguously in authored order under the
typed prerequisites. Nodes that no episode places remain visible as
**unauthored project blocks** ordered by prerequisite readiness; their cards say
so instead of inventing a motivation. Nothing rotates contexts to balance labels.

The source also binds thirteen **mechanisms** to explicit encounters. Each
encounter has a role (introduction, derivation, implementation, integrated reuse,
changed assumption, independent transfer), a node, one of that node's objectives,
the learner deliverable, the criterion whose objective matches, the assumption
that changes, prerequisite encounters and produced or consumed components. Each
mechanism names its small inspectable model, scalable implementation and
integrated consumer and what changes between them. Where no node and criterion
exist, the encounter is recorded as `missing` with the gap stated. A separate
**theory** strand binds loop invariants, termination, recurrences, amortized
analysis, greedy exchange, dynamic-programming recurrence design, graph
correctness and probabilistic assumptions to criteria whose text names the
plausible wrong answer they reject.

The objective and migration maps (`contextual-v2.coverage.json`,
`contextual-v2.migration.json`) retain every registered objective and node
identity. Historical hours remain descriptive; they do not define unit boundaries.
The predecessor `contextual-v1.json` with `coverage.json`, `migration.json`, its
source and every strand page its cards inline is frozen byte for byte; their
SHA-256 identities are pinned in `predecessor.frozen_sha256` of the v2 source and
verified by the generator and checker. contextual-v2 teaches from separate pages
under [`v2/`](v2/foundation.md), so v1 card identities cannot change.

Coverage is partial and stated as such. 52 episodes place 190 of the 1,983
increments (9.6%); the other 1,793 are unauthored project blocks with generic,
flagged `why_now` text. Three representative paths are authored as episode
sequences: streaming input to sampled-record consumption (`streaming-to-perf`),
deadline scans to measured representations (`deadline-structures`), and
representation choice to transfer (`representation-to-transfer`). They are not
fully assessed: the streaming path's transfer encounter (U-T43-09-03,
`pf_ring_drain`) has no criterion. Across the thirteen mechanisms 75 encounters
are bound and 9 remain `missing` with their gaps stated; 6 of 13 mechanisms have
all six roles bound. Several bound transfers still name the structure they
assess. The `statistics` field of `contextual-v2.json` is the current count.

Start with [the compact foundation](v2/foundation.md), then grow the shared systems
through [records](v2/records.md), [streaming](v2/streaming.md), [identity](v2/identity.md),
[deadlines](v2/deadlines.md), [storage](v2/storage.md), [graphs](v2/graphs.md),
[telemetry](v2/telemetry.md), [concurrent publication](v2/publication.md),
[target integration](v2/targets.md), [native measurement](v2/measurement.md) and
[independent integration](v2/integration.md). The unprefixed pages beside this
README are the frozen contextual-v1 teaching. These are recurring contexts, not
serial subject blocks. Follow actual assessed prerequisites in the CLI.

```sh
python3 -m cmastery study init /tmp/my-study
python3 -m cmastery study next /tmp/my-study
python3 -m cmastery init PR-T01-03 /tmp/my-first-unit --unit U-T01-03-01
```

Read the exported `ACTIVE.md`. It combines the selected unit or exercise, its
episode step and mechanism encounters, current objective criteria and explicitly
linked public successor contracts. `PROJECT.md`
points to that operative card. Catalog and historical receipt records remain
available as provenance; they do not require manually selecting a current version.

Existing study profiles stay on their pinned route until explicit adoption. A
profile pinned to `contextual-v1` keeps using the frozen v1 file for readiness and
acceptance; cards recorded under v1 keep their v1 identity.

```sh
python3 -m cmastery study route /path/to/existing-study
python3 -m cmastery study route /path/to/existing-study --adopt
```

Adoption appends one `route-migrated` event naming `contextual-v2.migration.json`
and preserves prior receipts. It grants no new acceptance; node identifiers and
typed prerequisites are unchanged between v1 and v2. Component edges require both a passed producer and a verified export;
acceptance also checks the imported contract version in the consumer workspace.
Conceptual mastery, component contracts, environment requirements and optional
preparation have distinct edge types.

The successor route declares five additional component identities without changing
the historical artifact catalog: `foundation.cursor`, `containers.depth.ring`,
`containers.depth.heap`, `containers.depth.csr` and `algorithms.depth.graph`.
They identify the actual `struct fd_cursor`, `CD*` and `ad_*` APIs required by the current
pointer and graph successors. An older `containers.ring` export has a different
contract and cannot stand in for `struct cd_ring`.

Each producer card names its export, unchanged public header and dependency IDs.
Implement each exported mechanism in separate source files so that a consumer
can link a ring, heap and CSR without duplicate definitions from unrelated stubs.
Keep the complete public header unchanged; source files may include that header
without implementing its unrelated declarations. A declaration supplied to
`artifact export --manifest FILE` lists your source files, header, include
directories, a unique CMake target, profile `C`, target `host`, and dependencies.
The graph solver export declares `containers.depth.csr`; the other four have
no linked component dependencies. The existing CLI verifies current producer
acceptance, immutable API identity and the complete pinned dependency closure.
It supports both unit and exercise producers, so the foundation cursor can be
exported after acceptance of `X-T03-001`. Public scaffolds remain unsolved.

Authors regenerate public maps with `python3 tools/generate_native_route.py` and
check them with `python3 tools/check_native_route.py`. The checker rejects dangling
references, invalid producer ordering, cycles, lost objectives, unsafe public
inputs and inconsistent migration maps. It also rejects overlapping or misordered
encounter roles, an encounter objective that is not the node's, a criterion that
does not exist or whose objective differs, a changed-assumption encounter without
an assumption change, a transfer whose node does not consume the mechanism,
episode constructions that violate prerequisites, produced or consumed
components that disagree with component contracts, and theory bindings whose
criterion does not name the rejected answer. Deliberately broken sources are
part of its tests. It also verifies the frozen predecessor, profile migration,
readiness and effective export behavior. A structural pass is not an independent
pedagogical review; read [the semantic review](semantic-review.md).

Separate assessor packets exist for three complete paths. Their withheld tasks,
rubrics and observation forms remain outside the learner package. No participant,
delayed attempt, independent review or educational-effectiveness result is implied
by their preparation.
