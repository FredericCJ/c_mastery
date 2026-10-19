# Reference graph and race assessment

R1: Let S be owned strong references and E explicit weaks. While S>0,
W=E+1. The 1 is not an observer and cannot be dropped by a caller. Value
destruction changes S from 1 to 0 and consumes the implicit weak only after
the destructor returns. W=0 alone authorizes control reclamation. Retain starts
with a strong, upgrade starts with an explicit weak, and neither a raw pointer
nor a nonzero count grants a new ownership token. The release RMW modification
chain followed by the last acquire fence orders each owner's prior accesses
before destruction. This is a manual argument under admitted owned-reference
calls; it is not evidence that arbitrary pointer publication is safe.

R2: Before external drops, a strong cycle has S=(2,2), E=(1,1) from audit
observers. External drops yield S=(1,1), so no destructor runs. Replacing the
first-to-second edge adds an explicit weak to the second, then its old strong
drop destroys second and queues second-to-first. Draining destroys first and
queues first's weak edge. Both controls remain through observers. The weak-back
topology instead destroys first immediately; the deferred first-to-second drop
destroys second, and its queued weak release completes the edge cleanup.
Final observers account for the two reclamations. The queue bound is two;
clearing an edge before enqueue and removing before dispatch prevents duplicate
release and callback reentry. Partial construction has no edges yet and can
unwind the one previous node directly. Missing output publication is correct
on every allocator refusal.

R3: Inspect the actual atomic functions, not only the oracle. Successful upgrade
has a compare/exchange linearization point before zero; the failed case cannot
read the payload. The original release/fence pattern is distinct from an
acq_rel decrement. Both satisfy this admitted lifetime protocol, but the latter
is stronger. Installed TSan's fence warnings are preserved and experimentally
compared with a minimal control and stronger-order translation; never silently
reinterpret the original run as clean. GenMC's selected-source, strong-CAS
comparison has explicit limitations. The weak-CAS timeout is not a pass and the
full-producer transformation abort is not a semantic defect. Ask the learner
which retry/liveness claims remain unsupported.

R4: In a native race, main's observer weak prevents inspection-after-free; the
upgrader's own weak permits its upgrade even when the value has already ended.
Successful upgrade owns the value before checking 41 and releases afterward.
The observer is released only after both actual joins. A finite run may contain
only successes or only DEAD; neither outcome distribution is a correctness gate.
Changing the test to inspect an unretained raw control after join is invalid even
if an allocator happens to leave its bytes intact. Challenge the learner to
transfer the upgrader weak to the joining thread, or explain why synchronous
edge release would violate the callback premise in the cycle.

Cost review: controls and payloads are separate real allocations; each cycle
uses two of each, two edges, two observers and at most two queued releases.
Reference operations have constant uncontended work, compare/exchange retries
under contention, and unbounded external destructor cost unless a separate
bound is supplied. Atomic size/alignment and lock-free status come from actual
platform results. Headers expose representations, so ISO C cannot prevent
fabricated tokens, raw mutation, forgotten drops or copied owners. Runtime
overflow checks do not enforce ownership. Compile time, code size, stack,
allocation/failure paths, data traffic, dispatch, synchronization, diagnostics,
composition, portability and enforcement are all assessed through the shared
13-dimension contract, with exact observed artifacts rather than invented costs.

The author suite is bounded evidence, not an independent assessment. Reject
overstated Rc/Arc equivalence, arbitrary payload race freedom, cycle collection,
sanitizer cleanliness, weak-CAS exhaustive proof or physical timing claims.
