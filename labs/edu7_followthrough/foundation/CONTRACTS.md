# EDU7 boundary, link, router and representation successor, revision 1

This required additive laboratory supports PR-T40-01,03,04,07 and all twelve
unchanged ordinary criteria and seven-day V1 tasks. Read those original cards
and `research/enhancement/networking_contracts.json` T40 family first. Their
hours and historical evidence remain descriptive and unchanged. This laboratory
adds missing implementations and experiments; it does not close the separate
physical, transport/session or whole-stack projects.

## Prerequisites and environment

Import the learner's actual `com_edu7.h` framing, CRC, network and record
implementation and `edu7_depth.h` link-admission component. They are prerequisites,
not replaced by a protocol library. Implement `foundation.h` in `learner.c`.
`codec_stubs.c` supplies explicit incomplete prerequisites for an isolated
workspace; replace it with the actual completed learner producer before claiming
composition. The historical codec scaffold has unused named parameters under
strict warnings. It remains unchanged; these compiling stubs permit semantic
rejection, without suppressing warnings. Author mechanism-stub tests keep the
actual completed prerequisites and replace only the new mechanism.
Use ISO C17, 8-bit octets and exact-width integer types. Host GCC/Clang and
sanitizers are verification tools; the target is strict freestanding Cortex-M4,
with explicitly named original byte helpers, startup and direct objects, no
libc/libgcc/compiler-rt/HAL. Newlib is not linked. Inspect every implicit helper,
map, retained assembly and stack bound. The changed host uses an actual i386
ILP32 runtime in addition to native LP64; byte equality is tested rather than
assumed from `sizeof`. These are implementation/ABI observations, not ISO C
requirements. Exact published C17/C23 access remains separately unresolved.

All controls start zeroed and remain live under one serialized owner. Inputs,
outputs and control objects are disjoint valid initialized spans. No function
can make a forged pointer safe. No callback, interrupt, concurrent thread or
device may mutate them. Scalars are runtime checked; lifetime, immutability and
truthful lower completion are programmer/adapter obligations. Each decoder
version is fixed for its initialized control lifetime. Never reset a live owner
to reuse generations or discard an outstanding borrowed span.

## Problem, mechanisms and contracts

**T40.01.a:** draw seven service and peer boundaries at A/R/B from the actual
wire contract, then compare a framed-radio lower service. A framed radio may
already delimit lower packets; this does not supply EDU7 addressing, grant
authority, version, error policy or upper reliability. The executable witness
recovers an original local body and independently checks its CRC, then shows
that valid framing does not grant an ungranted DATA delivery. No actual radio
hardware or ISO protocol suite is implemented. The X.200 reference model does
not define these original wire bytes or seven required software libraries.

**T40.01.b:** `ef_budget(schema,extra,out)` returns canonical L6 maximum, session
size, fragment count/final length and fixed outer maxima. Schema1 allows extra0
only: canonical33, session40, fragments24+16. Separately derive the conservative
80-byte envelope→87→24+24+24+15. Maximum generated L4/L3/L2 body is32/38/45;
generic L2 capacity remains47 and escaped upper bound96. Schema2 is a separately
negotiated extension: a32-bit value adds2, optional tag6 contains1..8 uninterpreted
bytes and adds2+length when present. With extra8 canonical45, session52 and
24+24+4. An absent extra uses0; schema1 rejects the new field. Do not confuse the
conservative envelope with a valid schema1 record or silently extend it.

**T40.01.c and T40.04.c:** implement a single egress `EfRoute` slot, copied or
borrowed by explicit mode. `ef_enqueue` first decodes the complete network PDU,
applies routing and copies the decremented header. Copy mode owns its payload
before return: poisoning the caller input while a grant is delayed must not
change egress. Borrow mode retains only the input payload pointer; caller keeps
it live and immutable until exact-token release. Source/header/control memory
must not overlap. FULL generation exhaustion, BUSY slot occupancy and DROP
admission failure preserve state and token output. Tokens increase without
wrap. `ef_peek` copies a complete local `EduFrame` into caller storage without
consuming the slot. `ef_release` requires the current nonzero generation and
actual end of all downstream access; stale or duplicate releases change nothing.
Lower BUSY cannot release. A lower copy-accept may end a borrow; a retained
asynchronous send requires its later completion/quiescence. The scalar token
does not itself prove either fact.

**T40.03.a:** retain actual EDU7-v1 CRC and encode/decode primitives. The wrapper
stages output and checks capacity. All-delimiter/escape payload, exact header,
CRC-before-escape and two delimiters must match the separate polynomial oracle.
The generic96-octet bound is conservative: legal fixed headers cannot all be
escape bytes. Changed frame profile2 uses version byte2 and escapes7e/7d with
XOR40 instead of XOR20. CRC still covers the unescaped bytes, including version.
This is an explicitly selected incompatible teaching protocol, not an accepted
v1 change or a new polynomial. Record which interoperability obligation changes.

**T40.03.b:** `ef_frame_byte` returns1 only for a complete validated frame,0 for
incomplete/no publication and-1 for an invalid candidate. It applies a strict
canonical-escape adapter to retained v1: only suffix5e/5d may follow7d. The raw
retained decoder accepts additional equivalent escapes; demonstrate that
counterexample before explaining the stricter successor. Profile2 allows only
suffix3e/3d. UART error invalidates only the current candidate. An overlong body
discards until delimiter; a trailing escape is rejected at delimiter; the next
candidate may succeed. Reuse one parser across every split of noise/overlong/
truncated-prefix plus valid frame. Coalesce valid/error/valid and retain the two
independent completed publications. Do not roll back an earlier completed frame.

**T40.03.c:** after syntax/CRC, apply actual `e7_link_admit` with local address,
grant, transmitting and UART fault. Wrong destination and ungranted DATA produce
no delivery. Build a changed original payload and recompute its public CRC:
the harmless observation is acceptance of modified bytes by the original local
target. Explain why a larger unkeyed CRC cannot authenticate an attacker who can
compute it. This does not authorize any deployed peer or imply arbitrary code
execution. Repair the overstated trust claim and retain independent upper
authorization; an authenticated protocol would require a separately specified
key/identity/replay mechanism, not a longer CRC.

**T40.04.a/b:** routing profile1 calls the actual retained `edu_route`: left
A17→B51 goes right/link1→2; reverse B51→A17 goes left/link2→1. Payload ports7/8,
request ID and checkpoints remain opaque and unchanged. At forwarding R, hop4
becomes3; local `ef_local` delivery at B accepts hop1 unchanged. Refuse own-origin
34, unknown route, forwarding hop<=1, same-interface route, protocol/length
mismatch and occupied slot. The changed route profile2 adds destination68 via
right/link2 from left only; it leaves payload and application authority opaque.
It does not change the L3 version1 byte format. Endpoint68 and route configuration
must be agreed separately; this is not discovery, authentication or a default
route. Source identities are untrusted packet claims, not verified principals.

**T40.07.a:** `ef_encode` stages actual schema1 encoding into33 bytes before
publishing; `has_value`/`has_unit` must be boolean. SET42/-4000/unit1 is15 octets
with valuef060. No native struct representation is sent. In schema2, `EfWide`
retains the original verb/request/unit/label fields, but `value` is int64 input
admitted only in INT32_MIN..INT32_MAX and encoded as four big-endian octets.
`base.value` is ignored during encoding and is zero in decoded schema2 output;
only the wider `value` is meaningful. The four-octet transfer value is not a
promise that a changed host has a particular `long` width. Reconstruct both
INT32 extrema and reject wider input before any output write. Schema1 must
reject schema2; do not alias two incompatible formats.

**T40.07.b:** `ef_wide_encode/decode` stage output too. Optional tag6 follows
ascending unique tags1..5, has1..8 uninterpreted bytes, and no application effect.
Input is bounded by45 bytes and rejects unknown/duplicate/reordered fields,
invalid signed length, invalid unit, missing required fields or noncanonical
UTF-8. Both versions retain the actual codec's canonical UTF-8 validation:
reject surrogate ED A0 80, overlong C0 80, values above U+10FFFF and truncation.
Test four legal four-octet codepoints at16 label octets. Buffer chunks need not
align with codepoints: reassemble a complete bounded L6 SDU before decoding.
One-byte-short encoder capacity preserves **all** output bytes and written count.

**T40.07.c:**12501 is representable but the actual application rejects SET and
retains its old setpoint. `%n%` is label data: never pass it as a format string.
Change a display label from ASCII to four four-octet codepoints and recompute
serialized/session/fragment lengths. Numeric unit1, request identity, route and
token authority do not change. Source interfaces carry no route/session control
pointer; do not infer authority from successful presentation validation.

## Deliverables and verification

D1: original ordinary predictions and service/peer/namespace/ownership ledger,
with both canonical and storage calculations. D2: implementation plus exact
provider identities, diagrams and emitted bytes; audit target dependency/stack
closure. D3: independent polynomial and signed-byte oracle, unchanged-state
sentinels, all splits, poisoned-copy and valid borrowed-release schedules,
compiled defect diagnoses and actual changed-host observations. D4: the twelve
unchanged named V1 tasks after their minimum seven-day delay, using a new
withheld payload/profile/ownership schedule and retaining the unaided attempt.

Run four selectors ledger/framing/router/codec, GCC/Clang O0/O3/ASan+UBSan,
actual selected i386 ILP32 and Cortex-M4 CPU execution. Defect controls change
canonical capacity, copied lifetime, next-hop namespace, generation release,
partial encoder publication, signed interpretation, escape convention and
canonical escape validation. A control must compile and fail the intended
property assertion, not merely crash or fail to build. Compare all exact bytes
with the separately implemented polynomial-division oracle. The test is bounded
evidence; byte equality does not prove all platforms, arbitrary inputs or mastery.

## Rubric, hints and delayed transfer

Score each objective separately. Level0 violates its concrete contract or
inflates evidence. Level1 reproduces bytes without identifying the relevant
layer, bound, lifetime or authority premise. Level2 derives and implements the
named mechanism, preserves failure publication, diagnoses an unseen boundary
counterexample and states complexity. Level3 independently executes its original
changed condition and revisits the affected proof without changing unrelated
contracts. .01a assesses reference-model vs protocol and radio duties; .01b
assesses two bounds and versioned extension; .01c assesses copy vs borrow;
.03a CRC/escaping, .03b recovery, .03c trust; .04a namespaces, .04b routing/local
hop, .04c backpressure; .07a representation, .07b validation/publication, .07c
semantic authority. A shared executable awards none of these reasoning scores
automatically. Complete all original delayed tasks; no replacement by constants.

Hints after saving an unaided attempt: identify the owner at each return;
derive a one-fragment byte ledger; then trace either a bad escape with valid CRC,
a stale release after slot reuse, or the sign bit of a four-octet value. Private
assessor material supplies exact analyses and withheld counterexamples.

## Costs, containment and validation status

All state is fixed and bounded. Encoding/decoding work is O(n), n<=96 wire,
45 schema2 or40 network bytes; CRC is O(8n). Route lookup/control is O(1), copy
or peek O(payload<=34); zero-copy enqueue still owns its header and requires
backpressure/lifetime discipline. Compare equivalent correctness contracts,
including caller retention and completion costs. Staging consumes bounded stack
and avoids partial publication; target stack evidence is a separate obligation.

Security demonstrations target only original curriculum programs and synthetic
bytes: modified valid-CRC payload, stale local release and failed publication.
No external bus/radio/network is accessed. No physical transceiver, contention,
voltage, termination or timing observation is claimed. Required dedicated-board
work remains named in the original projects. Author campaign, current guard and
export receipts are distinct from external review, independent learner/delayed
assessment and physical observations. Other T40 projects remain required.
