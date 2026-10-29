# X-T75-002 required owned-pressure and two-producer credit successor

This is required support for T75.02.a-d and X-T75-002-DELAYED. Keep the original
exercise rubric, exact inspected gRPC flow-control and resilience references,
shared exercise contract and13-dimension cost contract. Complete X-T75-001's
owned scope and the actual T73 `conc.channel` producer first. The original
RZQueue/RZOffer/rz_forward implementation remains the copied-message producer.
The containers.ring and io.reactor prerequisite contracts inform the bounded
queue/readiness analysis; this successor does not invent an API for either.

## Supplied interface and execution

Implement `src/pressure_follow.c` using `followthrough/pressure.h`, the public
`pressure_test.c`, original resilience core and actual MsgHub. No ready-made
flow-control library may implement the studied mechanism. C17 hosted Linux
uses pthread mutexes/conditions for the explicit adapter synchronization.
CMake, Python independent models and sanitizers are host tools. GCC/Clang
debug/release and instrumented configurations are declared in the family.
There is no freestanding or physical-device execution claim.

The supplied work is two related representations. `struct rzp_owned_queue` preserves
the original five overload policies while owning resources associated with
their unsigned values. `struct rzp_pump` implements the blocking policy with two
bounded registered producers and four credits across queue, transport and
receiver leases. The pump must call actual `rz_offer`, `rz_forward`,
`msg_select` and MsgHub lifetime functions. It may not fake transport acceptance.

## Owned policy contracts

`struct rzp_value` consists of an unsigned semantic value, nonnull owned pointer and
disposer. It is movable by a single descriptor transfer, not safely copyable
as an owner. `drop` clears the descriptor before calling the disposer. The
disposer must finish normally, be valid for the pointer and never reenter or
mutate the owning queue/pump. Context objects outlive all owners. No raw alias
may free or change the payload behind an admitted owner.

For the simple owned queue, valid offer admission consumes and clears the
input on every terminal status; BLOCK/PENDING retains the caller's owner.
Malformed pointer/policy/scope admission preserves it. A terminal REJECT,
DROP, cancellation, expiry or closed offer disposes the input exactly once.
OVERWRITE disposes the oldest queued owner and admits the new owner. DEGRADE
retains the resource associated with the larger newest value: replace and
dispose the prior newest only if the new value is larger; otherwise dispose
the input. Equal values preserve the older owner. This explicit disposer
policy is an extension of the copied-value contract, not an assertion that
the original `struct rz_offer` itself frees arbitrary objects.

Pop transfers one owner to an output that must own no prior resource; failure
preserves output. Closing rejects new offers while allowing complete draining.
The simple queue is externally serialized and has no hidden waiting list.
The public policy oracle disposes a caller-retained PENDING value explicitly;
the separate pump below demonstrates retaining that pending owner instead.

## Two-producer representation and protocol

The initialized pump is stable-address and noncopyable. It owns a mutex,
condition, actual MsgHub with channel0 capacity1..4, a local four-entry queue,
four payload/credit slots and exactly two producer records. No external caller
may directly operate its embedded hub or mutate its fields. Each producer
identity0/1 has at most one active operation, including an uncollected terminal
result. Each slot has a nonwrapping generation, payload owner and one state:
FREE, QUEUED, TRANSPORT, LEASED or RETIRED. The queue and actual channel carry
slot indices; the generation stays attached to the stable slot owner.

`begin` preflights valid input, monotonic virtual time, available producer
record, nonretired generation and open state. Admission errors preserve input
and ticket output. Once registered, it moves the owner, retains a scope borrow
and publishes an owner/index/generation ticket **before** testing readiness.
An immediately terminal accepted/refused operation still requires result
collection. Thus FULL from consumed-counter exhaustion is a terminal result
with an actual ticket and consumed input, whereas producer-record exhaustion
returns FULL with unchanged input/output. The ticket distinguishes these cases.

Under the same mutex, progress checks cancellation/expiry, then close, then
the nonwrapping consumed counter, then free credit. Admission moves the owner
to a free slot, increments that slot generation and the consumed counter,
decrements available credit, queues its index via the actual RZ producer and
releases the pending scope borrow. Already admitted queued/leased payloads no
longer depend on the offer scope; cancellation cannot retroactively remove
them. Pending records are considered from the rotating next-producer index.
This is a bounded deterministic policy, not an OS scheduling fairness theorem.

`poll` returns PENDING without changing result output, or collects the terminal
result once. Its blocking form loops on the predicate under the same mutex as
registration/progress. A second poller for the same operation gets BUSY;
cancel/tick/credit-return may run concurrently with its wait. A consumed or
old-generation ticket returns STALE. `cancel` only terminates a pending offer;
an already terminal admission returns DONE. Scope cancellation must be followed
by `tick` or another progress entry to wake a waiting adapter; an atomic flag
alone is not a condition notification. POSIX cancellation of an API thread is
outside this contract; stop via the ticket protocol and join it instead.

`forward` uses actual nonblocking channel admission and preserves the local
head on FULL. It changes QUEUED to TRANSPORT only after acceptance. `receive`
removes the slot index from the channel, changes it to LEASED and publishes a
borrowed view plus an owner/index/generation credit token. The receiver must
not drop this borrowed descriptor. Neither forwarding nor receiving returns
credit. Only `release` for the exact current LEASED generation disposes the
payload and returns receiver capacity. It then wakes/rechecks pending
producers. Duplicate, stale, forged-owner and nonleased releases return STALE
without increasing credit. Tokens are bounded teaching identities, not
authentication against arbitrary memory modification.

Initially four credits are granted. For ordinary nonretired slots,
`credits = 4 - consumed + returned`, and
`credits + queued + transport + leased = 4`. Counters never wrap. A slot
whose last admitted generation is UINT_MAX retires when released; its capacity
is deliberately not regranted. Including retired slots, the second sum is4;
the first still counts only usable grants. Refuse new admission when the
consumed counter reaches UINT64_MAX. At most four admitted payloads plus two
pending owners exist: a fixed queue by itself did not establish this bound.

Closing terminates pending offers and stops new begins, but admitted payloads
drain through the real channel. The adapter closes channel0 after its local
queue drains. A valid receiver lease prevents destruction. Destroy requires
closed state, collected producer results, no admitted payloads and all API
threads joined; refusal returns BUSY without destroying synchronization.
Do not reinitialize the object while old tokens/callers can survive. Mutexes
protect ordinary fields; ISO C alone provides no affine ownership, cleanup,
capability authentication or fairness.

## Required work, verification and rubric mapping

- D1 / R1: draw ownership transitions for all five policies and distinguish
  application queue acceptance, channel acceptance, receive and capacity
  release. Give a counterexample to returning credit on either earlier event.
- D2 / R2: implement both owned policies and actual channel adapter. Preserve
  ordering, closed draining, borrowed receiver views and exactly-once disposal.
  Compare full DROP9, OVERWRITE9 and DEGRADE10 against the original examples.
- D3 / R3: implement bounded registration, result collection, cancellation,
  explicit credit identities and finite retirement. Test a full four-slot
  pipeline with two blocked producers, a third begin on an occupied identity,
  registration followed by receive/release before polling, and actual two
  pthread waiters. Demonstrate that close/expiry wake and dispose pending
  owners but leave already admitted owners to drain. Reject duplicate and
  old-generation returns without credit inflation.
- D4 / R4: compare the common eight-offer/four-pop schedule for every policy.
  Report accepted, rejected, dropped, coalesced, displaced, queued and pending
  separately; do not equate an accepted overwrite with lossless delivery.
  The direct-policy experiment disposes caller-retained pending values; state
  that choice when contrasting it with the pump's retained owners. Add the
  delayed duplex experiment and the full cost dossier.

Run `resilience.pressure_owned`, all four named semantic controls, and
`tools/verify_resilience_pressure.py --driver <build>/resilience-pressure`.
The independent models check46650 owned-policy histories and74904 pump
histories, including longer full-capacity traces. Actual positive tests cover
two waiting native threads, generation/counter exhaustion, duplicate credit,
output preservation, all disposer balances and scope deallocation. The four
controls omit registration, accept a duplicate credit, leak an overwritten
owner and return capacity on receive. Each must fail its intended assertion
after the positive passes; an unrelated build/runtime failure is not success.

Pass requires exact model and observed event agreement plus the proofs above.
Level1 is nominal policy output without ownership/credit reasoning. Level2
requires the actual adapters, boundaries, independent model, semantic control
rejections and justified resource bound. Level3 requires independent changed
assumptions and delayed transfer; source-bound author execution does not
award learner mastery. Graduated hints: list every owner first; then retain a
credit through transport reception; finally put registration and predicate
recheck under one synchronization rule. Private analysis is separate.

## Delayed transfer, security and evidence limits

X-T75-002-DELAYED uses two endpoint actors, each with four full outbound
credits and a fifth registered offer. Both actors synchronously wait for send
completion and neither receives. Construct the wait-for cycle with snapshots
and actual waiting threads; do not hang the assessor indefinitely. The public
fixture supplies a separate receive-progress agent, receives/releases one
lease in each direction, observes both offers complete, joins both actors and
drains every remaining owner. Explain why forwarding alone cannot break this
cycle. The independent delayed assessment must change capacity/close order and
arrive after the required interval, with no copied worked solution.

The only security target is this original in-process queue/credit model with
allocated synthetic payloads. Adversarial inputs are bounded bursts and
duplicate/stale credit tokens; the defect is capacity inflation, hidden pending
memory or premature/double cleanup. The harmless success observation is an
incorrect credit or disposer count. Exploitability requires the original
model's admitted token transitions; arbitrary writes or malicious callbacks
are not simulated authenticated clients. Repair registration/identity/release
rules and retain the four regressions. No external connection, real data or
host setting is involved, and no sanitizer result is labeled arbitrary code
execution. Finite-generation retirement, caller lifetime discipline and
external scheduling remain residual limits.

Only the source-bound campaign establishes which profiles actually ran. These
finite histories and native schedules do not prove all interleavings, physical
throughput, a different data model or independent learner mastery. Exact C17
text remains an explicit source gap. Other T75 investigations remain required.
