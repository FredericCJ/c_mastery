# Cross-layer handoff assessor analysis, revision 1

Withhold this file and private implementations from learner packages. The public
contract and original criterion/V1 are cumulative. Passing this author campaign
does not administer independent assessment or measure a physical link.

<a id="t3909a"></a>
## T39.09.a

192.0.2.2/24 reaches192.0.2.9 directly;198.51.100.9 uses192.0.2.1 as the link
neighbor. The enclosing destination MAC changes, the final IP bytes do not.
For the40-byte envelope output is60 bytes before FCS, with6 bytes zero padding.
The route table and HTTP authority are independent namespaces. `eh_next` admits
only its explicit narrow profile; RFC1122's multiple gateways, route cache,
redirects and special destinations remain excluded. Withheld V1: /30 local
192.0.2.1, gateway192.0.2.2, destination198.51.100.9; network.0 and broadcast.3
are invalid local destinations. Require unchanged output on failure. The peer
record is trusted configuration, not an authenticated resolution result.

<a id="t3909b"></a>
## T39.09.b

Trace actual EnRing→NbNet→Ni→Nc→stream→Nh→HTTP parser/encoder and the reverse
copied TX path. The public raw peer reconstructs sequence bytes, not a hosted
socket response. Active close leaves TIME_WAIT; advancing the same owner's
absolute clock to `timewait_until` closes it. Passive peer-close can instead
finish CLOSED; do not demand TIME_WAIT for every closure direction. An early
pilot used passive close then asserted TIME_WAIT and failed; the final test
selects active close explicitly, preserving the retirement obligation.
Borrowed frame storage cannot survive RX release. TX admission copies data but
does not mean DMA completion; TCP ACK and device acknowledgement are distinct.
Withheld V1: stop after ACK while model TX remains owned, then allow terminal
device acknowledgement; reject releasing buffers at application DONE. Require
the learner to map actual functions and pins, not a component name alone.
Current NbNet is same-link; routed-envelope evidence cannot extend that scope.

<a id="t3909c"></a>
## T39.09.c

The changed feature requires1480 bytes,185 coverage bytes and retained header,
identity, final length, deadline and publication state. Offset16 plus offset0
leaves a hole8..15; highest seen byte24 is insufficient. Taking after offset8
returns44 bytes, zero fragment bits and a new valid header checksum. Copied
payload bytes0..23 must match the independent fragment generator. Tombstone
through60000 prevents immediate duplicate redelivery; expiry allows reuse.
New identity at60000 has fixed120000 deadline: identical duplicate at60001
does not extend it, conflicting byte at60002 poisons it. These policies differ
from RFC791's illustrative later-arrival-wins overlap and extending timer.
Withheld V1: use a different protocol with equal source/destination/ID, or fill
both slots then send a third identity. Require separate bounded refusal and no
cross-protocol assembly. General options, congestion and interoperability remain
unproven; a standard's example is not a complete modern security policy.

<a id="t3910a"></a>
## T39.10.a

Offset100 yields100,105,120 but cannot reorder0,20,5. The changed bound is
[L+d-ceil(d*p/10^6),U+d+ceil(d*p/10^6)]; test small d=1 because truncation would
incorrectly eliminate uncertainty. At d=2000000,p=100 the interval is
[1999898,2000302]. At UINT32_MAX,p=1000 drift4294968 yields
[4290672327,4299262263]. The quotient/remainder formula uses only bounded32-bit
division: r*p<=999999000, q*p<=4294000, totaldrift<=4294968. uint64 additions
are admitted before publication. Withheld V1: nonzero anchor100, local101,
global[500,503],p=1 yields[500,505]; two overlapping intervals cannot establish
strict causal order. This is a declared rate-bound model, not measured accuracy.

<a id="t3910b"></a>
## T39.10.b

Declared prerequisite order is link, descriptor, frame, route, transport,
application. Link-down wins over downstream symptoms. With bits0/1 admitted,
corrupt frame gives first2; repair only the byte/FCS relation, leave absent
route, and first becomes3. A route repair cannot retroactively authenticate a
bad frame; FCS success still grants no sender authenticity. Withheld V1:
correct bytes but wrong neighbor association: frame admission succeeds and
the route/association boundary refuses atomically. Assess minimal counterexample
and actual accepted handoff before the next transition, not a memorized index.

<a id="t3910c"></a>
## T39.10.c

IP=application+40; frame=max(64,IP+18); slot=frame+20. For32 bytes at100Mbit/s:
72,90,110 and8800ns. For0 at10Mbit/s:40,64,84 and67200ns. The latter includes
6 bytes padding. Neither is total request latency or throughput across ACKs,
two directions, loss and queueing. Withheld V1: compare a tagged minimum frame
under the separately stated tag contract; determine whether the counted gap
and capture span include preamble/FCS before recomputing. Count successful
contracts equivalently before any performance comparison.

## Evidence review

Reject intended mutants only at their named semantic assertions. Verify all
source hashes, actual object closure and target disassembly. Sanitizer success
covers exercised hosted paths only. The M4 harness runs serialized synthetic
models with IRQ masked; the conservative sum of retained stack frames assumes
known nonrecursive callbacks and makes no physical timing or watermark claim.
Separate missing source bodies, physical captures and independent assessment
from completed author implementation. Do not accept successful HTTP output as
proof that an arbitrary exclusion from the original scope has been implemented.
