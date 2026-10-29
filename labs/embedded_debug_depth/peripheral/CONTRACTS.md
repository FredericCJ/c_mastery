# Peripheral diagnosis and DMA ownership — original PR-T31-03 successor

## 1. Identity and exercise families

This is required support for `AX-PR-T31-03-T31.05.a/b/c` and
`AX-PR-T31-03-T31.06.a/b/c`, including their six `-DELAYED` variants.
It uses reasoning, construction, diagnosis, boundary testing, failure recovery,
portability and independent transfer. Historical project hours and acceptance
records remain unchanged. The expansion has no learner-time ceiling.

## 2. Objectives and inspected references

Keep three boundaries explicit: a C operation, a selected peripheral's register
contract, and an external electrical observation. The exact passage register is
`research/v5/expansion/legacy_embedded_debug_peripheral/resources.json`.
RM0090 Rev21 pp322–323 and325–326 establish the selected FIFO termination and
configuration sequence; pp974–975 and1011–1012 separate USART TXE/TC and
side-effecting status/data reads; pp897–901 describe SPI completion/draining.
Debug freeze bits are selected independently in pp1708–1709. ES0206 Rev24
§§2.12.1/2.12.5 qualify SPI termination: master and slave policies differ.
An erratum is scoped by device, revision, mode and conditions, not by a vague
claim that all busy bits are broken. No old errata edition was newly inspected;
the delta exercise compares the selected manual-only policy with the current
applicable errata, not invented revision history.

## 3. Prerequisites and learner artifacts

Supply the learner's PR-T31-03 fixture predictions and notebook, plus completed
UART, SPI and DMA components conforming to `labs/peripheral_depth/{uart,spi,dma}`
and the `sd_uart` contract from `labs/serial_depth`. Identify the exact producer
source hashes before composition. The author campaign links the actual frozen
implementations; `build.py` requires explicit completed learner source paths.
It never searches for or copies assessor code. Understand unsigned range
checks, object lifetime, borrowed buffers, memory-mapped I/O side effects and
the distinction between CPU ordering and cache maintenance.

## 4. Environment, edition and dependency profile

The new implementation is ISO C17 hosted portable logic with fixed storage,
no heap, threads or library-provided diagnostic mechanism. Verification uses
GCC/Clang debug, optimized and AddressSanitizer/UndefinedBehaviorSanitizer
profiles. Test `memcpy` and printing are host tools. No host pointer is formed
from a target address. `struct pd_extent` numerical addresses are keys in a scripted
memory model. Actual F439 MMIO and H745 cache ports are deliberately not invoked
on the host. Their existing producer contracts are dependencies, not newly
executed silicon evidence. Exact published C17 text remains unavailable; no
edition-specific change is inferred from that gap.

## 5. Problem, supplied inputs and outputs

Implement the four `peripheral.h` operations in the supplied unsolved
`peripheral.c`, then run `tests.c`, `replay.c` and `imports.c` with completed
producers. `oracle.py` computes independent expected finite traces. Preserve
all outputs on invalid/range refusal. Objects must be live, suitably aligned,
initialized, stable for the call and disjoint. Calls and callbacks are serialized;
callbacks do not re-enter or mutate unrelated state. No asynchronous driver
or lock-free claim follows from this contract.

* `dp_predict` is an original five-switch plant. Bits0..4 mean gate enabled,
  correct pin mode, correct sequence, correct external route, and working
  trigger. Readback requires the gate; accepted writes require gate+sequence;
  an edge additionally requires mode+route; capture additionally requires
  trigger. The output bits are readback/accepted/edge/captured. Configuration31
  predicts15; clearing route or mode gives3, gate gives0, sequence gives1,
  trigger gives7. These are **predictions**, not hardware acquisition records.
* `dp_correlate` maps a capture tick using the supplied common-time anchor and
  integer nanoseconds per tick. Center=`reference+(tick-anchor)*scale`; closed
  uncertainty interval is center±uncertainty. Compatibility means overlap with
  the supplied closed logical-access interval. All multiplication/addition and
  lower-bound subtraction must fit `uint64_t`; no wrap or silent saturation.
  Inputs below the anchor, reversed logical intervals, zero scale or unknown
  provenance refuse. `{1000ns,100ticks,10ns/tick,2ns,MODEL}`, tick105 yields
  [1048,1052]ns, overlapping [1049,1051] but not [1053,1060]. Provenance remains
  PREDICTED, MODEL or MEASURED exactly as supplied; an enum cannot authenticate
  an acquisition. Caller must substantiate any MEASURED label.
* `dp_spi_end` is a policy predicate, not a register driver. Master full duplex
  needs final receive consumption, TXE and no BSY. Selected slave receive uses
  known-final receive consumption; selected slave transmit needs observed final
  TXE and either no BSY or a justified remaining-frame time bound. That bound
  is nonzero and below2^31 ticks, measured from the final TXE event with the
  same monotonic timebase; elapsed also remains below2^31. A random watchdog
  timeout, an unknown final item or unsupported mode cannot authorize release.
  This function does not implement slave protocol framing or arbitrary reset.
* `dp_progress` executes1..64 discrete ticks with a64-bit halt mask, a watchdog
  budget1..64 and DMA length1..32. Per tick: one remaining DMA byte transfers;
  watchdog decrements unless both halted and its freeze bit is set; zero causes
  reset and ends the trace; otherwise an unhalted CPU observes completion and
  feeds. Report first completion/reset/observation ticks, feed count and remaining
  bytes. Reset on the same tick takes precedence over CPU feeding. No realtime
  units, peripheral bus scheduling or physical watchdog tolerance are assumed.

## 6. Why each mechanism is necessary

For `.05.a`, change exactly one fault from the valid configuration and predict
all four observations. Wrong mode and wrong route have equal software evidence;
this non-identifiability is the reason to add an external probe rather than
claim a unique root cause. For `.05.b`, an old status snapshot may precede the
write it is supposed to acknowledge. Actual `struct pu_driver` preserves line ownership
after the last data-register write until a later TC. Actual `struct ps_spi` consumes each
receive item even in a transmit-oriented request and delays chip-select release.
For `.05.c`, matching intervals admit timing compatibility, not causality or
correct pin selection. For `.06.a`, cancellation selects a result but does not
prove quiescence. For `.06.b`, stale completion, unreachable memory, cache
visibility and early owner reuse are different defects. For `.06.c`, stopping
one clock while another progresses changes the experiment itself.

## 7. Reasoning and invariants

Derive each arithmetic precondition before calculating. Prove output staging
on every refusal. All four operations have constant auxiliary storage;
prediction/correlation/end policy take O(1), progress O(steps) with explicit64
bound. A correlation result does not establish a device's analog threshold,
probe capacitance, timebase accuracy or valid event identification.

Trace actual `struct pd_transfer`:1running→2stopping→3ready→0finalized. A cancellation
request preserves borrowed payload. Under the selected FIFO contract, EN0 alone
does not suffice: FIFOempty+TC are also required. The first admitted terminal
choice is defined by serialized adapter order, not an inferred physical event
time. Cancel-before-ready yields CANCELLED even if all bytes later arrive;
ready-before-cancel retains OK. Both release exactly once through finalization.
Generation1 hardware state cannot complete generation2. At MAX no wrap is
permitted; a new separately identified, quiescent world is a new contract.

The cached scripted adapter separates CPU bytes from DMA-visible memory. It
models exclusive whole32-byte destination lines, dirty destination preservation,
source clean and possible clean speculative refill. Final invalidation removes
that refill before CPU ownership returns. The F439 branch uses uncached SRAM
and requires no data-cache calls. The H745-shaped branch is only the selected
producer's cache model; it is not a new cache-hardware experiment. A barrier
alone changes neither a dirty byte nor an unreachable memory-bank address.

## 8. Implementation and experimental deliverables

Deliver implementation, pre-execution predictions, a trace decoder/notebook and
the exact provider/source/tool identity manifest. Keep original three fixture
events: current generation2 with enabled clock; disabled clock; stale generation1
against current2. Add the following concrete experimental records:

1. Actual UART scripted TXE-only observations before and after the second byte,
   followed by later TC; clock-generation2 refusal after generation1 setup.
2. Actual SPI two-byte receive draining with bytes81/42, BSY retained after the
   last receive, then hold-time chip-select release. Explain the side-effecting
   DR read and why a debugger's repeated read can alter an experiment.
3. DMA generation1 cancellation after12/32 bytes, EN0 without empty FIFO, then
   empty FIFO without TC, then full quiescence. Finalize once, submitgeneration2,
   inject old generation1 completion, then completegeneration2 before cancel.
4. Cached and uncached versions of the same copy; CPU destination initially99,
   backing memory7, source1..32. A12-byte cancelled copy must publish bytes1..12
   and retain99 beyond the valid prefix. Probe the post-invalidation omission
   and F439 CCM10000000 admission refusal independently.
5. Twelve-tick schedules with firstseven ticks halted, budget3 and DMA length6:
   without watchdog freeze reset at3; with freeze DMA completes6 and CPU first
   observes8. Free-running CPU observes6. These are named **model** outcomes.

For a physical capture, author a ready-to-run dedicated-board plan, not an
invented measurement. Record exact part/revision, GPIO/AF route, peripheral
clock and voltage; schematic pin/testpoint; probe input capacitance/threshold;
sample period/bandwidth and calibration; trigger edge/level/pretrigger buffer;
shared timestamp or synchronization pulse; expected edge sequence; separately
bounded clock/quantization/trigger uncertainty. Check a known test signal, then
change only route, clock or trigger. Record raw samples and acquisition metadata
before concluding there was no edge. No board connection/flashing is authorized
or performed by this author campaign.

## 9. Verification and pass/fail evidence

Run six host profiles; preserve successful compilation separately from execution.
Each intended defect must compile and fail an exact `DP_CHECK` assertion, not
crash, sanitizer finding or arbitrary exit. The independent oracle covers32
configuration states,2048 halt schedules,1024 interval cases and480 end-policy
cases per profile. Remove a row or alter one result to falsify oracle admission.
Also test overflow at UINT64_MAX, underflow of uncertainty, invalid booleans,
missing time origin, stale completion, and repeat finalization. The campaign
executes the frozen actual producers with eight separate semantic controls;
these compiled variants do not modify their canonical source or historical
receipts. Finite checks establish only the listed cases and contracts.

## 10. Per-objective assessor rubric

Level0: missing or wrong mechanism. Level1: happy-path output reproduced without
the named distinction. Level2 requires every objective's following evidence.
Level3 adds its delayed changed-condition analysis, independently derived after
at leastseven days, with limits and counterexamples retained.

| Objective | Level2 evidence | Level3 changed condition |
|---|---|---|
| T31.05.a | Five single-fault interventions, predicted observations and equal-signature ambiguity | Register readback succeeds but route is wrong; justify the physical test and retain unmeasured status |
| T31.05.b | Actual UART buffer/wire ownership and SPI receive/hold trace; exact manual locators | Apply selected ES0206 policy by master/slave mode, rejecting blanket BSY suppression and unjustified timeout |
| T31.05.c | Checked common-time mapping and unchanged provenance; complete physical acquisition plan | No-edge experiment separately falsifies wrong pin, clock and trigger hypotheses; no fabricated capture |
| T31.06.a | Actual cancel→flush→finalize→reuse trace and exact generation1 rejection | Both serialized completion/cancel orders have one selected result and one release |
| T31.06.b | Dirty/clean/backing-memory trace, final invalidation and uncached contrast | Unreachable F439 CCM admission fails before enable; barriers cannot repair reachability |
| T31.06.c | Halt/free-running schedules with independently controlled watchdog freeze | Frozen watchdog and progressing DMA reorder reset/completion/observation; model and physical labels remain distinct |

## 11. Graduated hints and separate assessor analysis

First name who owns bytes, wire activity and the status register at each point.
Then write the predicate that would justify the next transition. Finally try
the supplied old-generation event or retain BSY/TXE for one additional step.
For correlation, check each multiplication/addition before publishing. For the
halt model, compare elapsed peripheral ticks with CPU instructions rather than
treat them as one counter. Reference reasoning and implementations are withheld
under `assessor/embedded_debug_depth/peripheral.*`; learner export contains only
the public contract, scaffolds, harness and oracle.

## 12. Independent transfer and delayed variants

Retain all six original `-DELAYED` identities. A fresh problem changes the
observable surface or sequencing premise, not merely constants. Reuse the
driver through its public contract: do not rewrite it to make a trace pass.
The independent attempt must disclose use of assessor materials and distinguish
self-reported independence from human review. Authored readiness and author
execution do not certify a delayed learner attempt.

## 13. Local scope and containment

Only original synthetic state/bytes and scripted ports are executed. Numeric
target addresses remain integers. No operational device, network, credentials,
host privilege setting or external target is involved. Intentional controls
operate on live host arrays; no invalid hosted memory access is needed to show
premature publication or stale data. Actual pin/watchdog/DMA behavior requires
a separately authorized dedicated-board experiment.

## 14. Validation status and limits

Read the source-bound campaign/support report for actual commands and outcomes.
This contract's presence is not execution evidence. Host logical models and
actual provider composition are distinct from F439/H745 hardware validation.
The physical pin trace and silicon freeze/erratum reproduction remain external
obligations. Independent human review and delayed learner assessment are not
claimed. The source register records the failed private errata download and
successful actual web-PDF passage inspection without claiming a local PDF hash.
