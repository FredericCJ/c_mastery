# EDU7 foundation assessor analysis

Private assessment material. Keep original criteria and V1 tasks intact. The
public successor supplies current contracts; a passing author test does not
award an independent learner score or close a physical gate.

<a id="t4001a"></a>
## T40.01.a

A/B have peer relations across all logical responsibilities; R relays original
L3 while its two L2/L1 sides terminate separate local links. Reference-model
consistency is not implementation of an ISO protocol suite. A framed-radio
service can replace byte delimiting and escaping at that interface, but its
error/address/duplicate service must be examined before removing any EDU7 duty.
The original addressing/grant/upper authority checks remain. Withheld variant:
radio packets include duplicate delivery but no local sender identity; require
an explicit mapping and retain end-to-end duplicate/authority state. No radio
driver or physical evidence is supplied by this finite body-conversion witness.

<a id="t4001b"></a>
## T40.01.b

Original canonical maximum33 differs from capacity80. Add7 to get40 vs87;
ceil(/24) gives2 vs4, last16 vs15. L4max32, L3max38 and generated L2body45
differ from generic body47/wire96. Schema2 with value32 and extra8 adds12:
45→52→24+24+4. Withheld variant extra1 gives38→45→24+21; extra0 omits both
header bytes. A schema1 parser must reject the changed schema or unknown tag.
Frame profile and route profile are separately selected changes, not implied
by presentation schema2.

<a id="t4001c"></a>
## T40.01.c

Copy mode's retained payload must remain correct after the original network
buffer is overwritten. Borrow mode intentionally retains the caller payload
pointer; its header is copied/decremented. Caller storage remains live/immutable
until exact-token release after downstream access ends. BUSY consumes nothing;
old generation cannot release a new occupant. Withheld variant stage a copied
peek then keep the asynchronous original borrow active: a successful `peek`
alone does not prove all users have stopped. Token matching is not a device
quiescence detector or an ownership proof enforced by ISO C.

<a id="t4003a"></a>
## T40.03.a

The independent oracle performs polynomial division with0x11021, rather than
duplicating the C shift recurrence. Check123456789 gives29b1. Alternating40-byte
payload produces v1 CRC7219 and v2 CRC1e5d; header/version is covered before
escape. Profile2 escape suffixes3e/3d differ from v1's5e/5d. The96 bound remains
conservative because a valid header cannot consist entirely of reserved bytes.
Withheld variant choose a payload whose CRC itself needs escaping. Require its
exact raw polynomial and escaped lengths, not only an encoder/decoder roundtrip.

<a id="t4003b"></a>
## T40.03.b

Overlong input enters discard; delimiter resets; an unfinished escape is rejected;
a following valid candidate publishes once at every split. Valid/error/valid
coalescing preserves the first publication and discards only the middle
candidate. The original retained raw decoder also accepts an unnecessary escape
of an ordinary version byte. The successor rejects that canonicality violation
without modifying the historical producer. Withheld variant place UART error
on a delimiter: it is invalid data, not a trustworthy synchronization delimiter;
the next clean delimiter resynchronizes. Reject vacuous tests that reinitialize
state for each chunk or count a CRC-valid wrong-address frame as delivery.

<a id="t4003c"></a>
## T40.03.c

Actual link admission rejects wrong destination and missing grant after valid
CRC. An original-target modified payload with freshly recomputed CRC is accepted;
that is a bounded integrity/authentication counterexample, not exploitation of
a deployed peer. A longer public polynomial remains computable by the attacker.
Withheld variant modify a valid encoded SET but retain the original CRC; show
corruption rejection, then recompute CRC and show that application range and
session authority must independently decide effects. No arbitrary execution
claim follows from this harmless changed-byte observation.

<a id="t4004a"></a>
## T40.04.a

Forward17→51 preserves ports/payload and rewrites left1→2 to right1→2, where R
changes from left local2 to right local1. Reverse51→17 exits left2→1. The equal
numeric pair in the forward direction must not hide that two separate link
namespaces exist. Withheld variant payload bytes happen to equal node/link
numbers: they remain opaque, not candidates for rewriting.

<a id="t4004b"></a>
## T40.04.b

Forwarding4→3 differs from local delivery of positive hop1. Profile2 adds68
rightward only; it keeps protocol4 and original payload bytes, with no session
or application API access. That proves this code boundary, not authenticated
source identity or dynamic routing. Withheld variant forwarding hop2 leaves1,
then endpoint68 accepts1; router hop1 still drops. Both route ends need a
declared compatible configuration before any interoperability claim.

<a id="t4004c"></a>
## T40.04.c

Decode protocol and enclosing length before routing. Refuse own-origin34,
unknown destination and hop1 at R. Fill one slot, retain the exact packet/token,
and refuse another without mutation. Egress remains correct after input poison.
Withheld variant release, reuse, then delayed old completion: stale generation
must not empty the new slot. Exhaustion returns FULL and cannot be solved by
zeroing a live control. Complete target lifetime argument includes any lower
asynchronous retention, not merely this serialized queue.

<a id="t4007a"></a>
## T40.07.a

SET42/-4000/unit1 is010101020202002a0302f060040101. Schema2 uses tag3 length4;
INT32_MIN is80000000 and INT32_MAX7fffffff, independent of native structure
padding or pointer width. Decode large unsigned wire values by representable
arithmetic, not an implementation-defined out-of-range signed cast. Native LP64,
actual i386 ILP32 and M4 ILP32 observations supplement the byte proof; they do
not establish every C implementation. Withheld variant inputINT32_MIN-1 must
leave output/count unchanged. `struct ef_wide.base.value` is not the decoded wide value.

<a id="t4007b"></a>
## T40.07.b

Stage into bounded local arrays and publish only after all validation and
capacity checks. Four legal U+1F600 scalars consume16 octets; surrogate, overlong,
out-of-range and truncated sequences reject. Duplicate/out-of-order tags cannot
be repaired by sorting attacker bytes. One-byte-short capacity preserves all
bytes/count. Withheld variant a valid scalar followed by an incomplete final
scalar; retain output sentinels. Split callbacks are not separate UTF-8 strings:
this API admits one fully reassembled L6 SDU.

<a id="t4007c"></a>
## T40.07.c

12501 survives representation but fails the original SET authority/range check;
setpoint remains2200. `%n%` remains three label octets and is never executed as
a format. ASCII label1 makes18-byte L6/25-byte session; sixteen multibyte octets
make33/40. Both happen to use two fragments here, with different final lengths;
do not invent a fragment-count change. Withheld variant omit label entirely:
15/22 fits one fragment. Numeric unit1 and application effect rules do not
depend on display language, routing or transport representation.

## Evidence boundaries

Require these analyses objective by objective, exact original delayed IDs and
fresh cases. Inspect actual imports and generated dependency/stack closure.
The first target pilot selected an interrupt-specific boot that required
`xf_capture` and failed linkage; final source explicitly selects the simpler
public fault-only boot. The failure is not counted as execution. No historical
receipt was edited. Physical captures and the remaining grant/transport/session/
whole-stack work stay separate. C tests and defect controls falsify specific
errors; they do not prove absolute mastery, full protocol conformance or safety
of unmodeled electrical installations.
