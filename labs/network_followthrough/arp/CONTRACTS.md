# T41.04: observed associations and bounded pending ownership

Implement `arp.h` in the incomplete C17 source. Reuse the earlier checked-span,
clock and frame-copy contracts. The author integration uses the actual historical
HTTP/TCP/IP serializer and current T41.01 length calculator; your independent
assessment must identify the corresponding learner artifacts. No ARP library,
socket stack or HAL may implement the studied mechanism. Python oracles and host
sanitizers are verification tools. Strict target imports require an explicit
original memory/string runtime and a linked-image audit.

## Representation and derivation

The14-byte Ethernet header precedes an ARP payload, not an IPv4 header. The generic
ARP payload requires `8 + 2*hlen + 2*plen` bytes. In this profile1/0x0800/6/4 means
28bytes. Opcode is a big-endian16-bit field. Serialize bytes explicitly; neither
native struct packing nor host endianness is a wire contract. Outbound frames
are padded to60bytes excluding MAC-supplied FCS. Incoming API spans may have padding
trimmed by a qualified lower adapter; never read beyond its actual received count.

The configured local MAC/IP and one peer IP are immutable during an incarnation.
An accepted ARP sender association supplies the peer MAC; it is never reconstructed
from the old fixture. Validate destination, EtherType, hardware/protocol types,
lengths, opcode, nonzero unicast Ethernet sender, matching Ethernet SA/ARP SHA,
configured sender IP and local target IP before publishing. A reply also names the
local target MAC. A request's target hardware bytes need not already be known.
VLAN and foreign destinations are outside this profile and rejected.

RFC826 defines the fields, request/reply exchange and replacement association;
its related-issues discussion leaves aging policy outside the protocol. The
following millisecond policy belongs to this curriculum contract, not ISO C or
a universal RFC timer rule. Cache lifetime is30000ms; exact expiry is invalid.
A single pending copied datagram has a3000ms **total lower-admission lifetime**.
This deliberately bounds both unresolved and resolved-but-backpressured storage.
At its deadline it is discarded with `NA_EXPIRED`, even if resolution arrived
just before the deadline but lower admission never occurred. Report that outcome
to the upper owner; it is not remote delivery. Additional queue submission is BUSY.

Resolution slots are `[0,1000)`, `[1000,2000)`, `[2000,3000)` relative to enqueue.
With prompt service, requests occur at0/1000/2000 and unresolved failure at3000.
A late2500ms service sends at most one request for the current slot, not a burst
replaying missed slots. Only accepted lower copies mark a slot used. A refused
request may be retried in the same slot; callers must schedule/back off attempts
to avoid busy spinning. Time is injected monotonic64-bit milliseconds. Backwards
time and checked deadline/expiry overflow fail explicitly; no wraparound clock
is silently treated as monotonic. The queued payload is an already validated
IPv4 datagram for the configured peer; ARP does not redo the IPv4 mechanism.

`prepare` produces a copied `struct net_packet` and epoch/serial/kind ticket. `submit(0)`
releases preparation but retains pending ownership and attempt eligibility.
`submit(1)` means the lower channel owns a complete copy. It does not mean DMA
completion, a TCP ACK, or peer receipt. Each ticket is resolved exactly once.
One prepared transaction excludes another API mutation; the copy-admission and
submit calls must not be interleaved with receive/time service. This is an
explicit serialization premise, not a C type-system guarantee. A DMA-borrowing
driver needs an owning adapter and quiescence protocol before it can satisfy this
interface. Reinitializing a live device is not cancellation.

The one-entry pending ARP reply has priority over pending data. A second receive
while a reply or lower-send transaction is pending returns BUSY unchanged; the
caller records the dropped input and the peer may retry. Cache updates carry no
authentication proof. Preparing observes time, invalidates expired cache and may
expire a queued datagram; WAIT/RANGE results can therefore advance service time.
Malformed receives and invalid queue submissions do not publish cache/queue data.

## Required progression and assessment

**T41.04.a — A,B,C,D,F,I.** Derive the generic and fixed sizes, then implement
field validation and request/reply serialization. Supply every truncated prefix
0..41, every alternative octet at the selected field positions in the assessor
matrix, and an independent literal-byte oracle. Prove no invalid association
publication and no read outside the actual span. Delayed V1 supplies a60-byte
Ethernet frame declaring hlen20/plen4: its46payload bytes cannot contain the
declared56-byte ARP record. Explain why a conforming fixed6/4 ARP body cannot be
truncated inside a genuine minimum-size60-byte frame; the changed condition must
name altered lengths or a lower-layer truncation/count discrepancy.

**T41.04.b — B,C,D,F,G,H,J.** Implement one owned queue, timed cache and the
prepare/submit protocol. Predict the entire0/999/1000/1999/2000/2999/3000 trace,
lower refusals, skipped service slots, backwards time and near-UINT64_MAX cases.
Change the peer MAC at expiry and show the newly learned bytes reach an actual
serialized IPv4/TCP response while its payload and checksum remain unchanged.
Overwrite the caller's original packet to demonstrate copy ownership. Record
the historical TCP provider's separate lower-admission defect rather than
claiming this ARP adapter repairs its transport state. Delayed V1 repeats the
association change with a different MAC, delayed service and refused copies.

**T41.04.c — A,C,D,E,F,G,I,J.** Compare a mismatched SA/SHA, a matching pair from
the wrong configured IP, and fully consistent forged configured-peer fields.
The target is only this original local program and synthetic packets; no network
traffic is sent. The adversary controls these byte fields. The harmless success
condition is publication of an attacker-selected synthetic MAC in the local
cache. Disabling a binding check exposes precisely that check's property; a fully
consistent forgery still succeeds under ordinary ARP, demonstrating the residual
authentication limitation. Restore all validation checks and rerun regressions.
An explicit pinned-peer policy may prevent a change but also prevents legitimate
mobility; it is a changed contract, not free authentication. A crash or sanitizer
finding does not establish code execution or privilege escalation.

Supply implementations, state/ownership invariants, failure and deadline proofs,
exact fixtures and independent expected outputs, compiler/object/link/resource
records, and a criterion-indexed dossier. Core state is bounded; byte copying and
serialization are linear in frame/datagram length, timer bookkeeping constant.
Report static retained payload, copied lower frame, local temporaries and actual
target stack separately. Compare both active queue counts and allocated capacity.

Hints: first label every address field and owner; then draw the three absolute
request slots; pause after prepare before lower acceptance; finally change the
observed MAC while preserving IP and transport bytes. Separate assessor code and
solutions are withheld. Rubric0wrong/absent,1ordinary example without invariant,
2complete contracts plus independent results and discriminating controls,
3independent changed-condition transfer. Original seven-day tasks require a real
later learner attempt. Source references are exact named RFC826 passages and
the bound historical parent; no published C17/C23 wording or full-host conformance
is inferred. Neither C edition enforces ownership, authenticates ARP or stops DMA.

Physical cable/capture observations remain separate from host and emulator runs.
The actual T39 driver must be completed and qualified before physical composition;
the observation protocol and capture schema accompany this package. Author-side
copy-port integration is not evidence that a learner driver satisfies it.
