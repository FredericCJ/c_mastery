# Withheld T25 atomic mechanism assessment

Use the unchanged twelve original rubric IDs. Passing C tests is necessary for
the implemented anchors, not sufficient for the semantic answers. Require the
learner to explain the following mechanisms without viewing this analysis.

## T25.06.a publication and reuse

Only a ready load reading the releasing write establishes the selected SW
edge. Initial-zero observation has no such edge and must not access payload.
The reverse edge is last payload access -> retirement release -> writer acquire
-> next overwrite. Transitive closure includes every intermediate event;
65,536 four-node graphs are checked by a separate worklist traversal. A graph
cycle is not automatically an invalid input to the generic closure function.
The closure's correctness does not decide which RF/HB graph is admissible in C.

## T25.06.b memory locations

Conflicting ordinary writes, or an atomic write concurrent with ordinary
representation access to the same location, require an HB ordering. Two
atomic operations avoid that data race but can violate a higher-level property.
Distinct array elements are distinct locations. Adjacent nonzero-width
bit-fields in one uninterrupted sequence share the relevant location; a
zero-width separator changes that example. The descriptors are supplied
semantic facts, not inferred from runtime byte addresses. Reject an explanation
that a sanitizer's silence determines any of these categories.

## T25.06.c transactional observation

The barriers force total90 from atomic fields without executing undefined
behavior. A single mutex makes that intermediate state inaccessible to the
observer. The changed SC sequence snapshot is linearizable at a point between
the reader's two equal-even sequence observations: the nonwrapping single
writer cannot have completed or begun a field update inside that interval.
All payload accesses are atomic; version retry alone cannot legalize ordinary
races. A permanently odd writer state returns LIMIT in at most the admitted
attempt count. Multiple writers, version wrap, relaxed replacements or
cancellation mid-write invalidate this proof and are outside the contract.

## T25.07.a operations and aggregate

Fetch-add returns UINT_MAX while the stored value becomes1 after adding2.
Each atomic object is initialized before sharing. Aggregate value transfers
are actual atomic operations and may call libatomic; semantic field equality
does not establish a padding representation. The initialized destination is a
different object, not a representation-copied live synchronization object.

## T25.07.b actual lock-free evidence

The queried object's ABI and alignment matter. Record results, including0,
instead of requiring a predetermined macro value. Both natural and stronger
valid alignment are tested; no misaligned atomic is executed. A lock-free
query is not evidence of fairness, instruction count, ISR admissibility or a
whole algorithm's progress class. The local admission predicate deliberately
requires an independent context authorization and an audited helper count.
Actual i386 execution differs from a foreign-target frontend-only observation.

## T25.07.c strict closure

Inspect actual final symbols and disassembly. Cortex-M4 native-width operations
can lower to exclusive-load/store and barriers; widened64-bit or aggregate
operations can introduce calls. Cortex-M0 lacks the same native operation
sequence and the selected compiler introduces helpers even at32bits. The
negative link must identify its helper, not fail for an unrelated missing entry
symbol. No libatomic is silently admitted to profile C. A future helper
provider requires separate lifetime, blocking and preemption analysis.

## T25.08.a bounded models

Using outcome bit index2*a+b: MP masks for relaxed/RA/SC/mixed are15,11,11,15;
SB masks are15,15,14,15. MP ready1 under RA carries the first write HB before
the final load, excluding(1,0). SB(0,0) has no RF-based interthread SW edge and
is permitted under RA. All-SC adds a total-order contradiction for that pair.
The independent24-permutation oracle is appropriate only for all-SC. Mixed
SC stores do not pull relaxed loads into that total order. Constant stores
exclude value-generation cycles; this deliberately small solver is not a
general C memory-model checker. Ask for a new fence/RMW example to expose this
limit rather than treating a larger table as a theorem.

## T25.08.b multiple readers

Each acquire reads the writer's publication or an RMW in its release sequence.
The first reader's release RMW carries its prior ordinary read to the next
reader's acquire RMW; the final release reaches the writer's acquire of zero.
Thus both reads finish before overwrite. The writer never performs another
mask store while any reader bit remains. Fixed identity, single reader call
per identity and no departure are essential. Early clear permits the writer
to replace payload before the paused reader copies it. The independent finite
model gives a concrete seven/eight-transition witness. It separately checks
one/two readers for two generations, not all C executions or liveness.

## T25.08.c source and compiler comparison

Direct value dependency reaches a field through the consume-loaded pointer.
A condition that merely selects an unrelated load lacks that dependency.
kill_dependency explicitly removes it. Acquire-like selected compiler lowering
can mask this distinction in a particular binary; reject claims that the
binary establishes a portable source guarantee. Fence publication depends on
the relaxed flag load observing the store between the release/acquire fences.
Signal fences concern a different synchronization scope. TSan's missing fence
model is disclosed; its disabled fence branch is not a passing fence check.
N1570 is a C11 draft and N3096 a2023 draft. Exact published C17/C23 passages
remain unavailable; keep this leaf and delayed source-comparison work partial
even when all currently executable investigations pass.

## T25.09.a CAS orders

The declared valid failure sets are relaxed:{relaxed}; consume:{relaxed,consume};
acquire:{relaxed,consume,acquire}; release:{relaxed};
acq_rel:{relaxed,consume,acquire}; SC:{relaxed,consume,acquire,SC}.
This gives14 accepted pairs of36. Release-only success cannot retain acquire
failure under this contract. Failure updates expected as a load, never performs
the desired write or transfers the offered object's ownership.

## T25.09.b weak failure

The scripted trace is desired11/spurious, desired11/mismatch->expected20,
desired21/success. A stale desired value would overwrite the concurrently
changed counter incorrectly. A single weak attempt may fail with equal values;
strong excludes that spurious explanation but still legitimately fails on a
mismatch. Bounded attempts do not promise eventual success under interference.

## T25.09.c lifetime and publication

The sender retains the offered node across every failed attempt. `expected`
may now point to another live owner's node, which must not be freed by the
sender. Derivation requires a real preexisting lease before field access;
pointer-set membership merely checks the caller's declared lease set. The
receiver may free the newly published object immediately, including inside the
successful callback. The sender's report therefore uses saved scalar state
and does not inspect the offered node after publication. Actual C pointer CAS
and ASan test that terminal path. None of this supplies general reclamation,
address reuse protection or a hazard-pointer acquisition protocol.

## Independent assessment and evidence limits

Use each exact original delayed variant after its minimum learner interval.
Change which load reads the initial write, reader count, permitted attempt
budget, atomic width/ABI, and CAS expected-node lease availability. Do not award
credit for merely rerunning the provided sequence with another constant.
Require the learner to predict a counterexample before revealing the mutated
implementation. Author source campaign, finite model, external review,
independent learner work and physical execution remain distinct records.
