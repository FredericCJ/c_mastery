# T44 assessor derivations

Author observations do not establish independent or delayed learner performance. Raw negative controls are bounded original specimens.

## AX-PR-T44-01-T44.01.a

Original task: Build a four-column dossier for the parent's assignment-in-condition defect: expected admission relation, actual observations, inferred cause and untested alternative. Include a misleading successful input1 beside rejected-boundary inputs0and5.

The four-column certificate keeps expected acceptance F,T,T,F separate from the faulty all-accepted observation. Input1 is consistent with both implementations and cannot establish correctness. The actual GCC/Clang assignment runs bind this table to emitted binaries.

Changed condition: Use a stale configuration with correct source to produce the same visible admission error.

The correct implementation with DX_CAP=8 accepts5, reproducing the admission error without an assignment defect. The environment is part of the replay contract, so source correctness alone does not identify the running configuration.

Evidence selectors: gcc-debug-specimen-0-1-assignment, gcc-stale-config, certificates.

## AX-PR-T44-01-T44.01.b

Original task: Create a replay manifest containing source, executable, symbols, input, configuration, working-directory assumptions and a finite timeout. Deliberately pair new source with an old executable and show the mismatch before diagnosis.

Replay hashes source, executable, separate debug file, input, configuration and cwd, and records mode, seed and finite timeout. It rejects changed executable or symbol identity before interpreting the selected failure. Every parser candidate starts in a new child and runs twice.

Changed condition: Make an environment variable alter a parsing mode while all file hashes remain unchanged.

Changing DX_MODE changes the admitted capacity with unchanged files. The manifest mode field therefore rejects the replay; seed is separately bound and cannot share a reduction cache.

Evidence selectors: replay.

## AX-PR-T44-01-T44.01.c

Original task: Before editing, distinguish removed validation from input mutation in the parent's defined defect by observing the parameter immediately before and after the guard. Record predictions for both hypotheses and the smallest discriminating case.

Removed validation preserves each parameter; assignment changes nonzero parameters to0. Observe before/after and capacity separately. Input1 is the smallest distinguishing parameter probe; input0 has the same value under both hypotheses.

Changed condition: Diagnose an unchanged parameter with a corrupted capacity instead.

With corrupted capacity8, parameter5 stays5 but is admitted. The third probe column distinguishes this from mutation. The correct-source stale-configuration executable provides the changed-condition observation.

Evidence selectors: certificates, gcc-stale-config.

## AX-PR-T44-01-T44.02.a

Original task: Compile the intentional assignment defect with the selected warning set under both GCC and Clang, then parenthesize the assignment without changing its behavior. Preserve warning and runtime evidence separately.

Both compilers warn about assignment in a condition. Parenthesizing the assignment removes that diagnostic while preserving the all-accepted defect. Debug and release results remain distinct records with exact flags and binary hashes.

Changed condition: Use an optimizer-eliminated local and explain a missing debugger value without blaming corruption.

Og/O2/LTO GDB transcripts record vanished as optimized out and preserve source/machine mapping. Missing state is unknown, never a zero substituted into the causal account.

Evidence selectors: gcc-parenthesized-build, clang-parenthesized-build, scope-O2.

## AX-PR-T44-01-T44.02.b

Original task: Extend the three-record diagnostic ring with a separate immutable first-cause field and a logging-failure seam. Inject input failure followed by cleanup and logging failures; preserve the original cause while the ring overwrites old events.

The no-allocation DxLog holds immutable first1 and last2,3,4 with dropped1. Recording cleanup and log failure cannot overwrite the first input failure. Saturating dropped count prevents diagnostic wrap from inventing lost history.

Changed condition: Allow two nested operations and define whether first cause is per-operation or global.

Nested operation records keep independent first causes1 and9. A process-global first cause would conflate operation identity and requires an explicitly different policy; the original per-operation implementation does not make that claim.

Evidence selectors: gcc-debug-core, clang-release-core.

## AX-PR-T44-01-T44.02.c

Original task: Create debug/release and logging-on/off builds with a diagnostic-only counter expression. Verify external validation and cleanup still occur with NDEBUG, while disabled diagnostic expressions do not execute.

All GCC/Clang debug/release and logging-on/off combinations execute validation and cleanup once. Only the diagnostic macro counter changes with logging. Required transitions occur outside optional instrumentation.

Changed condition: Replace the macro with an inline function and explain eager argument evaluation.

The inline logging function evaluates its counter argument even when its body discards it: final counter is macro counter+1 in every profile. This is an executed language-semantics distinction.

Evidence selectors: gcc-release-specimen-1-0-macros, clang-debug-specimen-0-1-macros.

## AX-PR-T44-02-T44.03.a

Original task: Define a same-failure predicate for an original bounded parser error, distinguishing timeout, malformed grammar, unrelated crash and the selected wrong transition. Replay each classification twice with reset state.

The bounded original parser recognizes bracketed A/N/T/C chunks. The same admission transition is FAIL; normal completion is PASS; malformed grammar, bounded timeout and unrelated abort are UNRESOLVED. Each classification is replayed twice in fresh children.

Changed condition: Introduce a schedule seed and make replay identity include it.

DX_SEED is frozen in both manifest and selected transition output. Cache scope is one reduction invocation, so a different seed cannot inherit a prior classification.

Evidence selectors: replay.

## AX-PR-T44-02-T44.03.b

Original task: Implement subset/complement reduction for at most16 ordered chunks with a finite trial budget and cached classifications. Preserve grammar delimiters through an explicit chunk policy; compare trial order with the visible fixture without hardcoding its answer.

dx_reduce implements subsets then complements over an ordered <=16-chunk mask, adaptive granularity, a512-entry cache and finite trial budget. The callback knows the failure; the reducer does not. The real parser callback reduces NNANNNNA to A with fixed outer brackets.

Changed condition: Use a grammar-aware tree reducer and explain which reductions it can now express.

The executed balanced-tree reducer removes whole child subtrees and uses a validated leaf frontend to the same C failure oracle. It reaches [[A]], then independently checks every one-deletion neighbor. It cannot splice parent delimiters to produce the flat [A] result. Character deletion can also create unresolved unbalanced text. The certificate is local to that rewrite set, not a global tree minimum.

Evidence selectors: replay, gcc-debug-core, certificates.

## AX-PR-T44-02-T44.03.c

Original task: Independently enumerate every one-chunk deletion of candidate[1,7,8], then enumerate the eight-chunk finite universe to find the smaller trigger[2,3]. Retain all tri-state outcomes.

All256 subsets are independently classified:24 PASS,88 FAIL,144 UNRESOLVED. The [1,7,8] candidate has only UNRESOLVED single-deletion neighbors; it is1-minimal. The complete enumeration also finds cardinality2 failure [2,3], so it is not globally smallest.

Changed condition: Change allowed reduction from one deletion to two deletions and recertify minimality.

Admitting two deletions adds exactly three neighbors of this three-member witness; each is checked and does not FAIL. The stronger local certificate still does not establish global minimality.

Evidence selectors: gcc-debug-core, clang-release-core.

## AX-PR-T44-03-T44.04.a

Original task: Use source and instruction stepping on the initialized admission counter to stop before the fifth acquire at capacity4. Compare the state before guard, after increment and after a later release.

GDB stops before the fifth acquisition with used=capacity=4, displays the actual load/add/store instructions, single-steps and observes the hardware watch at used5. The original raw trajectory is1,2,3,4,5,4; the repaired trajectory is1,2,3,4,4,3. The raw release hides the prior5 if only final4 is inspected. A separate increment/clamp callback case tests symptom suppression.

Changed condition: Add a callback between increment and clamping to expose the transient invalid state.

The actual callback between increment and clamp records observed5 while final state is restored. The fixed path never calls that callback with an invalid committed state.

Evidence selectors: debugger-counter, gcc-raw-history, gcc-fixed-history, gcc-debug-specimen-0-1-counter, gcc-debug-specimen-1-1-counter.

## AX-PR-T44-03-T44.04.b

Original task: Write caller and callee scopes each named status with distinct initialized values; select frames explicitly and inspect arguments versus locals. Include an inner shadowing block and record when its lifetime ends.

Selected-frame observations distinguish callee status7, caller status2 and the inner shadow11. Each value is attached to its live scope and exact stop.

Changed condition: Inline the callee and compare logical debugger frames with physical call frames.

The optimized inline helper appears as a logical frame sharing a physical call site with main and has status13. Its source-level frame is useful evidence but not an extra physical stack activation.

Evidence selectors: scope-Og, scope-O2.

## AX-PR-T44-03-T44.04.c

Original task: Capture matched backtraces for debug and optimized builds, including an intentionally unavailable local. Pair transcripts with executable/symbol hashes and describe exactly what state is missing.

Og/O2/LTO backtraces record unavailable locals and binary identity. A backtrace is a present call-stack observation, not a complete history. The saved report keeps heap absent rather than assigning zeros.

Changed condition: Use a tail call and explain why a source-level call may lack a retained frame.

Actual O2/LTO disassembly shows a jump from dx_tail to dx_tail_leaf. GDB may synthesize a logical tail-call frame from DWARF; that does not restore a physical caller frame. Compare raw instructions with frame display.

Evidence selectors: scope-Og, scope-O2, scope-LTO.

## AX-PR-T44-03-T44.05.a

Original task: Choose a pure conditional breakpoint for used==capacity and a watch on the initialized live used object. Compare with a condition that calls a state-changing helper; reject that condition from the diagnostic protocol.

The condition used==capacity is pure. The actual live global watch is installed and its insertion/hit state is recorded. Calling dx_acquire in a condition is rejected by the diagnostic protocol because it changes the state being investigated.

Changed condition: Replace the watched scalar with one field of a moving allocation and re-establish the watch after relocation.

The relocation transcript records two different live addresses, deletes the old watch before free, and creates a new watch at the new live allocation. The subsequent write5 is observed without reading a stale alias.

Evidence selectors: debugger-counter, debugger-relocate.

## AX-PR-T44-03-T44.05.b

Original task: Relate the first actual watchpoint stop to the admission invariant and the source/instruction trace. Include a logical-state violation that changes no watched byte until a later publication.

At the private-candidate marker, candidate5 already violates the capacity4 invariant while published used remains4. The watch on used stops only after the later store5. The first observed store is not the first logical invalid state.

Changed condition: Use a buffered event log whose flush occurs after the invalid transition.

The finite event ledger puts buffered log flush after candidate, publication and clamp. A late log is evidence of an earlier event only with its captured value/sequence; flush time cannot be substituted for transition time.

Evidence selectors: debugger-candidate, certificates.

## AX-PR-T44-03-T44.05.c

Original task: Request the parent's impossible watch width/slot combination in a model and attempt only valid bounded watches in the actual supported debugger. Record insertion/resume failure and choose a narrow watch or conditional breakpoint fallback.

The fixture width8>4 and requested3>available2 reject the modeled hardware plan. These modeled limits are not asserted as actual host limits. Actual GDB records hardware type and successful resume/hit for bounded live watches; errors remain explicit in command records.

Changed condition: Move the writer to another thread and assess whether the selected observation still applies.

The actual producer-thread write to predicate is observed by an unrestricted hardware watch with thread identity and all-thread stacks. A software or thread-restricted watch would not establish equivalent coverage.

Evidence selectors: certificates, debugger-thread-watch.

## AX-PR-T44-04-T44.06.a

Original task: Map a real stopped PC to a source range in matched Og and O2 images, then inspect raw instructions around a moved/eliminated statement. Distinguish the synthetic line-map certificate from real debug metadata.

Og/O2 mapping receipts contain real disassembly and source annotations, with compiler flags and exact executable hashes. Statements are not treated as one-to-one instructions; the optimized value can be absent. Synthetic PC ranges remain fixture models.

Changed condition: Enable LTO across a helper boundary and repeat the mapping.

LTO repeats source/PC/locals observations on its own emitted binary. The helper body may be specialized and boundaries moved; the LTO transcript and instructions, rather than a prior map, define its observations.

Evidence selectors: mapping-Og, mapping-O2, scope-LTO.

## AX-PR-T44-04-T44.06.b

Original task: Adapt the retained ABI probe to detect one deliberately unpreserved callee-saved register while stack restoration remains correct. Keep the faulty callee inside an original local test binary and test the corrected pair.

The retained x86-64 ABI probe seeds RBX/RBP/R12-R15, checks restored stack and nested-call alignment and exposes the algorithm return separately. The deliberate clobber keeps lower_bound result1 but changes the preserved-register verdict; fixed code passes.

Changed condition: Change ABI or target and rebuild the list of obligations from its authority.

The separate Cortex-M4 Thumb/AAPCS experiment uses R4-R11 and eight-byte public-interface stack alignment. QEMU runs the deliberate R4 clobber and the fixed pair, both returning1. This selected R9 convention is explicit; it is not inherited from the x86 ABI.

Evidence selectors: gcc-abi-0, gcc-abi-1, thumb-0, thumb-1.

## AX-PR-T44-04-T44.06.c

Original task: Pair a bounded saved-state record or permitted core with its exact executable/symbols. Ask for a deliberately absent heap region and distinguish unavailable state from a null/zero value.

The GDB JSON is a bounded saved observation with exact binary/symbol hashes and present PC/stack variables. It is explicitly not a native core. Omitted heap is unknown. Replay rejects mismatching image and symbols before interpreting the state.

Changed condition: Use a core from a different build with identical source line numbers.

The same source line labels do not identify a build. The manifest rejection is based on actual image/symbol digests, and the Og/O2/LTO emitted artifacts are separately hashed.

Evidence selectors: replay, scope-Og, scope-O2.

## AX-PR-T44-05-T44.07.a

Original task: Diagnose separate original bounds, initialization, post-free and post-successful-resize cases using integer generation histories before any native negative run. Identify the first invalid transition and the required caller precondition.

Integer identity histories separately reject extent overflow, uninitialized read, post-free read and use of a pre-resize generation. Failed resize leaves generation and initialized prefix unchanged; successful resize ends the prior generation before any native stale-pointer evaluation.

Changed condition: Change a physical out-of-bounds defect into a legal-address wrong-field write.

A write to the other field stays within an allocated array and can be language-valid, yet violates the field contract. The semantic check detects it; extent tools alone need not.

Evidence selectors: gcc-debug-core, certificates.

## AX-PR-T44-05-T44.07.b

Original task: Run isolated original negative/repaired controls in separate ASan/UBSan and unsanitized Memcheck builds where supported. Keep exact flags and reports, and include an initialization case ASan alone does not establish.

Separate negative/repaired ASan, UBSan and unsanitized Memcheck processes bind extent, lifetime, signed overflow and initialization reports to their build flags. The ASan initialization case can terminate silently; it does not discharge definedness.

Changed condition: Move the invalid value through copies before consuming it in a branch.

The additional copied-value specimen propagates the uninitialized integer through a separate function before the conditional use. Memcheck with origins reports the eventual branch; repaired initialization removes it.

Evidence selectors: gcc-asan-diagnostic-0-0, clang-ubsan-diagnostic-0-3, gcc-plain-diagnostic-0-2, clang-copied-init-0.

## AX-PR-T44-05-T44.07.c

Original task: Repair the first lifetime/extent transition, then inject failed allocation, successful relocation and an adjacent exact-capacity case. Compare full ownership state, not just disappearance of the crash.

Failure injection at all three acquisition prefixes preserves publication and frees exactly the initialized resources. Length3/4/5 runs force failed resize and distinct live replacement allocation, preserve the prefix, and avoid all stale-pointer evaluation. Debug, release and instrumented controls agree.

Changed condition: Introduce a borrowed callback view surviving beyond the call.

A callback view contains an owner generation. The assessor-only hidden pair rejects an expired generation and accepts the new owner; the defective pair accepts the wrong owner82. The safe model exposes ownership escape without native post-free reads.

Evidence selectors: gcc-debug-allocation, clang-allocation-sanitized, hidden-0, hidden-1.

## AX-PR-T44-06-T44.08.a

Original task: Repair the off=n cursor defect using the parent's seven-byte script, then split every positive result across a different boundary and insert bounded EINTR/EAGAIN events. Verify the exact emitted byte sequence.

The required event sequence advances offsets2,2,4,4,7. All64 positive partitions of seven bytes are interleaved with bounded EINTR/EAGAIN retries; the complete reconstructed bytes must match ABCDEFG. Over-report and zero progress preserve accepted offset and fail.

Changed condition: Transfer to scatter/gather buffers with zero-length segments.

Scatter/gather traversal explicitly skips zero-length segments without advancing bytes or dereferencing them, preserving ABCDEFG across2,0,3,0,2 segment lengths. Read0 is an EOF boundary; writer0 is no progress under this adapter.

Evidence selectors: gcc-debug-core, clang-release-core.

## AX-PR-T44-06-T44.08.b

Original task: Save errno immediately after a failing I/O call before a logging seam overwrites it; compare default and explicitly handled SIGPIPE in separate bounded child processes. Use an injected cap8 descriptor ledger for exhaustion.

The injected errno seam saves ENOSPC before logging sets EIO. Owned children separately observe default SIGPIPE termination and ignored SIGPIPE returning EPIPE. The eight-slot ledger with base2 admits six acquisitions and classifies the seventh as EMFILE, not ENFILE.

Changed condition: Use a pthread API returning a direct error number and adapt the wrapper.

pthread_mutexattr_settype returns EINVAL directly while the intentionally set errno remains ENOSPC. The caller checks the returned status rather than consulting stale errno.

Evidence selectors: gcc-host, clang-host.

## AX-PR-T44-06-T44.08.c

Original task: Model Linux close releasing generation1 despite an error, followed by reuse as generation2. Reject an automatic retry that would close the new resource; repair the wrapper's ownership state and preserve error reporting.

The Linux model releases fd generation1 despite EIO, then rejects an old ticket after generation2 reuse. No automatic native close retry is issued. The selected POSIX.1-2024 EINTR contract is documented separately.

Changed condition: Design an adapter with explicit close-policy selection and test both modeled contracts.

The explicit policy selector preserves the live generation for the POSIX EINTR case and releases it for Linux EINTR. Both contracts execute as models; neither is inferred from a generic retry rule.

Evidence selectors: gcc-debug-core, clang-release-core.

## AX-PR-T44-07-T44.09.a

Original task: Build wait-for and happens-before graphs for forced two-thread schedules; include a lock-order cycle and an unordered non-atomic conflict. State edge provenance rather than deriving ordering from timestamps.

The finite certificate distinguishes wait-for edges from happens-before edges and unordered non-atomic conflicts. The forced opposite-lock ownership experiment captures all-thread stacks, then terminates through bounded timed waits. A possible order cycle alone is not a stopped-deadlock proof.

Changed condition: Add a timeout edge that can break waiting and reconsider the deadlock classification.

A timeout removes a wait edge. The actual bounded experiment therefore records a transient cycle and completion, not a permanent deadlock; ownership and timed-wait return values are retained.

Evidence selectors: certificates, gcc-cycle, debugger-cycle.

## AX-PR-T44-07-T44.09.b

Original task: Force check-false, producer-set/signal, consumer-wait in the retained predicate program using bounded schedule gates. Capture whole-thread state and compare with the repaired same-mutex recheck loop.

The actual data-race-free schedule forces false predicate check, producer update/signal, then consumer timed wait. It times out with predicate1. The retained queue repairs the protocol by waiting/rechecking under the same mutex; whole-thread debugger stacks locate the blocked consumer.

Changed condition: Add a spurious wakeup and a second consumer.

Empty broadcasts, competing two-consumer consumption and three blocked consumers exercise rechecking. The repaired queue preserves exact once-only payload ownership across these schedules.

Evidence selectors: gcc-lost-wake, debugger-lost, gcc-queue, clang-queue.

## AX-PR-T44-07-T44.09.c

Original task: Test the synchronization repair under producer-first, consumer-first and cancellation-near-wakeup schedules, plus supported detector runs. Keep object destruction after all waiters are quiescent.

Retained producer-first, consumer-first, close/timeout and cancellation cases are rerun. The new near-wake hook permits cancellation delivery or completion before the next cancellation point, and checks the corresponding payload owner. All workers join, active count reaches0 and only then can retirement destroy synchronization objects.

Changed condition: Introduce a third waiter and bounded queue ownership.

Three actual waiters wake spuriously and consume three distinct queued values with a complete ownership mask14. Queue capacity3 rejects a fourth publication. Separate race-detector controls demonstrate what a data-race claim can and cannot establish about protocol progress.

Evidence selectors: gcc-queue, clang-queue, gcc-tsan-0, gcc-tsan-1.

## AX-PR-T44-08-T44.10.a

Original task: Decode the parent's basic PSP frame and vector arithmetic only after validating readable bounds, alignment and scoped EXC_RETURN assumptions. Add a truncated frame and an extended-FP marker that the basic decoder must refuse.

The retained basic PSP decoder validates EXC_RETURN, byte alignment, readable captured extent and frame state before reading. All short lengths reject without changing output; extended FP is rejected by the basic policy. The example recovers PC0x08000402 and pre-SP0x20001000.

Changed condition: Add alignment padding with an explicitly inspected target contract.

The explicit extended policy accepts a ninth alignment word when xPSR alignment is set and recovers pre-SP0x20001004. The basic policy still rejects it. This is captured-data decoding, not a current physical exception.

Evidence selectors: gcc-debug-core, clang-release-core.

## AX-PR-T44-08-T44.10.b

Original task: Discriminate bad startup range, invalid fault frame and DMA early reuse using safe captured status and ownership events. Mark side-effecting MMIO registers as unsuitable for blind debugger polling.

The captured certificate derives reset Thumb PC, IRQ3 vector at0x0800004c, startup ordering and BFAR validity separately. A precise-valid capture permits the address; an imprecise capture without BFARVALID does not. Side-effect registers are not blindly polled.

Changed condition: Move the model to a cached MCU and list the added visibility obligations.

A cached-MCU variant adds clean-before-device-read, descriptor publication ordering, completion/quiescence and invalidate-before-CPU-read obligations. These premises are analyzed explicitly; no cache or board observation is fabricated.

Evidence selectors: certificates, gcc-debug-core.

## AX-PR-T44-08-T44.10.c

Original task: Repair cancel_request-to-CPU ownership so only confirmed stop or completion returns the buffer. Replay duplicate acknowledgment, timeout and reset histories in the portable model; keep hardware execution pending unless performed.

The retained DMA driver keeps cancelling state through timeout, returns ownership only after observed idle, rejects duplicate completion and rejects old tickets after a new start. A cancellation request alone does not permit a CPU write.

Changed condition: Add a late completion from the pre-reset generation.

The reset-epoch model refuses epoch/ownership change without quiescence; after acknowledged reset, ticket7 cannot complete epoch8. Generation overflow blocks restart. Device reset quiescence remains a named platform premise.

Evidence selectors: gcc-debug-core, certificates.

## AX-PR-T44-09-T44.11.a

Original task: Choose detector/static/fuzz tools for an extent fault, uninitialized value, race and defined saturation bug. Require a negative control for each claimed capability and record actual support rather than assuming installed means applicable.

Property selectors bind actual extent/lifetime ASan reports, initialization Memcheck reports, race TSan reports, static Eva paths and the exhaustive defined-narrowing semantic oracle. Repaired controls exist for each tool family. Instrumented core execution does not convert semantic disagreement into undefined behavior.

Changed condition: Move code into an uninstrumented dependency and reassess coverage.

An uninstrumented dependency is outside sanitizer load/store coverage. The certificate states that its internal accesses need an instrumented rebuild or a documented gap; a clean caller-only run cannot close the dependency claim.

Evidence selectors: gcc-asan-diagnostic-0-0, gcc-plain-diagnostic-0-2, gcc-tsan-0, eva-1, gcc-core-sanitized, certificates.

## AX-PR-T44-09-T44.11.b

Original task: Exhaustively compare all65536 uint8 operand pairs for the original saturating-add contract against a wide mathematical oracle. Preserve the first mismatch and reduce under a stated ordering without changing the contract.

The wider reference enumerates all65536 u8 pairs and compares saturated min(a+b,255) with defined narrowing. Exactly32640 overflow pairs disagree. Under the declared minimum-sum ordering, first failing sum is256, without claiming a unique pair.

Changed condition: Change the contract to checked addition with unchanged output on failure.

Checked addition returns failure for the same32640 overflow pairs and leaves sentinel777 unchanged. Successful pairs publish the wider exact sum. This variant has a different failure/output contract from saturation.

Evidence selectors: gcc-debug-core, clang-release-core.

## AX-PR-T44-09-T44.11.c

Original task: Triangulate a static alarm, a dynamic report and the minimized source transition for one original defect. Include a path infeasible under an explicit precondition and a dynamic trial that never reaches the fault.

The four-element static specimen separates guarded admitted n<=4 from the weakened access a[n]. Eva and ASan point to the same n4 indexing transition; normal dynamic n3 does not reach it. The minimum bad index equals the extent4.

Changed condition: Weaken the precondition and show when the previously infeasible path becomes reachable.

The weakened predicate makes the n4 path reachable while preserving the source transition. Re-running Eva reports the alarm and the explicitly reached ASan control reports it; the unreached run remains silent.

Evidence selectors: eva-0, eva-1, gcc-unreached-1, gcc-static-reached, certificates.

## AX-PR-T44-10-T44.12.a

Original task: Compare reject-before-commit with clamp-after-commit using an observer callback inserted between mutation and clamp. Show a neighboring case where both return the same final count but only one preserves the invariant throughout.

The actual callback sees5 on the increment-then-clamp path while the repaired path rejects before publication. A final value alone cannot distinguish causal repair from symptom suppression.

Changed condition: Replace the callback with an asynchronous consumer and derive publication ordering.

The asynchronous derivation requires validation before release publication and acquire before payload observation. Atomic storage of5 would still publish an invalid state, and later clamping cannot revoke an already observed value.

Evidence selectors: gcc-debug-specimen-0-1-counter, gcc-debug-specimen-1-1-counter, certificates.

## AX-PR-T44-10-T44.12.b

Original task: Replay the minimized trigger, adjacent valid/invalid boundaries, every acquisition failure prefix and debug/release/instrumented builds. Retain exact expectations for semantics and flexible categories for diagnostics.

The minimized A trigger, non-trigger N, malformed neighbors and timeout/crash exclusions are replayed with semantic classification. Debug/release/logging profiles, all allocation-failure prefixes and sanitized relocation controls preserve their declared ownership and result contracts.

Changed condition: Change optimization and allocator policy while keeping the API contract fixed.

Optimization and forced allocation relocation change execution conditions while the external API remains the same. Assertions check accepted states, byte prefixes and publication, not diagnostic wording or incidental addresses.

Evidence selectors: replay, gcc-release-specimen-1-1-counter, clang-release-allocation, clang-allocation-sanitized.

## AX-PR-T44-10-T44.12.c

Original task: After at least seven days, diagnose a withheld variant with the same outward symptom but a different cause. Freeze predictions, replay manifest, first-invalid-state evidence and repair before revealing assessor analysis.

DELAYED.md defines an actual independent assessment workflow: wait at least seven days, freeze learner predictions/manifest/first-state/repair before reveal, record hints, then compare to the withheld assessor oracle. Author runs establish only that the exercise works. Independent performance remains unobserved.

Changed condition: Transfer the diagnosis to another component with a different ownership boundary.

The withheld changed component is a callback-borrow generation boundary rather than admission capacity. Its faulty and repaired binaries are validated separately, but these author runs are never labeled independent or delayed learner evidence.

Evidence selectors: hidden-0, hidden-1.

