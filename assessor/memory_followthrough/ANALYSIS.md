# T45 assessor derivations

Author observations do not establish independent or delayed learner performance. Raw negative controls are bounded original specimens.

## AX-PR-T45-01-T45.01.a

Original task: Build a property-to-observer matrix for physical extent, logical field boundary, initialized value, stale lifetime and retained-resource budget. Give one original negative control each and predict which observer can miss it.

Physical range, logical capacity, initialized mode, ended allocation, and reachable24/budget16 each have an original negative control. The matrix pairs ASan, semantic bounds, Memcheck, generation checks, and explicit budgets without treating one observer as universal.

Changed condition: Move the same mechanism into a custom pool whose objects share one allocated region.

The custom pool writes legal backing index6 outside logical4; ASan remains silent while the fixed semantic guard denies before mutation.

Evidence selectors: certificates, gcc-O0-asan-0-extent, gcc-O0-asan-0-pool, gcc-O0-plain-0-init, gcc-O0-asan-0-lifetime, gcc-O0-plain-0-leak.

## AX-PR-T45-01-T45.01.b

Original task: Run one original negative/repaired pair in separate supervised processes with matching inputs and exact build identities. Verify setup and an entry marker before interpreting report, signal, timeout or silence.

Every observer run is a separate supervised original process with SETUP and ENTRY checked, exact compiler/input/flags bound, and negative versus repaired builds. A report without entry cannot count as exercised control.

Changed condition: Remove instrumentation from one dependency and repeat the capability control.

The same ASan caller links dependency.c compiled with and without ASan; actual extent detection versus silence measures coverage.

Evidence selectors: gcc-O0-asan-0-extent, gcc-O0-asan-1-extent, gcc-coverage-plain, gcc-coverage-asan, clang-coverage-plain, clang-coverage-asan.

## AX-PR-T45-01-T45.01.c

Original task: Review proposed golden tests expecting a particular crash address, signal and uninitialized value from three UB examples. Replace them with invariant-based analysis and scoped actual diagnostic observations.

No particular crash address, signal, or uninitialized value is a portable UB oracle. The invariant fails at an out-of-range write, missing field assignment, or ended allocation before the reported symptom. Reports are scoped to the executed original child.

Changed condition: Change optimization and explain why differing symptoms neither repair nor further define the original program.

O0/O2 negative and fixed builds preserve input and independently record actual detector outcomes. Optimization can remove or move a symptom without restoring the source invariant.

Evidence selectors: gcc-O0-asan-0-lifetime, gcc-O2-asan-0-lifetime, clang-O0-plain-0-init, clang-O2-plain-0-init, certificates.

## AX-PR-T45-02-T45.02.a

Original task: Implement the parent's fill, bounded scan and byte-to-string publication with capacity, initialized length, signed index and overlap policy explicit. Exhaust every index/count combination in a small declared domain, including one-past zero-length operations.

The C implementation validates signed indices and span premises before access, exhausts1287 index/count cases including one-past zero-length, scans285 initialized/NUL placements, and publishes strings only after all checks.

Changed condition: Introduce a subview with independent logical length and shared backing capacity.

Subview offset4 has logical capacity0..8 inside backing24; exhaustive failure snapshots retain the independent logical extent.

Evidence selectors: gcc-O0-core, clang-O2-core.

## AX-PR-T45-02-T45.02.b

Original task: Trace lower-bound, exact upper-bound, missing-NUL and embedded-NUL cases before running the checked operations. Identify the first forbidden pointer formation or access, not only the final error return.

Negative index is rejected before conversion, index==capacity with positive count before access, a missing NUL before any absent-byte read, and embedded NUL before string publication. Native lower/missing-NUL controls separately expose actual access faults.

Changed condition: Use a length-delimited binary payload that legitimately contains zero bytes.

The length-delimited move copies bytes including zeros; explicit serialization also preserves payload7,0,9 without treating zero as terminator.

Evidence selectors: gcc-O2-core, gcc-O0-asan-0-lower, gcc-O0-asan-0-nul.

## AX-PR-T45-02-T45.02.c

Original task: Snapshot every destination byte and published length before each rejected operation. Add a legal backing-array write to index6 with logical_limit4 and demonstrate a semantic monitor that catches it while an extent-only check does not.

Rejected fill/string/move/scan paths preserve all bytes and published outputs. The logical index6/capacity4 control proves an addressable write can violate the object contract.

Changed condition: Allow overlapping source/destination under a newly versioned move contract and derive staging direction.

All729 offset/length moves compare against an independent memmove oracle; later destination copies backward and earlier destination forward within one array.

Evidence selectors: clang-O0-core, gcc-O0-asan-0-pool, gcc-O0-asan-1-pool.

## AX-PR-T45-03-T45.03.a

Original task: Derive checked arithmetic for count*stride+header+terminator under a declared limit, testing each intermediate before evaluation. Compare the parent9360/9361/9363 cases with actual SIZE_MAX-derived boundaries.

The helper separates multiplication, header, terminator, zero-size, and alignment stages. It retains9360 success65532,9361 addition failure,9363 multiplication failure, and actual SIZE_MAX boundaries with unchanged outputs.

Changed condition: Add alignment rounding and derive the extra checked addition before masking/division.

Positive power-of-two alignment computes padding then checks it against limit-minus-size; SIZE_MAX-2 rounded to8 is refused before overflow.

Evidence selectors: gcc-O2-core, clang-core-sanitized.

## AX-PR-T45-03-T45.03.b

Original task: Classify negative signed count conversion, unsigned multiplication wrap, signed multiplication overflow and narrowing to a smaller interface width. Provide mathematically distinct counterexamples without executing UB as a result oracle.

Unbounded arithmetic computes -1 to u16=65535,9363*7=65541 wrapping5, signed16 multiplication200*200 exceeding32767, and300 narrowing44. These mechanisms have different premises; signed UB is never used as a mathematical oracle.

Changed condition: Compare a C23 checked-arithmetic interface only after inspecting its exact edition contract, retaining the C17 implementation.

N3096 April2023 draft7.20 and installed GCC/Clang stdckdint.h are actually inspected. The separately compiled C23-header probe compares98 boundaries through temporaries and demonstrates wrapped output on error. Published C23 remains an explicit source gap; C17 remains active.

Evidence selectors: certificates, gcc-ckd-header-read, clang-ckd-header-read, gcc-checked23, clang-checked23.

## AX-PR-T45-03-T45.03.c

Original task: Connect the size helper to a counted allocation seam and inject refusal at every accepted size. Verify zero allocator calls after size rejection, unchanged output/owner and correct cleanup after allocation refusal.

Twelve accepted bounded sizes each encounter deterministic allocation refusal, while eight rejected arithmetic sizes make no allocator call. Owner/output remain unchanged, and native allocation operations are counted.

Changed condition: Make allocation succeed but a subsequent initializer fail, and stage publication until cleanup completes.

The staged record initializer fails at five stages, including after allocation; it drops unpublished storage and retains the complete output snapshot.

Evidence selectors: gcc-O0-core, clang-O2-core.

## AX-PR-T45-04-T45.04.a

Original task: Compute a conditional call-path budget using actual .su qualifiers and explicit recursion, callback and interrupt nesting limits. Include an unknown indirect callee and separate MSP/PSP where applicable.

Actual provider/work/original .su reports retain static and dynamic qualifiers; conditional graph derives foreground144,IRQ152,total296 and separate PSP144/MSP152. Unknown indirect target leaves the whole bound null; recursion depth6 and callback/IRQ premises are explicit.

Changed condition: Permit a logging callback during fault handling and rederive the feasible nesting graph.

A feasible fault logging callback24 changes shared peak296 to320, exhausting margin24; different exception FP frame32 to104 raises peak368.

Evidence selectors: gcc-O0-provider-stack-usage, gcc-O2-provider-stack-usage, gcc-O2-work-stack-usage, certificates.

## AX-PR-T45-04-T45.04.b

Original task: Compare changed-pattern scan, recorded minimum SP and actual frame reservation in a bounded test. Include untouched local space and bytes restored to the fill pattern; retain native exhaustion as a separate child-only observation.

Original assembly paints an actual64-byte reserved frame, changes32, restores8, and compares changed24 with recorded stack displacement64. Child stack limits64KiB/core0/CPU2s are read back before ENTRY and bounded depth128 exhaustion.

Changed condition: Enable a different optimization or FP-context configuration and recollect rather than reuse the number.

Both compilers recollect O0/frame-pointer and O2/omitted-frame-pointer object and stack records; the FP alternative is a separately labeled conditional derivation rather than a physical measurement.

Evidence selectors: gcc-O0-pattern, gcc-O2-pattern, clang-O2-pattern, gcc-O0-bounded, clang-O2-bounded, certificates.

## AX-PR-T45-04-T45.04.c

Original task: Repair a bounded recursion/large-frame case by converting recursion to an explicit caller-owned work stack with capacity and overflow status. Compare semantic outputs, stack use and failure preservation with local-buffer corruption as a different defect.

Caller-owned DFS work storage replaces recursive full binary traversal.896 depth/capacity/strategy cases verify leaves2^depth, capacity denial with unchanged result/peak, and actual static frame records alongside the recursive specimen. Local buffer faults remain separate ASan controls.

Changed condition: Use breadth-first work storage and analyze the changed peak occupancy.

BFS requires2^depth pending slots versus DFS depth+1; depth6 changes peak7 to64 with equal64-leaf semantics.

Evidence selectors: gcc-O2-core, gcc-O2-pattern, gcc-O2-work-stack-usage, gcc-O2-original-stack-usage, gcc-O0-asan-0-extent.

## AX-PR-T45-05-T45.05.a

Original task: Classify four supplied failures by first invariant: fixed section beyond region, zero range touching retained bytes, custom heap crossing its reservation and wrong legal-cell write. Require distinct evidence for each.

Oversized fixed RAM has an actual failed link; zeroing retained12 is refused before write; heap192 crosses its guard reservation; GDB observes legal cells[1] corruption with identical before/after symbol sizes. These are four different first invariants.

Changed condition: Replace the contiguous embedded region with hosted virtual memory and reject the transferred collision model.

Hosted virtual mappings do not imply one descending contiguous heap/stack region. Native stack limits and allocations are separately measured; no MCU collision inference is transferred.

Evidence selectors: image-ram-overflow-link, gcc-O0-core, watch-gdb, watch-map-before, watch-map-after, gcc-O0-bounded.

## AX-PR-T45-05-T45.05.b

Original task: Implement a checked offset-interval layout certificate for data,bss,retained,heap,guard,stack with alignment. Recompute the parent560fixed bytes and heap gaps48/16; add near-limit alignment overflow and zero-size sections.

Checked aligned intervals reproduce fixed560,gaps48/16,and fixed overflow40; near-SIZE_MAX alignment/addition refuses unchanged reports and zero-size sections remain valid.

Changed condition: Give data a Flash load address and RAM run address and account for both capacities.

Actual ELF sections and linker map distinguish .data Flash LMA from RAM VMA; separate RAM and Flash capacity assertions reject both deliberate oversized images.

Evidence selectors: gcc-O2-core, image-sections, image-ram-overflow-link, image-flash-overflow-link.

## AX-PR-T45-05-T45.05.c

Original task: Repair a wrong zero-range ending at13 in the16-byte startup model so retained[12,16) remains unchanged. Test overlapping copy/zero ranges and refusal before the first write; separately retain an actual deliberately oversized experimental link failure.

Startup validates data,bss,retained intervals before first write, refuses bss[4,13) intersecting retained[12,16), then copies4/zeros8 while retaining4. The actual QEMU image separately performs data96/bss400/retained64 twice; oversized link failure is retained.

Changed condition: Introduce a reset-retained checksum field and specify which resets preserve it.

Checksum corruption160 to161 forces invalid retained state; only a documented warm-reset preservation premise permits reuse, while cold/unknown resets require initialization. No physical reset claim follows from QEMU initialization.

Evidence selectors: gcc-O0-core, image-qemu, image-ram-overflow-link, certificates.

## AX-PR-T45-06-T45.06.a

Original task: Instrument every acquisition, transfer, borrow and release for the three-resource component; inject failure before and after each acquisition and before publication. Add cleanup failure while preserving the first cause.

Retained f_construct uses zero-success acquisition callbacks.800 prefix cycles inject before/after each acquisition and before publish, record32/48/16 payload, borrows and reverse releases, and preserve the first cause despite cleanup failure.

Changed condition: Transfer one child resource to another owner before parent failure.

Allocation0 transfers to a child before parent acquisition1 fails; parent no longer releases it, and the child releases exactly once.

Evidence selectors: gcc-O0-resources, clang-O2-resources.

## AX-PR-T45-06-T45.06.b

Original task: Extend the finite ownership graph with an unreachable cycle, a root through an interior token and a reachable cache exceeding its budget. Compare semantic categories with separately observed native leak reports.

Computed strong reachability distinguishes lostA32,indirectB16,rootedC24,interiorD8,and unreachable E/F cycle. Actual Memcheck leak controls report native conservative categories separately; C excess8 remains semantic even if reachable.

Changed condition: Add weak edges and recompute strong reachability.

Adding weak C-to-A must not make A/B strongly reachable; the explicit counterexample records how treating weak as strong incorrectly changes the reachable set.

Evidence selectors: certificates, gcc-O0-plain-0-leak, gcc-O0-plain-1-leak.

## AX-PR-T45-06-T45.06.c

Original task: Repair cleanup and cache-retention policy, then run bounded repeated construct/use/destroy cycles with failure at every stage. Record live, retained and peak bytes plus release counts, not only exit leaks.

Repeated prefix cleanup returns live bytes to0 with acquisition/release counts equal and peaks0/32/80/96. A repaired cache admits16 and refuses24, yielding50 denials,peak16,live0 over100 requests.

Changed condition: Let destruction be deferred until readers leave an epoch and update the retained-byte bound.

Two pinned reader epochs retain32 each; admission refuses a third retired generation until a relevant reader leaves. The finite retained bound64 depends on this admission policy, not eventual reader progress.

Evidence selectors: gcc-O2-resources, clang-O0-resources.

## AX-PR-T45-07-T45.07.a

Original task: Implement the nonzero resize wrapper using a temporary result and deterministic refusal seam. On success initialize new tail bytes and rebuild all published borrows from the returned owner; use integer histories to model same-address reuse.

Temporary nonzero realloc retains owner on refusal; success publishes only its returned pointer, zeroes the grown tail, and advances generation before handles can be reacquired. Integer same-address reuse makes old generation1 stale without evaluating an ended pointer.

Changed condition: Replace resize with allocate-copy-commit-free and compare failure guarantees.

Allocate-copy-free keeps old storage alive through copy and refusal, forces simultaneous distinct live allocations, then publishes generation2 with initialized tail.

Evidence selectors: gcc-O0-core, clang-O2-core, certificates.

## AX-PR-T45-07-T45.07.b

Original task: Diagnose double, interior, foreign and stale-generation releases using the supplied integer model, then one original isolated native negative control where supported. Keep repaired code free of invalid pointer evaluation.

Integer release cases distinguish null,no-longer-live,interior,foreign,and stale-same-address. The original native interior-free control is isolated and ASan observed; repaired code frees only its current owner.

Changed condition: Retire a generation at its maximum instead of wrapping and test capacity exhaustion.

Max-generation owner/async slots retire without wrap; exhausting both retired slots refuses a third submit and preserves the output ticket.

Evidence selectors: certificates, gcc-O0-asan-0-free, gcc-O0-asan-1-free, gcc-O2-core.

## AX-PR-T45-07-T45.07.c

Original task: Test forced refusal, relocation and modeled address reuse across repeated resize/drop operations with two borrowers. Verify exactly-once release and require borrowers to reacquire after success.

Two borrowers remain valid after forced refusal and both expire after success across200 native resize/drop cycles; all live payload returns0 and each allocation is released exactly once. Copying while old storage remains live forces relocation.

Changed condition: Allow a borrower to outlive the resize call and choose pinning, copy or rejection with an explicit contract.

A live pin rejects both resize and drop before allocator calls. This explicit long-lived-borrow policy requires unpinning or a separate owned copy, then reacquisition from the new generation.

Evidence selectors: gcc-O2-core, clang-core-sanitized.

## AX-PR-T45-08-T45.08.a

Original task: Create a field-state table for length,mode,payload pointer and owned-resource tag. Distinguish acquired storage, typed initialization, semantic validity and publication; reject a partially initialized active record.

The staged record distinguishes acquired payload, length/mode assignment, active owned tag, version semantics, and final publication. Five failure prefixes leave a sentinel output unchanged.

Changed condition: Use a tagged union whose inactive member need not be initialized and specify legal reads.

The field-state model admits tag1 length/integer without inactive pointer fields, rejects incomplete tag2, and admits tag2 only with pointer/extent established.

Evidence selectors: gcc-O0-core, certificates.

## AX-PR-T45-08-T45.08.b

Original task: Trace a missing field write through a copied record to a later branch, then compare actual supported Memcheck observation with an address-only control. Record the first missing initialization separately from the report site.

The original missing mode write crosses a record copy and reports at a later branch under Memcheck with origins. ASan on the same input remains an address-only control; report site is not first missing initialization.

Changed condition: Use an uninstrumented library boundary and state what initialization evidence is lost.

A separately compiled dependency without instrumentation/debug information still reports under Memcheck binary instrumentation; missing source attribution and source-instrumented detector coverage are distinguished.

Evidence selectors: gcc-O0-plain-0-init, gcc-O0-asan-0-init, gcc-memcheck-uninstrumented, clang-memcheck-uninstrumented.

## AX-PR-T45-08-T45.08.c

Original task: Repair initialization with typed fields and staged publication, then vary initialized fill patterns and fail each acquisition. Compare records field-wise and serialize only explicit initialized fields.

Typed staging, explicit payload copy, five failure prefixes, two surrounding fill patterns, and field/serialized-byte assertions avoid padding equality. Failed serialization preserves bytes and published length.

Changed condition: Add a versioned optional field and specify absent/default semantics.

Version1 defaults optional to0 and omits it from encoding; version2 accepts a bounded optional value and explicitly encodes it. Binary zero payload remains legal.

Evidence selectors: gcc-O0-core, clang-O2-core.

## AX-PR-T45-09-T45.09.a

Original task: Draw separate intervals for request buffer, callback context, callback borrow and scratch through submit, timeout, cancel request, completion, callback return and reclaim. Add completion before submit returns.

Submit-active, terminal-pending, callback-active and operation-started intervals separately prevent premature reclamation. Six order cases include completion and callback before submit returns; timeout/cancel do not imply ownership return.

Changed condition: Permit the callback to submit a successor sharing the context.

A callback submits a successor in another slot with shared context; reclaiming the first does not end the successor interval. Assessor-only timing composition catches a caller that frees shared context early.

Evidence selectors: gcc-O0-core, hidden-0, hidden-1.

## AX-PR-T45-09-T45.09.b

Original task: Reject duplicate/stale completions and reentrant scratch reuse in the bounded generation protocol. Enumerate terminal-event orderings and test generation exhaustion without wrap; preserve state on denied reclaim.

Duplicate/stale terminal events cannot increment results twice; denied reclaim preserves state. Max generations retire and both-slot exhaustion denies reuse. Terminal order cases cover cancel-before/after terminal and synchronous submission.

Changed condition: Allow nested callbacks with separate scratch leases and derive a finite nesting bound.

Two nested callbacks need distinct scratch leases0/1; same-lease admission is denied and peak nesting is2. A third operation is denied while both slots remain live.

Evidence selectors: clang-O2-core, gcc-core-sanitized.

## AX-PR-T45-09-T45.09.c

Original task: Compare the repaired hosted history with a target-evidence checklist for reachability, publication ordering, interrupts, cache maintenance where applicable and device stop acknowledgment. Use only a dedicated board/emulator route for future experiments.

Hosted FAsync plus callback ownership establishes only software intervals. Reachability, publication, IRQ priority, independent-device quiescence, cache and stop acknowledgment remain explicit target obligations.

Changed condition: Move F439 SRAM buffers to an M7 cached region and derive the changed obligations from exact manuals.

Exact AN4839 Rev2 sections3.2/4 describes write-back clean and received-data invalidate; the computed32-byte cache-line counterexample shows1-to9 TX and9-to7 RX. Exact M7 part/controller/bank route must be requalified; the F439 CCM rule is not transplanted.

Evidence selectors: certificates, gcc-O2-core, image-qemu.

## AX-PR-T45-10-T45.10.a

Original task: For two histories with the same final crash, construct competing extent, initialization, lifetime and capacity explanations and choose a discriminating observation before repair. Minimize the first violating prefix rather than the final symptom.

The event interpreter computes first-invalid indices2,1,3,2 for extent,lifetime,initialization,capacity despite a shared final crash token; minimum first-bad prefixes and distinguishing observations are recorded.

Changed condition: Use the same mechanism with a different delayed symptom.

Moving the extent symptom to event7 leaves first-invalid event2; minimizing only the final crash would hide the repair boundary.

Evidence selectors: certificates.

## AX-PR-T45-10-T45.10.b

Original task: Select a valid-object watch, allocation-generation history, linker map or bounded fault record according to the suspected defect. Include an unavailable region and an invalid fault-address flag; refuse invented values.

GDB watches a valid legal cell and records actual hardware-watch state; map size stays fixed. Integer generations qualify ownership, and captured BFAR without its valid bit plus omitted heap remain unknown.

Changed condition: Move the object during the replay and rebind the observation safely.

The replay removes all old watches before free, then binds the newly live allocation after generation advances to2; actual watch observes82 without evaluating the old pointer.

Evidence selectors: watch-gdb, watch-map-before, watch-map-after, certificates.

## AX-PR-T45-10-T45.10.c

Original task: Freeze an invariant repair and compare it with canary/guard mitigation on the same original target. After seven days, solve an unseen capacity, signed-width, failure-prefix or completion-order variant before revealing the oracle.

Range/generation/initialization prevention is compared with actual detector mitigation on the same original hosted specimens. Author tests establish these local repairs; independent unseen and seven-day performance remain unobserved.

Changed condition: Compose the repaired component with an asynchronous caller that changes ownership timing.

The separate hidden composition changes caller timing: a completed callback starts a successor sharing context. Bad caller reclaims early; fixed ownership holds through the successor. This is an author oracle, not an independent delayed attempt.

Evidence selectors: gcc-O0-asan-0-canary, gcc-O0-asan-1-canary, hidden-0, hidden-1.

