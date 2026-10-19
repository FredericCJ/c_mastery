# Ordering edges and bounded C memory-model investigation

This required successor supplies the ordering investigation in X-T73-006-R4 and
X-T73-009-R4. Their pointer lifetime variants have a separate contract in
`LIFETIME.md`; this ordering investigation alone does not discharge them. Implement
`src/weak_follow.c` using `followthrough/weak.h`. The fixed eight-slot deque retains
the original one-owner/multiple-thief, finite-index, unchanged-failure contract.
The sequence object retains one serialized writer, atomic payloads, bounded reader
attempts, nonwrapping sequence and output-canary contracts. Controls and outputs
must be live, suitably aligned and disjoint. Initialization is quiescent; no copying
or destruction of live atomic controls is allowed. This is ISO C17 source using
C11 atomic facilities; GenMC's selected RC11 model is identified separately and
is not the published ISO C17 text. The exact published C17 access gap remains.

## Deque derivation

Use relaxed owner-bottom loads and stores, acquire top observation on push,
relaxed atomic slot accesses, and a release fence between slot initialization and
bottom publication. Pop publishes its decremented bottom before a sequentially
consistent fence, then observes top. Steal observes top with acquire, executes a
sequentially consistent fence, then observes bottom with acquire. Both final-item
pop and steal use sequentially consistent compare-and-exchange on top, with relaxed
failure order. The owner restores bottom on an empty or lost final-item attempt.
Only a winner publishes its output. Refuse push at SIZE_MAX-1 before addition.

Explain the publication path from the release fence, through the subsequent
bottom store read by an acquire load, to the relaxed slot read. Explain separately
the cross-index arbitration supplied by the sequentially consistent fences and
compare-and-exchange; atomicity of each index alone supplies no joint snapshot.
List every operation's linearization point, including unsuccessful reads and
permitted CONFLICT. Keep all slot accesses atomic even when a losing consumer
discards its speculative value. A successful finite model does not automatically
prove the published growable algorithm or its reclamation assumptions.

Three original controls remove only (1) the push release fence, (2) the pop
sequentially consistent fence, or (3) the steal sequentially consistent fence.
The publication case detects an uninitialized zero returned as an accepted task.
The two-item owner/two-thief case exposes duplicate consumption of task42 when
either cross-index fence is missing. Predict the reads-from and coherence edges
before reading the recorded model trace. An edge whose removal passes one small
case is not therefore redundant: publication-only cases cannot assess arbitration.

## Sequence derivation

The writer loads its sequence relaxed, publishes odd relaxed, stores each atomic
payload with release, then publishes even with release. A reader loads the first
sequence with acquire, rejects odd, loads both payloads relaxed, executes an acquire
fence, then reloads sequence relaxed. Publish only if both sequence observations
are equal. The first acquire prevents old payload values after an observed completed
publication. If a payload load observes an overlapping later writer, that payload's
release synchronizes with the following acquire fence, placing the writer's prior
odd publication before the final sequence observation. Atomic coherence then prevents
accepting the earlier even sequence. Both directions are needed; merely checking
two equal numeric values is insufficient. Nonwrapping counters are a premise.

Controls separately remove payload release, reader acquire fence, final-sequence
release, and initial-sequence acquire. Every one has an actual bounded RC11 safety
counterexample in the supplied two-publication/one-read case. All payloads remain
atomic; a mixed pair is a defined atomic-ordering failure, not an ordinary data-race
experiment. The C++ proposal P0690R1 section3.2 is a comparison source for this
ordering pattern, not an adopted C feature or permission for racing ordinary data.

## Model and native evidence

`tools/run_messaging_models.py --genmc <private-binary> --source <private-checkout>`
records exact commands, return codes, complete output and source/tool hashes. It
uses GenMC0.19.0 commit9f6c4c0772d0c42b325681581acc6a0f3eb9b5f7 with LLVM21.1.8,
explicit RC11, exhaustive mode, disabled memory-model autodetection and disabled
size estimation. It includes the same implementation C file used by native tests;
the model client does not replace its atomic operations with a Python simulation.
The checker runtime supplies its documented pthread and atomic model. No real I/O,
clock, random input, unbounded operation stream or hardware scheduling is modeled.
Four finite deque clients cover publication, two preloaded items, publication with
competing pop, and a reused bottom slot with a second owner pop. The sequence client
has two publications and one bounded read. The SC control compiles the same operations
with all explicit memory orders strengthened. Keep passing and failing traces.

Pass requires successful compilation/transformation, completed verification and no
reported error for each positive control; every negative case must report the named
safety violation after compilation, not an unsupported API or compiler failure.
All seven removed-edge controls must fail their selected client. The full matrix
also records expected cases in which a defective edge is not exercised. Supply a
written explanation of why those misses limit the oracle.

Native `messaging.weak_deque` checks128 capacity/reuse rounds,128 recorded three-call
histories against all six orderings consistent with observed real-time precedence,
and4,000 distinct tasks with two real thieves. CONFLICT is an explicitly admitted
no-output result in that bounded history specification; this is not full general
linearizability verification. `messaging.weak_sequence` checks zero attempts, odd
state, exhaustion, output preservation and10,000 publications. Both record actual
`atomic_is_lock_free` results. GCC/Clang debug/release, ASan+UBSan and TSan campaigns
are separate observations. Native controls alter a returned task, admit exhausted
indices, overwrite zero-attempt output or omit one payload write. They are not
substitutes for the removed-order-edge model controls, which x86 may conceal.

Submit D1 ordering graphs and counterexamples; D2 implementation; D3 exact model,
native, mutant and generated-code evidence; D4 the13-dimension cost analysis and
pointer variants required by the parent briefs. Record per-profile lock-free
results, emitted fences/instructions and an equivalent mutex snapshot comparison
before making cost claims. Graduated hints: distinguish atomic from ordered;
identify the publication read; then find the overlap-to-final-check synchronization
edge. The separate assessor analysis explains the observed traces. Independent
learner and delayed assessment remain unperformed. No physical/weak-ISA run is
claimed. Model checking is bounded by these clients and the chosen semantics.


The inspected author errata is a required comparison for growth: item2 corrects
the reload after resizing; item3 corrects the C11 array-publication dependency.
Explain why a fixed-array client cannot test that omitted edge. `LIFETIME.md`
uses protected array publication and retention, not a transplanted weak-order
reclamation proof. The current model evidence applies only to its named fixed
array clients; no ARM execution or general consume-implementation claim follows.
