# Withheld TCP criterion analysis and transfer packets

Use the shared learner contract and original immutable AX tasks. Never export
this directory as learner material. Award each objective separately: a checksum
test is not evidence of close-state understanding. Reference runs are author
execution; do not supply them as a learner attempt or seven-day assessment.

## 06a

`admission` rejects104 concrete prefix/bit/semantic packets before publication.
Explain why bit flips with recomputed checksum require different semantic tests
from unchecked flips. MSS100 limits data to100 even though physical buffers are
larger. An ACK of unsent sequence cannot update WL or release retained storage.
The WL1 wrap trace first accepts an in-window future pure ACK (no data stored),
then rejects the older segment's window update. The WL2 test installs a valid
wrapped outstanding-flight ledger explicitly; it is a transition fixture,
not claimed as a full on-wire history. Require the learner to construct its
preceding history independently. EOL ends option parsing; NOP consumes1;
length0/1 and extents past data offset reject. Accepting SYN payload in this
profile fails even though another TCP implementation could support it.

V1 sealed task: choose local ISN fffffff0, peer sequence fffffff0, a newer
acceptable WL1=10, then older fffffff1 with window0. Predict unchanged40.
Separately WL1 equal, WL2 fffffff0, ACK10 with SND.NXT10 is newer. Change the
live-span premise to exactly2^31 and require an explicit ambiguity rejection
or strengthened environment, not a signed-cast argument. Valid alternatives
may represent absolute counters plus checked modular conversion.

## 06b

Old prefix length8 leaves four new bytes. Capacity truncation is a different
cut from old-prefix trimming; the mutant advances by unadmitted tail and the
small-capacity assertion catches it. The independent oracle uses absolute
integer windows and a Python byte list instead of C circular storage. It covers
24576 transitions at capacities1/2/7/32 and three sequence origins, seed141067.
It does not establish correctness for every packet schedule. Capacity describes
retained bytes; HTTP status does not authorize dropping bytes already ACKed.

V1: capacity3, admit five in-order bytes, consume2, resend the original five;
retain exactly the remaining byte plus the two newly admitted bytes. Move head
across physical wrap, leave parser DONE and demonstrate leftover preservation.
Changing the storage policy to all-or-none admission is valid only after changing
the externally advertised window/ACK contract and its expectations explicitly.

## 06c

`close_states` covers active, passive and simultaneous close, both TIME-WAIT
entries and accepted repeated FIN. FIN immediately after a completely filled
receive window is outside that window: drain and retransmit before accepting it.
The actual HTTP composition delivers two of four body bytes then FIN and yields
400; TCP half-close did not supply missing HTTP bytes. Abort clears logical
queues but retains any copied lower DMA owner until its separate completion.

V1 security packet: only the original local NcStack byte arrays are targets.
Attacker controls a syntactically valid peer IPv4/TCP packet and knows the tuple;
inject seq=RCV.NXT+1 inside a16-byte window. Mutant6 aborts the connection;
correct code schedules a challenge ACK and preserves ESTABLISHED. Outside-window
reset has no response, exact-RCV.NXT reset still aborts. Harmless success is the
state-abort violation, not a crash, code execution or authentication bypass of a
real deployment. Repair uses exact equality after window admission. Regress all
three categories, zero window and wrap. Residuals: an attacker with the exact
sequence can still reset, checksums are not MACs, and unbounded challenge traffic
needs an independently specified rate policy in a general host.

## 07a

Mutant1 accepts an ambiguous sample; mutant2 updates variance using new SRTT;
mutant4 starts flight at construction. Each compiles, then fails a distinct
core assertion. Delay acceptance to5001: first data deadline6001, not construction
time+1000. A refused retransmission at8000 neither increments retries nor moves
the deadline; acceptance8001 starts the doubled RTO. A partial ACK restarts it.
Receipt at the exact timeout can legitimately precede or follow a serialized
timer action; state the event order, and test both, rather than assigning an
impossible simultaneous operation to ISO C. This package's methods are serialized.

V1 runs shift0 and8 with G=1/10/1000 over12288 exact rational comparisons. The
oracle quantizes each recurrence independently with Fraction, tracks unquantized
error separately and rounds final RTO upward. Students must bound shifts and
products:60000*256 and7 times that fit uint32; no signed overflow argument is
needed. Granularity and time wrap are explicit inputs, not implicit host clock
facts. A floating estimator may be valid with a newly proved rounding/error
contract, not merely matching80/120.

## 07b

The one-flight invariant is stronger than `flight_bytes < rwnd`: wrapped32
outstanding bytes and rwnd40 leave8 usable, but no second segment is permitted.
The window0 probe uses no new application byte and is not ACK evidence.
Mutant3's constant512 advertisement fails after retention; mutant5's DMA-based
flight retirement already destroys the SYN flight, so the real handshake ACK
is refused; the earliest semantic assertion catches it before data transfer. Candidate tickets are invalidated by
receive/consume changes; adapter copy+commit must be one serialized transaction.

V1 uses the actual seven-byte queue and original parser, budgets3, a four-byte
body followed by NEXT, then FIN. DONE retains NEXT; EOF is deferred until retained
bytes are dealt with under an explicit application policy. Test opposite DMA/ACK
orders, stale epochs, a refused copy and callback after abort. Alternatives that
borrow lower buffers must implement a different proven lifetime protocol rather
than pretending the copy contract holds.

## 07c

Timeouts1000/3000/7000/abort15000 are relative to actual accepted transmissions.
The source preserves exact retained bytes even after lower DMA release. A fully
ACKed connection can release TCP ownership while lower DMA is still active;
reinitializing before that owner drains remains invalid. RTO cap60000 and SYN
loss floor3000 are checked separately from the three-retry policy.

V1 comparison uses1024 identical bytes, MSS64 versus256,100ms delivery/ACK service
plus3ms admission delay:16 versus4 segments, virtual finish1748 versus512 from
the shared initial100ms time. Neither is measured network throughput. Require
the same byte result and partial-I/O/ACK contract before comparing costs. Change
to delayed ACKs, loss, or a constrained peer window and recompute using a new
event ledger. The separately required congestion package must implement growth,
fast retransmit/recovery and investigate fairness; this finite stop-and-wait
comparison cannot close that obligation.

## Evidence limitations and diagnosis

Public stubs must translate and fail semantic checks. Reference dependencies are
explicit, unchanged and source-pinned. Native ASan/UBSan and finite models cannot
prove all lifetimes or all protocol histories. Target images execute only the
CPU packet model; actual T39 admission/cancellation, physical peer captures and
independent learner integrations remain separately tracked. The stack audit
uses the maximum nonrecursive direct-call path, retains caller cost even for
tail calls, adds1024 allowance and rejects dynamic frames/unreviewed indirect
calls. It is not a measured interrupt high-water mark.

## Local abort and external owner
The driver consumer requires a distinct `nc_abort` API. Clearing flight is a logical policy decision; clearing DMA would falsely release a live external owner. `local_abort()` therefore aborts after copied admission, checks DMA remains, rejects resubmission, retires that exact ticket once and separately aborts an unsubmitted prepared send. A consumer must establish actual ring stop before forced retirement, not treat a failed stop as quiescence.
