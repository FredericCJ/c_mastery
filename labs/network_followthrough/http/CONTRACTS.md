# T41.08: HTTP over the actual embedded packet stack

Required predecessor artifacts: completed T41 driver/ARP/IP/TCP and original
`HttpParser`; the separately identified T42 `hf_response` encoder is an author
reference dependency until replaced by a learner artifact. This is the existing
PR-T41-08 ordinary and seven-day changed assessment, with its IDs unchanged.
It implements the previously missing binary/chunk/half-close, Date and actual
link-incarnation integrations rather than crediting the old bytewise GET alone.

## Problem and concrete input/output

Implement `NhHttp` using the declared dependencies, C17 fixed storage and the
strict freestanding target profile. Supply `learner.c`, an ownership/state
diagram, criterion-indexed reasoning, raw test/capture evidence and a linked
runtime/resource audit. Run the supplied original raw peer over actual EnRing,
ARP, reassembly, TCP, receive queue and parser. The target contains no hosted
socket stack or libc: host-side compilers, sanitizers and capture programs are
separate verification tools.

Input is a same-link synthetic192.0.2.1 client to192.0.2.2:8080 with authority
`mcu.test`. Required cases include a binary12-byte body in5/7-byte chunks,
a1024-byte body in one400-hex chunk, a trailer line, a missing final trailer
terminator followed by FIN, Expect:100-continue, and a trailing second request.
Bodies contain NUL/CR/LF as data. Output is a byte-exact response with the decoded
body, explicit length/content type and Connection:close; an incomplete request
produces400 rather than a successful partial echo. A second request is not
dispatched by this one-request policy. Do not treat TCP segmentation as HTTP
framing or use a NUL-terminated body API.

The chunk terminator includes completion of the trailer section; a zero chunk
alone does not authorize early dispatch. HTTP octets and the decoded entity
have distinct lengths. These premises follow the inspected
[RFC9112 section7.1](https://www.rfc-editor.org/rfc/rfc9112.html#section-7.1);
the stricter parser resource limits are original curriculum policy.

## State, ownership and failure contracts

`NhHttp` owns its parser and1600-byte pending output, borrows a stable `NbNet`,
and binds its transport epoch. All calls are serialized and non-reentrant;
this application is the transport's only application writer/consumer. The
state progression is handshake → parse → interim/final → closing → finished,
with failure terminal. Each call first services one bounded network step, then
consumes at most64 request bytes. Receiving a partial request is not an error.
FIN becomes parser EOF only after queued bytes are consumed.

An interim100 response is retained until copied admission and acknowledgement
before parsing resumes. Final output stays owned until `nb_offer` copies it;
close requests follow that admission, and actual TCP serialization waits for
retained data/flight gates. This does not release DMA early. The application
finishes at closed/TIME_WAIT; the enclosing network owner must still service
the full TCP quiet-time interval. Destroying its storage at application DONE
would violate the lower lifetime contract.

Use a monotonic uint64 millisecond argument, rejecting regression/overflow
margin violations. The request has15s absolute and5s idle limits from completed
handshake; consuming bytes advances only idle progress. An interim or final
output has its own15s bound. A zero peer window cannot extend that bound.
Failure requests network stop while preserving device ownership until the
actual stop protocol establishes quiescence. No timeout is evidence of release.

## Date and changed assumptions

Date is either absent or supplied by a trusted callback returning current UTC
in IMF-fixdate form. The callback may return NULL if UTC is unavailable, does
not reenter, and keeps its text valid through the call. The encoder rejects
wrong length/control bytes; it does not prove calendar correctness or clock
truth. The ordinary test callback is explicitly synthetic. A monotonic tick
counter cannot stand in for UTC, and an old literal date is not a clock.
The presence/absence rule follows inspected
[RFC9110 section6.6.1](https://www.rfc-editor.org/rfc/rfc9110.html#section-6.6.1).
C23 does not supply these application clock guarantees.

Independently vary inbound packet chunks1/7/16, peer MSS1/17/256, advertised
window1/9/512, and receiver capacity16/512. Verify reconstructed sequence bytes
and complete response framing. The32 supplied raw cases distinguish segment
size, flow control, parser packet independence, maximum entity, interim output,
EOF and trailing-request policy. Add a fresh changed assumption and justify
which previous arguments survive; parameter sweeps alone do not show transfer.

For link recovery, stop with actual copied response DMA outstanding, confirm
the application fails while the lower copy remains held, observe positive
model quiescence, requalify PHY/ring, then initialize a new epoch and ISN.
Inject the exact queued old ACK in LISTEN and during the new handshake. It
must not complete the new request. Explain why a local generation is absent
from the wire and why guessed correct sequence numbers still are not an
authentication guarantee. Do not turn this selected test into an anti-replay
or full Internet-host claim.

## Verification and assessment

Use `run.py --providers providers.json --source your_http.c --build build/http`.
The15 native provider keys are explicit in that tool. Reference providers must
be named as assessor work; missing dependencies fail instead of silently
selecting private sources. `target_build.py --help` exposes the additional
original runtime/board-port requirements. Verify the emitted25-object image,
every object import, final closure, placement and stack budget. The encoder
uses stack scratch space as well as the owned output; account for both.

The raw peer constructs checksums by byte position, reconstructs outgoing TCP
sequence bytes and compares literal HTTP expectations; it never calls the
response encoder to generate expected bytes. It uses declared TCB state only
to schedule its finite test exchange, not to derive the expected entity. These
are finite, loss-free exchanges; predecessor RTO/loss and congestion campaigns
remain required and are not replaced. Diagnose the compiling mutants that
accept incomplete EOF as success or reintroduce a stale literal Date. Explain
why their first semantic mismatch matters. A compile failure or crash alone
does not count as their intended rejection.

Per-criterion rubric: level0 for early dispatch, incorrect bytes, false UTC or
early release; level1 for matching supplied output without the ownership/framing
argument; level2 for a complete implementation, independent byte/deadline proof,
negative diagnosis and new falsifier; level3 only after independent delayed
transfer with no assessor access. For T41.08.a assess both framing and changed
MSS/window; for08.b assess both Date modes and clock separation; for08.c assess
ownership/new handshake plus physical-evidence limits. Author runs do not
establish learner or reviewer independence.

Hints: (1) list the three byte counts—packet, parser prefix, entity;
(2) identify the queue bytes left when a terminal parser returns;
(3) draw the separate application, TCP-flight and descriptor lifetimes.
Keep the unaided attempt before using hints. The assessor solution is separate.

## Dedicated physical and security boundary

Use only original teaching firmware and private synthetic traffic on dedicated
authorized hardware. No flashing or external target interaction is part of
this authoring campaign. Complete the linked PHY/ring and TCP board protocols:
record revision/clock/link/cable, old/new tuple and ISNs, offload settings at each
capture location, raw bytes, compiler/image hashes and calibrated timestamps.
The repeated physical run after seven days is external evidence, not a reason
to leave today's adapter or assessor variant unimplemented. Empty capture
records stay empty until observations exist.

A hostile local synthetic client may truncate frames or replay old ACKs. The
harmless success condition is an incorrect local echo or incarnation/lifetime
violation; it is not arbitrary code execution. The repaired parser/epoch/stop
rules are regressed, but no cryptographic peer authentication is provided.
The selected one-connection/one-flight application omits DNS, routing, TLS,
general TCP option/recovery behavior and persistent HTTP concurrency; the
required broader mechanisms remain in their named curriculum packages.
CPU/mapped-register passes do not validate electrical behavior or wire latency.
