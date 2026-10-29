# Normal Ethernet ring and changed-condition laboratory contract

This required successor supplies the missing author implementation behind
PR-T39-05, PR-T39-06 and PR-T39-07. Original criterion and seven-day variant IDs
remain unchanged. It also produces the precise normal-ring dependency of
PR-T41-03. A passing finite author campaign is evidence about its named inputs
and toolchains, not learner mastery, external review or a physical-board pass.

## Prerequisites and environment

Use the inspected T27 `struct bu_range`/`bu_f439_dma` implementation and the T39
`struct ep_phy` lifecycle and retained `struct ed_rx`/`struct ed_tx` models. These are actual source
imports. A completed T27 clock/pin dossier is still needed for a physical image:
the generic `struct bu_clock` model does not configure a real PLL. Supply a stable,
known core/HCLK at most180MHz and a continuous selected-board50MHz RMII clock;
record supply, flash latency and clock-tree configuration separately.

The core is C17 with8-bit bytes,32-bit unsigned/uint32_t and fixed caller-owned
storage. Profile C permits no target allocation, OS, libc, HAL or driver that
implements the taught mechanism. Profile E adds the original MMIO/Thumb port,
explicit startup/linker/runtime producers and audited helper closure. Native
test printing, Python integer oracles and QEMU are host verification tools.
The MPS2 image executes Thumb code and original RAM effects, not F439 Ethernet.
GCC/Clang extensions used for section placement and inline assembly are
identified explicitly; neither follows from ISO C. Exact published C17/C23
text remains outside the selected hardware-source inspection.

## Representation and authority

`struct en_descriptor` is four volatile32-bit words. `struct en_memory` is exactly12384 bytes:
four RX and two TX descriptors plus six2048-byte, four-byte-aligned buffers.
The selected T41 hardware profile uses normal ring RER/TER, not chained mode,
enhanced descriptors, descriptor skips, timestamps, checksum offload or jumbo
frames. T39's original1536-byte per-frame validation and two-descriptor models
remain separately exercised. Their limits are not silently substituted for the
T41 buffer allocation. `en_range` composes `bu_f439_dma` for the complete
interval in `[0x20000000,0x20030000)`; alignment is also checked. A valid numeric
address is not proof of a live C object or correct physical mapping.

All owners begin zeroed, stay at stable addresses and are single-owner,
nonreentrant objects. Input, output, owners and internal storage are disjoint
unless an operation explicitly returns a borrow. Caller proves actual typed
objects, alignment, extents and lifetimes; the API cannot validate arbitrary
pointers. Callbacks terminate, do not reenter/mutate owners, and do not retain
borrowed controls. Fallible model callbacks fail **before their own effect**;
prior effects remain. A real bus fault is not translated into ordinary `ED_IO`.
No other CPU, interrupt, debugger or device modifies the owned register/state
domain. Serial RAM effects are not ISO C threads. Volatile alone supplies
neither thread synchronization nor cache maintenance, bus reachability or
device quiescence.

## Problem and concrete input/output contracts

Implement the public stubs from primitives. Use synthetic MAC02:00:00:00:00:01,
bounded original frames and the public selectors. `en_init` validates controls,
nonzero unicast MAC and the whole DMA interval without hardware effects.
`en_reset_begin(now,polls,timeout)` admits1..1024 observations and a positive
nonwrapping clock window. DMA process states must be stopped and no RX borrow
held. A new reset advances the nonwrapping epoch, requests SWR, and exposes
`EN_RESETTING`. `en_reset_step` must observe SWR clear before `EN_STOPPED`.
Timeout retains the pending reset and storage. A fresh explicit finite window
can resume read-only observation without rewriting SWR or advancing the epoch.

`en_start` requires stopped state and a current successful `struct ep_phy` snapshot.
Initialize all descriptor metadata before OWN, install bases only while DMA is
stopped, use TSF/RSF, no FEF/FUGF, MACCSTF/APCS/IPCO zero, normal format and
PBL1. Configure the accepted10/full or100/full mode, perfect local unicast plus
broadcast, no pause or promiscuous/hash policy. Publish RXOWN with ordering,
enable MACRE/TE, then enable DMASR/ST last. Failure before the final register
write leaves the modeled engines stopped; the component enters `EN_FAULT`
and never fabricates rollback or CPU ownership of already published buffers.

`en_tx_copy` accepts14..1514 bytes from DA, excluding preamble/FCS, only while
the cached PHY incarnation is current. Success means a complete internal copy
exists before return: caller may mutate/free its input immediately. It does
not mean OWN publication, local completion, physical transmission or delivery.
Two occupied slots produce bounded `ED_BUSY` and preserve the output ticket.
`en_tx_publish` reconstructs all control words on every reuse, prepares length,
address and zero word3, then publishes OWN last. Hardware status writeback can
clear prior FS/LS/TER/IC controls, so OR-ing OWN into old status is incorrect.
MAC supplies pad/FCS. `en_tx_reap` requires current ticket and returned OWN;
an IRQ is insufficient. A valid error completion writes raw status, returns
`ED_IO` and releases only local storage. No peer-delivery claim follows.

`en_rx_take` gates status by OWN, then FS/LS, ES/DE, FL64..1518 and the declared
single-descriptor policy. It exposes exactlyFL−4 bytes from DA. A downstream
consumer must not subtract four again. Invalid/truncated fragments publish no
span, track drop recovery, and recycle only CPU-owned storage. A successful
borrow consumes a nonwrapping serial and holds its buffer until the one exact
owner/epoch/slot/serial release. Four held RX buffers produce backpressure;
neither full ring nor parser rejection authorizes overwriting a consumer.
Release while stopping/faulted records the end of the CPU borrow without
publishing OWN again. A release callback failure can leave OWN published and
the engine uncertain; stop/reset recovery is still required.

`en_status` captures once and acknowledges only observed, known W1C bits.
Fatal bus error is captured and contained without blind acknowledgment and
continuation. `en_stop_begin` clears DMAST/SR; timeout is not a stop witness.
`en_stop_step` requires both process states stopped and all CPU borrows returned,
then disables MACRE/TE. It reaches `EN_BOUND`, not reusable running state.
A fresh successful SWR/new epoch is mandatory before another start. Old
completion/release tickets cannot affect new requests. Serial/epoch exhaustion
returns `ED_FULL`; it never wraps and reissues an identity.

Except explicit raw status on `en_status`/valid error completion, failures do
not publish a new output span/ticket. State and prior hardware effects may
change; preserving an output is not transactional rollback of the peripheral.
No call frees memory. Keep all storage live through known quiescence.

## Selected target port and limits

`en_target_open` requires privileged, exclusive ownership of the debug-counter
session. It preserves prior DEMCR/DWT values, enables TRCENA before inspecting
optional CYCCNT, does not reset its value, and rejects absent/disabled counting.
`en_target_close` restores only after all users are quiescent. During the session
no other agent changes the counter, features or core-clock bound.

Every ring register write invokes `settle`. The target first executes a DSB,
then waits for720 active CYCCNT cycles with an independent2048-read cap.
At at most180MHz this is at least4us; the selected continuous50MHz RMII clock
divided by20 gives the slow2.5MHz MAC clock, whose four cycles take1.6us.
This conservative margin also covers the faster100Mb/s mode. A stopped counter
fails; debug halt/interrupt delay can lengthen waiting. This is no WCET or
physical-clock proof. In particular a missing REF_CLK cannot be repaired by
waiting on CYCCNT. Readback of affected registers is not the ES0206 workaround.

`en_target_cold` is permitted only on an independently established cold,
quiescent peripheral with ETH clocks off and no outstanding storage users.
It rejects already-enabled ETH clocks, enables GPIOA/B/C/G and SYSCFG, asserts
the RCC Ethernet reset, selects RMII before Ethernet clocks, configures the
declared nine board-route pins AF11/push-pull/no pull/high speed, enables
MAC/TX/RX clocks, and releases RCC reset. It neither configures a PLL nor proves
electrical load, board straps or clock presence. RCC reset and DMA software
reset are distinct operations. The former is not an emergency cancellation
operation on live buffers. Selected non-model object audits require actual
32-bit MMIO and DSB code, with no undefined target helpers.

## Changed-condition implementations

Retained `ed_rx_acquire` is exercised with two descriptors, late LS after a
dropped prefix, unknown first-descriptor FL, capacity splits4/96 through96/4,
held/stale tokens, capacity failure and DMA ownership. Only the final valid
FL and total capacity establish the completed frame bounds.

`ev_span` changes only the capture boundary: caller supplies validated actual
stored bytes and the explicit CSTF setting with APCS zero. CSTF strips only
type fields greater than0x0600; length frames and0x0600 retain FCS. The adapter
normalizes the upper span once. It does not guess how a particular changed
silicon configuration encodes RDES0FL; that mapping requires a source/capture
argument before a hardware variant can use it. The selected ring stays CSTF0.

Retained `struct ed_tx` exercises the original two-descriptor/one-publication policy.
`struct ev_chain` extends the separate borrowed publication model to2/3 descriptors:
all metadata and every tail OWN precede head OWN. A borrowed caller buffer
must remain immutable/live through documented returned ownership or confirmed
stop. Partial tail publication plus unconfirmed abort retains the borrow;
neither a completion flag nor a fresh token authorizes reuse. This model is
not an undocumented switch of the normal hardware ring to chained mode.

`struct ev_visibility` models two32-byte cache lines with distinct CPU/device arrays.
Dirty CPU data requires clean before device ownership; all bytes of an owned
line, including unrelated-looking neighbors, are unavailable to CPU stores.
Returned device writes require invalidation before CPU observation. An explicit
stale-clean counterexample loses a device write. This derives noncoherent
ownership obligations; it is not execution of an M7 cache-maintenance opcode.

The upper-consumer test rejects an original synthetic IPv4 version field and
releases the accepted ring frame exactly once. Separate accepted/drop/released
and upper-rejection counters prevent a layer-boundary failure from becoming a
DMA failure. The discriminator is intentionally not a complete IP parser.

## Deliverables, verification and assessment

D1: predict every ordinary and changed-condition selector before execution,
including exact first refusal, output preservation and ownership state.
D2: implement the three learner sources, interface/lifetime diagram, register
and source-authority table, checked arithmetic and reset/stop proof.
D3: retain actual commands, source/compiler/image hashes, raw traces, maps,
undefined-symbol and disassembly audits, stack bound and independent oracle
comparisons. Include GCC/Clang debug/optimized/sanitized and actual Thumb runs;
the deliberately invalid CCM linker placement must reject before publication.
D4: construct a fresh counterexample to an overstated guarantee and complete
the original changed task after at least seven days without solution access.
D5: physical closure, when authorized separately, follows BOARD_CAPTURE.md.

The source-bound runner checks positive results, a compiling but semantically
rejected learner stub, mechanism-specific compiled mutants and actual target
closure. Assess each original criterion explicitly: wrong boundary or claimed
physical proof is level0; reproducing observations without invariants is
level1; a correct implementation, derivation and fresh falsifier is level2;
independent delayed transfer with justified changed premises is level3.
Author tests do not administer the independent level3 gate. Resource arguments
must account for12384 bytes plus owner/model state; copies and frame scans are
O(frame bytes), hardware waits are bounded by admitted poll counts, and model
instrumentation is excluded from target resource claims only with evidence.

## Graduated hints and security scope

First identify whose ownership authorizes each access. Next separate the
first descriptor's publication from later status interpretation. Then compare
the first rejected prefix against the source's valid-bit conditions; do not
patch the expected result. If needed, inspect only the relevant public model
effect and preserve the unaided attempt before opening assessor analysis.

All malformed inputs target original arrays, original RAM effects or dedicated
teaching firmware. A visible harmless success condition is a wrong synthetic
span, premature reuse or lost synthetic write. No external network, production
equipment, credentials, host security setting or physical flashing is part of
this author campaign. A software ownership violation is not arbitrary code
execution or an electrical observation. Repairs must reject the intended
mutants and retain late/failure regressions without broadening the claim.

## Validation status

Author execution is reported only by the current source-bound
`research/v5/expansion/legacy_ethernet_ring/validation.json` and its guard.
Source inspection is registered separately in `resources.json`/`research_gate.json`.
Until those reports pass against current bytes this material is authored and
pilot-tested only. Assessor solutions stay under `assessor/`; learner exports
contain public stubs/contracts/tests and explicit producer interfaces only.
Physical observation, external review and independent learner assessment remain
unperformed unless their own separately identified receipts are supplied.
