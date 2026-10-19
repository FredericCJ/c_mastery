# Diagnostic transports and USB transfer — PR-T28-07 / T28.12

This required successor retains the original three ordinary and three seven-day
criteria. The historical “elective” title/hours are unchanged. Its source register
and assay reports distinguish author reference runs from learner assessment.
The bounded diagnostic representation is shared; its transports are not interchangeable.
CAN arbitration/transceiver work remains the separate required T38 route; the
controller/PHY implementation and measured Ethernet route continue through T39.

Use C17. Public `.c` files are deliberately unfinished learner artifacts; private
solutions are selected explicitly. No dynamic allocation, libc, HAL, networking
stack, or USB stack implements these mechanisms in the restricted portable core.
Actual `f_take` and `ed_*` prerequisite artifacts are imported rather than copied.
The POSIX socket adapter and Linux gadget guest are separate hosted/provider
experiments. Type headers do not authorize target runtime libraries: inspect each
object's imports and final linked closure, including original compiler helpers.

All objects and counted arrays are live, correctly aligned for their declared C
types, disjoint unless an operation explicitly stages its input, and controlled
by one serialized owner. Control records start zeroed and are modified only by
their API. Do not pass corrupt internal `used`/phase fields as if these were wire
inputs. Concurrent ISR/DMA users need a separately verified publication and
quiescence adapter. ISO C enforces neither lifetime nor these ownership rules.

## 12.b — representation, replay policy and changed stream boundaries

`DGMessage` contains sequence1..65535, length0..32 and exactly that many payload
octets. Encode sequence16BE, length16BE, payload: sequence1 and bytes01,02 yield
`00 01 00 02 01 02`. The C struct's padding never enters the wire. Encode stages
all source bytes before publication; insufficient capacity leaves bytes and
written count unchanged. Decode uses the prior `FCursor/f_take`, accepts exactly
4+length bytes, and stages a fully initialized output. Truncation, excess bytes,
zero sequence and excessive length reject without output publication.

`DGSession` accepts a sequence only if greater than its last sequence; gaps are
allowed, duplicates/reordering rejected, and wraparound never accepted. The count
cannot exceed65535. Failed decoding/admission changes neither session nor message.
Sequence exhaustion requires a new independently established session identity;
zeroing a live session while old packets remain admissible defeats replay policy.
USB reset is not an application-session reset. This policy provides bounded
duplicate rejection, not authentication, persistence, exactly-once side effects,
or delivery. After a crash, restoring only a counter or replaying an effect needs
the T48 transactional protocol; derive that distinction in the dossier.

Delayed stream input adds a two-byte big-endian **whole diagnostic length**4..36.
`dg_stream_feed` consumes at most one frame and reports the exact prefix consumed.
A ready frame returns BUSY without consumption. `take` consumes even a rejected
inner datagram because its outer boundary is known. An invalid outer length
poisons the stream: do not scan untrusted bytes for a convenient next header.
EOF is END only with no partial/ready frame; partial or poisoned state is FRAME,
ready is BUSY until taken. A caller may resume the unconsumed suffix after take.
Work O(n), fixed38-byte stream buffer and36-byte codec staging; no unbounded scan.

Derive invariants `used<=needed<=38` once a valid prefix exists, atomic message
publication and strictly increasing accepted sequence. Test every split of all
lengths, coalesced frames, partial EOF, poisoned prefix, replay and exhaustion.
Explain why a read callback's return value is neither a datagram boundary nor a
proof of peer liveness. A valid alternative framing scheme needs a different wire
version and a full parser/compatibility argument, not changed golden bytes.

## 12.a — host and controller composition

`dg_udp_receive` executes one nonblocking `recvmsg` on a caller-owned connected
local datagram socket. No data/EINTR returns BUSY; other syscall failures IO.
MSG_TRUNC rejects the entire consumed datagram, even if the retained36-byte prefix
is a valid message. Empty UDP datagrams are malformed messages, not stream EOF.
Callers handle retry/deadlines and retain socket ownership. The supplied experiment
binds and connects only its two127.0.0.1 ephemeral sockets, with bounded poll waits.

The Ethernet adapter instead uses original IEEE802.3 **length-frame** formatting
with fixed synthetic locally administered addresses02:00:00:00:00:01→...:02,
length4..36 and zero transmit padding. It is not an IPv4/UDP encoder. Received
padding is ignored through `EDFrame.payload_length`, not decoded as application
bytes. Wrong MAC/type/length rejects. Addresses alone are not access control.

`dg_descriptors` acquires actual `EDRx` snapshots, copies the60 meaningful frame
bytes across the two descriptor buffers, admits the message, and releases the
borrow on every successful acquisition path, including malformed/replayed frames.
The descriptor contract strips its reported retained FCS; the adapter relies on
the provider's hardware-error checks and does not invent a valid FCS observation.
Every split1..63 includes FCS-straddling boundaries. Logical release is not a
hardware OWN store, cache operation, or proof of stopped DMA.

`dg_publish` uses actual two-slot `EDTx`: prepare copied frame, publish tail,
barrier, publish head. A BUSY publication retains a phase2 slot. Resolve it through
the original retry/abort API; calling prepare again is not rollback. Completion
with returned ownership permits reuse; neither local completion nor abort proves
peer receipt. Barrier callbacks cannot reenter. Cost is fixed60-byte stack frame
plus the original two1514-byte transmit slots and bounded copying.

## 12.a delayed — original full-speed USB endpoint contract

Build `usb.c` independently after a recorded seven-day interval. The assessor
solution exists now; no actual delay or learner performance is inferred. USB2.0
base specification April2000 and separately identified errata are the protocol
references, not ISO C rules. The model is request/packet level: a lower provider
supplies valid SETUP8 bytes, packet CRC validity, ACK events and stop confirmation.
It does not implement signaling, CRC/bit stuffing, EP0 data packetization,
suspend timing, current draw or electrical conformance.

One full-speed configuration1 has vendor-specific interface0, alternate0 and
bulk endpoints01/81, each maximum packet8. Device/configuration descriptors are
18/32 bytes and little-endian USB fields; `bcdUSB=0200` also applies to this
full-speed-only model. Qualifier requests stall. VIDFFFF/PID0001 are synthetic
simulation identifiers, not assigned IDs or authorization for real distribution.
Supported request shapes are explicit in the header/reference task: GET_DESCRIPTOR
(device/configuration index0), SET_ADDRESS1..127 in default/address state,
SET/GET_CONFIGURATION0..1, GET/SET_INTERFACE0, GET_STATUS for device/interface/
configured endpoints and SET/CLEAR_FEATURE(ENDPOINT_HALT). Unsupported shapes
return FRAME (provider translates to request stall), without claiming that every
unsupported shape must universally stall on every USB device.

Each new SETUP invalidates the previous status token, even if rejected. Tokens
increase without wrap; exhausted controls return FULL. SET_ADDRESS publishes only
after successful `status` for its current token. SET_CONFIGURATION and interface
changes occur before successful status. They require no active IN request; the
provider must cancel/drain before retrying, within independently measured control
request timing limits. Lost status does not roll back an already changed config.
CLEAR_FEATURE resets the selected toggle toDATA0 even if the endpoint was not
halted. Clear OUT also abandons partial application data; this is an explicit
application resynchronization policy. The base specification's contradictory
zero-address prose is recorded, not silently resolved: this model admits only
nonzero SET_ADDRESS. Bus reset establishes address0 independently.

OUT accepts0..8 octets. Bad CRC produces no handshake/no mutation; unconfigured
or resetting NAKs; halted STALLs. A duplicate DATA toggle ACKs but cannot append,
dispatch, or toggle again. Fresh data toggles once. Short packet (including ZLP)
ends one application message; a multiple-of8 wire length therefore requires a
ZLP in this **chosen application protocol**. Oversized accumulation discards
through the next short packet and reports FRAME, never a truncated valid prefix.
USB ACK and application acceptance are separate outputs. Duplicate suppression
at USB packet level does not replace application replay policy across resets.

IN queues an owned encoded copy with a strictly increasing32-bit ticket. Packet
views borrow that storage; retries before ACK return identical bytes and toggle.
Only ACK advances the cursor. A final full8-byte packet requires a separate ZLP;
the final short/ZLP ACK ends ownership. Cancel without confirmed quiescence keeps
phase2 and rejects new queue/packet/ACK operations. Confirmed stop releases once.
Reset invalidates enumeration immediately but retains outstanding request storage
until the lower provider proves quiescence; stale completions cannot free a new
request. Time O(message length), bounded36-byte IN and36-byte OUT storage, no
allocation. These software tickets do not prove device DMA has stopped.

The Linux guest compares the actual USB gadget API with its selected dummy
controller. It is a provider integration experiment using the **same** diagnostic
and cursor sources under an explicit kernel type-header adapter. Inspect queue,
dequeue, disable and callback code. Generic documented deferred completion and
an inspected controller's inline small-IN giveback must not be conflated. Record
whether return means buffer ownership returned, FIFO buffered, or host consumed.
No guest observation establishes cable/PHY behavior or arbitrary UDC compatibility.

## 12.c — evidence columns and changed hardware

For each claim retain four separate columns: controller/descriptor, PHY/link,
protocol-stack, and application framing. A test may support multiple columns only
with distinct observations. The host UDP run supports OS-datagram/application
behavior; the raw Ethernet model supports descriptor/frame contracts. It provides
no PHY measurement. USB is another transport route, not Ethernet conformance.

The required board dossier identifies exact MCU/board/schematic/PHY revisions,
pin alternate functions, clock/reset sources, DMA SRAM reachability and cache
policy, MDIO address/ID/divider, negotiated mode, descriptor format and barriers,
isolated peer, capture location, timestamp alignment and uncertainty. T39's
original MDIO/PHY/ring providers are prerequisite artifacts. Replacing the PHY or
board routing invalidates identity/register, strap/reset, pin/clock and electrical
observations. Preserve applicable application golden bytes; do not relabel them
as a new link test. Physical capture and external conformance remain unobserved
until the declared equipment and protocol are actually used.

## Progression, assessment and containment

Deliver predictions, implementations, boundary traces, a minimized failing
control, generated-object closure/cost analysis and criterion-indexed reasoning.
Hints: (1) draw publication and borrow boundaries; (2) distinguish packet, transfer
and application lengths; (3) track last ACK, current toggle and outstanding ticket
separately; (4) identify which layer supplied each observed fact. Worked private
analysis and altered fixtures belong only in the assessor package.

Level0: wrong result/ownership, missing required artifact or unsupported claim.
Level1: ordinary examples pass but a stated boundary or misconception remains.
Level2: every original ordinary task clause is implemented and supported by its
own control, invariant and bounded evidence. Level3: independent delayed transport
and changed-PHY reasoning meet their separate obligations; no copied solution.
Passing finite tests cannot prove all schedules, conformance or absolute mastery.

Security demonstrations are confined to these original programs and synthetic
data: valid-prefix truncation, replay, duplicate packet and premature reuse mutants
have harmless local success conditions (wrong acceptance/count/buffer ownership).
Specify attacker-controlled bytes and timing; descriptor status control is a model
capability, not proof of remote hardware compromise. Compare repair and regressions;
neither a crash nor a CRC acceptance demonstrates code execution. Execute kernel
modules only inside the dedicated diskless networkless QEMU guest. No host module
loading, external services, real credentials, board flashing or deployed equipment.
