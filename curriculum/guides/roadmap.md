# Roadmap and prerequisite map

Domain numbering preserves coverage, not chronology. Begin with the diagnostic, build a shared portable core, then branch into Linux and firmware. Contracts, error handling and tests recur from the first project; their deeper modules consolidate practice.

## Phases and handoffs

| Phase | Main work | Handoff and gate |
|---|---|---|
| Entry | T01 plus targeted mathematical remediation | Predict, repair and explain a small multi-file program |
| Precise C | T02–T06, selected T12, early T18/T21/T32 habits | Object/representation/contract evidence on small components |
| Tools and reusable mechanisms | T07–T09, T12–T16, T18–T22 | Audited portable container/parser/allocator library; P1/P3 |
| Runtime boundary | T10–T11/T13, selected Linux interface foundations | Explain actual entry, linking and helper closure; P2 |
| Linux branch | T23–T25 plus testing/security refinements | Bounded telemetry, ownership/backpressure/shutdown; P5 |
| Firmware branch | T27–T31 plus selected T25 leaves | Reset, drivers, contexts, DMA and timing; P4 |
| Deeper reasoning | T17, T26 awareness, T32–T35 | Numerical, formal, performance and resilience evidence; P6 |
| Algorithms and architecture | T36–T37 after their named C/tool/ownership foundations | Handwritten assembly, invariant-backed algorithms, machine and MCU evidence |
| Communication foundations | T38–T39 after drivers and relevant architecture preludes | Measured serial/electrical channels and audited Ethernet frame ownership |
| Complete stack families | T40–T42 after their distinct prerequisites | Seven-layer telemetry, Ethernet-to-HTTP and socket-based HTTP server defenses |
| Measurement and diagnosis | T43–T45 after their named foundations; use introductory habits from the first component | Multi-axis cost evidence, causal diagnosis, repaired fault experiments and delayed transfer |
| Capstones | Shared core plus completed relevant branches | Independently defended Linux and firmware applications |

T18 ownership and API foundations precede T14 container design. T29 requires selected race/atomic/ordering leaves from T25, rather than finishing all POSIX threading projects. Module front cards enumerate the precise preludes. Complete an explicitly named prelude before its dependent project even if the full domain is scheduled later. T26 contributes core awareness; advanced implementation work is elective.

## A valid full-domain order

The following order is computed from full prerequisites and the owning domains of selected module and project objective preludes. It supports full-domain completion. Early projects may begin after their own named entry/prelude gates; this serial traversal does not require postponing them until all later milestone foundations are complete:

{ref}`T01` → {ref}`T02` → {ref}`T03` → {ref}`T04` → {ref}`T05` → {ref}`T06` → {ref}`T07` → {ref}`T08` → {ref}`T09` → {ref}`T12` → {ref}`T18` → {ref}`T32` → {ref}`T14` → {ref}`T15` → {ref}`T16` → {ref}`T19` → {ref}`T20` → {ref}`T21` → {ref}`T22` → {ref}`T13` → {ref}`T11` → {ref}`T10` → {ref}`T17` → {ref}`T23` → {ref}`T24` → {ref}`T25` → {ref}`T26` → {ref}`T27` → {ref}`T28` → {ref}`T29` → {ref}`T30` → {ref}`T31` → {ref}`T33` → {ref}`T34` → {ref}`T35` → {ref}`T36` → {ref}`T37` → {ref}`T38` → {ref}`T39` → {ref}`T40` → {ref}`T42` → {ref}`T41` → {ref}`T43` → {ref}`T44` → {ref}`T45`

## Domain dependencies and selected preludes

| Domain | Full domain foundations | Selected objective preludes |
|---|---|---|
| {ref}`T01` | Entry | None additional |
| {ref}`T02` | {ref}`T01` | None additional |
| {ref}`T03` | {ref}`T02` | None additional |
| {ref}`T04` | {ref}`T02` | None additional |
| {ref}`T05` | {ref}`T02` | None additional |
| {ref}`T06` | {ref}`T02` | None additional |
| {ref}`T07` | {ref}`T02`, {ref}`T03` | None additional |
| {ref}`T08` | {ref}`T07`, {ref}`T03`, {ref}`T04` | None additional |
| {ref}`T09` | {ref}`T07`, {ref}`T08` | None additional |
| {ref}`T12` | {ref}`T03`, {ref}`T04`, {ref}`T06` | None additional |
| {ref}`T18` | {ref}`T03`, {ref}`T12` | None additional |
| {ref}`T32` | {ref}`T07`, {ref}`T12`, {ref}`T18` | None additional |
| {ref}`T14` | {ref}`T03`, {ref}`T04`, {ref}`T18` | {ref}`T18.01.a`, {ref}`T18.02.a`, {ref}`T18.02.b`, {ref}`T18.03.a`, {ref}`T18.03.b`, {ref}`T18.04.b` |
| {ref}`T15` | {ref}`T14`, {ref}`T04` | {ref}`T14.01.a`, {ref}`T14.02.c` |
| {ref}`T16` | {ref}`T03`, {ref}`T04`, {ref}`T18` | {ref}`T18.01.a`, {ref}`T18.02.a`, {ref}`T18.02.b`, {ref}`T18.03.a`, {ref}`T03.07.a`, {ref}`T03.07.c`, {ref}`T04.03.b` |
| {ref}`T19` | {ref}`T18`, {ref}`T06` | None additional |
| {ref}`T20` | {ref}`T18`, {ref}`T19` | None additional |
| {ref}`T21` | {ref}`T18`, {ref}`T20` | None additional |
| {ref}`T22` | {ref}`T18`, {ref}`T21` | None additional |
| {ref}`T13` | {ref}`T07`, {ref}`T12` | None additional |
| {ref}`T11` | {ref}`T07`, {ref}`T08`, {ref}`T09` | None additional |
| {ref}`T10` | {ref}`T09` | None additional |
| {ref}`T17` | {ref}`T04`, {ref}`T14` | None additional |
| {ref}`T23` | {ref}`T07`, {ref}`T18` | None additional |
| {ref}`T24` | {ref}`T23`, {ref}`T15` | None additional |
| {ref}`T25` | {ref}`T02`, {ref}`T03`, {ref}`T05`, {ref}`T18` | {ref}`T18.02.a`, {ref}`T18.03.a`, {ref}`T02.08.a`, {ref}`T03.01.b`, {ref}`T05.05.a` |
| {ref}`T26` | {ref}`T25`, {ref}`T14` | {ref}`T25.06.a`, {ref}`T25.08.a`, {ref}`T25.09.a` |
| {ref}`T27` | {ref}`T08`, {ref}`T09`, {ref}`T11` | None additional |
| {ref}`T28` | {ref}`T04`, {ref}`T18`, {ref}`T20`, {ref}`T27` | None additional |
| {ref}`T29` | {ref}`T27`, {ref}`T28` | {ref}`T25.06.a`, {ref}`T25.06.b`, {ref}`T25.06.c`, {ref}`T25.07.a`, {ref}`T25.07.b`, {ref}`T25.07.c`, {ref}`T25.08.a`, {ref}`T25.08.b`, {ref}`T25.08.c` |
| {ref}`T30` | {ref}`T20`, {ref}`T21`, {ref}`T29` | None additional |
| {ref}`T31` | {ref}`T27`, {ref}`T28`, {ref}`T29` | None additional |
| {ref}`T33` | {ref}`T03`, {ref}`T04`, {ref}`T14`, {ref}`T18`, {ref}`T32` | None additional |
| {ref}`T34` | {ref}`T08`, {ref}`T17`, {ref}`T32` | {ref}`T25.02.a`, {ref}`T25.06.a`, {ref}`T25.06.b`, {ref}`T25.06.c`, {ref}`T25.07.b` |
| {ref}`T35` | {ref}`T18`, {ref}`T21`, {ref}`T23`, {ref}`T32` | None additional |
| {ref}`T36` | {ref}`T03`, {ref}`T04`, {ref}`T05`, {ref}`T06`, {ref}`T07`, {ref}`T08`, {ref}`T14`, {ref}`T15`, {ref}`T18` | {ref}`T14.08.a`, {ref}`T14.09.a`, {ref}`T15.06.a`, {ref}`T15.07.a`, {ref}`T18.01.a` |
| {ref}`T37` | {ref}`T03`, {ref}`T04`, {ref}`T07`, {ref}`T08` | {ref}`T27.02.a`, {ref}`T27.03.a`, {ref}`T29.01.a`, {ref}`T29.07.a`, {ref}`T36.03.a` |
| {ref}`T38` | {ref}`T04`, {ref}`T18`, {ref}`T27`, {ref}`T28`, {ref}`T29`, {ref}`T31` | None additional |
| {ref}`T39` | {ref}`T04`, {ref}`T18`, {ref}`T27`, {ref}`T28`, {ref}`T29`, {ref}`T31`, {ref}`T38` | None additional |
| {ref}`T40` | {ref}`T18`, {ref}`T27`, {ref}`T28` | {ref}`T15.12.a`, {ref}`T15.12.b`, {ref}`T15.12.c`, {ref}`T15.13.a`, {ref}`T15.13.b`, {ref}`T15.13.c`, {ref}`T38.02.c`, {ref}`T38.09.a`, {ref}`T38.09.b`, {ref}`T38.09.c` |
| {ref}`T42` | {ref}`T15`, {ref}`T18`, {ref}`T23`, {ref}`T24`, {ref}`T32` | None additional |
| {ref}`T41` | {ref}`T18`, {ref}`T27`, {ref}`T28`, {ref}`T29` | {ref}`T42.02.a`, {ref}`T42.02.b`, {ref}`T42.02.c`, {ref}`T42.03.a`, {ref}`T42.03.b`, {ref}`T42.03.c`, {ref}`T42.04.a`, {ref}`T42.04.b`, {ref}`T42.04.c`, {ref}`T42.05.a`, {ref}`T42.05.b`, {ref}`T42.05.c`, {ref}`T39.02.a`, {ref}`T39.02.b`, {ref}`T39.02.c`, {ref}`T39.05.a`, {ref}`T39.05.b`, {ref}`T39.05.c`, {ref}`T39.06.a`, {ref}`T39.06.b`, {ref}`T39.06.c`, {ref}`T39.07.c`, {ref}`T39.09.b`, {ref}`T39.09.c` |
| {ref}`T43` | {ref}`T13`, {ref}`T15`, {ref}`T18`, {ref}`T23`, {ref}`T34` | {ref}`T36.03.a`, {ref}`T36.03.b`, {ref}`T36.03.c`, {ref}`T37.07.a`, {ref}`T37.07.b`, {ref}`T37.10.a`, {ref}`T37.10.b`, {ref}`T37.10.c`, {ref}`T30.04.a`, {ref}`T30.04.b`, {ref}`T30.04.c`, {ref}`T30.05.a`, {ref}`T30.05.b`, {ref}`T30.05.c` |
| {ref}`T44` | {ref}`T03`, {ref}`T07` | {ref}`T21.07.a`, {ref}`T21.08.a`, {ref}`T32.01.b`, {ref}`T32.14.a`, {ref}`T32.12.a`, {ref}`T08.01.a`, {ref}`T08.02.b`, {ref}`T08.04.a`, {ref}`T08.10.a`, {ref}`T16.01.c`, {ref}`T16.11.b`, {ref}`T16.11.c`, {ref}`T32.08.a`, {ref}`T21.04.b`, {ref}`T24.07.a`, {ref}`T24.07.b`, {ref}`T24.08.b`, {ref}`T24.01.b`, {ref}`T25.03.a`, {ref}`T25.03.b`, {ref}`T25.04.a`, {ref}`T25.04.b`, {ref}`T25.12.c`, {ref}`T27.03.a`, {ref}`T27.05.b`, {ref}`T29.01.a`, {ref}`T29.07.a`, {ref}`T31.03.a`, {ref}`T31.05.a`, {ref}`T32.05.a`, {ref}`T32.06.a`, {ref}`T32.09.b`, {ref}`T33.07.a` |
| {ref}`T45` | {ref}`T03`, {ref}`T21`, {ref}`T32` | {ref}`T13.03.a`, {ref}`T16.01.a`, {ref}`T16.11.a`, {ref}`T23.02.a`, {ref}`T23.13.a`, {ref}`T27.04.a`, {ref}`T27.05.a`, {ref}`T29.01.a`, {ref}`T29.03.a`, {ref}`T29.07.a`, {ref}`T29.08.a`, {ref}`T31.03.a`, {ref}`T31.04.a` |

## Parallel progress

After the portable foundations, a Linux learner can advance processes/events while a firmware learner completes reset/driver work. Both retain a common protocol and data-structure core. To complete the full core curriculum, return to the other branch and defend both capstones. A branch-only portfolio is useful but is not full-core completion.

Source reading and proof/measurement techniques recur at progressively deeper gates. Where a selected prelude introduces a mechanism early, mark those exact objectives complete with evidence and reuse them later; do not double-count the work. Use the complete objective map to identify unassessed leaves before final review.
