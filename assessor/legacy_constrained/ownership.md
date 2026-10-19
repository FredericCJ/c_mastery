# Assessor analysis: PR-T30-02

The three ordinary mistakes are publishing a pointer while its state is still
OWNED, treating completion notification as quiescence, and counting only the
queue's copied bytes as payload storage. The concrete repair reserves state
before the potentially blocking call; successful return performs no token access.
Failed send alone authorizes rollback. The controller collects notification,
then observes the worker suspended before deleting it. Static storage remains
live throughout. This is an explicit selected-port protocol, not ISO C magic.

`ownership.c` uses a generation-qualified finite typed pool with two normal
slots and a separate reserved slot. Terminal generation is not incremented
again; release retires it, and close/drain remain legal. `ownership_replay.c`
does not compare the implementation with itself: Python models all 9^4 words
independently with state lists and generation tuples. The bounded horizon does
not cover arbitrary histories; targeted MAX/world/zero-byte tests cover omitted
boundaries. Controls 1/2/3 respectively invade fault reserve, free OFFERED data
and substitute a zero assumption for unknown stack bounds.

The target first blocks a real FreeRTOS producer against a full queue. Its
ten-tick retry bound permits the controller's two-tick observation before stop.
The sender then aborts and releases once. Releasing before failed-send authority,
deleting a merely notified worker or busy-waiting on a lower-priority worker
would invalidate the proof. Target controls deliberately omit release, omit
suspension, and call the FromISR interface at priority96; all must fail normal
assertions, never merely hit a harness timeout. The control's priority64 nested
IRQ still runs before the forbidden API assertion, isolating the API check.

Normal payload sum is sum(n*(n%16+1),n=0..99)=41780. The ISR fault payload is
separate byte201, not part of that sum. Controller priority4 exceeds consumer2
and producer1; optional monitor3 makes seven bounded snapshots. Controller waits
for all independent notifications, then separately verifies every suspension.
The IRQ API sees priority160≥128; IRQ1 priority64 uses no kernel API. After IRQ0
wakes controller, the interrupted producer has not yet executed its following
store. This is observed in the selected actual guest, not a simulated trace.

T30.03.b and .07.b require annotating real port source and generated instructions.
PSP carries task frames; MSP carries nested handler work. Hardware base32 plus
software36 gives68. Extended104 plus software36 plus high-FP64 gives204. Four
alignment bytes may be added; local C frames and probe72 are separate. The probe
loads/checks all s16–s31 across vTaskDelay, while nested IRQ1 overwrites S0 and
the outer checks restoration. EXC_RETURN low values ED/E1 versus FD/F1 distinguish
FP/thread and nested handler frames. Actual MSP deltas vary with compiler frames;
equality to104 or32 would be an incorrect test. Fill bytes can remain untouched
inside reserved frames, so high-water is an observation, not a bound.

T30.07.a must preserve historical800 as its stated synthetic model. The actual
target uses three1024-word stacks plus optional fourth, two/three work roles and
controller, actual TCB/control/queue/payload sizes and separate idle allocation.
The map includes other kernel globals and padding. Host sizes are separate from
target sizes. No hidden heap or runtime library is admitted by the strict link;
the explicit byte helpers are inspected dependencies, not forbidden surprises.
The call graph and compiler .su files expose local frames; indirect or cyclic
paths without a bound remain unknown and budget admission must refuse them.

T30.03.c's cooperative replacement imports actual sc_schedule_next. First the
consumer blocks, then a start gate wakes the producer, publication wakes consumer,
and ordinary alternating work completes. Two terminal steps plus the initial
blocked step explain2*n+3. Every stop cut drains all admitted values. Preemption
and ISR feature requests preserve output and returnUNKNOWN. This comparison
does not equate explicit polling with priority preemption or claim to completeT50.

Assess each of six public rubric rows separately. Require the reason for every
ownership edge, independent expected arithmetic, failure evidence and exact
changed-condition identities. A passing reference campaign is author support;
no independent reviewer, physical target result or learner level is manufactured.
