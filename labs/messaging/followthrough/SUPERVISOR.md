# Execute restart topology against actual owned actors

This required X-T73-005-R2/R3/R4 successor executes the original `MsgSupervisor`
plans against actual `MfaActor` instances from X-T73-004. The fixed two-level tree
contains four child supervisors, each managing four actors; the parent supervises
those four branches. Parent and child restart-intensity limits are independent
1..4 counts with the same explicit nonzero tick-window parameter. Absolute ticks
are caller observations, not a new wall clock. Retain the original all-12-plan
enumeration, exact-window and timestamp-boundary assessments.

## Representation and interfaces

`MfsTree` owns five bounded supervisor records, sixteen actual actors, a resource
live bit per actor and a common lifecycle policy. It borrows stable handler
contexts and resource callback context. The resource `open(ctx, group, child)`
constructs one instance, returning true only after successful initialization; on
false it must leave no resource to close. `close` receives each successful instance
once and completes cleanup before returning. Both callbacks return normally,
without reentering/mutating this tree or retaining its borrowed controls. Neither
callback may destroy the tree. Contexts and required outputs do not alias the tree
or still-live input arrays. Serialize **all** actor/tree operations externally.
No actor handler is running while lifecycle actions execute; a stepping actor
causes BUSY before any mutation. The API does not stop native threads.

Initialization validates arguments before changing output, initializes all actor
controls before any resource construction and starts nodes in branch/child order.
It uses actual checked actor restarts; initial live generations are 2. Any failed
initial constructor rolls back all earlier instances in reverse order and returns
FAILED with a closed, resource-empty tree. The tested constructors allocate real
heap objects, and every initial position is fault-injected. A failed initialized
tree is disposable; no constructor failure is hidden by a passing nominal test.

Fault input includes branch/child, nondecreasing uint64 tick, ONE/ALL/REST strategy
and an eight-bit failure mask. Bit0 chooses the first constructor attempt in this
fault call, bit1 the second, etc. It is an explicit constructor-boundary injection,
not a claim that the OS allocator failed. At most four local plus four parent
attempts occur. The callbacks may also fail independently. INVALID/BUSY/CLOSED
leave the report unchanged. A valid recovery may change state even when it returns
FAILED; this is a lifecycle transition, not an all-or-nothing function.

Use a staged supervisor plan to reject invalid time/strategy before publishing
budget changes. Stop in reverse order by destroying the actual actor's pending
owners before closing its associated resource. Start forward, constructing the
resource before making the actor live. Child2 (zero-based index1) depends on child1
(index0) being initialized. No callback runs between lifecycle actions. Any
retained sibling resolves the current dependency on its next handler call and
must not retain an old instance pointer across a stop. Code or data with a stronger
dependency requires a different restart closure.

## Failure policy and changed conditions

Seal this choice: on partial local restart failure, roll back the **whole branch**,
including successfully restarted nodes and otherwise healthy siblings, then ask
the parent to restart that branch once. Parent recovery stops any remaining branch
instances, starts all four forward, then creates a fresh child-supervisor budget.
The parent's budget is retained across child-supervisor reincarnations; resetting
both would permit an unlimited restart loop. Local intensity exhaustion follows
the same parent path. On parent intensity exhaustion or failed parent recovery,
stop all branches and permanently close the tree. There is no recursive unlimited
retry or undocumented degraded state. Destruction is quiescent reverse cleanup;
it can be repeated because it has no native destroyed-mutex representation.

The report contains up to64 signed node events (`-(4*g+c+1)` stop,
`+(4*g+c+1)` successful start, `100+(4*g+c+1)` failed start), constructor-attempt
count, parent-use, rollback and closed flags. Stop events are emitted only for
actually live actor/resource instances. A failed handler can leave the actor dead
while its resource remains live; supervisor stop must still release that resource.
Bounds follow from at most two restart phases and a final reverse tree stop.
Resource destruction is ordered and potentially expensive; constant storage does
not imply constant elapsed shutdown time.

Required experiments include all12 strategies/failed-child cases with actual
resource lifecycle; every initial construction failure; a real handler failure;
BUSY quiescence refusal; old-time refusal; separate parent/child intensity budgets;
and child2 initialization failure after child1 succeeded, followed by both a
successful parent recovery and another failure during that recovery. The normative
partial-recovery trace for branch1, ALL, local second-attempt failure is
`-8,-7,-6,-5,5,106,-5,5,6,7,8`. Independent set-based modeling compares all3,072
strategy/child/failure-mask combinations with compiled execution. Reject controls
that leave resources live after failed parent recovery or reverse startup order.

Implement `src/supervisor_follow.c`; run `messaging.supervisor_tree` and the
separate host sanitizer profiles. Explain why the failure policy is stronger than
merely producing a plan, why a counter alone is not an error boundary, and why the
parent budget survives child restart. Hints: first separate actor liveness from
resource liveness, then draw the failed second constructor, and finally carry the
parent's history through successful branch reconstruction. Assessor analysis is
separate. No process isolation, physical target execution or learner mastery is
established by these finite fixtures.
