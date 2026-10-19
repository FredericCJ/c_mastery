# Context switches and the suspension premise

This additive X-T52-001 V1 adapter imports actual `b5_response`. Keep its fixed
priority, single CPU, preemptive, independent, non-self-suspending tasks, known
cost/jitter/period, constrained deadline and bounded-work premises. The caller
supplies a bound s on each dispatch/return transition; its origin must be stated
as assumed, measured or derived. These fixtures use assumed integer costs.

Charge target C+2s and each interfering job Ch+2s exactly once. A job dispatch
and return cost at most two transitions. Nesting preemptions does not justify
charging the preempted job twice again: the preempting job's pair already covers
those edges. The target's initial dispatch/final return are the separate pair.
This conservative bound allows extra pairs for jobs already running at the
interval boundary. B, if nonzero, must already include its own resource-protocol
overheads; arbitrary scheduler work, migration, interrupts, blocking wakeups and
time-dependent dispatch costs require separate bounds. Do not silently add the
same measured overhead to C and again to s.

`sx_switch_response` validates the original ranges, stages charged task costs,
refuses unrepresentable arithmetic/charged costs and delegates the recurrence.
All refused calls preserve output. Inputs and output are live, disjoint; calls
sharing storage are serialized. Zero switch cost recovers the original model.
With target C1, higher C1/T4, s1: estimates are 3,6,9,12,12. D12 passes; D11
rejects. This is a sufficient analytical test, not an observed missed deadline.

V2 breaks a premise explicitly: target C2/D3 with no higher task has original
bound2, but execute [0,1), suspend [1,5), execute [5,6) finishes6 and misses3.
Suspension contributes wall time without CPU execution. Before applying a new
analysis, declare a bound on total suspension, release jitter/segments and
blocking interactions. For this single job only, C+S=2+4=6 is exact. No general
self-suspension theorem follows, and adding S to an unrelated higher task's C
without a new arrival/backlog argument is not authorized by this example.

Implement the public stub, derive the transition charging argument, report raw
recurrences and the six-tick schedule, diagnose omitted and double overhead
controls, then repeat with fresh costs after the original seven-day delay.
Private Python compares 1728 cases using mathematical integer ceilings. Author
execution is not independent retrieval, physical WCET or a universal proof.
