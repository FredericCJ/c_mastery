# Exact changed-condition tasks at the T39/T40 service boundaries

This additive contract supplies the previously unimplemented variants
X-T39-002-V1/V2, X-T39-004-V2, X-T39-005-V2, X-T40-001-V1/V2 and
X-T40-003-V2. The original exercise, rubric IDs, prerequisites, seven-day
delay and separate assessor analysis remain authoritative. Families A/B/C/D/F/G/I/J
apply to these mechanisms; the source-specific PHY comparison also requires
explicit hardware-authority reasoning. These are required transfer tasks, not
substitutes for the richer parent projects or the original Ethernet/EDU7 wire.

## Prerequisites and environment

Bring the learner's actual `ethernet_depth.h` implementation (`ed_bridge`,
`ed_mdio_read`), `edu7_depth.h` sender and `com_edu7.h` fragment codec. The
unfinished `learner.c` imports their public interfaces; no assessor fallback
exists. Know modular arithmetic, overflow admission, live-object and retained
borrow obligations, C17 object representation, copied lower acceptance,
timeouts, VLAN scope and destructive register reads. First complete the
original exercise and defend its invariant without this successor's hints.

Use ISO C17, octets, exact-width integers and stable caller-owned controls.
No allocator or library implements a studied mechanism. Native GCC/Clang
O0/O3/ASan+UBSan are author-verification configurations; strict Cortex-M4
soft-float builds use explicit original byte helpers, selected startup and
semihosting only. Audit object imports, retained code, map, stack bounds and
implicit helpers. C23 changes no ownership, packet or device contract here;
neither a compiler mode nor C11 draft text establishes final C17/C23 coverage.

All input/output/control objects are live, sufficiently sized and disjoint;
the specified payload borrow is the only retained input. Controls start zeroed,
stay at stable addresses, and have one serialized nonreentrant owner. Callbacks
return and do not retain outputs or change the control. Invalid C pointers are
caller violations, not safely detectable errors. No concurrency guarantee is
inferred from a successful sequential model. Do not mutate public state to
manufacture a desired transition.

## X-T40-001: arbitrary cycles and acknowledgement windows

Implement `lt_order(modulus,a,b,&order)` for modulus2..65535 and admitted
members0..modulus-1. It returns order0 for equal, -1 when the forward distance
is nonzero and at most floor(modulus/2), +1 for the reverse. For an even
modulus the exact half-cycle is ambiguous: return LT_AMBIGUOUS without writing
the output. Invalid inputs likewise preserve it. For odd cycles no antipode
exists, but the relation still need not be transitive. Derive forward distance
without relying on unsigned machine wrap or a power-of-two mask.

`lt_ack` models a cumulative next-sequence acknowledgement. The live outstanding
span is0..floor((modulus-1)/2); base and next are admitted members. An acknowledgement
at forward distance no greater than outstanding advances base and subtracts that
distance atomically; otherwise LT_STALE preserves both. Base13, modulus15,
outstanding3, ACK0 leaves base0/outstanding1; delayed ACK14 is stale; ACK1 drains.
Explain why an indefinitely old ACK that survives a full sequence cycle can
alias a current acknowledgement. A half-window bound alone does not constrain
packet lifetime; an epoch or explicit maximum-lifetime premise is also needed.

Construct the modulus12 antipode and a nontransitive modulus15 triple. Enumerate
all93664 pairs for moduli2..65 against an independent step-around-the-ring
oracle. Compare masking with14→0 under modulus15. Constant work/storage is
per operation; exhaustive test work is cubic in the maximum modulus. This
extension does not permit reuse of the original EDU7 monotonic message IDs.

## X-T39-002-V1: borrowed records over a partial-progress stream

Implement `struct lt_borrow`: two-byte big-endian length0..256 followed by exactly that
many payload bytes. `lt_begin` retains the caller's immutable live payload;
only its prefix is copied. It refuses an already active/terminal control.
`lt_offer` exposes the current contiguous prefix or body suffix and its length;
it records the offered extent. `lt_accept(n)` commits exactly0..offered bytes
that the downstream service has **copied**. The next offer invalidates the old
offered-count premise. The lower service does not retain a pointer after this
call. Partial acceptance preserves the suffix borrow; LT_DONE clears the borrow
only after the last payload byte (or the empty record's second prefix byte).

No pointer arithmetic on NULL occurs for an empty record. Positive acceptance
without a current sufficient offer is invalid and leaves state unchanged.
After partial emission, `lt_cancel(...,0)` moves to a nonofferable cancellation
state and retains the borrow. Only an independently established stream reset
and downstream quiescence (`reset_confirmed=1`) permits clearing it. Before
any byte is accepted, or after DONE, clearing needs no reset. A boolean argument
is a caller proof obligation, not authenticated evidence of a device stop.
Explain why discarding a suffix without a record-boundary reset corrupts the
next record. Caller mutation during an outstanding borrow violates the contract.

Test every length0..256 with maximum accepted chunks1..17, a one-byte prefix
acceptance, a too-large acceptance, cancellation and empty payload. Retain
byte-for-byte wire output and the exact last point at which caller storage is
needed. Operations are constant work except downstream copying, which is
O(record length) in total; payload storage is borrowed rather than duplicated.

## X-T39-002-V2: padding-dependent trailers

`lt_sizes` accepts at most eight wrappers. Each adds its header, rounds upward
to its own alignment1..256, then appends one trailer octet for every started
16-byte padded cell plus fixed tag0..64. These are original synthetic size
rules, not Ethernet cryptography or an IP encoder. Use uint32 extents, checked
addition and a temporary result array; any late overflow or invalid layer
leaves **all** result entries unchanged. Application12 with layers(3,8,2),
(5,7,1) yields19 then31. Replacing the second header with UINT32_MAX must
preserve canaries91/92, including the already calculated first result. Work
O(layer count), bounded eight-result scratch. Derive why counting cells from
the unpadded input is wrong, and investigate exact-alignment and zero inputs.

## X-T39-004-V2: VLAN-keyed learning

`struct lt_bridge` holds two independent actual `struct ed_bridge` tables for configured VIDs10
and20; each retains four entries, three ports and TTL10. It supplies a global
nonwrapping monotonic time boundary and atomically stages the selected table
and output mask. Other VIDs are invalid. The same source MAC on VID10/port1
and VID20/port2 must remain independently reachable. This is selected VLAN
keying, not tag insertion or complete VLAN membership/admission policy; consume
the separate inspected single-tag parser before selecting a VID from wire.

Derive how independently aging tables behave when only one VID receives input.
Compare global memory capacity with a fully shared `(VID,MAC)` table, and explain
the fixed partition's refusal tradeoff. Reuse the actual loop counterexample in
the parent wire laboratory on each VID: a broadcast cycle survives keyed learning
and a larger table. Only the explicitly acyclic topology repairs that fixture;
no spanning-tree implementation is claimed. Cost: two four-entry tables, O(4)
lookup/staging, no allocation. Diagnose the cross-VID shared-table mutant.

## X-T39-005-V2: source-specific changed PHY

Implement `lt_dp83848` over actual `ed_mdio_read`, keeping the F439 management
bus encoding distinct from the selected PHY register meaning. The changed
source is TI DP83848-EP SLLSEC6E (June2019), inspected tables5-10/11/12/18,
printed59–60 and65–66. Admit the documented revision0 only (2000/5C90);
reject another revision until its compatibility and errata are investigated.
Read status twice to separate latched history from current readiness, then
read PHYSTS at0x10. Require link, completed negotiation, full duplex and no
loopback. Decode the speed bit as10 when set,100 when clear. Failure preserves
the output. This contrasts with LAN8742 register31 and its different IDs/mode
field; neither source supports copying those meanings to this part.

The callback model supplies five bounded transactions with at most the retained
budget each; it injects each of15 read failures, stuck busy, link/negotiation
and unsupported modes. Report operation status separately from physical
negotiation or reset. This query does not implement DP83848 board reset, straps,
clock routing or a new physical adapter; those remain explicit target prerequisites
for deployment, and no board is flashed. PHY identifiers are not authentication.

## X-T40-003-V2: partial-fragment admission

`struct lt_partial` composes the actual E7 sender with the original fragment codec and
the borrowed-record emitter above. It owns at most32 encoded fragment bytes
plus the prefix; its pointer into that array is rebound after staging. This
changed lower service carries length-delimited fragment records, **not** the
original RS485 wire. `lt_partial_begin` stages `e7_send_offer` and encoding before
publishing either owner. Until completion, no other call may modify that sender.
The caller obtains byte offers through the embedded `lower` control, then calls
`lt_partial_progress` with the exact copied prefix. Only final-record acceptance
invokes `e7_send_accept(...,1,now)` and starts the100ms retry clock. Prefix
acceptance starts no transport acknowledgement clock. Separately a fixed50ms
emission deadline begins at preparation, preventing indefinitely held offers;
positive progress does not renew it. At equality the adapter is poisoned and
holds all ownership until reset/quiescence confirmation.

Offer at0, accept the first prefix byte1, second2, and remaining body10:
the sender must be nonwaiting until10 and due110 afterward. A prefix accepted49
then tick50 times out without transport acceptance. Abort without reset retains
the sender offer and private bytes; confirmation52 releases the offer through
the actual rejected-acceptance operation. A later attempt may then begin.
Do not pretend previously copied bytes can be rolled back. Time is nonwrapping,
monotonic, and at most UINT32_MAX-500 for compatibility with the retained sender.
Storage is fixed; encoding/copying is O(fragment length), other calls O(1).

## Deliverables, assessment, hints and containment

D1: derive the exact original variant's representation, pre/postconditions,
invariants, ownership and failed-operation effects before running tests.
D2: implement the APIs and actual producer imports; include an authority table
separating ISO C, diagnostics, runtime checks, external evidence and discipline.
D3: retain the six selectors' raw results, independent expectations, semantic
mutant rejection, unchanged-output canaries, host/target commands and helper
closure. Explain each counterexample, including assumptions that invalidate it.
D4: solve the original named delayed task after at least seven days with fresh
input, without assessor access. D5: preserve physical/source/independence gaps.

Each original criterion remains scored separately. Level0 violates the contract
or invents evidence; level1 reproduces ordinary output; level2 derives and
implements the boundary, explains the intended mutant and changed condition;
level3 transfers independently with bounded evidence. Author execution cannot
award level3. Finite tests do not prove absolute mastery or complete protocols.

Graduated hints: first identify the retained bytes and the event authorizing
reuse; then separate offered from accepted progress; finally stage the output
at a single commit point. For cycles, step around the ring rather than masking.
For the PHY, read the actual meaning of each sampled bit before adapting code.
Stop hints and retain the unaided attempt before the delayed gate.

Security targets are original local arrays, bus callbacks and emulator programs
with synthetic identities only. The adversary controls offered bytes, lengths,
registered local status transitions and stale acknowledgements within valid
storage. Harmless success is wrong local forwarding, false speed, premature
retry or lost suffix. Repairs reject the corresponding semantic mutant and
retain boundary regression tests. No external network, credential, production
system, host security setting or physical flashing is involved. A model failure
does not demonstrate arbitrary code execution or electrical behavior.

Separate assessor analysis is `assessor/link_transfer/ANALYSIS.md`. Current
authored/executed status is established only by the source-bound
`research/v5/expansion/legacy_link_transfer/validation.json` and guard. No
external review, independent learner success or physical observation is asserted.
