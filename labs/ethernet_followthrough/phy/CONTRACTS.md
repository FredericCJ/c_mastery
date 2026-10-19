# T39.05 bounded PHY management, contract 1

This required additive laboratory implements PR-T39-04 and its original
AX-PR-T39-04-T39.05.a/b/c criteria and seven-day `-V1` tasks. The original
project, criterion wording and historical acceptance remain unchanged. Read
the parent brief and `labs/ethernet_depth/CONTRACTS.md` PHY/queued-TX sections.
The supplied `learner.c` and `target_learner.c` are unfinished; reference
implementations and technical answers are separately held by the assessor.

## Prerequisites and environment

Use the retained `ed_mdio_word`, `ed_mdio_read`, `ed_phy_query` and `EDTx`
interfaces from `ethernet_depth.h`. Submit the actual producer sources and
contract recall, not a similarly named replacement. Knowledge: fixed-width
arithmetic, register side effects, nonwrapping time, ownership and generation
exhaustion; artifacts: the PR-T39-04 management fixtures and preceding T27
clock/pin dossier. The target is C17 profile E, caller-owned fixed storage,
without allocation or a PHY/vendor-driver library. ISO C does not define MMIO,
device readiness, pin routing or an Arm register barrier.

The selected adapter is F439 MACMIIAR/MIIDR. Compile native GCC/Clang C17 at O0,
O3 and with AddressSanitizer/UndefinedBehaviorSanitizer. Separately cross-build
Cortex-M4 soft-float Thumb with GCC/Clang at O0/O3 and an explicit reset/byte
helper closure. MPS2 runs original RAM register effects; it does not emulate
the physical F439 Ethernet peripheral or LAN8742A. Target compilation and
execution use no hosted library. The object-only real-register adapter has
its own disassembly and unresolved-symbol audit.

## Problem and supplied inputs

Implement `phy.h`, using `model.c`, `tests.c`, `mapped.c` and `leases.c` as
inspectable teaching targets. Distinguish a completed MDIO bus transaction
from PHY reset completion, restart acknowledgement and a resolved link.
Return a mode snapshot only after all required observations. The concrete
baseline is HCLK168000000, PHY0, ID1=0007, ID2=C13A, first BSR=0020,
second BSR=0024 and special register31=1018. Predict the word for register2,
divider, two link meanings and negotiated speed before running anything.
Investigate 1008 half-duplex, 1014 10/full, all-ones identity, duplicate matching
addresses, busy management registers, frozen/regressing clocks and every
fallible callback prefix. Output preservation is part of the result.

Controls must start zeroed and stay at a stable live address. Inputs/outputs,
controls and callback context are disjoint. The bus, clock context and owner
outlive all calls and snapshots. One serialized owner performs all operations;
callbacks terminate and neither reenter nor alter the control, and retain no
borrowed pointer. Inputs and public control fields are not adversary-writable
inside this contract. `EpLink` is an ordinary copyable C record, not an
unforgeable authority token. Reinitializing an owner to make old snapshots
appear current violates the contract.

## Operations, effects and invariants

`ep_write` validates a positive timeout and poll budget1..1024, address/register
0..31 and the retained HCLK policy. Read MB before any write; MB1 returns BUSY
without writes. Set configured MACMIIAR with MB0, then MIIDR, then MW/MB1.
Poll at most the budget, checking the nonwrapping monotonic clock before each
read. Completion returns OK. Callback errors return IO; elapsed>=timeout or
time regression returns TIMEOUT. Rejection cannot undo effects already sent
to a device. In particular, timeout may leave MB1; another transaction must
refuse to overwrite it. No output result exists for writes.

`ep_find` performs exactly two ID reads at every address0..31, unless a bus
error stops it. Exactly one 0007/C13x match publishes the address; none returns
WRONG_PHY and multiple matches INVALID. Failure preserves the output. Discovery
is explicit and finite, not performed on every send. This PHY revision exposes
writable ID registers, so matching IDs are neither authentication nor a silicon
attestation. Discovery does not prove configured board address/strap state.

`ep_init` validates configuration before publishing a control. An initialized
control returns BUSY. `ep_begin` requires1..1024 future steps and a positive
outer timeout. It invalidates every old snapshot first, increments the
incarnation without wrapping, records the outer time scope and writes BMCR
8000 alone. Active reset/restart/negotiation returns BUSY without starting
another attempt. UINT32_MAX retirement clears readiness and returns FULL;
it cannot be bypassed by clearing the object while stale observers exist.

Each `ep_step` consumes one finite step. Read BMCR until reset bit15 clears;
then issue1200 (AN enable/restart), and independently wait for restart bit9
to clear. Only then invoke the retained ID/two-BSR/resolved-mode query. BUSY
means unfinished. Link-down or negotiation-pending continues within the same
bounded attempt; unknown identity, half/unknown mode and callback failure end
the attempt. Only 10/full or100/full publishes READY. Check the outer clock
both before and after the step; exhausted budget, equality at the deadline
and regression fail closed. A frozen clock cannot defeat the independent
step/poll bounds. A callback that never returns is outside this finite-work
argument; neither a hardware watchdog nor wall-clock WCET is supplied.

`ep_poll` is legal only when READY. It checks the same retained query and
monotonic clock. Any error retires readiness and advances the incarnation.
A first BSR latch-low indication, even followed by current-up, or a changed
resolved mode also retires the old incarnation before publishing the new
ready observation. An unchanged ready observation preserves incarnation.
`ep_invalidate` explicitly retires the current observation. `ep_snapshot`
publishes only READY; failure leaves output intact. `ep_validate` checks live
owner identity, incarnation, mode and current-ready state. No snapshot can
guarantee that a physical cable remains attached after the observation.

MDIO/PHY readiness never acknowledges MAC DMA quiescence. The lease exercise
composes the actual retained `EDTx`: link loss refuses further admission but
does not free an already published copy. Unconfirmed abort retains the lease;
only the separate device-stop confirmation permits release. This experiment
does not supply a normal descriptor ring or establish a physical DMA stop.
That producer remains a separate T39.06–.08 requirement.

## Target preparation and authority boundaries

`ep_f439_bus` publishes callbacks at MACMIIAR0x40028010 and MIIDR0x40028014.
Its chosen accesses are32-bit; RM0090 also permits8/16-bit access and is not
misquoted as requiring32 bits. Invalid clock/output rejects before publication.
Its volatile access does not turn a bus fault into an ordinary error result.
Prepare the actual MAC clock, RMII selection, pin mux and PHY route before
using it; this function does not configure RCC, GPIO, bridges or straps.

Apply `BOARD_CAPTURE.md` to the selected board. The ordinary baseline assumes
an awake, all-capable strapped PHY and an available continuous reference clock.
Software reset retains the relevant mode configuration; it is not a substitute
for changing unsuitable straps or leaving a special power-down mode. Record
such a prerequisite failure explicitly. Actual MAC/DMA initialization belongs
to PR-T39-05, with its independent stop/reset ownership obligations.

## Reasoning and resource deliverables

D1: derive the management word/divider, register effects, timing units and
RMII/MDIO distinction, citing the named inspected source passages. Derive the
four-stage reset/restart/query state machine and plausible wrong explanations.
D2: implement the public API and actual imports; provide a pre/postcondition,
ownership, output-publication and no-wrap argument for every function. Include
a counterexample to treating `EpLink` or writable IDs as authentication.
D3: replay the exact fixtures and failure prefixes, provide independent expected
packets and identify the first divergence for each flawed implementation.
D4: submit native and target build commands, maps, retained assembly, undefined
symbols, stack usage and code size. Account for one fixed control and snapshot;
copy cost is constant, while `EDTx` separately owns two1514-byte slots.
D5: submit the original task, delayed variant and physical observation dossier
as separate evidence dimensions; retain unavailable observations as gaps.

A write has at most budget+1 CSR reads and three writes. Discovery has64
read transactions. A PHY query has at most five. An outer step has at most
five such transactions, so a steps×5 bound is conservative; reset/restart
steps are cheaper. State storage is O(1), with no retained frame in `EpPhy`.
The teaching model stores32×32 synthetic registers and is a host/verification
cost, not an embedded-driver requirement. The target harness reserves16KiB
stack for these large synthetic test frames; this does not revise a historical
production image budget or prove a measured watermark.

## Verification and rubric

Use each test selector0..5 from `native.c`, then independently reconstruct its
observations. There are78 callback-failure positions in the modeled successful
boot path: every one must leave readiness false and an unpublished snapshot.
Compile unfinished learner sources first: they must compile and fail named
semantic assertions, rather than fail to build. Reject each defect independently:
missing incarnation advancement, combined reset write, missing restart wait,
busy overwrite, an extra poll, ignored latch-low history, readiness retained
after error, and wrong MMIO register stride. Diagnose the actual failing property.

The exact parent rubric remains mandatory. For .05a, credit a derived command
and both clocks, not a memorized0x91. For .05b, require distinct all-ones/busy/
timeout/link results, preserved outputs and the frozen-clock proof. For .05c,
require resolved-mode admission and the link/DMA ownership counterexample.
Level0: absent/wrong result. Level1: examples pass without the invariant or
failure distinction. Level2: correct implementation and explanation with
independent boundary evidence. Level3: original and delayed changed-condition
tasks transfer correctly under the changed clock, failure or ownership model,
with limitations stated. Passing this finite suite is author evidence, not
absolute mastery, independent learner assessment or electrical validation.

## Graduated hints and changed conditions

Hint1: name the owner and acknowledgement for every pending operation.
Hint2: a bus transaction can finish while the PHY operation is still pending;
trace BMCR separately from MACMIIAR. Hint3: preserve the first BSR sample and
make old snapshots observable before performing the second query.

Retain the exact seven-day IDs AX-PR-T39-04-T39.05.a-V1 (excluded HCLK10MHz),
AX-PR-T39-04-T39.05.b-V1 (frozen clock and finite budget), and
AX-PR-T39-04-T39.05.c-V1 (link loss with queued ownership). Add a board-revision
change, writable-ID spoof, and owner-epoch exhaustion as transfer investigations;
these do not replace the original delayed attempts. A real normal-ring adapter
must preserve the same lease distinction under link loss and late completion.

## Security and validation limits

Targets are original curriculum programs, synthetic bus effects and dedicated
emulator firmware. The modeled adversary can supply status/error transitions
and malformed identities; it cannot modify protected C controls or arbitrary
real MMIO. Harmless success is premature local READY or stale-snapshot acceptance,
not a claim of network access, privilege escalation or code execution. Repair
the violated transition and keep the mutant as a regression control. No external
packets or host security changes are involved. Physical board execution requires
the separate authorized laboratory session; this authoring task does not flash it.

Current execution results live in the source-bound research validation receipt.
An authored scaffold, executed reference, independent assessment and physically
observed board are different statuses. Do not promote any one to the others.
