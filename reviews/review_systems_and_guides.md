# Independent systems and common-guide review

**Status: accepted after fresh revision review. All six findings are resolved and independently verified.**

Reviewed 13 systems domains, 156 original subtopics/468 objectives, 79 project contracts, and the entry, roadmap, standards, reading and workload guides. Exact source-locator and budget checks pass. All 79 systems fixtures pass their independent oracle and canonical-file correspondence check. The original 13 boundary probes and 12 fresh revision probes pass. The machine-readable receipt preserves initial findings, resolutions and final file hashes.

## SYS-01 — medium, resolved

**Reviewed paths:** `curriculum/modules/T13.md`, `curriculum/projects/PR-T13-04.md`.

Freshly read T13 and PR-T13-04. They now assign inspected glibc 2.41 and musl 1.2.5 qsort sources, demonstrate storage/fallback branches and require only an at-most-sixteen-int local insertion-sort replacement. Read actual archived glibc entry code and checked 1024-byte and 32-byte threshold counterexamples. Transitive helpers/comparator effects remain explicit unknowns.

## SYS-02 — medium, resolved

**Reviewed paths:** `fixtures/systems/PR-T26-03.json`, `curriculum/projects/PR-T26-03.md`.

Freshly read T26 and PR-T26-03 plus kernel_lifetime fixture/oracle. The ordinary-work graph models producer/work retention, unchanged barrier state, producer stop, synchronous drain and object release. Independent probes reject early drain, nonsleepable context, worker-lock dependence and stop-without-drain. Real unloading and IRQ/timer/RCU/delayed-work obligations remain excluded.

## SYS-03 — medium, resolved

**Reviewed paths:** `curriculum/modules/T15.md`, `curriculum/projects/PR-T15-01.md`.

Freshly read T15 and PR-T15-01. The stable two-run merge traces labeled equal keys and caller scratch; heap extraction traces a shrinking max-heap and increasing suffix. The project explicitly reuses accepted PR-T14-05 build/sift-down rather than implementing the heap twice.

## SYS-04 — medium, resolved

**Reviewed paths:** `curriculum/modules/T25.md`, `curriculum/projects/PR-T25-01.md`.

Freshly read T25 and PR-T25-01 and exact reading records. The bounded comparison explains C/POSIX handle and start-function types, int versus pointer results, direct POSIX error statuses and optional C17 thread support. One POSIX worker plus availability/signature evidence keeps the project scope bounded.

## SYS-05 — low, resolved

**Reviewed paths:** `tools/verify_systems_fixtures.py`.

Freshly read the updated verifier. It compares canonical module/project/fixture references and identifiers against the recursively discovered owned JSON set. Fresh independent isolated-tree controls reject a missing fixture, nested orphan and incorrect fixture identifier; 79/79 canonical fixtures pass.

## GUIDE-01 — low, resolved

**Reviewed paths:** `curriculum/guides/workload.md`, `tools/write_program.py`.

Freshly read final regenerated workload guide and generator. They explicitly state that the 208-hour integration slice is already included in catalog/core totals. Final canonical arithmetic is 2348 catalog, 2268 core with full T01, 2246 core after passed 3-hour diagnostic, and 80 elective implementation/verification hours.

The final workload arithmetic is 2,348 hours for the full catalog, 2,268 for the conservative core with the full T01 allocation, and 2,246 after passing the three-hour diagnostic. The 80 elective hours are separated; 160 implementation/verification plus 48 reading/assessment/consolidation integration hours are already included. The computed domain order respects every full-domain prerequisite.

Profile/harness boundaries, C17 storage legality, history/draft/live-source distinctions and the T26 core-awareness/elective-implementation split remain sound. Independent study uses predictions, counterexamples and objective-specific evidence rather than counting nominal successful runs.

Limits: this review does not execute learner C, prove every algorithm, validate physical hardware, establish full compiler conformance or certify real kernel/module lifetime behavior. Fixture and source-branch probes support only their declared bounded claims.
