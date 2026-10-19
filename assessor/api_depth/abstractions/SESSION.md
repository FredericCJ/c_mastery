# T19 session and table assessor analysis

T19.01.a: The384-cell declarative relation covers four states,16counts,sixevents.
The increment row admits only active count<15. Start resets the NEW counter and
acquires resources; stopped never implicitly restarts. Suspension retains mask3
and rejects increments. Stop/cancel converge to stopped and release exactly once.
Counter/state/resource validation is separate from C object validity. An active
tag with missing ledger resources is rejected before work.

T19.01.b: At construction callback, ready=1, mask1 and stateNEW. The observer
needs initialized fields but cannot treat the object as active. Moving stateACTIVE
before acquisition1 is the deliberate publication mutant; a valid repair changes
publication ordering, not observer assertions. Reserving four log events before
effects guarantees a rollback can record its release. A failed second acquire
has acquisition/release counts(1,0)/(1,0); success then stop has(1,1)/(1,1).
Repeated stop changes neither count. A callback that retains the partial object
needs a new lifetime/acknowledgment contract and is not supported by this version.

T19.01.c: Plain assignment count16 is legal C but invalid abstraction state.
Incomplete `AsOpaque` prevents ordinary member access by the client. It cannot
prevent double-destroy/dangling pointer calls, malicious casts, races or ownership
copies. Complete typed `AsSession` supports aggregate embedding with no allocation
and exposed layout; private allocated state hides layout but costs one allocation.
Destroy returns failure retaining ownership if it cannot complete cleanup;
successful destroy reads allocator into a local before consuming memory.

T19.02.a: Callback type adaptation is an ordinary exact-signature wrapper around
the actual context type. Function-pointer casts do not establish compatibility.
The second adapter increments its extra-context call counter, checks exhaustion,
then calls the same state core. Its counter changes on an admitted invocation
even when the session operation rejects: this instrumentation policy is explicit
and separate from session unchanged-on-failure. Test lifetime of both layers.

T19.02.b: Shared descriptor memory is paid once; three backend values each store
two function pointers and one context pointer after validation. Direct descriptor
contains header+one function pointer; v2 contains header+two. The expanded backend
copies validated slots, so distinguish source descriptor sharing from downstream
adapter representation. The copied-table comparison pays three descriptors even
though behavior is equal. No portable byte size is assumed; use measured sizeof
and alignment. Runtime variation can be per-context or per-table; neither is
universally faster. A static backend is valid when runtime variation is absent.

T19.02.c: Extent checks precede each memcpy into a correctly typed local. Objects
must originate from compatible native descriptors containing valid callable
pointers. Header parsing is not a deserializer for adversarial pointer bytes.
Known-version compatible-prefix extension succeeds; changed version and unknown
flags reject. Optional-null flush returns unsupported; required-null run rejects.
Do not read the flush slot just because storage happens to be longer than the
declared represented extent. Preserve output on every rejection.

Alternative solutions may use separate opaque v1/v2 descriptor constructors,
typed unions or factory validation with stronger capability handles. Assess them
against all original contracts, including short extent rejection and explicit
source/ABI/lifetime cost. A nominal passing state trace without the failure and
changed-condition arguments is at most1. Score each of the six leaves separately
with the public0–3 rubric and original delayed variant. Finite native results and
compiler diagnostics are bounded author evidence; no independent mastery claim.
