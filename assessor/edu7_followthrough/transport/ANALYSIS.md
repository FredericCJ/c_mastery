# Withheld transport assessment analysis

<a id="t4005a"></a>
## T40.05.a

The original25-byte message creates network packets38 and15bytes; ACKs14bytes.
The accepted duplicate first does not append, identical final re-ACKs, altered
final rejects without a published ACK. Stage the existing primitive because it
writes ACK early. A complete120-byte SDU holds credit0; retaining final peer
credit across sender messages matters because the primitive final-ACK path exits
before updating its credit field. Probe-response credit is not acknowledgment
of an unaccepted DATA fragment. Every length1..120 under both profiles is a finite
author check, not exhaustive input/state proof. Short-capacity failure and a
compiled duplicate-publication mutant distinguish superficial output matching.

<a id="t4005b"></a>
## T40.05.b

Accepted250 starts deadline350. Refusal at350 leaves retries0/due350, acceptance
there consumes retry1/due450; further accepted450/550 consume retries2/3. Failure
at650 is400ms after the first acceptance. It is not universally400ms wall time
from an earlier offer: refused submissions can shift actual accepted origins.
The changed schedule must state them explicitly. E7Sender's actual offer/accept
API is consumed, not renamed by a solved transport library. Epoch and increasing
offer-token checks prevent old completion callbacks acting on a reopened owner.

<a id="t4005c"></a>
## T40.05.c

A first fragment at0 with duplicate499 expires at500. Retiring ID1 after expiry
prevents an old first fragment from allocating another500ms resource window.
Completed final receipt and ready upper data are distinct and must survive an
unrelated sender failure. Blocked work beginning1 expires501 despite probes.

Original L3 protocol4 contains no epoch. Resetting both local controls can make
old ID1 indistinguishable; a boolean label supplied by the test is not a wire
solution. Explicit profile2 protocol0x84 adds BE16 epoch before the old8+24L4
bytes, reaching34payload/40network bytes. Actual original codecs operate on
staged validated subobjects; the selected-tag adaptation is explicit. Old packets
from epoch1 cannot attach after epoch2, but rewriting the public prefix to2 is
accepted by a fresh original target. This counterexample bounds the guarantee:
namespace separation under unchanged old traffic, not authentication. The later
router/profile integration and physical exchange remain required.

Require independently attempted original V1, rationale, false-claim diagnosis
and residual limits; never award independence from the author's campaign.
