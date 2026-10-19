# Required T75.09 composition: transport work, durable effects and quiescence

This shared contract extends X-T75-009-R1--R4 and X-T75-009-DELAYED. Implement
`src/composition_follow.c` against `composition.h`; run
`resilience.service_transport`, both compiled controls, and the independent
`tools/verify_resilience_composition.py` oracle. It composes the actual scope,
fractional limiter, wheel, transport retry, owned hedge, journal, outbox and
MsgHub implementations. The adjacent SERVICE_IO contract supplies actual Linux
socketpair partial I/O and cancellation; this controller supplies reproducible
transport fault histories. Neither observation substitutes for the other.

## Prerequisites and dependency boundaries

Complete X-T75-001 through X-T75-008 and the OUTBOX/SERVICE_IO portions of this
exercise. Supply accepted learner `conc.channel` (PR-T73-02),
`communications.tcp_fault` (PR-T41-06 / LAB-EX-09), and `io.deadline` / `io.reactor`
(PR-T24-05 / PR-T24-04). The imported reactor is only its documented completion
byte aggregator; the new frame parser and send loop are original adapters. The
transport uses actual NetStack retry state, retained bytes, net_receive ACK
validation and v4_tcp_delivery. No networking or resilience library implements
the studied mechanism. Python heap/map models are host verification tools.

Use C17, hosted Linux, 8-bit octets, signed32 int and available uint64_t, with
GCC/Clang and explicit POSIX synchronization dependencies. No freestanding,
physical board, real network throughput or general TCP implementation claim is
made. The supplied established-flight fixture omits the TCP handshake; its
retained outgoing payload is the 18-octet application frame. Incoming completion
is an actual checksummed Ethernet/IPv4/TCP ACK passed through net_receive.

## Inputs, outputs and valid calls

`rzc_run(config, journal_device, ack_device, allocator, out)` runs one synthetic
logical request `(epoch1,client0,id1,delta3)`. Both devices initially contain zero
bytes and satisfy JOURNAL/OUTBOX's independent-storage callback contracts. They
and allocator context remain live for the call, with no concurrent access,
reentry, retained borrowed spans, longjmp or thread cancellation. Output storage
is live, suitably aligned, disjoint from input/control/device storage, and owns
no prior resource. The allocator obeys SCOPE/HEDGE, including matching frees and
nonoverlapping successful allocations. These are caller obligations, not facts
proved by a pointer type.

Configuration has two 4-bit loss masks, two response delays1..24, deadline1..16,
consumer start0..32, cancel tick0..16 (zero disables cancellation), capacity1..8,
Retry-After0..1501 milliseconds, and hedge/restart booleans. Bit k drops dispatch
k for that attempt; up to one original and three retries are possible. The
response delay starts at its first accepted delivery. Invalid input returns
INVALID without publishing output. Failure returns the underlying status,
preserves output and releases owned resources, but does not roll back device
effects. In particular UNCERTAIN requires device recovery before reuse.

The result distinguishes started attempts, dispatched work, retries, losses,
admission denials, newly applied effects, replays, delivered notifications,
durable acknowledgments, restart count, response allocation/disposal counts,
remaining tokens, winner and total. `winner=-1` means no acceptable in-scope
reply; `reply_at=0` alone is not a success indication. Tick fields are virtual
events. Timer work fields are diagnostic operation counts, not CPU cycles.

## Order, ownership and invariants

The scope and transport clocks use milliseconds. A controller tick is1000ms;
the journal's monotone clock is the tick count. Every original, retry and hedge
consumes one shared token. The configured rate is1/65535ms, so the bounded
history receives no whole refill before its work stops. Refused admission does
not debit credit. Each retry uses the actual Rzr random generator, full jitter,
Retry-After and retained transport RTO; scheduling rounds an absolute due time
up to the next wheel tick. Rounding down cannot authorize an early dispatch.

Schedule the scope deadline and retry/hedge events on the actual two-level
wheel. At each tick: publish configured cancellation; complete due responses;
take a newly available winner; expire ordered wheel events; observe quiescence;
forward, deliver and acknowledge notifications. Responses at the deadline are
already expired according to the scope contract. Same-tick responses are tried
in attempt order0 then1. The optional hedge decision occurs at tick2, if still
in scope, without a winner and with work credit. A failed original response
does not make the request quiet before that decision. A winner cancels future
retry/hedge work; it does not erase an already owned response or permit.

Each delivered request reaches the actual frame decoder and durable outbox.
Only the first equal-intent request applies delta3; later deliveries replay its
retained result. Each attempt allocates at most one owned18-byte reply. A reply
allocation failure is a failed response, after a possible committed effect.
The arbiter accepts only a valid successful nonnegative reply from its original
decoder contract. ACK validation/transport finish precede response completion;
this is the explicit backend quiescence event. An attempt with no delivered
response is reset one tick after the wheel observes the deadline, discards its
old transport and completes empty. A delivered late response remains owned
until its configured completion, even after cancellation or another winner.

Notification forwarding uses real MsgHub channel0. Consumer delay does not
return the four outbox credits early. With `reset_before_ack`, wait until all
attempts/callbacks/permits are quiet, reconstruct journal/outbox from the same
devices, and redeliver the same durable event before ACK. Do not byte-copy or
reinitialize a live pthread object: the already empty channel survives this
logical service restart. OUTBOX's separate fork experiment performs actual
process death; this model restart is not that experiment.

Derive conservation: `dispatched + tokens = capacity`; total is0 or3;
applied<=1; every allocated reply has exactly one disposal; each applied event
has one durable ACK and one delivery, or two deliveries if the pre-ACK restart
is selected. The stronger claim "a committed effect implies a successful
reply" is false. The stronger claim "durable outbox means exactly-once external
delivery" is also false. Exactly-once new-event creation within the retained
identity window and at-least-once notification delivery are distinct contracts.

## Concrete expected results and failure investigation

No drops, delays1/1, deadline10, capacity4, no hedge/restart: one dispatch,
total3, winner0, reply/quiet tick1, one delivery/ACK, three tokens. Drop original
dispatch0 with Retry-After1501: the retry occurs no earlier than tick2 and the
reply is tick3. With delays4/1 and hedge, winner1 replies tick3 while original
cleanup holds quiescence until tick4. Add pre-ACK restart: two deliveries of the
same event and one ACK. Correlated losses can spend extra hedge work without
improving the outcome; derive this from the complete trace, not its title.

Run 4096 systematic loss/deadline histories and5000 independently generated
mixed histories. The Python oracle uses ordered event tuples and logical maps,
not the C representation; compare all19 exported oracle fields. Inject each
scope/response allocation failure and45 journal write refusal positions;
require balanced ownership and unchanged output on failure. Additional journal
and ACK durability cuts, native response races, blocked socket sends and real
process restart belong to their named adjacent tests and remain mandatory.
Reject the floor-deadline mutant and lost-recovered-notification mutant.

## Deliverables, hints and assessment

Submit an event/owner ledger at every listed boundary, the implementation,
oracle traces, generated-code dependency evidence and an equivalent-contract
cost dossier. Explain why cleanup may finish after the client deadline, why a
reset must preserve durable sequence identity, and which assumptions make the
finite loop terminate. Hints: first draw the permit/response owners; then keep
the journal commit distinct from reply publication; finally round transport
time into the wheel without granting early work. Separate assessor analysis is
not learner starter code.

R1 requires the complete event and owner invariant; merely naming the
components is level1. R2 requires actual imports and checked adapter boundaries,
not a second mock of NetStack or MsgHub. R3 requires exact counterexamples and
failure accounting, not only a sanitizer pass. R4 requires the sealed decision,
all13 cost dimensions and changed-condition analysis. Level3 remains a delayed
independent learner attempt; these author executions cannot award it.

For the delayed variant change response-delay correlation and restart after
delivery before ACK; retain equal durability/ownership contracts when comparing
alternatives. Investigate another clock quantum or identity-retention policy:
change the conversion/expiry proof and expected oracle, rather than merely
substituting a different delay constant. Physical clock, storage and network
models need new evidence if selected; current host counts cannot establish them.

Only original local teaching objects, local socketpairs and synthetic requests
are in scope. Adversarial influence is bounded loss/reordering/delay/overload,
malformed18-byte frames and fixture storage faults. Harmless success is a lost
notification, duplicate effect or violated capacity in the faulty target.
Prerequisites for each violation are its stated fixture model; a timeout or
sanitizer finding is not arbitrary code execution. Restore the invariant and
retain the mutant-specific regression. No external target or credential exists.

Status: authored and executable; current reports enumerate actually executed
profiles, counters and models. No independent human assessment, learner mastery,
physical validation or exhaustive C execution proof is inferred.

The actual local burst workload queues six distinct complete frames on six
socketpairs before serving any of them. Under two deterministic service orders,
four effects/notifications fit, the last two requests return FULL before effect,
and all six consume their admitted work token. Replies do not drain the slow
notification consumer. Only the subsequent four durable ACKs return event
capacity, preserving the admitted identity set and total4. This queued-arrival
experiment is separate from the composition's burst-loss timing workload and
from the native two-worker duplicate-request race.
