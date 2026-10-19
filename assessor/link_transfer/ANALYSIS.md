# Assessor analysis of the exact service-boundary variants

Keep this separate from learner packaging. The author implementation supplies
bounded technical answers, not independent review or learner acceptance.

## Cycles

For modulus15, forward14→0 is1 while `(0u-14u)&14u` is2. Forward differences
0→5,5→10,10→0 are all5, so the relation is nontransitive. Odd cycles remove
an antipode without producing a total order. Counted outstanding arcs can
reject delayed14 after ACK0 only while the old packet cannot survive a whole
reuse cycle. Require the learner to show a full-cycle alias counterexample.

## Borrow and partial admission

The prefix is a separate span: its final byte is not the last payload byte.
`used` counts both spans, and `offered` bounds one downstream copied acceptance.
An accepted prefix is irreversible; reset confirmation is an external premise.
The no-progress cancellation path releases immediately because no bytes escaped.
Zero-length input terminates after the prefix and must never form NULL+0.
The partial transport adapter owns encoded bytes and must rebind its embedded
borrow after staging the control; an uncorrected pointer would target a dead
automatic object. Its50ms admission timer and the sender's100ms acknowledgement
timer are different contracts. The former bounds unfinished lower publication;
the latter starts only after the full record is copied. An early retry can
otherwise interleave duplicate fragment prefixes into one stream.

## Padding-dependent trailers

For12 bytes, add3→15, align8→16, trailerceil(16/16)+2=3→19.
Next add5→24, align7→28, trailerceil(28/16)+1=3→31.
Each addition is checked before execution; the ceiling uses quotient/remainder
instead of potentially overflowing n+15. Stage all eight outputs so a later
error cannot publish a successful prefix of results. The same arithmetic
does not imply that a corresponding protocol encoder exists.

## VLAN scope

The equal MAC maps to port1 in VID10 and port2 in VID20. One shared MAC-only
table overwrites the first location. Separate tables are a valid bounded
representation with hard partitioning; they may refuse one VID while the other
has spare capacity. No automatic VLAN membership/untagged admission policy is
provided. Broadcast flooding in two parallel bridge links still recurs in
each VID; keying and loop control address different properties.

## Alternate PHY

The inspected TI tables establish the bounded revision0 interpretation used
here; new revision values are refused rather than assumed compatible. Five
actual bounded MDIO reads obtain identifiers, two BMSR samples and PHYSTS.
The first low sample retains history even if the second is up. Full-duplex
and speed are meaningful only under admitted link/negotiation premises.
The zero speed bit means100, opposite a tempting positive-bit intuition.
PHYSTS does not clear BMSR's link latch; two reads of an arbitrary status
register do not substitute for the required destructive sample sequence.
None of these model observations proves board wiring or physical completion.

## Fresh assessment packets and evidence limits

Use modulus21 with an arc crossing20→0; introduce an odd-cycle three-way
ordering counterexample. Split a prefix after its first octet and cancel
after one body byte. Change wrapper alignment to3 and place overflow in the
third trailer. Learn the same MAC twice on each VID, then age only one.
Supply the correct PHY ID with half-duplex or loopback and a read error after
current-link observation. Let the partial service accept no byte until deadline
equality. Require predictions and ownership ledgers before execution; do not
award transfer from memorized supplied observations.

Review the exact semantic marker for every mutant. A build failure or crash
does not count as detecting it. Six host profiles and four Thumb runs observe
only the declared paths. No physical PHY, bus, coherent-DMA target, external
review or independent learner is represented by these author tests.
