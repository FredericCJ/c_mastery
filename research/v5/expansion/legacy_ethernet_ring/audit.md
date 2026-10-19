# Actual normal-ring producer: bounded depth audit and implementation

Actual PR-T39-05/06/07 and PR-T41-03 contracts were read in full. The latter
requires four RX and two TX normal16-byte descriptors, each with a2048-byte
buffer, ring RER/TER mode and no chained/enhanced format. This differs from
T39's finite two-descriptor models. Neither model headings nor the retained
EDTx status words implement this driver. Preserve the T39 multi-descriptor
variants as separate adapters; do not silently change the T41 baseline.

The authored `EnMemory` is exactly12384 bytes. `en_tx_copy` owns a complete
copied frame before success, separately from publication/completion. The normal
TX path rebuilds control bits on every reuse; the actual selected descriptor
writeback clears them. RX retains FCS in device memory and exposes FL−4 only
after current ownership, FS/LS/error/range checks. Held receive buffers remain
unavailable to DMA until the matching release. The original RAM effect model
exercises status writeback, wrap, no-copy-on-refusal, W1C and late ownership.

Current native strict GCC pilot passed initialization,34 TX cycles with one
local error,21 successful RX borrows/releases, link-loss/quiescence/epoch
transitions and six malformed RX statuses plus fatal-error containment.
These are pilots, not a final source-bound campaign or physical observations.
An initial shell loop accidentally supplied empty selectors; that output was
not credited as five tests. Selectors1..4 were separately invoked and passed.
The reset-timeout continuation changes subsequently passed all thirteen
native selectors and actual MPS2 Thumb execution. Exact current evidence is
the source-bound validation report; an earlier pilot is not its replacement.

Fresh actual RM0090 Rev21 text inspected: selected transmit processing/normal
descriptor pp1173–1176/1178; receive pp1184–1188; DMA registers/process state/
ownership/W1C/reset pp1226–1234; MAC mode/filter pp1194–1198; MAC address0
pp1208–1209; debug p1205; selected F42/43 RCC reset/enable pp173/182–183;
F42/43 SYSCFG p299/Table42 p304. Normal descriptor figures379/382 were rendered
and visually inspected. F405/F415 RCC pp235/244 and SYSCFG p293 were also read
while locating the target chapter, but are not used as F439 authority.

The actual source distinguishes core DMA software reset from RCC peripheral
reset and requires idle list-base writes. ES0206 Rev24 selected Ethernet
passages were inspected in the preceding PHY package. They require explicit
store-and-forward/error/status/spacing policy; a readback alone does not prove
the successive-write delay. The current ring model is serialized, not an ISO C
threaded device. Original T27 BuRange/clock and T28 PmGpio APIs/contracts were
inspected: their clocks/pins are models or preparation preconditions, not a
complete RMII GPIO/RCC preparation implementation. The required real target
adapter must therefore be supplied explicitly instead of claimed imported.

The follow-through now supplies those author implementations, original
callback-failure controls, object/link/stack audits, separate two-/three-
descriptor and two-line visibility variants, an explicit changed CSTF capture
adapter, learner stubs and three isolated project exports. Source-bound
validation and exact original criterion/V1 mapping must pass before freeze.
Physical board/frame captures, external review and independent delayed learner
assessment stay separate. The changed CSTF adapter uses validated stored
length; it does not invent the changed silicon RDES0 length interpretation.

Additional actual inspection: RM0090 pp65–66 bases, p154 clock tree rendered
and viewed, pp284–285/288–289 GPIO register fields, and the selected reset/clock
pages listed in resources. DS9484 Rev14 primary web Table12 pp75/77/82 and
electrical section6.3.20 pp156–157 were read. Requested web screenshots did not
display images, so no visual or local-file inspection is asserted for that PDF.
DDI0403E.e C1-699/706–707/736–738/740–741 was read from the exact cached source:
DWT presence and trace-enable ordering, active cycle counting, overflow and
debug-stop limits. The selected port waits720 cycles after a DSB with a2048-
read cap; missing RMII clock remains an unmet physical prerequisite.

The retained `ed_initialize` is a different ten-event model: it permits clocks
before RMII as long as both precede reset completion and separates MAC/DMA
start. It cannot be credited as the exact original eight-event fixture.
`ev_order` therefore checks that original seven-edge relation explicitly;
the bad order violates RMII-before-clock, bases-before-start and mode-before-
start (edge mask98). The actual port uses the stricter selected hardware order.
Retained `ed_rx_acquire` and `EDTx` were read in full and are genuinely imported
for their separate original chain/lease models. No existing provider is edited.

The first full campaign's independent integer oracle rejected an incorrectly
transcribed decimal OMR observation: expected35659778 for0x02202002, while the
test had printed33562626. Its register assertion was already correct. The failed
report is retained as attempt01; the observation was corrected and the entire
source-bound campaign restarted. This is a reporting repair, not an adjusted
hardware expectation. Raw target/model/physical distinctions remain explicit.

The second campaign passed the native profiles and first Thumb execution but
rejected the non-model object audit: GCC materialized the actual RCC register
address rather than the assumed peripheral base. The audit now recognizes the
specific equivalent address forms emitted by the four selected target profiles;
it still requires the selected 32-bit access sites and empty undefined symbols.
The failed report is retained as attempt02. A separate four-profile preflight
then found a real Clang O0 implicit `__aeabi_memclr4` dependency caused by local
constant pin tables. Moving those tables to static const storage removed that
helper without adding a runtime implementation. All four non-model object
preflights passed, and the complete campaign was restarted against those exact
sources. This does not establish behavior on physical F439 equipment.

Attempt03 exposed a vacuous negative linker control: GNU ld evaluated the
script's `DEFINED(ETH_BASE)` before a later command-line `--defsym`, so the
requested CCM placement silently retained the default SRAM address. The builder
now defines the override before loading the script. A direct relink of the
actual objects rejects the CCM address with `EN_DMA_REACHABILITY`; the invalid
image is never executed. The full campaign and learner export are rerun after
this harness repair. The old passing placement was not credited as rejection.
