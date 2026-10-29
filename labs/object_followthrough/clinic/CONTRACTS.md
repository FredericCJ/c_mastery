# Memory defect clinic, version 1

This required PR-T03-05 successor supplies implementation and investigation for the unchanged T03.14.a–c criteria and their exact `-DELAYED` variants. Read it with the project's canonical legacy assessment and `curriculum/v5/expansion/legacy_assessments/shared-contract.md`; the latter supplies submission and assessment administration, not missing mechanisms. Families A, B, C, D, E, F, G, I and J apply. Timing and throughput are not a correctness gate here.

## Prerequisites and environment

Re-establish PR-T03-01's allocation/lifetime rules, PR-T03-02's ranges and PR-T03-03's overlap contract. Supply your actual `fd_move` implementation from `labs/foundation_depth/include/depth.h`; its direction and range invariants remain assessed. Native C17/Profile A permits injected allocation primitives and host testing, not a ready-made ownership library. The concurrent changed-condition adapter additionally uses POSIX pthread mutexes, barriers, condition variables and joins on Linux. These synchronization contracts are POSIX contracts, not ISO C library promises. The portable owner does not acquire a target or freestanding acceptance from the hosted campaign.

Use GCC and Clang at O0 and O3, separate AddressSanitizer/UndefinedBehaviorSanitizer and ThreadSanitizer builds, and preserve exact commands, versions, diagnostic output and process status. `defects.c` uses the GCC/Clang `noinline` extension solely to expose the original diagnostic target. The component remains ordinary C17. Inspected N1570 clauses are C11 draft evidence; published final C17/C23 access remains a source gap. A newer compiler mode is not a replacement source inspection.

## Problem and supplied artifacts

Implement `learner.c` and `shared_learner.c` against `clinic.h` and `shared.h`. Run `tests.c`, `history.c` and `shared_tests.c`. The original local `defects.c` has four separately selected faults: an escaped automatic object, repeated release, access after actual release and overlapping `memcpy`. Analyze each before compiling its instrumented child process. Supply defined repairs with equivalent intended values, rather than suppressing the detector.

`struct mc_owner` begins as `{0}`. `mc_acquire` obtains exactly one suitably aligned, writable `int` allocation, initializes it and publishes it only on success. Failure leaves an empty owner unchanged. A live owner cannot acquire again. `mc_read`/`mc_snapshot` copy the current value to a distinct live `int` output; invalid/dead input leaves output unchanged. A snapshot is an independent value, never an escaped address. `mc_release` consumes the unique pointer before invoking the allocator's release callback, increments its diagnostic count once, and rejects a second release. Count instrumentation is limited to fewer than UINT_MAX successful releases per control object. `mc_move` implements the actual existing `fd_move` range contract, including both overlap directions, without delegating to a library copy routine.

Callbacks supply genuine allocated storage or return null. They are nonreentrant, do not mutate control objects, and do not retain a pointer after their declared release lifetime. Controls, outputs and callback contexts are distinct live objects. Raw borrows must finish before unique release; copying the public representation is not a new ownership right. The quarantine diagnostic allocator explicitly retains the C allocation until harness cleanup, although the logical owner has released its authority. Its saved pointer remains within a live allocation; this is a defined counterexample to treating a sanitizer-clean access as an ownership proof.

For `mc_member`, the caller supplies a live `const struct mc_container` and a candidate `const int *`. Derive the actual member address from the whole object's character representation and `offsetof`, compare the candidate, and reject an unrelated same-typed member with output unchanged. Do not subtract a nonzero offset from an arbitrary member pointer and call the result a portable owner. For `mc_initial`, use the actual first member of a live `struct mc_first`; preserve const qualification. The initial-member conversion rule permits this specific conversion, not the fabrication of a containing object around an unrelated `int`. Null inputs reject; invalid nonnull pointers cannot be probed portably.

## Concurrent changed condition

`struct mc_shared` starts zero-initialized and has a stable address until every user has joined. `mc_shared_init` acquires one initialized unsigned payload and one default nonrobust pthread mutex; construction failure releases acquired storage. One successful retain adds one distinct ownership right, up to three, and publishes the destination only on success. Retain requires the caller already own a reference; resurrection and attempting to retain through an unprotected raw pointer are outside the contract. Transfer a reference to the worker before creator release.

Every payload access and reference-count update is protected by the same mutex. Add rejects arithmetic overflow before changing the value. Read leaves its output unchanged on rejection. Drop consumes the caller's handle once; the last drop releases the payload exactly once under the lock. The release callback must not reenter. A reference count supplies lifetime rights; by itself it does not serialize access to the referenced value. Each operation is O(1) in local work, with unbounded mutex waiting under an unspecified scheduler. There is one payload allocation and one fixed control block, no claim of lock freedom, fairness, cancellation safety, robust-owner recovery or bounded response time.

`mc_shared_finish` refuses while references remain. Successful destruction requires external quiescence: all possible callers and borrowers joined, no waiter and no concurrent finish/retain. Zero references alone is not proof that an arbitrary caller has stopped inspecting the control. The exercise assumes valid default nonrobust mutex operations succeed after initialization; unexpected mutex failures are terminal protocol failures, not transactional rollback promises. The harness terminates such a process. Do not add robust mutex semantics without defining recovery of an inconsistent payload.

## Required reasoning and deliverables

D1: Four fault dossiers, each identifying the earliest violated precondition, relevant ISO/library boundary, a plausible incorrect explanation, a repaired ownership/copy operation and a regression that discriminates it. Reproduce the original abstract trace `acquire A; release A; release A; access A` with violations at indices 2 and 3 through defined API refusals.

D2: Implementations plus a state diagram of empty/live/consumed ownership, callback rollback, preserved outputs and the borrowed-pointer lifetime. Explain why allocating a larger buffer, adding a cast, replacing free with a leak or disabling instrumentation is not the specified repair.

D3: The container audit: actual type/member/offset/qualifier/lifetime ledger, a foreign-member rejection, and the offset-zero conversion argument. Explain the difference between deriving a member within a known whole object and claiming a whole object from arbitrary subobject arithmetic. Do not execute the latter as a correctness experiment.

D4: A shared-reference implementation, reference ledger for each thread, happens-before graph and three schedules: simultaneous increments, serial execution, and a paused borrower whose creator releases first. Predict where last release occurs and when mutex destruction becomes legal. Show a separate intentionally racy mutation under ThreadSanitizer; no result value from that program is a semantic oracle.

D5: A falsification matrix for every repair. Include allocator quarantine versus physical free; O0/O3 GCC/Clang diagnostic observations; unrelated live containers; allocation refusal; and serial versus simultaneous schedules. Record a remaining untested assumption for each repair, not a blanket claim of safety. Retain command logs, actual binary identities and independent model output.

## Verification and acceptance

The defined unique tests require copied value 7 after release, exactly one real release for the original trace, refusal with output 88 unchanged, and snapshot-correct overlapping byte moves. The container checks require actual values 17 and 23, offset zero for the first member and refusal of foreign members. `history.c` enumerates all 81 four-event acquire/release/read histories; derive statuses, live state and release counts independently, without copying the implementation.

The simultaneous and serial shared schedules each finish with value 20000 and one payload release. In the paused schedule, creator drop must not free storage; the borrower later reads 7 and its final drop releases it. Reject fourth ownership, construction failure and finish while references exist. Validate the normal programs under separate memory and race instrumentation. Original faulty children expose stack-return, double-free, heap-use-after-free and overlapping-copy diagnostics. Compare direct `memcpy` with the same overlap through a volatile function pointer: inspect emitted code when optimization removes a detectable library boundary. Retain missing reports as failed sensitivity observations, never as proof of defined behavior. The deliberate race must be detected in the overlapping schedule; the serial schedule must explain why the same unsynchronized function can be race-free on that particular execution.

Pass requires all defined contracts, independent history agreement, intended mutant rejection and a source-specific reasoning dossier. A finite campaign does not prove every lifetime or schedule. Performance analysis must count allocations, copies, reference operations and blocking, without comparing an unsafe implementation as though it provided the same contract.

## Rubric and hints

Retain the canonical criterion IDs. Level 0: missing or unsafe repair, invented detector result, or unspecified ownership. Level 1: ordinary test success without explaining fault onset, foreign-member provenance or shared access. Level 2: complete D1–D5 evidence with correct contract distinctions, rollback and admitted limits. Level 3: independently derives and passes the delayed changed condition and provides a discriminating counterexample to an overstated guarantee. Each of T03.14.a, .b and .c is scored separately; passing one does not cover the others.

Hint 1: Separate the allocation's C lifetime from the application's permission to access it. Hint 2: Track reference ownership and payload access in different columns; then mark which lock protects each. Hint 3: Start the container proof from a known whole object, and read the precise initial-member rule before generalizing it. Assessor implementations, model and analysis remain under `assessor/object_followthrough/clinic/` and are not learner exports.

## Delayed transfer

Preserve `AX-PR-T03-05-T03.14.a-DELAYED`, `AX-PR-T03-05-T03.14.b-DELAYED` and `AX-PR-T03-05-T03.14.c-DELAYED` with the canonical minimum seven-day learner interval. The respective tasks change unique ownership to shared references with a concurrent borrower; change recovery to an initial member at offset zero; and change allocator, compiler or lifetime schedule and predict lost discrimination. The supplied controls specify assessment obligations; author execution neither satisfies the learner delay nor establishes independent transfer.

## Security containment and validation limits

Targets are only these original teaching programs and learner implementations, with synthetic integers/bytes in short-lived local child processes. The learner controls the case selector and event schedule; no external service, credential, production device, privileged process or real data is involved. The harmless observation is a named detector report, a refused ownership transition or a local data-race report. The faults establish invalid lifetime/copy behavior; they do not demonstrate arbitrary code execution or privilege escalation. Repair and regression must be assessed alongside residual alias, callback and caller-discipline assumptions.

Current author execution is recorded in `research/v5/expansion/legacy_object_clinic/validation.json`; actual exported-stub checks are separate in `exports.json`. Named source inspections are in that folder's resources and gate. Independent review, delayed learner assessment and physical observation remain unperformed.
