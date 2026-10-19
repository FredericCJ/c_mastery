# IPv4 admission, checksum parity and bounded reassembly

This required successor implements PR-T41-05 and its three original delayed
variants. X-T41-001 remains the prerequisite checksum exercise. The additional
X-T41-002 decomposes T41.05.a's fragmentation variant into independently assessed
competencies; it does not change accepted units, hours or historical receipts.

## Premises and progression

Use ISO C17, 8-bit octets and exact-width uint16_t/uint32_t/uint64_t. Integer
fields on the wire are big endian, regardless of the processor. Do not cast a
packet pointer to a struct or uint16_t pointer. Supply valid initialized objects,
actual readable spans and disjoint writable output/state. All calls are externally
serialized; neither a C pointer nor a checksum proves ownership or authenticity.
Configuration names two distinct, administratively selected unicast hosts on the
isolated teaching link. A link adapter must reject broadcast/multicast and retain
actual frame length before this IP interface. Link admission is a separate contract.

First derive contiguous checksums by hand and complete X-T41-001. Bring that
learner's `SChecksum` provider, the T41.01 length/ownership dossier and original
T41.04 association component. Next implement IP admission and Echo; diagnose the
field matrix before adding reassembly. Only then compose completed datagrams
with the actual original `net_receive`/`net_checksum` implementation. The public
`ip/ip.c` legacy template and exercise `src/ip.c` are intentionally incomplete.
In an X-T41-002 exercise workspace edit `src/ip.c` and `src/checksum.c`; files
outside `src/` are the trusted harness. The author reference is never a learner artifact.

Hosted GCC/Clang are verification environments. `run.py --systems <your-systems.c>
--source <your-ip.c> --build <empty-build>` selects explicit sources. For Cortex-M4,
`target_build.py` requires every provider and header root explicitly. Hosted author builds select the unchanged two checksum definitions through
`checksum_slice.py`, retaining full-source and generated-body identities. This
explicit adapter avoids an unrelated reducer warning in the frozen multi-mechanism
translation unit; diagnostics stay enabled. Learner imports are explicit and are
not silently rewritten. The target compiles the full original provider. Its direct
link has eleven original objects, no libc/libgcc/compiler-rt/libatomic/HAL. The
declarations in newlib headers do not authorize linking newlib. Audit object
imports, discarded sections, linked undefined symbols, map, assembly and stack
files. MPS2 packet-array execution is CPU evidence, not Ethernet or F439 observation.

## T41.05.a: validate before interpretation

`ni_parse(config,p,n,fragments,out)` admits n=20..1500, version4/IHL5, IP total
20..1500 no larger than n, nonzero TTL, protocol1 or6, configured addresses and
a valid header checksum. Reserved flags are zero. DF cannot coexist with MF or
nonzero offset. With fragments=0, any MF/offset is rejected. With fragments=1,
nonfinal payloads are nonempty multiples of8; noninitial payloads are nonempty;
offset*8 plus payload must fit1480. These limits intentionally exclude options
and larger datagrams. Successful output borrows only total-length bytes; trailing
Ethernet padding never becomes transport input. Failure leaves `NiView` unchanged.

Explain each subtraction and multiplication bound before pointer formation.
An untrusted total length is not an allocation size or readable extent. Distinguish
an unsupported valid IP option from malformed memory access. TTL is not a local
millisecond timer. Predict a padded33-byte packet, IHL6, MF, offset8, TTL0 and
total34 in a33-byte extent. Recompute checksum after semantic mutations so a
stale checksum does not mask the field under investigation.

## T41.05.b: checksum state and cost

Reuse `s_checksum_add/finish` from the actual X-T41-001 contract. Its representation
is a folded sum, a pending flag and one high-order octet. Add borrows its input
only during the call; finish does not mutate state. Byte61 alone is the word6100,
whose complement is9eff. Adding62 after finish must produce9e9d. Padding a chunk
boundary is incorrect. Empty chunks do nothing; the final odd byte alone gets a
zero low byte. A returned number is not a wire-order object representation.

Derive the largest unfolded32-bit sum of 16-bit words:65537*65535=UINT32_MAX;
the next all-ones word exceeds it. The actual provider folds every pair, so the
pre-fold sum is at most131070 and the folded result at most65535. Show why adding
a final pending octet also fits. The tests pass131076 all-FF bytes in257-byte
chunks, then compare every split of lengths0..257. Also feed a TCP pseudoheader
and odd payload without intervening padding. Work is O(n), state O(1); this is
error detection and has easy word-permutation collisions. No authentication or
memory safety follows from a zero checksum. C23 does not supply a network parser,
ownership checker or automatic checked pointer arithmetic; the same C17 argument
is required. Exact published C17/C23 text access remains a separate source gap.

## T41.05.c: construct exact Echo responses

`ni_echo` validates the fixed unfragmented profile, ICMP type8/code0, correct
whole-ICMP checksum and8..72 ICMP bytes, then writes a complete IP reply. Return
type0/code0, preserve identifier, sequence and all0..64 payload bytes; swap
configured addresses; emit IHL5, ID0, DF1, TTL64 and new IP/ICMP checksums. Invalid
input leaves all output bytes and length unchanged. Successful calls define
only bytes[0..length); the Ethernet serializer supplies zero padding to60 bytes
before FCS. For payload6162636465/id1234/seq1 derive IP33, Ethernet47, pad13 and
wire64 including FCS. Maximum64 and ordinary56-byte Echo data require separate
captures. A host displaying an outbound checksum before offload is not proof of
a bad wire checksum. The physical protocol is required, presently unobserved.

## Required reassembly extension: X-T41-002

`NiReassembly` contains one or two statically owned slots. Each slot has1480 data
bytes, a185-byte coverage bitmap, the first20-byte header and identity/control
fields. Identity is (source,destination,protocol,ID); the local configuration
constrains the first two without removing them from the key. State is FREE,
ACTIVE, READY or POISON. Initialize only fresh/quiescent state; reinitializing a
live object discards ownership and is forbidden. No allocation or payload borrow
survives a call. A successful receive copies data and returns slot index; WAIT
means an admitted incomplete datagram, OK means READY. BUSY refuses new identity
when capacity is exhausted or refuses additional fragments for READY. Index is
a synchronous owner-local handle, not an asynchronous generation token.

All payload bytes must be present, fragment zero must be seen and exactly one
consistent final length established before READY. A high-water mark is not a
coverage proof: offsets0 and16 leave an8-byte hole. Reject conflicting final
lengths, bytes beyond a known end and a nonfinal fragment reaching that end.
Keep the first admitted offset-zero header. `ni_take` copies a complete datagram,
updates total length, clears fragment flags and recomputes header checksum.
It publishes no partial output. READY is retained until take, even after its
deadline: the application owns that bounded capacity and must drain it.

Policy NI_STRICT poisons the identity on *any* overlap, including duplicates.
NI_IDENTICAL permits partial overlaps only when every shared byte agrees; any
disagreement poisons the whole identity before publishing any new byte. Both are
explicit teaching policies, not a claim that RFC791 mandates strict rejection.
Its example chooses later overlapping data; demonstrate the resulting ambiguity
in the assessor's isolated model. A POISON slot consumes capacity until expiry.
After take, retain a tombstone through the original deadline to prevent immediate
duplicate redelivery. After that boundary ID reuse is possible: this is not
permanent replay protection. Poisoning can deny service; quotas bound memory but
cannot ensure fairness or authenticate a peer.

Use monotonic, unwrapped uint64 milliseconds, fixed timeout1..120000 for testing,
60000 in the normal laboratory profile. Addition must fit; backwards time is
invalid. A new slot's absolute deadline never moves on duplicate/partial arrival.
At now>=expiry ACTIVE/POISON become FREE. `ni_tick` is a pure-model discard
operation. The wire adapter must call `ni_expire` before each receive/timer poll:
it first copies up to two type11/code1 reports into caller-owned output[2], then
expires state. The two outputs, state, count and input must be disjoint. It quotes
the unchanged first header and first8 data bytes. No report is generated without
fragment zero, for a tombstone or for an ICMP type outside Echo request/reply;
the admitted ICMP teaching scope excludes other queries. Only administratively
selected unicast link input is authorized. Error-on-error and broadcast storms
must be prevented by these preconditions and the outer link adapter. Reports
must be submitted through the copied-transmit port; constructing one does not
prove delivery. `ni_receive` performs model expiry too, so omitting the preceding
wire `ni_expire` silently loses required timeout reports and is an integration bug.

For malformed input or invalid time, no state/output is published. For valid
input, expiry is an observable transition even if admission later returns BUSY
or CONFLICT; distinguish that from a transactional promise. A conflict preserves
stored bytes while retiring their deliverability. Explicitly explain the zero-data
whole-packet conflict in this reassembly-only path; ordinary unfragmented zero-data
IP is handled by `ni_parse`, not a claim that zero-length IP is generally invalid.
Receive costs O(slots+payload), take O(total), expiry O(slots+quoted bytes).
No recursive calls, dynamic storage or implicit operating-system timers occur.

## Evidence, deliverables and assessment

Submit D1 field/parity/coverage and lifetime arguments, hand-derived packet bytes,
and a premise table separating ISO C, protocol, ABI, emulator and board claims.
D2 includes source, explicit provider identities, actual integration adapter,
failure behavior and resource accounting. D3 contains per-objective positive and
negative traces, independent expectations, rejected compiled mutants and exact
commands/tool/source hashes. Explain the first divergence; a compile failure is
not a semantic mutant kill. D4 is the withheld changed-condition attempt after
at least7 days, including new evidence; author execution is not that attempt.

Pass author checks only with all selected native profiles, independent dictionary/
set oracle, actual old parser composition and target runtime/resource audit.
The field matrix must preserve sentinels, hole/identity/overlap/expiry mutants
must fail the intended assertions, and the learner stub must compile then fail.
For every criterion, level0 violates its invariant; level1 repeats an ordinary
trace; level2 derives it, implements it and diagnoses the discriminating failure;
level3 independently transfers under changed assumptions. Require level2 on every
applicable criterion. Finite exploration, sanitizers and observed timing are
bounded evidence, not proof of absolute mastery.

Hints after preserving an unaided attempt: H1 draw byte positions and owners;
H2 contrast the highest received offset with the set of received offsets;
H3 separate final-length knowledge, byte coverage, first-header ownership and
deadline. For checksums, finish after one byte, then append its partner. Private
analysis and unseen variants live under `assessor/network_followthrough/ip`.

All adversarial packets target these original local arrays or dedicated teaching
emulators/boards with synthetic addresses. The harmless violation is acceptance
of altered checksum-colliding data, premature datagram delivery or mixed identity;
no crash or undefined behavior is labeled code execution. Do not transmit to
external networks, modify host security or flash hardware in this authoring work.
Physical capture, non-author review and independent/delayed learner attempts remain
separate. See the source register for the exact inspected RFC passages and gaps.
