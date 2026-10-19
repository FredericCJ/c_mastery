# T32 assessor derivations

Withheld from learner exports. Each original task and exact changed condition has a separate argument; tool/host/emulator/physical/learner authority remain separate.

## AX-PR-T32-04-T32.07.a

The five-byte scripts2,3;2,0;2,-1;2,4 and-2 distinguish complete, zero, timeout, oversized and invalid progress. Only accepted counts add to committed; all failure paths reverse local ownership without erasing external bytes. The core allocates only through the injected acquisition callback; deterministic failure models allocation refusal at each boundary.

Changed condition: Cancellation arrives after partial publication; define residual effects.

Cancellation after the first2 commits exactly the two-byte prefix and releases all local identities. Output ownership remains unpublished. Claiming zero external effects is false: cancellation is not rollback of a remote receiver. A stronger atomic contract needs staged remote commit and ambiguity recovery.

Evidence selectors: transaction, ipc.

## AX-PR-T32-04-T32.07.b

Original failure indices0,1,2 give acquired0,1,2; released0,1,2; live0,0,0. Success transfers3 and no release occurs until explicit destroy. The test checks reverse identity order and every live slot.

Changed condition: Insert a fourth acquisition and enumerate new prefixes.

Insert15 between10 and20: prefixes[],[10],[10,15],[10,15,20],success[10,15,20,30]. Failure rollback reverses each exact prefix; success destroy consumes all4. A release omission is killed by live-resource state, not output alone.

Evidence selectors: transaction.

## AX-PR-T32-04-T32.07.c

After accepted2 of5, remaining3; callback4 is invalid and cannot advance the pointer. Reentrant invocation sees busy and returns before acquisition. Guarded returns do not constrain arbitrary same-process writes, longjmp or concurrent destruction; those violate stated callback premises.

Changed condition: Move adapter behind a process/message boundary and specify validation/lifetime costs.

The actual child returns token7/sequence9/u24count. Parent requires exact5bytes, correct identity/sequence and count<=5. Oversize6, sequence8 and truncated3bytes reject before commit; child is reaped. Copying and kernel crossings cost time/storage; no pointer can outlive the message. Separate address spaces do not prove device commitment or privilege isolation.

Evidence selectors: transaction, ipc.

## AX-PR-T32-05-T32.08.a

Separate native ASan heap-bound/lifetime controls, UBSan signed overflow, unsanitized Memcheck bound/lifetime/uninitialized branch controls, and TSan conflicting-counter controls record their own markers and repaired outcomes. ASan returning normally on the uninitialized branch does not prove initialization.

Changed condition: Move to a target lacking one runtime and provide a supported host route plus remaining target gap.

Cortex-M3 guest has no hosted diagnostic runtime. The Linux C17 route executes portable logic under supported runtimes; the separate guest exercises its modeled core. MMIO, target ABI and physical schedules remain gaps. Runtime absence is not an accepted pass.

Evidence selectors: diag, tsan, guest.

## AX-PR-T32-05-T32.08.b

The fixed512x4096 modular workload has an independently recomputed exact checksum. size text/data/bss, process peak RSS,0.01s outer elapsed resolution and monotonic inner resolution remain bound to each of3samples/compiler/profile. No universal overhead multiplier is inferred.

Changed condition: The instrumentation changes allocator behavior and masks the original timing failure.

If allocator quarantine changes reuse/timing, losing a symptom is not a repair. Preserve original allocator/runtime config and add an event-history property. This package demonstrates configuration masking separately with assert side effects and controlled schedules; it does not pretend to observe an allocator timing fault that was never captured. Recollect that fault under its original precondition before causal attribution.

Evidence selectors: workload, releasecase, schedule.

## AX-PR-T32-05-T32.08.c

Compatibility is actual compilation plus startup plus intended mechanism. Both compiler families run separate ASan/UBSan/TSan and plain Memcheck. Every command keeps exit and diagnostic. The guard rejects generic nonzero termination or missing marker as a successful test.

Changed condition: Compiler upgrade changes available checks; rerun the probes before reusing receipts.

A compiler change changes source/tool identity and requires fresh positive/negative probes. Compare exact old/new warning and behavior records rather than accepting the same filename. The source-bound guard deliberately invalidates changed manifests; it does not fabricate a future compiler upgrade.

Evidence selectors: version, diag, tsan.

## AX-PR-T32-05-T32.09.a

The public wrapper validates header, length0..10, opcode1, count<=4 and exact payload before private access. Runtime cases0,1,2,9,10,11 and count4 have explicit publication checks; Eva explores a nondeterministic entry length over0..10. Tool context is not the set of runtime samples.

Changed condition: An external callback modifies length and invalidates a local range invariant.

Callback changes n to11 after n<=10. The previous local range fact no longer justifies packet[n]. Eva case2 retains that alarm; case3 revalidates n<10 and rejects the access. Input lifetime/concurrent mutation still need separate premises.

Evidence selectors: packet-runtime, eva-0, eva-2, eva-3, warnings-packet.

## AX-PR-T32-05-T32.09.b

unchecked requires count<=4 and sufficient live payload. Eva case1 supplies count5, retaining the counterexample; public wrapper rejects an invalid count/extent instead of merely assuming it. The private proof cannot be exported to every caller.

Changed condition: Another caller bypasses validation and require contract propagation.

A second caller bypassing admission inherits both range and extent obligations. Case1 explicitly represents the bypass. Use the public wrapper or establish the same invariant at that entry; annotate and analyze every reachable entry rather than globally suppressing the access alarm.

Evidence selectors: eva-0, eva-1, packet-runtime.

## AX-PR-T32-05-T32.09.c

The suppression disposition retains exact raw alarm, source hash, entry, owner, tool and command. It rejects global suppression of the count5 counterexample. The validated entry can carry a narrowly scoped precondition argument, whose regression includes the bypass and callback variants.

Changed condition: The analyzer upgrades and warning locations change; reconcile by mechanism rather than line number.

Warning line shifts are not semantic equivalence. Reconcile by unchecked payload access, count/extent authority, entry context and source/tool versions. The guard rejects a changed identity; the worked shifted-location example keeps the same mechanism key but demands reanalysis. No analyzer-upgrade execution is invented.

Evidence selectors: eva-1, eva-2, eva-3, warnings-packet.

## AX-PR-T32-06-T32.10.a

Truth assignmentsTT,FT,TF produce1,0,0. B is unevaluated inFT. Holding B=true,TT/FT isolates A; holding A=true,TT/TF isolates B. Branch outcomes alone omit these constraints. Independent model enumerates evaluated terms and pairs.

Changed condition: For(A&&B)||C derive new pairs and evaluation traces.

For(A&&B)||C, A pair(T,T,F)/(F,T,F), B pair(T,T,F)/(T,F,F), C pair(F,T,F)/(F,T,T) preserve other truths and flip result. In the first true case C is skipped; when A=false B is skipped. The model derives all8rows and actual component tests check the changed decision.

Evidence selectors: independent-models, coverage.

## AX-PR-T32-06-T32.10.b

Actual GCC --coverage/-fcondition-coverage artifacts and gcov --conditions remain bound to their ELF/source. count>4 is defensive and unreachable from initialized state through capacity-preserving API; keep it with the invariant, not deleted for a denominator.

Changed condition: Prime-path instrumentation exceeds its configured path limit.

Actual -fpath-coverage-limit=1 emits a path-limit diagnostic and leaves over-limit functions uninstrumented. This is absent instrumentation, never100% prime-path coverage. Manual independent-effect evidence is still separate.

Evidence selectors: condition, prime-path-limit.

## AX-PR-T32-06-T32.10.c

Weak return-only rejection traverses the same branch with rejection counter unchanged by mutant4. Raw weak gcov branch maps are compared. A rejection_count==1 assertion kills the wrong effect while the original branch objective remains unchanged.

Changed condition: A cleanup omission leaks while returning correct values; add a resource oracle.

Cleanup mutant3 leaves live3 while returning the same nominal values. Weak suite survives; strong live==0 assertion exposes it. This is a resource oracle, not an arbitrary extra iteration or private-layout check.

Evidence selectors: condition-weak, coverage-4, coverage-3.

## AX-PR-T32-06-T32.11.a

Fault models are capacity+1<=4 changed to<4, authorization inverted, and cleanup omitted. Discriminators are fourth push, unauthorized request, and live-resource zero. Each compiles and fails a named semantic assertion; unrelated crash/build failure is rejected.

Changed condition: A mutation affects only a previously excluded boundary and revisit the exclusion.

The fourth push was excluded by the two-push weak suite, but is admitted by the original API. Revisit exclusion using the exact full-boundary case; the capacity mutant now fails. An actually unreachable defensive state is different and keeps its stated invariant.

Evidence selectors: coverage-1, coverage-2, coverage-3.

## AX-PR-T32-06-T32.11.b

The two-push weak queue suite accepts the premature-full mutant because it never reaches count3->4. Strong public pushes and visible contents/count/refusal expose the missing capacity; no ring index assumption is needed.

Changed condition: A cleanup mutant survives because only final values are checked.

Cleanup mutant survives when only returned values are compared. Track declared live ownership before and after cleanup; strong suite requires zero and kills the omission, while the repair passes.

Evidence selectors: coverage-1-weak, coverage-1, coverage-3-weak, coverage-3.

## AX-PR-T32-06-T32.11.c

For integer n, n<5 iff n<=4. Under0..4 they both accept every value. n<4 differs at4. The finite enumeration distinguishes equivalent versus meaningful boundary mutants.

Changed condition: Expand admitted count to5 and reevaluate equivalence.

Expanding to5 does not break equivalence: both predicates reject5. If the new specification admits5, both are wrong together, but remain equivalent to each other. This changed-condition premise must be checked, not forced into a fictitious counterexample.

Evidence selectors: independent-models.

## AX-PR-T32-07-T32.12.a

For bytes12,34 predict wrong extent(n!=2), stale owner(live_owner!=1), and endian defect(return0x3412). GDB sees n2, owner1 and bytes12,34 in the live static object, rejecting the first two explanations for this execution. Return13330 versus expected4660 supports the encoding hypothesis.

Changed condition: Optimized local unavailable; reconstruct from registers/memory instead.

Optimized value can be unavailable before assignment. Inspect actual x86-64 SystemV rdi/rsi/rdx and two memory bytes at debug_decode entry, plus exact disassembly and return. Argument registers and stack frames are tied to this ABI/ELF, not portable C facts.

Evidence selectors: debug-O0-0-gdb, debug-O3-0-gdb.

## AX-PR-T32-07-T32.12.b

Original12,34 and reduced00,01 preserve the same byte-order disagreement. Equal bytes00,00 remove it. Two bytes is the minimum admitted extent; repair changes only the fold direction. Both versions and raw results remain retained.

Changed condition: The minimal case needs an environment precondition; retain it in replay.

Minimal bytes alone are insufficient: n2, live_owner1 and declared big-endian wire contract are required. If the environment changes byte order or ownership, this replay no longer asserts the same claim. The saved replay record includes these premises and literal bytes.

Evidence selectors: debug, minimal, equal-control.

## AX-PR-T32-07-T32.12.c

The release-only failure is caused by NDEBUG removing acquisition inside assert. GDB enabled/disabled preserves the outcome; ASan+UBSan does not restore the side effect. Repair calls acquisition outside assert. This controlled case rejects a scheduling explanation without claiming all debugger perturbations are harmless.

Changed condition: Use a deterministic schedule harness for the logical interleaving while keeping real-concurrency evidence separate.

Actual pthreads follow six controlled read/write schedules. Atomic load/store loses an update on four interleavings while serial schedules retain2; fetch_add retains2 on all6. This isolates logical atomicity with defined C behavior; arbitrary native scheduling and weak-memory behavior remain separate.

Evidence selectors: releasecase, schedule.

## AX-PR-T32-07-T32.13.a

Host covers grammar/FIFO/ownership. The two identified QEMU Cortex-M3 machines cover the observed SVC entry/return and byte fold on their modeled cores. Routing, UART clock and analog behavior require physical measurements and remain open.

Changed condition: Swap emulator machine while reusing protocol bytes.

Swap mps2-an385 for lm3s6965evb using the same bound image/bytes. Both commands are retained separately; success on each does not transfer unmodeled peripherals or physical timing. Semihost output is explicitly not UART evidence.

Evidence selectors: mps2-an385, lm3s6965evb, independent-models.

## AX-PR-T32-07-T32.13.b

ADC accuracy cannot follow from host synthetic numbers; simulated ticks cannot establish actual baud. Rewrite both as arithmetic/model observations and require a calibrated voltage reference/meter or calibrated timing capture, with board/clock/temperature setup.

Changed condition: A physical capture has unknown timebase accuracy; quantify rather than ignore uncertainty.

Unknown timebase accuracy gives no justified finite accuracy interval. The separate illustrative104us example assumes100ppm calibration and0.5us quantization; invert the resulting positive period interval to derive a baud interval. It is not a physical capture.

Evidence selectors: independent-models.

## AX-PR-T32-07-T32.13.c

Vector01,02,12,34,ab,cd means two big-endian values4660 and43981, sum48641. Host independent decoder and both QEMU machines agree. The board protocol is provided but unexecuted; logical agreement does not close transport/routing/timing.

Changed condition: A big-endian target exposes a native-layout assumption.

A native little-endian interpretation produces a different sum; explicit wire-byte folding is representation independent. The model counterexample and Cortex-M guest are not a claimed executed big-endian target. A genuine big-endian adapter must reexecute with its own ABI/transport identities.

Evidence selectors: mps2-an385, lm3s6965evb, independent-models.

## AX-PR-T32-07-T32.14.a

Replay retains saved bytes, generator version/seed, source/ELF/symbol/log hashes, exact commands and the failing relation. abc hashes to the supplied nonzero digest; zero is rejected. Raw defective observations remain alongside repairs.

Changed condition: Input generator changes but seed stays same; replay saved bytes directly.

Generator v1(seed0) gives00,01 and v2 gives00,02: the same seed is insufficient. Saved bytes00,01 are replayed directly; metadata identifies original generator without silently regenerating input with changed code.

Evidence selectors: independent-models, minimal.

## AX-PR-T32-07-T32.14.b

Six GCC/Clang debug/release/ASan+UBSan semantic matrices include completed references, compiled incomplete stubs and exact semantic mutants. Each output/exit is recorded separately; startup or sanitizer failures are not accepted as mutant kills.

Changed condition: Compiler update changes warnings/codegen and requires new evidence.

Tool upgrade requires fresh source/version-bound observations, including warning/codegen and negative-control sensitivity. Guard mutations simulate stale source/tool metadata only; no actual future upgrade is asserted. Re-run matrix under the upgraded compiler before reusing evidence.

Evidence selectors: version, transaction, coverage.

## AX-PR-T32-07-T32.14.c

Each exact criterion maps to its mechanism selectors, source-bound campaign artifacts or explicitly named physical/open gap. Counterexamples include oversize count, corrupted state effect, private unchecked entry, release assert omission and wrong endian fold.

Changed condition: Independent reviewer challenges an assumption and revise the claim rather than the evidence.

Reviewer scenario challenges whether a child count proves device receipt. Preserve the IPC bytes and narrow claim to validated message acknowledgment, leaving device delivery open. This is a worked review response; external independent review and delayed unaided learner evidence remain not-run.

Evidence selectors: ipc, independent-models.

