# Channel-to-actor owner and incarnation boundary

This required X-T73-004-R2/R3/R4 successor consumes the actual `MfcHub`,
`MfaActor` and unique `MfoValue` APIs from X-T73-002 and X-T73-004. Implement
`src/actor_executor_follow.c` against `actor_executor.h`, without replacing
channel or actor behavior with a mock queue. Hosted C17/POSIX assumptions and
the existing `OWNED.md` callback/owner contracts apply. Inputs are original
synthetic messages, actor incarnation numbers, close/cancel transitions and
two destinations. Outputs are explicit send statuses and per-dequeued-message
`MfeEvent` records; failure to dequeue preserves the event output.

## Representation and ownership

The executor contains two actual actors, an actual two-channel owned hub and
eight stable envelope records. Each envelope stores destination, the submitted
generation and a unique payload. A separate pool mutex protects envelope slot
allocation/release and published incarnation observations. Actors are owned
and manipulated by **one executor thread**, which alone calls pump, restart or
destroy. Other threads may send, read the published generation, close or cancel
while the executor remains live. The actor fields are not a concurrent public
mutation interface. A generation read is only an observation; a later send can
legitimately become stale before dispatch.

Channel capacities are0..4, including the original rendezvous contract. Envelope
capacity is8 even when several senders wait. FULL from either pool or channel
preserves the caller's payload, including a blocking send that cannot reserve
an envelope. Callers may retry under their stated scheduling policy. A send
which returns OK transfers its unique payload to the executor; later actor
failure or reincarnation cannot return that ownership to the former sender.
All failure statuses preserve the caller's descriptor. The descriptor is
exclusively borrowed for the entire send call; it and the executor remain live
until the call returns. No input/output descriptor may overlap executor storage.

The receiver may dequeue, dispatch and destroy a payload **before the successful
sender returns**. Therefore the successful sender clears only its own descriptor
and never inspects the envelope after channel publication. On failed submission
there was no accepted receiver, so the sender can reclaim its private envelope
without destroying the caller's payload. The underlying channel's separate
send-complete/delivered bookkeeping remains necessary.

Pump detaches the accepted payload and copies its generation/destination into
locals, releases the envelope slot, then invokes the actor. A successful actor
admission is immediately followed by one run-to-completion step. Its callback
may enqueue another message through a **nonblocking** executor send; it must not
recursively pump, restart, destroy, directly mutate either actor or use a
blocking self-send. This keeps mailbox execution nonrecursive. Callback contexts
remain alive until all executor operations finish. All callbacks return normally.
Payload destructors and dead-letter callbacks must not reenter this executor.

A stale incarnation transfers to the actor's explicit dead-letter sink. A
channel-accepted message whose actor is now dead also transfers there; it is
not silently leaked or presented as an unaccepted send. Pump returns OK for a
dequeued event and records actor OK/FAILED/STALE/CLOSED in `event.delivery`.
Its return EMPTY/CLOSED/CANCELLED concerns the input channels and preserves the
event. Restart is executor-owned and only valid for a dead actor; it publishes
the new generation after successful construction. Generation exhaustion retires
that actor rather than wrapping. Already queued envelopes retain their original
generation and cannot be retagged for the replacement actor.

Close drains accepted channel messages. Cancellation prevents further dequeue;
destruction explicitly drops queued owners. Destroy requires an external gate
preventing new callers and all producers eventually joined. It returns BUSY
without teardown while a channel operation is active or an actor callback is
running. Cancel, join users, and retry. Once channel destruction succeeds,
destroy both actors and only then the envelope mutex. Cooperative cancellation
does not unwind thread stacks; POSIX `pthread_cancel`, abnormal callback exit,
owner death and process termination are outside this profile. Failure while
initializing synchronization objects leaves no usable executor and no payload
ownership. Post-initialization mutex failures are outside the valid-call model.

## Required evidence and assessment

Derive the send/publication/dispatch/destruction ownership graph and every
linearization boundary; account for the envelope's reuse independently of its
payload lifetime. The eight envelopes bound admission metadata, not arbitrary
payload size or the duration of a stalled sender. There are no executor-owned
heap allocations: payload allocations belong to callers, and pthread runtime
resources remain host dependencies. Pump uses bounded metadata work plus user
callback cost; blocking channel operations have no unconditional finite time
bound. Exposed C structs do not enforce unique ownership or single-thread actor
access; those remain caller requirements with targeted runtime checks/tests.

`messaging.actor_executor` checks repeated envelope reuse, a self-enqueued second
message without recursive dispatch, an effect that survives handler failure,
closed-actor and stale-incarnation dead letters, full-pool refusal, cancellation
with retained owners, actual accepted/unaccepted rendezvous, BUSY teardown and
generation retirement. Two actual producers send2,000 heap messages; the single
executor checks every delivery exactly once and compares each state transition
with a separately mutex-protected reference under the same observed event order.
The reference comparison establishes ordering/value behavior on that trace,
not equal queueing latency or a scheduler fairness proof.

Two compiled controls omit envelope release or retag an old queued message with
the current generation. Both must fail named semantic assertions; compiler
errors, hangs and sanitizer termination do not count as accepted control
rejections. The independent Python oracle enumerates all words through length5
over generation1 send, generation2 send, failing-handler send, pump, restart,
close and cancel, checking delivery/ownership counters and final cleanup. This
sequential state-space check does not explore every concurrent schedule.

Submit implementation, ownership/progress reasoning, actual test/oracle evidence,
and a comparison with direct shared-object locking. Hints: (1) place incarnation
identity in the queued message, not just the destination; (2) separate envelope
and payload ownership; (3) forbid post-publication sender reads of reused
envelopes. Superficial work forwards only unsigned values or ignores stale
replies. Understanding explains every failed-send and dead-letter transition.
Independent mastery repeats the original delayed late-reply assessment with
changed callback/admission timing and defends why no previous-incarnation reply
can mutate replacement state. The assessor analysis and implementations remain
separate from this learner scaffold. Only current source-bound reports establish
actual author execution; they do not certify learner mastery or physical targets.
