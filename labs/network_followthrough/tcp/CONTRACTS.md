# TCP acceptance, retained capacity and clocked recovery

Required successors for PR-T41-06 and PR-T41-07. The six original AX criteria
and six V1 identities remain unchanged. Read the original project, then perform
this package; historical hours are not a limit. The separate required congestion
investigation remains in `legacy_network_scope/remaining_work.json`.

## Prerequisites and permitted components

Bring working Ni IPv4 validation, SChecksum streaming checksum, NwRx storage,
the Nw HTTP pump and the original HTTP parser. Supply each implementation
explicitly to `run.py`; a missing prerequisite is not silently supplied. Complete
`tcp.c` from these interfaces and C17 operations. The host uses GCC/Clang,
sanitizers and Python oracles; the M4 target uses original memory/string helpers,
direct linking and no libc, libgcc, compiler-rt, libatomic, HAL or TCP library.
Headers used for declarations do not provide a target runtime. Inspect all object
imports, retained code, linker map and stack reports. Host `printf` is not linked
into the target. C23 adds no ownership, sequence-space or protocol enforcement;
this is a C17 implementation, not a claim about an ISO networking API.

The supplied packet builder is an independent public fixture constructor. It
does not implement your transport. Assessor implementations, additional traces,
oracles and delayed solutions are withheld. Native invocation:

```
python tcp/run.py --build out --source my_tcp.c --ip my_ip.c --scope my_scope.c --stream my_stream.c --http my_http.c --systems my_systems.c
```

Run from a directory where these explicit paths exist. `run.py` finds public
declarations relative to its installed `labs` tree. It extracts the unchanged
two checksum functions from the identified systems provider; the extraction
manifest pins the complete provider and exact body. It does not substitute an
alternate checksum. Target building uses the full systems translation unit.

## Representation and enforcement boundary

One NcStack owns one passive connection, a 1..512-byte circular RX queue, a
1600-byte copied application offer, one at-most-256-byte outstanding segment,
one prepared packet and its retained retransmission copy. Effective MSS is
min(peer option,256); absent option defaults to536 before the cap. IP addresses
are 192.0.2.2/192.0.2.1 in fixtures, local port8080, nonzero peer port. Input is
validated raw IPv4 (no Ethernet header); the link/ARP adapter must enforce its
own destination, admission and lifetime contract. All input/output spans are
valid, disjoint from state and one another; calls are externally serialized.
ISO C does not enforce those pointer or scheduling premises.

All live sequence differences must be strictly below2^31. Use unsigned modulo
arithmetic and prove the half-range ordering domain; signed casts of wrapped
differences are not the proof. Monotonic uint64 millisecond time is restricted
to0..UINT64_MAX-240000, rejecting reversal and insufficient deadline headroom.
One incarnation has a nonzero externally allocated epoch. An old packet is not
authenticated by the epoch: epochs only identify local lower-layer tickets.
Serial tickets stop before reuse at UINT32_MAX. Reinitialize only after actual
DMA/callback quiescence, with a fresh epoch and an isolated-lab ISN policy.

`nc_prepare` copies a candidate but commits no sequence consumption, flight,
timer or application progress. `nc_submit(ticket,0,now)` means the lower port
copied nothing; the candidate remains retryable and no RTO starts. With1, the
port has synchronously accepted a complete private copy under the same serialized
transaction; commit advances sequence/progress once and starts the timer at now.
This acceptance timestamp is not a physical on-wire timestamp. Record its queue
delay when interpreting board captures. Do not invoke another mutating method
between lower copy acceptance and submit. Receive/consumption may invalidate a
still-unaccepted candidate; old tickets then return STALE. A retry reuses exact
retained TCP bytes and consumes no new sequence space. `nc_dma_done` releases
only the copied lower ownership gate; a valid peer ACK alone retires TCP flight.
An ACK can arrive before DMA completion; new output still waits for DMA release.
Abort clears logical RX/TX work but preserves the independently owned DMA ticket
until reap. Treat abort as failure, not application EOF or successful delivery.

## State and storage derivations

Validate header extents, IPv4/TCP checksum, tuple, reserved bits and options
before state publication. Option walkers must advance or reject; MSS is nonzero,
length4 and SYN-only. This profile rejects SYN data, urgent/ECN flags, active or
simultaneous open, unsupported IP options/fragments and arbitrary out-of-order
storage. Those scoped omissions do not exclude the separately required advanced
curriculum work. Invalid input leaves the state unchanged. Valid but unacceptable
segments may set an ACK request and invalidate a pending candidate.

SYN and FIN each occupy one sequence position. ACK occupies none. ACK beyond
SND.NXT cannot release bytes or update the peer window. For admissible ACKs,
WL1/WL2 gate freshness; advancing SND.UNA restarts a running timer, full ACK
stops it. Window is a receiver limit; usable window subtracts the wrapped flight
size. Even positive usable window permits no second flight in this profile.

For RX, derive the acceptable circular interval, trim an old prefix, admit at
most actual free queue capacity, and advance RCV.NXT by exactly copied new bytes.
No gap is buffered. A FIN immediately beyond the advertised right edge is not
accepted merely because preceding data fit. Free space reopens only when
`nc_consume` or the actual HTTP pump consumes a prefix. HTTP_DONE/ERROR retains
leftovers. `nc_http_eof` requires peer FIN and an empty retained queue, then asks
the original parser whether its own framing finished. FIN after two of four
declared body bytes remains a400 error.

Trace SYN-RECEIVED, ESTABLISHED, CLOSE-WAIT, FIN-WAIT-1/2, CLOSING, LAST-ACK,
TIME-WAIT and CLOSED. CLOSED requires explicit new-incarnation initialization.
An already accepted FIN position restarts240000ms TIME-WAIT. Reset handling
distinguishes exact RCV.NXT, unequal in-window and outside-window sequences.
The challenge ACK is not authentication and this small serialized profile does
not implement a production challenge-ACK rate limiter.

## Timers and changed precision

Use initial/minimum1000ms, cap60000ms, three accepted timeout retransmissions,
then abort at the next expiration. With immediate acceptance at0, the loss trace
is1000,3000,7000,abort15000. BUSY does not increment retry count or backoff; a
successful retry starts its backed-off timer at actual lower acceptance. A
retransmitted SYN makes initial data RTO at least3000. Ignore ambiguous samples
without timestamps (Karn); samples greater than60000 are outside this estimator
experiment and remain unmeasured rather than truncated to a smaller value.

Estimator shift0 floors each weighted estimate to whole milliseconds; shift8
stores units1/256ms. Granularity is an explicit1..1000ms input, final RTO rounds
up, then clamps. Derive variance using the old smoothed RTT. First sample80
gives80/40; then120 gives85/40 and1000. Compare both quantized policies against
an independent rational recurrence and report quantization error; neither policy
is mathematically identical to infinite-precision estimation. Empty/zero elapsed
samples use the stated1ms resolution floor. Zero-window probes use SND.NXT-1,
no new application byte, separate exponentially backed-off probe deadlines and
no new TCP flight. This is the declared ACK-probe policy, not a claim that every
peer or general zero-window event is covered.

## Six assessment paths and acceptance

1. **T41.06.a**: derive MSS100, SYN/ACK sequence positions, malformed-option
   rejection and unsent-ACK nonpublication; implement the wire/handshake path.
   V1 must create both WL1 and WL2 wrap cases, including an older acceptable
   segment that cannot shrink a fresher peer window. Explain the half-range
   premise and the provenance of any injected valid internal state.
2. **T41.06.b**: reproduce fffffff1+16→1, duplicate→1, fffffff9+12→5;
   identify the exact20 retained octets. V1 repeats with a queue smaller than
   the new tail, drain/retry and physical ring wrap. Acceptance counts copied
   bytes, not parser interest or attempted input. Add an independent interval
   oracle and a root-cause diagnosis of an over-ACK defect.
3. **T41.06.c**: implement all listed close/reset states and deferred HTTP EOF.
   V1 demonstrates forged unequal reset versus exact reset solely against this
   original local packet program. Supply state/output differences, repair and
   residual authentication/rate-limit analysis; a crash is not the success
   condition. No external targets or network traffic are needed.
4. **T41.07.a**: derive weighted estimates, admissibility and deadlines, then
   postpone lower acceptance beyond initial RTO. V1 implements both precision
   policies and independently computes rounding/granularity bounds. Test ACK at
   deadline, partial ACK restart, SYN loss, backoff cap and clock reversal.
5. **T41.07.b**: hold zero peer window and full parser storage; reopen8 with
   wrapped32-byte flight. V1 implements actual queue/HTTP composition, wrapped
   consumption, stale prepared-ticket rejection and a no-new-byte probe. Show
   ACK-before-DMA and DMA-before-ACK traces independently.
6. **T41.07.c**: retain exact bytes across three retransmissions and abort;
   compare MSS64 and256 under the same1024-byte delivery and100ms service model
   with3ms lower admission delay. Count segments, retained storage, copied bytes,
   virtual completion time and measured code/stack costs separately. V1 changes
   the service/ACK model and justifies its consequences. Explicitly identify
   missing congestion growth/fast recovery; complete the linked required
   congestion investigation instead of treating this fixed-window comparison
   as an implementation of those algorithms.

Submit source/build identities, prior predictions, event ledgers, invariants,
all expected successes and intended rejections, independent oracle trace, raw
target objects/maps/assembly and criterion-indexed analysis. O(payload) packet
checksum/copy and bounded O(options) parsing must be distinguished from O(1)
state transitions and finite storage. Run original mechanisms at GCC/Clang O0/O3
and sanitizers, then actual M4 images with no runtime imports. Compiled mutants
must fail the intended semantic assertion, not translation. Passing finite
cases establishes only their stated bounds.

Rubric per criterion:0 incorrect/missing mechanism;1 repeats outputs or merely
builds;2 derives invariants, implements the named mechanism and explains an
independent counterexample with reproducible evidence;3 independently solves
the V1 changed assumptions after at least seven days and diagnoses a new failure.
Author reference execution cannot award the learner score3. Hints in order:
draw separate storage/timer owners; label construction/copy/ACK/consumption
transitions; use ordinary integer intervals before wrapping; only then consult
the worked constants above. Preserve unaided attempts before hints.

## Sources and evidence

Source register: `research/v5/expansion/legacy_network_tcp/resources.json` pins
actually inspected RFC9293 receive/ACK/close passages, RFC5961§3.2 and RFC6298
§§2–5. These are protocol sources, not ISO C or board specifications. Exact
physical Ethernet integration remains an explicit T39 consumer obligation;
CPU packet-array execution is not a board capture. External review, independent
learners, delayed assessment and physical observation are separate dimensions.
See `curriculum/guides/network-tcp-board-protocol.md` for the required capture.

## Local link-failure abort integration

`nc_abort(state,now)` is a serialized, idempotent local logical abort for a link/driver failure. It validates the initialized epoch and monotonic bounded clock, closes the transport, invalidates prepared sends and clears logical queues/timers. It deliberately preserves any lower DMA ticket. Only an actual reap or positively established DMA quiescence permits `nc_dma_done` and subsequent reinitialization with a new epoch. A stop request or timeout is not such proof. The caller never fabricates a peer RST and does not confuse graceful `nc_close` with immediate containment. The validation campaign executes both copied and merely prepared abort states, backward time rejection and stale resubmission.
