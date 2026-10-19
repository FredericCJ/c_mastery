# Original diagnostic records over the actual Ethernet ring

Version1. This required LRUN-24 successor composes the existing T28.12 diagnostic
with T39 EnRing. It supplements PR-T28-07 and its original T28.12.a/b/c rubrics;
it does not change historical evidence or replace the separate CAN/USB routes.
Families B/C/D/F/G/H/I/J: construct, diagnose, inject failure, verify ownership,
measure bounded resources and transfer under changed assumptions.

## Prerequisites and environment

Complete actual DGMessage/DGSession codec and dg_ethernet_build/accept, EnRing,
EpPhy, retained Ethernet frame codec and foundation cursor. Contracts are
labs/peripheral_depth/diagnostic/CONTRACTS.md and
labs/ethernet_followthrough/ring/CONTRACTS.md. Bring actual learner producers;
matching typedef names or an always-successful mock is insufficient.

C17, eight-bit octets, native GCC/Clang verification and strict Cortex-M4 model
execution. The bridge has no allocator, network stack or runtime library.
Host printing/Python and emulators are verification tools. Target construction
requires explicit source inputs; inspect every object and final helper closure.
The model uses selected F439 normal-ring semantics; physical MMIO, PHY timing,
cable captures and cache coherence on other targets remain separate evidence.

## Problem and mechanism

Implement bridge.h in learner.c. Control starts zeroed. It borrows a live stable
EnRing, its memory/PHY/backend and epoch; all calls are serialized and callbacks
must not reenter or destroy these objects. No caller edits private state fields.
Input/control/output objects are live, aligned and distinct. ISO C does not enforce
these lifetime or ownership rules. Opening binds a running nonzero epoch. Reopening
the same owner requires a strictly newer running epoch produced through the actual
stop/reset/start protocol. A forged epoch is not a quiescence proof.

dr_send constructs the existing fixed-direction IEEE802.3 length frame
02:00:00:00:00:01→02:00:00:00:00:02 with DG sequence/length/payload and zero padding.
The complete60-byte frame is copied by actual en_tx_copy before DG_OK. No caller
borrow remains. This is copy admission only: no publication, device completion
or peer delivery follows from it. One active bridge ticket prevents another send.
EnRing's own two-slot pressure remains authoritative. No retry loop or allocation.

dr_poll publishes a prepared ticket once, then observes en_tx_reap. Successful
publication returns BUSY while ownership remains with the ring. Raw completion
status publishes only on reaped success/error. A failed doorbell or settle can
occur after OWN publication: quarantine the ticket, refuse new send and do not
republish it. Only the external owner's actual bounded stop/reset/new-epoch
sequence permits reopening. Timeout is not cancellation or permission to reuse.
After terminal reap, the bridge can admit the next message. Local error/success
does not disclose whether a peer received or committed a diagnostic operation.

dr_receive acquires one actual EnFrame, stages DGSession and DGMessage through
dg_ethernet_accept, and releases the actual EnTicket on every acquired path.
Malformed/replayed records still return the borrow. Commit the staged session
and caller output only after release succeeds. Release failure leaves application
state unchanged even though the lower layer may already have returned OWN; the
ring is faulted and must be recovered. A malformed descriptor rejected before
acquisition does not create an application borrow. Hardware status supplies CRC
validity in this model; four injected FCS bytes are not an independent CRC proof.

The original fixed-direction diagnostic format is deliberately not IPv4/UDP.
Addresses do not authenticate a sender. Its nonwrapping sequence policy still
needs an external session identity across resets. Do not infer exactly-once
effects from local duplicate rejection or reset the application session merely
because the ring obtained a new hardware epoch.

## Concrete workload and reasoning

Send sequence7/payload01,02,03; mutate caller payload after copy and require
transmitted01. Keep raw status123 unchanged before completion. Complete another
frame with device error8002. Fail publication after OWN, keep phase3 quarantine,
hold the stop-state witness busy, then recover through a new epoch.

Receive sequence1/payloadAA,55; inject it eight more times, a wrong-MAC frame,
one hardware-error frame and sequence2 with an injected release failure. Exactly
eleven acquired leases are released, application accepted count remains one,
and the hardware-error frame is a separate drop. Distinguish actual descriptor
ownership from the copied application data at each point.

Derive constant bridge space (60-byte TX staging, one staged DG record/session),
bounded copied work and unchanged lower12384-byte ring storage. Count copying and
barriers under the real provider contract. Prove at most one active bridge ticket,
release on every acquired RX exit, epoch checks before lower access, and no
successful application publication after a failed release.

## Deliverables, verification and rubric

D1 implement four APIs and a state/ownership diagram. D2 predict and run public
tests selectors0/1 against completed producers; retain complete source/image,
commands and transcripts. D3 independently derive the frame layout, release count
and first irreversible state in publication failure. D4 reject local mutants for
early ticket reuse, lost quarantine, publication mistaken for completion, omitted
RX release and publication after failed release. D5 retain native sanitizer,
target map/symbol/assembly/stack evidence and a list of unobserved physical facts.
D6 after the original seven-day interval, change the publication failure point,
replay a stale epoch and move RX rejection between parse and release. Re-derive
the applicable ownership boundaries without consulting assessor code.

Original T28.12.a assesses actual producer composition and its external limits;
.b assesses exact representation/replay semantics; .c assesses the separation of
descriptor/PHY/protocol/application evidence. Level0 incorrect/missing; level1
happy-path outputs only; level2 D1–D5 with sound failure/ownership arguments and
discriminating tests; level3 independently successful D6. Every original rubric
and its broader USB/changed-hardware task remains required. Finite successful
tests do not certify universal conformance or absolute mastery.

Hints: first separate copied admission from publication; then draw the OWN store
before the failing doorbell; finally stage the application result until the RX
borrow is returned. Technical solution and hidden transfer schedules stay under
assessor/diagnostic_ring and are excluded from learner exports.

## Security, status and limitations

Only original teaching programs, synthetic local frames and emulator backends.
Adversarial control is replay/malformed bytes and injected backend failures;
harmless success is a duplicated count, premature reuse or leaked borrow. Repair
the named invariant, regress the witness and state that arbitrary corrupted
control memory is outside the admitted contract. No external network/board or
host setting changes. Author status is the source-bound validation report under
research/v5/expansion/legacy_diagnostic_ring. External review, independent learner
attempt and physical observation remain unperformed by the author campaign.
