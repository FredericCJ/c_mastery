# The active contextual route

The subject catalog answers what is covered. The typed prerequisite graph answers
what must already be assessed or available. `contextual-v1.json` answers what to
study next and why, interleaving baseline units and required expansion exercises.
Its objective and migration maps retain every registered objective and historical
identity. Historical hours remain descriptive; they do not define unit boundaries.

Start with [the compact foundation](foundation.md), then grow the shared systems
through [records](records.md), [streaming](streaming.md), [identity](identity.md),
[deadlines](deadlines.md), [storage](storage.md), [graphs](graphs.md),
[telemetry](telemetry.md), [concurrent publication](publication.md),
[target integration](targets.md), [native measurement](measurement.md) and
[independent integration](integration.md). These are recurring contexts, not
serial subject blocks. Follow actual assessed prerequisites in the CLI.

```sh
python3 -m cmastery study init /tmp/my-study
python3 -m cmastery study next /tmp/my-study
python3 -m cmastery init PR-T01-03 /tmp/my-first-unit --unit U-T01-03-01
```

Read the exported `ACTIVE.md`. It combines the selected unit or exercise, current
objective criteria and explicitly linked public successor contracts. `PROJECT.md`
points to that operative card. Catalog and historical receipt records remain
available as provenance; they do not require manually selecting a current version.

Existing study profiles stay on their historical route until explicit adoption:

```sh
python3 -m cmastery study route /path/to/existing-study
python3 -m cmastery study route /path/to/existing-study --adopt
```

Adoption appends a migration event and preserves prior receipts. It grants no new
acceptance. Component edges require both a passed producer and a verified export;
acceptance also checks the imported contract version in the consumer workspace.
Conceptual mastery, component contracts, environment requirements and optional
preparation have distinct edge types.

The successor route declares five additional component identities without changing
the historical artifact catalog: `foundation.cursor`, `containers.depth.ring`,
`containers.depth.heap`, `containers.depth.csr` and `algorithms.depth.graph`.
They identify the actual `FDCursor`, `CD*` and `ad_*` APIs required by the current
pointer and graph successors. An older `containers.ring` export has a different
contract and cannot stand in for `CDRing`.

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
inputs and inconsistent migration maps. It also verifies profile migration,
readiness and effective export behavior. A structural pass is not an independent
pedagogical review; read [the semantic review](semantic-review.md).

Separate assessor packets exist for three complete paths. Their withheld tasks,
rubrics and observation forms remain outside the learner package. No participant,
delayed attempt, independent review or educational-effectiveness result is implied
by their preparation.
