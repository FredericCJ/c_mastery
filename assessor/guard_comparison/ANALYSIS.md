# Assessor: guard enforcement and incomplete cleanup

Withhold this analysis from learner exports. Read the public contract first.
Hints, offered progressively: (1) mark the final permitted payload access;
(2) identify which operation removes the last protection; (3) separate
unreachability, quiescence, destructor execution and acknowledgement.

History 1: the old cohort blocks the second advance. N remains retained.
The reader's safety argument survives a stall; an eventual collection claim
does not. A bounded three-node pool can refuse a new publication without
violating safety. Increasing its capacity postpones exhaustion, not the proof.

History 2: clearing the Boolean releases protection still owned by the second
handle. Rejecting nested enter is a valid original-contract answer. A nesting
extension needs a participant-owned count, checked increment, unique handle
consumption and final-release unpin. Count zero alone does not authenticate
a caller or prevent copied handles from being consumed twice. A false answer
that merely replaces Boolean with unsigned count fails the ownership rubric.

History 3: a live borrow used after exclusive repinning conflicts with the
safe interface's exclusivity requirement. C accepts a copied pointer and an
ordinary call unless a particular diagnostic extension adds checks. Reject
answers claiming that compilation proves pointee validity. The raw-pointer
case bypasses the useful borrow relationship and requires a separate proof;
the assessor must never demand actual undefined dereferencing as evidence.

History 4: there is no acknowledged destructor result. Preserve a separately
owned completion record until callback completion is recorded, and preserve
external-resource ownership on failure according to an explicit policy.
Cancellation can withdraw interest without retracting already published work.
The record must itself outlive the callback and synchronize result visibility.
A nonblocking caller may return PENDING with retained ownership. A bounded
timeout is not a certificate that the callback will never access its context.
Compare the actual MC-FALLIBLE-CLOSE integration separately; this logical
exercise is not fresh execution of that adapter.

History 5: unprotected access removes the exclusion used in the reclamation
proof. The distinguishing trace is `read-token; remove; immediate reclaim;
logical read-request`, rejected at the logical lifetime check. No C dangling
access is necessary. In teardown, the proof must establish joined/otherwise
quiescent participants, no future publications and unique teardown authority.

Score each objective separately on a 0–3 scale: T55.07.a requires correct
retirement/protection state in histories 1/2/5; T55.07.b requires enforcement
boundaries in histories 2/3 and counterexamples; T55.07.c requires bounded
space, scan/callback costs, lack of liveness guarantee and history 4 ownership.
Score 0 for unsupported assertions, 1 for assisted correction, 2 for complete
justification under given premises, 3 only for an independently solved changed
condition with its revised premises and counterexample. Passing all ordinary
cells requires at least 2 in each; no ordinary score supplies delayed evidence.

Delayed migration analysis: a thread-pinned participant cannot simply be
treated as task-local state moved to a different worker. Valid proposals can
end all borrows before suspension and reacquire on resume, or use a different
explicit registration/lifetime design with a justified synchronization proof.
Reject carrying a raw pointer across suspension merely because its numerical
address stays unchanged. Fallible cleanup must separate logical retirement,
external close outcome and eventual resource recovery; leaked-on-purpose
containment can be a documented failure policy, not successful cleanup.

The inspected upstream files support a narrow interface comparison, not all
collector internals or the C standard. No independent review or learner
attempt is represented by this worked analysis.
