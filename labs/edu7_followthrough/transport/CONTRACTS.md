# EDU7 accepted-send clocks, bounded receive ownership and epoch extension

This required successor implements the three unchanged PR-T40-05 criteria
AX-PR-T40-05-T40.05.a/b/c and their original seven-day V1 tasks. Apply the original
project and legacy shared assessment contract, the exact EDU7-v1 wire fixture,
and `labs/edu7_depth/CONTRACTS.md` staged-offer/ACK contract. Historical hours do
not limit this additional work. The source register records actual inspected
original codec/receiver and staged sender passages; no title-only source credit.

## Prerequisites and environment

Supply completed learner EduRx, EduFragment codec, EduDatagram codec and E7Sender
artifacts with their contracts and source identities. This adapter must compose
those actual interfaces; an already solved transport library is prohibited.
Implement `transport.h` in `learner.c`. ISO C17 profile E, fixed storage,
single serialized owner, explicit byte helpers only on the target. Host tools
may use stdio/sanitizers; Python expected packets are verification inputs, never
target dependencies. Native GCC/Clang, ILP32/i386 and Cortex-M4 soft-float audits
have separate observed evidence. Final link dependencies and stack are checked,
including implicit compiler copies. C11/C17 fixed-width and unsigned rules used
here do not confer protocol semantics; C23 is not required. No published C17/C23
text or physical UART execution is newly asserted.

## Problem and supplied inputs

Reassemble the original25bytes0..24 from24+1, repeat the first fragment after a
lost ACK, repeat the identical final fragment and alter one final byte. Report
exact ACK bytes, upper publication count, complete bytes and retained storage.
Run the120-byte changed task while the consumer holds the complete message:
all five24-byte fragments must publish once, and subsequent probes advertise
credit0 without overwriting the held SDU. The supplied tests also enumerate all
lengths1..120 under each declared wire profile with a duplicate of every fragment.

Stage sender offers while the lower service refuses at0,25,...225ms, accept at
250ms, then interleave refused and accepted retries at350/450/550ms. The fourth
accepted-send window expires at650ms. A separate blocked sender begins at1ms,
probes at101/201/301/401 and fails at501 regardless of probe replies.

Expire incomplete receive storage at500ms from its first accepted fragment,
including a duplicate at499. Reopen before ID wrap and inject an unchanged stale
packet with the same numeric ID. Show both the original format's missing epoch
information and the explicitly selected extension's behavior. The original
canonical wire fixture remains unchanged.

## Mechanism and explicit wire profiles

Profile1 is unchanged EDU7-v1: L3 protocol4, original8-byte L4 header,24-byte
maximum fragment,5fragments/120bytes, local endpoints17/51 and ports7/8. There is
no epoch on that wire. A local counter reset alone cannot distinguish a delayed
old frame from a new identical ID. This adapter consequently refuses profile1
reopening into a reused-ID lifetime; it does not pretend an external boolean
proves all delayed frames disappeared.

Profile2 is a required changed-condition experiment, not a revision of the old
accepted contract: L3 protocol0x84 and a big-endian16-bit epoch before the original
L4 bytes. Two extra bytes plus8header+24data exactly fit the existing34-byte L3
payload,40-byte L2 payload and96-byte escaped-frame envelope. Old protocol4 is
rejected by profile2 before interpreting an epoch. The unchanged codecs operate
on staged validated subobjects; the explicit adapter changes the protocol tag.
Both peers and any router must opt into this profile. The retained protocol4-only
router rejects it; later whole-stack work must provide an explicit profile2
router adapter. No automatic negotiation or compatibility is implied.

## Representation, invariants and ownership

EtTx owns an actual E7Sender, identity and remembered peer credit. EtRx owns an
actual EduRx, identity, retired-ID floor and monotonic deadlines. Fresh controls
begin zeroed. Open binds node/peer/ports, nonzero epoch and profile. A later open
requires quiescent sender and receiver, no held complete SDU, and a strictly
greater epoch with profile2; token and publication histories persist. Exhausted
16-bit epochs retire; neither epochs nor transfer IDs wrap. The initial open
assumes a separately established fresh peer with credit1; it is not authentication
or permission to erase a remote live receiver.

All control/input/output regions are live, correctly aligned and disjoint.
No ISR/concurrent call or forged control state is permitted. Inputs are borrowed
only through return; sender bytes, reassembly, receipt and offers are copied.
Pointer validity and ownership discipline are programmer obligations; scalar
bounds and identity checks are runtime enforcement. Retained APIs have no
allocation or callback. No observation here establishes thread safety.

Sender staging is transactional: output capacity failure does not leave a hidden
pending offer; a matching epoch/token resolves accepted or refused lower admission
once. Only actual acceptance starts100ms or consumes a retry. Matching ACK of the
final fragment remembers credit0 for the next message, even though that message
has completed. New work probes instead of presuming the receiver became free.
Current upper-layer completion and physical transmitter completion are distinct.

Receive validates full network/source/port/profile/epoch, stages the actual
EduRx acceptance and ACK, then commits only if the copied ACK fits. This matters
because the retained primitive writes an ACK before some rejected cases. A
rejected candidate cannot leak that ACK or partially replace ownership. The
first accepted fragment fixes a500ms assembly deadline; duplicate/probe traffic
does not renew it. At/after expiry clear only active partial length/index and
retire that ID, preserving completed receipt and held-complete ownership. An
identical completed final receipt still re-ACKs without another publication.

`et_peek` copies only a complete SDU and never releases it. A too-small output
preserves output/length. `et_release` requires current epoch and completed ID,
then invokes the actual receiver release. Counters cannot silently wrap; exhausted
publication/expiry state requires explicit owner retirement. None of these
operations promise cryptographic erasure of logically released bytes.

## Failure and time semantics

Time is nonwrapping uint32 milliseconds, monotonic, with500ms headroom. No
wall-clock timing or arbitrary timer-wrap inference. A receive call first checks
its staged expiry; a due timeout commits the specified cleanup and returns
TIMEOUT without ACK, even if incoming bytes are malformed. Other invalid-input
and capacity refusals preserve state/output. Sender retry failure and receiver
assembly expiry are separate resources; a failed sender cannot free a peer's
already complete held message. Lost final ACK is handled by the retained exact
final-byte receipt, not a hash or blind numeric-ID acknowledgment.

Run actual ordinary/changed schedules, all finite lengths and both profiles,
wrong epochs, altered final bytes, too-small output, held credit, ID/epoch
exhaustion and stale release. Explain why no-progress calls cannot establish
network fairness or eventual completion. Copies/work are bounded by120 bytes;
fragment operations by40-byte network packets. Record `sizeof` and retained
function stack for each ABI; bounded stack is not physical high-water evidence.

## Deliverables, verification and rubric

D1: preconditions, identities, ownership/state/deadline proof and exact original
versus extension byte ledger. D2: adapter and prerequisite sources, final helper
closure, stack/resource accounting. D3: independent packet construction, observed
ACK/publication/timing traces and first-divergence repairs. D4: each original V1
after seven days. D5: later full routed/physical evidence remains required by the
parent and is not supplied by this isolated L4 package.

Compare actual captured packet hex with independently constructed Python bytes
and exact deadline arithmetic. Six native profiles, actual i386 and four Thumb
model profiles must run; unsolved stubs must compile and fail intended semantic
assertions. Distinct mutants erase epoch, start clocks on offer, omit retired
floor, renew on duplicates, advertise credit while held, redeliver final duplicates,
publish partial data, ignore capacity, charge refused retry and renew blocked
deadline. A passing finite campaign is bounded author evidence, not mastery.

For criterion.a, level0 publishes partial/duplicate SDUs, level1 reproduces25bytes,
level2 derives capacity/receipt/ACK atomicity and holds120bytes correctly, level3
independently handles the changed upper refusal and fresh discriminating cases.
For.b, level0 times offers, level1 lists retry times, level2 derives accepted-clock
origins with refused submissions, level3 independently changes lower availability
without violating retry accounting. For.c, level0 erases held/retired identity,
level1 reports a timeout, level2 proves independent cleanup and exact old-epoch
refusal limits, level3 independently derives the required wire/namespace change
and distinguishes stale traffic from malicious modification.

## Hints and changed conditions

Hint1: draw ownership at staging, admission, partial assembly, completion and
upper release. Hint2: compare the bytes available to the receiver before and
after local reopen; identify the missing information. Hint3: stage a full
receiver/ACK transaction and keep a separate first-fragment deadline. Retain the
unaided attempt before reading hints or withheld technical analysis.

Original AX-PR-T40-05-T40.05.a-V1 holds120bytes; .b-V1 interleaves accepted/refused
retries; .c-V1 reopens before ID wrap with stale fragments. All retain seven days.
Additional transfer may add an authenticated envelope or durable receive receipt,
but must account for overhead and new contracts rather than asserting equivalence.

## Security, validation and limitations

Only original local synthetic programs/emulators. The unchanged-old-frame test
demonstrates incarnation separation. A second original local control rewrites
the epoch bytes and is admitted: a public epoch is not authentication. An attacker
who can forge current tuples can inject data or exhaust the unauthenticated
namespace; no arbitrary-code execution or external-target claim follows. An
authentication extension requires separate key, integrity and replay design;
do not silently claim this parser provides it. Repair/regress the stale/partial
and ownership defects under the stated trusted-peer model.

Exact current commands/source hashes are in validation.json. Independent review,
delayed learner performance, physical UART relay and session/application effects
remain separate unperformed dimensions. No historical receipt is rewritten.
