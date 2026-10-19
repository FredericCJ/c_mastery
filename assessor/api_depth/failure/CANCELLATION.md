# Private T21.06 analysis

06.a requires separate timeout-winner and quiescence variables. The model evaluates
5461 words over deadline,ack,complete,joined through length6. `AE_MUTANT_EARLY_RELEASE`
reports at timeout and fails. No-ack cancellation has no finite safe reclamation
bound without a new environmental guarantee. Keeping ownership indefinitely is a
correct safety answer with an explicit liveness/resource limitation.

06.b's heap receiver really frees operation storage. All needed fields and payload
are local before receiver invocation. The hosted variant races256 completions
against timeouts with a stable routing control; either winner is permitted. Do
not require both winner counts in a finite scheduling run. The publication gate
orders initial data, the mutex orders operation access/nulling, and join protects
outer-control lifetime. TSan/ASan observations support only the selected executions.

06.c checks actual FAsync release counters across generations1,2,3; a fourth start
refuses. `AE_MUTANT_STALE` accepts an old generation and is rejected. Resetting a
counter while old tokens escape is not retirement. A new control lifetime is valid
only after all old references and callbacks are impossible. The API does not
enforce this globally through ISO C's type system.

The delayed packet changes completion to another thread and removes cancellation
acknowledgment; require the learner to justify a stable routing owner and backend
join or admit that reclamation cannot be bounded. This supplied implementation
executes the modeled join and actual thread protocol, without claiming an OS
I/O cancellation primitive. Award independent transfer only after a real learner
attempt and the original minimum seven days, not after author execution.
