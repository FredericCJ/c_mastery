# PR-T41-04: withheld association, timing and ownership analysis

The ordinary and V1 tasks retain their original IDs, wording and minimum-seven-day
delay. Current author executions prepare an assessment; they are not an independent
attempt, reviewer approval or physical capture. Grade each criterion separately.

## 04a

The parser proves a14-byte Ethernet span before reading type and a42-byte combined
span before fixed ARP fields. Reject non-Ethernet/IPv46/4 records rather than using
untrusted lengths to index. The valid profile has28meaningful ARP bytes with no
IPv4 wrapper. Independent literal request/reply bytes and zero padding are a
serialization oracle; checksums do not exist in the ARP payload itself.

The matrix varies each of19 selected positions through all255 alternatives,
accepting only the opcode change2→1 as a valid request. Add42 truncated prefixes,
an explicit VLAN frame and the variable-length truncation case:4888 rejected
cases plus two accepted controls, the latter including a fully consistent changed
association. Snapshot complete state before invalid input; it must remain unchanged.
The state has no invalid-pointer access specimen: all spans actually exist, with
the passed count intentionally restricting access. Sanitizers supplement the
checked-prefix argument; they do not prove every input safe.

The delayed task needs careful premises. A genuine minimum60-byte pre-FCS frame
has46payload bytes, enough for fixed6/4 ARP. Claiming those complete bytes contain
a truncated28-byte ARP body is contradictory. A valid outer frame declaring hlen20
requires56ARP bytes and is too short; alternatively a qualified lower adapter can
report truncation even when a separate capture saw a complete wire frame. Require
the learner to state which changed assumption they use. Outer integrity never
justifies reading beyond the actual software span. An answer that only supplies
a41-byte runt without acknowledging its Ethernet premise earns at most level1.

Use explicit octet serialization, not packed-struct dereference. A valid alternate
implementation may decode through checked views, but must explain alignment,
effective type, endianness, nested length and lifetime. C23 changes none of these
ARP field or hardware contracts; do not assert that a newer C edition authenticates
the source or enforces ownership.

## 04b

Queue invariant: at most one copied datagram, length1..1500 while pending; BUSY
leaves a second caller's span untouched. Its externally supplied contents are an
already validated IPv4 datagram for the configured peer. The ARP layer retains
bytes through resolution and lower-send refusal. `prepare` returns a separate
copy and a nonwrapping epoch/serial/kind ticket; a refused copy clears preparation
only. Accepted data transfers a copy to the lower owner and clears the pending
state. A subsequent DMA callback cannot refer to the released caller frame.

The lower channel's copy contract is essential. It is not satisfied by handing a
pointer to a DMA engine. The planned physical T39 bridge must retain its own
frame and wait for actual completion/quiescence; that producer's implementation
and physical evidence are separate obligations. Existing author composition uses
an explicit owned `struct net_packet` assignment as a reference copy port, not a device.

Absolute slots, not poll counts, define request eligibility. Successful service
at0/1000/2000 yields three requests; failure at3000 clears pending bytes. At2500,
only the current slot can be used. Repeated refusal does not consume that slot,
but external scheduling must prevent busy spin. The independent Python oracle
derives occupied intervals from prior accepted events; it does not replicate the
C bitset. It checks72 start/step/refusal groups and51024 events, including origins
near32-bit wrap and near64-bit exhaustion. Deadline arithmetic is checked before
addition; pending elapsed time is below3000 before narrowing to unsigned.

This successor deliberately chooses a3000ms total admission lifetime, also covering
known-but-backpressured frames. Therefore resolution at2999 cannot justify an
unaccepted copy after3000. This is an explicit bounded-storage policy, not a rule
from RFC826. An alternative separate resolution and send deadline is acceptable
only with a complete changed contract, cleanup path, workload bound and tests.
Do not silently extend lifetime whenever an unrelated ARP packet arrives.

Cache expires at the exact30000 endpoint. The delayed variant learns MAC7, queues
at29999, refuses a lower copy, expires at30000, learns MAC99 and serializes it at30001.
The original parent fixture remains unchanged. The actual HTTP/TCP/IP composition
produces a response with the historical stack, copies its IP datagram into this
queue, overwrites the old packet, rejects three lower admissions, then copies a
frame with newly learned MAC88. Every IP/TCP byte remains identical; the IPv4
checksum still verifies because Ethernet addresses are outside it. The historical
TCP stack advanced transport state before this adapter; this demonstration does
not repair or validate that separate lower-admission contract. Later T41.06/.07
must integrate the corrected transport state rather than cite this run as proof.

Cost accounting includes1500 retained bytes, a full copied NetPacket, reply state
and metadata, plus any lower-owned copy. The current frame builder zeros its fixed
1518-byte capacity before serialization, so its work includes that bound even for
a tiny packet. Compare a prefix-only zeroing alternative under identical padding
and confidentiality contracts. Inspect compiler local-frame usage; aggregate
initialization can allocate a temporary even when persistent state is static.

## 04c

Target: original local C teaching process or MPS2 synthetic arrays, with no outbound
network interface. Adversary: controls Ethernet and ARP fields and event timing.
Property: malformed or wrong-configured-peer input must not publish an association;
accepted pending bytes must survive lower refusal. Defects: omitted SA/SHA match,
omitted configured-IP check, hard-coded transmit MAC, ignored cache expiry,
100-tick timer substitution, and release-on-refusal. Each compiled mutant must
reach and fail its own assertion; failed compilation is not semantic evidence.

The bounded security result is a synthetic MAC published in the local cache after
removing a binding check. Correct those checks and reject the corresponding
regression inputs. Then retain the positive control with matching fields for a
different MAC: normal ARP accepts it. This is the exact residual authentication
limitation; equality between two attacker-controlled fields does not establish
identity. No memory corruption, crash, arbitrary execution or privilege escalation
is claimed. A pinned association mitigates this local policy threat at the cost
of rejecting legitimate changes; cryptographic peer authentication is a different
layer and not secretly supplied by this exercise.

For V1, withhold mutation locations and ask the learner to select two single-field
changes and a consistent multi-field change, predict cache state, then demonstrate
the result in the original local program. A statement that the SA/SHA comparison
"prevents spoofing" without the consistent-field counterexample fails level2.

## Assessment and evidence limits

Level0: wrong spans, ownership or unsupported conformance; level1: fixture matching
without timer/association reasoning; level2: invariant argument, working code,
independent results, compiled discriminators, exact costs and evidence limits;
level3: unaided changed-condition transfer with complete revised premises. Do not
average away a missing field-validation, lifetime or authentication distinction.

Release hints in order: label wire offsets, identify mutation publication points,
draw absolute time intervals, then separate caller/queue/lower/remote owners.
Record assistance before delayed assessment. Physical packet traces require the
dedicated protocol, actual T39 adapter, exact board/clock/PHY information and raw
captures. A structurally valid synthetic capture is not a physical observation.
Exact final ISO editions and independent reviewer/learner evidence remain open.
