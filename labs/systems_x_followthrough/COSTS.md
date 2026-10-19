# Cost and enforcement review for modern once and reclamation

Compare equal successful output and failure policy first. The journal adapter
is a serialized state machine; CDOnce supplies the actual blocking wrapper.
Hazard/epoch measurements compare the first three retirements with identical
payload capacity and pause schedules, while keeping their later reuse policies
distinct. The selected x86-64 observations below come from `costs.c` compiled by
both GCC and Clang; they are ABI facts, not portable layout assertions.

| Dimension | Concrete mechanism, observation and limit |
| --- | --- |
| Object representation | Selected host SxList144 bytes/alignment4, node16, reader24, MBEpoch32; SxOnce88, MBOnce16 and borrowed journal32. Field offsets are emitted separately. C structure padding is not a wire format. |
| Reachable storage | Three typed payload nodes and two participants with two hazard slots each; no external node allocation. SxOnce borrows one live journal and keeps eight table entries. Host pthread stacks are verification resources, not hidden target storage. |
| Lifetime | Gate/domain and participant control live until all callers join or target leave acknowledgments complete. Generation/section checks cannot make a freed owner pointer valid. |
| Mutation and aliasing | Gate acquire/release excludes ordinary list metadata/payload races; immutable values may be read only with a valid current lease/section reference. Actual MBEpoch fields stay in this exclusion domain. Journal effect/receipt is one declared serialized transition. |
| Allocation | Append scans at most3 nodes; no heap. Register scans2 participants. Exhaustion yields FULL without evicting live ownership. CDOnce's separate three-cell allocator is caller supplied and has its own cleanup contract. |
| Movement and copying | Scalar values and identity records are copied; no public payload pointer escapes. Live atomic controls are not copied by callers. Fresh/quiescent initialization is separate. Native moved resize belongs to its distinct MwOwner provider. |
| Destruction | Leave invalidates references; inactive departure invalidates registration; hazard collection scans3x2x2 protections. No arbitrary destructor callback is hidden in this list. CDOnce retains/drains its registered resources separately. |
| Dispatch | Direct C calls; MAC and copy-between callbacks are explicit borrowed interfaces with separately stated work/lifetime limits. Journal steps make no external I/O call. |
| Concurrency | One acquire atomic_flag test-and-set and release clear per admitted operation; contention returns BUSY without blocking inside the core. Native callers may retry/yield; no fairness/lock-free or bounded waiting guarantee follows. Target atomic lowering includes exclusive access/barriers, audited per image. |
| Failure | Stale registration/reference refuses, failed revalidation clears only the new protection, active departure refuses. Node/registration generations retire before reuse, epochs stop at3. Permanent once failure and retry EMPTY are distinct configured policies; journal dedup is bounded to three identities. |
| Compilation | Six C17 host compiler/optimization/sanitizer profiles and four actual M4 GCC/Clang O0/O2 images. No C23 facility or target runtime archive is imported. Exact command/source/tool/dependency identities are in receipts. |
| Binary and stack | M4 GCC O0/O2 text9924/6376 and bss9880; Clang O0/O2 text11656/8068 and bss9944 in this run. Four explicit512-word task stacks plus reserved handler space; per-function stack reports are retained. These figures are not a whole-call-chain or nested-interrupt stack theorem. |
| Boundary | Original MB tracker contracts, new gate/container protocol, CMX machine task port, host pthread adapter and synthetic journal each retain separate premises. Crossbeam overview does not supply missing Guard-detail evidence; exact published C17 remains a source gap. Emulation does not prove board timing or electrical behavior. |

Hazard mode retained1 of3 removed nodes with one or both readers paused on the
same head; epoch mode retained3. Adjacent protection retained2 in hazard mode.
After legitimate leave both policies reclaimed their retained nodes; permanent
epoch exhaustion and hazard's nine finite incarnations still bound later reuse.
Report retention and progress independently. No time measurement was made for
these costs; lower storage is not automatically better throughput or semantics.

The enforcers are explicit: C lifetime/type rules plus caller-owned live storage;
runtime identity/range/state checks; acquire/release synchronization; exact
compiler/ABI audit for generated helpers; finite native/model/target observations;
and caller discipline for initialization, quiescence and callbacks. Counterexamples
in tests refute dropped revalidation, ignored reader1, active departure and early
publication. None proves all schedules or author-independent mastery.
