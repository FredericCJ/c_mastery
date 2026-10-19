# Assessor-only IPv4 and reassembly analysis

## 05a

Check n before header reads, then total>=20 and total<=n before constructing the
payload span. Offset multiplication is bounded by13 bits; require offset<=1480
before evaluating remaining capacity. IHL6 is unsupported by the chosen profile;
it is not automatically invalid IPv4. A stale checksum would reject all changed
header fields for the same reason, so the semantic matrix recomputes it. The
314 rejects include33 prefixes,13 checksum-correct semantic defects,264 individual
bit flips, one65-byte Echo and three fragment extent/granularity cases. This
does not exhaust all2^12000 packet strings. Mutants1/2 skip length/checksum checks;
both must reach a named semantic assertion, not sanitizer termination.

Changed original V1: construct the actual two-slot reassembler, then change to
one slot and NI_IDENTICAL. Reorder fragments, preserve a hole, send two protocols
with the same ID, fill both slots, retain READY across expiry, overlap an already
covered span, contradict final length and retry at expiry-1/expiry. Prediction
must include state and output, not just the return code. Require the six R-series
competencies in taxonomy_extensions.json separately. An interval tree is a valid
alternative if it proves bounded nodes and identical admission/overlap semantics.
Do not award full credit to a high-water counter or a timer restarted per fragment.

The bitmap invariant is: covered equals the number of set bits, and every set
bit owns exactly one initialized byte. A final bound never permits data outside
its interval. Thus covered==total implies no hole only because all coverage lies
inside[0,total); first and final are separate required facts. The maximum data
extent1480 is deliberate, not a universal IP limit. Dictionary/set oracle uses
byte keys and interval endpoints rather than the C bitmap/counter. Exhaustive
permutations cover304 two-policy orders of2..5 fragments, plus600 seeded mixed
histories and boundary lengths including556/1480. Source identity still needs
review; a second representation is not an independent human reviewer.

## 05b

For bytes61,62,63,64,65 the words are6162+6364+6500. Fold carry once, complement,
and serialize the numeric result high byte first. Byte61 alone gives9eff. Every
chunking retains parity; a chunk-local zero pad creates a different sequence.
An alternative byte-parity weighted sum is valid if it prevents accumulator
overflow and proves concatenation parity. allFF131076 bytes falsify unchecked
32-bit accumulation past65537 words. Compiler optimization cannot repair modular
overflow after information has been lost. The C17 unsigned wrap is defined but
the mathematical checksum is wrong. Do not call this signed overflow or UB.

V1: present allFF at every split, then append one byte after repeated finish.
The expected result must come from arbitrary-precision arithmetic with end-around
carry, not a second copy of the C loop. TCP pseudoheader's twelve bytes must be
concatenated with the payload before assigning parity. Existing net_checksum
adds a differential implementation; agreement alone is insufficient, hence the
independent contiguous bound and modular Python oracle. The compiled
S_MUTATE_CHECKSUM provider pads each call; a first odd chunk must expose it.

## 05c

The Echo response changes type and both checksums, not identifier or sequence.
For n payload bytes, IP total=28+n and pre-FCS Ethernet=max(60,42+n). Padding
does not enter IP or ICMP checksums. Compare all defined IP bytes and padding
against the actual frozen net_receive after a real original ARP request has
established association. Directly setting peer_known is insufficient integration.
The65 length cases include0,5,56,64; new reassembly feeds64 of them into the old
parser after that parser has demonstrably rejected a fragment. No fallback to
a different parser is permitted when this composition fails.

V1: preserve the original default56/max64 capture task. Hand-construct a56-byte
request first, then compare actual dedicated-link capture only when available.
Require source/destination, checksums, count of capture bytes versus padded frame,
FCS availability and offload point. An authored template is not a capture.
Reject a bytewise expected result derived from the same implementation alone.

## Reassembly security and failure reasoning

The original vulnerable mutant5 overwrites overlapping bytes. Feed offset0/MF1
and a changed overlapping fragment, followed by the missing suffix: the harmful
property is contradictory datagram interpretation, confined to inspectable
arrays. Correct strict mode poisons on any overlap; identical mode first checks
all shared bytes, then publishes new ones. Both retain a tombstone until the
original deadline. Neither authenticates the sender nor prevents resource denial.
Mutant4 omits protocol from identity; it joins ICMP and TCP fragments carrying
the same ID. Mutant7 promotes a first+last pair with a hole. Assertions catch
both before uninitialized bytes can be delivered. Mutant6 refreshes expiry on
duplicate arrival, allowing indefinite retention. Test the original deadline
directly. Use no undefined golden output or exploitability claim beyond these
observed property violations.

RFC791's example later-wins overlap and TTL-based timer are historical algorithm
choices. RFC1122's fixed reassembly timeout recommendation motivates the60000ms
profile. Contrast the texts precisely. `ni_expire` emits type11/code1 with the
first original header and8 bytes, before discard. Missing fragment zero, ICMP
error input and tombstones emit none. Mutant8 suppresses the valid report. The
wire adapter must first reject link broadcasts and use ni_expire before receive;
calling only ni_tick loses required feedback. This packet-array package does not
claim a full RFC1122 implementation, options handling, routing, general ICMP,
arbitrary datagram sizes or completed actual T39/physical binding.

## Withheld transfer administration

Administer after>=7 days, without reference source. Keep original V1 identities.
Choose an unseen sequence from two noncommuting policies: strict duplicate
poisoning versus byte-identical overlapping resegmentation. Require the learner
to prove which state can be shared and which transition differs, then implement
both and derive resource bounds. Provide a late fragment at60000, a READY
datagram at60001 and an ID reuse at60002. Only ACTIVE/POISON expire; READY remains
owned until take, while take after expiry creates a tombstone eligible for the
next tick. Explain that the old ID may be reused after that explicit window.
Change scratch layout and input alignment, preserving output nonaliasing. Require
a fresh smallest counterexample to one overstated safety equivalence. Record
actual attempt time, source hashes, hints used, assessor identity and each rubric
score. The author campaign cannot populate this record.
