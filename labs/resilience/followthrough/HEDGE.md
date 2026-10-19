# Required T75.07 successor: concurrent owned responses

This is the implementation and changed-condition support for X-T75-007-R1--R4
and X-T75-007-DELAYED. Keep the original RZHedge model and its tests. Implement
`src/hedge_follow.c` against `followthrough/hedge.h`; the new anchor is
`resilience.hedge_owned`. Bring X-T75-001's owned scope, X-T75-006's actual window
gate, and the retained RZHedge/RZReceiver implementation. The family explicitly
imports the T73 channel needed to link retained resilience code; this arbiter's
mutex is a POSIX dependency, not an ISO C feature. Host Python is an independent
verification tool. No hedging, ownership, decoding or resilience library may
implement the studied mechanism.

## Input, representation and API

Use two attempts: original0, delayed1. A stable, noncopyable RzhArbiter owns one
mutex, one RZHedge, the actual RzbGate with two tenant0 permits, one RZReceiver,
one request-scope reference, and at most one winning variable-size blob. Tenant1
is unused. Preserve gate health across successive requests; policy is an eight
completion window, minimum8, failure threshold1/1. Attempt permits each cost1.
The request's client0..1, identifier1..15, fence>0 and receiver retention16 use
the actual receiver contract. Both attempts use exactly the same identity and
intent. There is no durable identity allocator in this increment.

`init` takes a clock callback and a fallible decoder plus their borrowed context.
The clock returns nondecreasing uint64 ticks; all reads occur under the arbiter
mutex. Its origin and rate equal the request scope's clock. The decoder takes a
borrowed byte span, returns RZ_OK only after validating the whole response, and
publishes a decoded unsigned value only on success. It must reject the
all-empty failed/no-response representation
with a non-OK result; an application where an empty response is meaningful needs
a different explicit response representation. It may run concurrently for
the two attempts, so its context must support that. It runs outside the arbiter
mutex and can be deliberately paused in the harness. Callback objects, the
arbiter and context remain live until all calls have returned and threads joined.
Callbacks must not reenter, mutate, move or destroy this arbiter, throw across C,
or retain the borrowed bytes. No API supports POSIX thread cancellation inside
an active call. Cleanup and decoding must eventually return to obtain quiescence.

`begin(scope, delay, client, id, fence, out)` retains the scope and admits the
original permit before publishing an owner/generation/attempt ticket. It requires
the previous request quiescent and its result moved or discarded. It refuses
generation UINT64_MAX rather than wrapping. It rolls back its extra reference
if the gate refuses. Invalid input, reference exhaustion, expired/cancelled
scope, unavailable permits, delay overflow, or a busy previous request do not
publish a ticket or replace the previous request. A clock observation may advance
the recorded observation. An initial mutex-creation failure returns RZ_FULL and
does not create an initialized arbiter. Never destroy or use that failed object.

`start(out)` observes the current clock, stages the actual RZHedge transition,
and admits the second actual permit before publishing either. Before hedge_at
it returns PENDING; after a winner or an already started hedge it returns DONE.
An abandoned request returns CANCELLED. No active request returns STALE.
The imported scope check takes precedence over the retained core's DONE check.

`effect(ticket, delta, out, duplicate)` uses the actual RZReceiver under the same
mutex. Delta is -8..8. A completed/claimed ticket is STALE; cancellation,
abandonment or expiry refuses an effect. Duplicate live identities return the
stored original result without a second effect. Different intent is CONFLICT.
Example: both attempts apply delta3, returning3 twice with duplicate=false then
true, while total remains3. Finite retention can permit a later replay again;
the caller must bound all overlapping attempts/replays to that window or supply
the durable extension. Neither a mutex nor hedging makes arbitrary effects safe.

`complete(ticket, blob)` accepts either a nonempty owned span with a disposer,
or the all-empty data/size/disposer representation for a failed/no response.
Empty context is allowed. The descriptor must be distinct from every control,
other descriptor and output object. On STALE, BUSY or malformed admission the
caller still owns it unchanged. On admitted completion the descriptor becomes
empty, and this call owns the bytes through decoding and disposal or winner
transfer. Another concurrent completion of that attempt is BUSY while claimed;
after cleanup it is STALE. The ticket must match this live arbiter and current
generation. Old tickets are invalid after destruction/reinitialization even if
the address is reused; generation checks cover reuse within one initialization.

Acceptance is decided only after decoding, under the mutex, using a fresh clock
read and the request's abandonment state. First acceptable response strictly
before deadline wins. A malformed or late decoder cannot win. Once chosen, no
later response replaces the value or ownership. The return status follows the
actual core: OK for the winner, DONE for a rejected/nonwinning response,
EXPIRED/CANCELLED when the shared scope rejects at publication. Failure of the
decoder is an unacceptable response, not a C exception or allocator failure.

Every losing/rejected payload is disposed exactly once outside the mutex.
Its actual gate permit and scope reference remain owned until disposal returns.
Only then is the permit completed and the callback count decremented. Successful
decoding records gate success even if another valid response won; failed decoding
records failure; scope expiry/cancellation or abandonment is neutral. The health
outcome is snapshotted at the post-decode publication point. Expiry, cancellation
or abandonment while subsequent disposal is blocked does not revise that health
observation; disposal still retains the permit until it returns. The winning
payload moves into the result and is no longer backend-owned, so its permit can
complete while the client still owns the result bytes.

`take(out)` moves the winning blob/value/attempt once. Output must own no prior
blob. Before a winner it returns PENDING, or CANCELLED for an abandoned request;
after a move or discard it returns DONE. Failed queries preserve output.
`abandon` requests cancellation, discards any untaken winner outside the mutex,
and is idempotent. It does not cancel the shared scope for unrelated users and
does not terminate outstanding workers. A moved result remains the caller's.
`cancelled` queries a live attempt; cancellation cannot revoke an already applied
effect. `snapshot` is a coherent diagnostic copy, not an exposed mutable lease.

`quiescent` means all started attempts completed AND every callback returned AND
every actual permit was released. Result availability alone does not imply any
of these. `destroy` additionally requires no retained result. All callers must
have joined before destroy; a successful query is not a lifetime lock against a
new caller. Owner references from the request persist until replacement/destroy.
No raw pointer may access storage after release. These lifetime prerequisites
are programmer discipline; ISO C does not enforce them.

## Invariants and required investigation

Derive `finalized subset completed subset claimed subset started subset {0,1}`;
at most one winner; gate permits equal started minus finalized; and blob owners
partition into caller, admitted completion, winning result and disposed. The
callback counter includes a blocked loser disposer and the first abandonment
disposer. Abandonment is idempotent, so there are at most three such callbacks.
Publication and final cleanup are separate linearization points. A mutex protects
metadata but never grants permission to destroy still-running callback storage.

Prove stale-generation rejection before input consumption. Trace the case where
both completion bits are set but one disposer remains blocked: quiescent must
still be BUSY, its permit still held, and begin/destroy refused. Compare that
counterexample with the original serialized completed-mask predicate.

The supplied synthetic response decoder uses tag0/1, marker0xa5, unsigned32
big-endian value, then bytes equal to their offsets modulo256. It validates size
and all trailing bytes. This is an original teaching wire format, not a protocol
claim. A decoder can pause before reporting failure while the other worker wins.
Allocate different response lengths and count every exact disposer invocation.

## Verification and independent transfer

Run both `resilience.hedge` and `resilience.hedge_owned`, plus the three compiled
controls. Their defects are accepting failed decoding, treating completed masks
as quiescence before cleanup, and dropping the request-generation check.
The controlled pthread harness runs both completion orders for all four
acceptability pairs, 32 simultaneous races, failure after a competing winner,
expiry equality, scope cancellation, abandonment, blocked cleanup, admission
failure, repeated completion, unstarted attempts, and stale request reuse.
ASan/UBSan and a separate TSan profile are required; instrumented runs are evidence
for those executions, not a proof of all interleavings.

`tools/verify_resilience_hedge.py --driver <build>/resilience-hedge` independently
enumerates 7,388 post-decode histories with effects/replay, rejection, abandonment,
deadline equality, repeated completion and result moves. It also executes16,384
actual C request lifecycles under fixed independent versus perfectly correlated
heavy-tail delay samples, with/without a hedge at tick4 and common deadline20.
Both policies have capacity2; the unhedged policy uses one. Losers ignore
cancellation and finish, so backend work includes them. Report successful latency
quantiles together with timeout counts, attempts, work and quiescence durations.
Do not hide timeouts by reporting only surviving-request quantiles. These are
virtual ticks and duration work proxies, not host throughput or CPU measurements.

Before a fresh measurement, seal whether hedging is justified under each delay
model and which work/capacity budget it may consume. Explain why correlation can
remove all latency benefit while adding backend work. Record all13 modern cost
dimensions; later service integration must also charge every admitted hedge to
its common limiter. This arbiter alone enforces permits, not a global work rate.

Pass requires exact owner/disposer counts and expected results, no premature
quiescence/publication, every named control rejected by its intended assertion,
and an explanation for each R1--R4 criterion. For the delayed independent attempt,
change the rejection boundary from the header to the final payload byte and
hold a losing callback past client abandonment; reuse the arbiter only after
quiescence, then replay the old ticket. First explain the ownership proof without
copying the assessor implementation. The supplied variable-size late-failure
variant is preparation, not evidence that a learner independently passed it.

Scope is original local C programs, synthetic bytes and controlled pthreads.
Malformed responses may violate acceptance or lifetime properties in the named
mutants; no external service, credential, deployed device or real user data is
involved. The harmless observation is wrong winner, premature quiescence, or
stale completion of a fresh request; it is not an arbitrary-execution claim.
The source records in X-T75-007 bound comparisons to inspected hedging,
idempotency and cancellation passages. Exact published C17 access, independent
learner assessment, physical timing and production transport equivalence remain
separate evidence limitations.
