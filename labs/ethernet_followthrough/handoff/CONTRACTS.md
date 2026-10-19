# T39 cross-layer handoff successor, revision 1

This is required support for the six original PR-T39-08 criteria and their
unchanged `AX-PR-T39-08-T39.09.a-V1` through `T39.10.c-V1` tasks. It adds concrete
construction and falsification; it does not replace the original questions,
seven-day delay, learner rubric or historical acceptance. Read the exact parent
criterion before using this contract. Solutions remain under assessor/.

## Prerequisites and environment

Bring the learner's T39 framing/FCS, PHY and normal-ring implementations, the
T41 scope/ARP/IP/TCP/driver/HTTP artifacts, and their successful interface checks.
The T41 consumer is a later composition assessment of T39, not a new reversed
prerequisite for teaching basic Ethernet. Public headers are the actual
`ethernet_depth.h`, `phy.h`, `ring.h`, `scope.h`, `stream.h`, `arp.h`, `ip.h`,
`tcp.h`, `driver.h` and `http.h` under their declared labs directories. Understand
finite-state ownership, byte order, integer admission, monotonic deadlines and
checked protocol lengths first. A similarly named replacement is not an import.

Use the active ISO C17 baseline, 8-bit octets and exact-width integer types. All
objects and spans must be live, sufficiently sized, initialized and disjoint
unless a producer explicitly permits otherwise. Calls and device-model progress
are externally serialized. No allocation, hosted network stack, kernel socket,
protocol library or external connection implements the assessed mechanism.
Hosted GCC/Clang and sanitizers are verification tools. In strict M4 builds the
newlib headers supply declarations only; the direct link selects original byte
helpers and no libc/libgcc/compiler-rt/HAL. Inspect object imports, linker map,
retained assembly, stack usage and final undefined symbols, including compiler
helpers. GNU target assembly and semihosting are separate ISA/tool contracts.
Neither C23 nor C diagnostics confer checked spans, ownership or protocol safety.
Exact published C17/C23 text access remains separately recorded, not inferred
from the C11 draft or a compiler accepting `-std=c17`.

`run.py --root <workspace> --providers <json> --build <new-directory>` requires
every named provider in its `KEYS`; it defaults only the new mechanism to
`learner.c`. The manifest maps actual learner sources to those keys. No assessor
fallback exists. `--source` selects an explicit completed mechanism. `--compile-only`
permits a separate run per selector. `target_build.py` additionally requires an
explicit header root/compiler; its boot and linker recipe are public original
CPU-harness components. A missing provider must fail before compilation. Do not
replace it with an empty successful implementation.

## Problem and representations

Implement `eh_next`, `eh_frame` and `eh_correlate` from `handoff.h`; retain the
actual ED and T41 interfaces. Supply the following six dossiers. All outputs on
refusal are unchanged, including output length, unless a retained producer says
otherwise. Null required controls and malformed values return `EH_INVALID`;
invalid C pointers are a caller-contract violation, not detectably safe inputs.

**T39.09.a, namespaces.** An `EhRoute` contains host-order local IPv4 address,
contiguous /1 through /30 mask and optional gateway. Source must be an individual
host, first octet neither0,127 nor224..255; host bits cannot be all zero/all one.
Reject self, local network/broadcast and nonindividual destinations. For a remote
destination, gateway0 yields `EH_NO_ROUTE`; otherwise require a distinct valid
host on the same local prefix. `eh_next` returns the peer for local traffic and
gateway for remote traffic. This is a single-route teaching profile, not full
RFC1122 routing, redirects, multicast, /31 point-to-point or multihoming.

`eh_frame` accepts an opaque IPv4 envelope of20..1500 bytes: version4/IHL5,
matching total length, configured source and valid20-byte header checksum.
It checks next-hop association against `EhNeighbor.address`; a nonzero individual
neighbor MAC is required. The configured source MAC is trusted individual,
nonzero and administratively unique. Caller supplies a separately admitted IP
datagram; this envelope does not validate transport, TTL, fragments or options.
Use actual `ed_frame_build` with type0800, zero minimum-frame padding and no FCS
in CPU output. On missing/wrong association return `EH_NEIGHBOR`; insufficient
capacity returns `EH_FULL`. Copy the IP bytes unchanged. Trace local192.0.2.2/24,
peer192.0.2.9 and remote198.51.100.9 through gateway192.0.2.1. In the changed case,
identify which Ethernet address changes while final IP and HTTP authority stay
fixed. A MAC is not authentication and the HTTP Host field is not a route.

**T39.09.b, actual composition.** Inspect and complete `stack.c` against the
actual retained providers. Its raw peer drives copied frames through EnRing,
NaArp, Ni parsing/reassembly, NcStack, NbNet and NhHttp, then reconstructs TCP
sequence bytes and compares a literal `/health` HTTP response. Use MSS17,
receive capacity16 and seven-byte request writes for the fresh exchange. Draw
the borrowed RX view, released descriptor, copied pending packet, DMA-owned TX
buffer, transport flight and application output separately. Run absolute time
and events until the active-close TIME_WAIT deadline; DONE is not storage
retirement. Include zero-window failure, stale application epoch and link loss
with delayed DMA acknowledgement. Retain producer failure codes and cleanup.

This actual stack is **same-link**: current NbNet associates the configured ARP
peer with its transport peer. The separate routed-envelope exercise is not a
routed HTTP stack. Do not set the TCP peer to the gateway to hide that limit.
Contrast this source graph with a hosted socket program that delegates the
lower layers; equal HTTP output does not imply equal implemented mechanisms.

**T39.09.c, limit and extension.** Choose reassembly as the changed feature.
Use actual `NiReassembly`, its one-slot/1480-byte data/185-byte bitmap, tuple key,
first-header storage, READY and POISON states, absolute deadline and copied
output. Send fragment offsets16(final),0,8 for24 data bytes. No partial output
is permitted while the middle eight bytes are absent. Compare identical overlap
and conflicting overlap; duplicates cannot renew the deadline. A take retains
a tombstone until the original deadline. `reassembly.c` supplies inspectable
synthetic peer fragments; derive independently their lengths/checksums and
poststates. Contrast RFC791's illustrative last-arrival overlap procedure with
this explicitly stricter policy. Explain bounded capacity denial of service,
ID reuse after expiry, unavailable option handling and absent arbitrary-peer
interoperability. A successful original GET never established these properties.

**T39.10.a, time.** First preserve original offset conversion0,5,20 ->100,105,120
and rejection of0,20,5. An `EhClock` gives local uint32 anchor, nonwrapping samples,
global uint64 microsecond anchor interval[lower,upper], and rate-error bound
0..1000 parts per million. For delta `d`, widen by `ceil(d*ppm/1000000)` on each
side. This bound is an explicit model premise, not an oscillator measurement.
At most16 nondecreasing samples, all>=anchor, are accepted. Any underflow,
overflow, reversed interval or invalid sample rejects the entire batch without
publishing a prefix. Empty input permits null spans. Explain why overlapping
global intervals cannot prove order. Use anchor interval[98,102],100ppm and
samples0,1,2000000, plus UINT32_MAX/1000ppm. Establish integer bounds without
relying on an unaudited target division helper.

**T39.10.b, diagnosis.** Preserve the original first-prerequisite ordering.
Compare link-down/unavailable-descriptor/timeout with link-up/bad-FCS/missing-
route. Actually corrupt and repair a teaching frame, then invoke the route
admission; do not merely print a classification table. Find the first accepted
handoff that differs, not the last timeout. Separate a repair of the bad frame
from suppression of the later missing-route symptom.

**T39.10.c, costs.** Compute IP, frame-with-FCS and accounted wire-slot lengths
for32 application bytes at100Mbit/s and zero at10Mbit/s. Headers are20-byte TCP,
20-byte IP and14-byte Ethernet, minimum frame64 including4FCS, plus8 preamble/SFD
and12 byte-times of interframe gap. Compare serialization-only nanoseconds with
end-to-end delay; exclude processing, propagation, ARP, handshake, ACK, response,
contention and retransmission explicitly. No instrumented timing is invented.

## Deliverables, verification and rubric

D1: unaided byte/ownership/time predictions, assumptions and invariants for each
original criterion. D2: completed sources, provider identity graph and actual
command/flags/map/assembly/stack evidence; keep host tools separate from target
dependencies. D3: independent expected bytes/intervals, first-failure diagnosis,
mutation counterexamples and stated scope limits. D4: each exact original V1
task attempted after its minimum seven-day delay, with a fresh withheld case
and retained unaided response. Do not copy assessor implementations.

Run selectors0..5 and the independent arithmetic/literal oracle. The ordinary
route and drift stubs must compile yet fail semantic assertions. Mutate
next-hop identity, final-IP rewriting, drift omission, anchor uncertainty,
padding budget, HTTP encoding, coverage-hole completion, overlap and deadline
renewal separately. Require the intended property assertion, not merely a crash
or unrelated compiler failure. Repeat GCC/Clang O0/O3 and ASan/UBSan, then audited
M4 CPU execution where available. This finite campaign cannot prove arbitrary
input safety, interoperability, learner independence or absolute mastery.

For **each objective**, level0 violates its named contract or invents evidence;
level1 reproduces output but cannot explain its namespace/ownership/interval
premise; level2 implements and derives its contract, diagnoses an unseen
boundary failure and bounds resources; level3 independently performs the exact
changed task with new evidence and explicit revised limits. Assess .09a's
next-hop/final identity, .09b's actual imports and terminal ownership, .09c's
new representation/failure claims, .10a's rounding/admission, .10b's causal
handoff and .10c's equivalent accounting separately. Shared test success cannot
award an objective whose reasoning was not assessed.

Hints, in order after saving the unaided attempt: (1) label which owner and
namespace each field belongs to; (2) split successful publication from pending
ownership and list every terminal obligation; (3) derive one boundary by hand:
a remote IP with a local gateway, an eight-byte reassembly hole, or a one-tick
drift bound. The assessor holds exact traces and further counterexamples.

## Security, validation and limits

All executable inputs target original curriculum programs, synthetic arrays and
isolated CPU/register models. No network interface or deployed peer is opened.
An attacker here can choose finite malformed packet fields, fragment overlap,
arrival times and stale local events. The harmless observations are refusal,
bounded capacity, unchanged output or controlled model state. Checksums detect
some corruption; they do not authenticate senders. No crash or sanitizer result
is evidence of arbitrary code execution. Physical work uses a separate dedicated
board bench and requires identity, topology, capture span, clock uncertainty,
fault containment and cleanup records before any physical claim.

Current source-bound author results live in the handoff validation receipt;
unrun or failed attempts remain explicit. External review, independent learner
assessment and physical capture/clock observations remain separate not-run
dimensions. Existing accepted records are not rewritten by this successor.
