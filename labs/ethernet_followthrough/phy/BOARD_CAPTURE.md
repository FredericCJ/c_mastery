# Physical PHY observation dossier

This is a required unperformed observation contract, not a claim that a board
was connected or flashed. Record actual NUCLEO-F439ZI product, PCB/assembly and
silicon revision, PHY markings, image digest and instruments before evaluating
the original PR-T39-04 physical gate. Use synthetic local traffic and a dedicated
peer only; no operational network or other people's data.

Compare actual routing to UM1974 Rev8 §6.11/Table11 printed28–29, and separately
to current Rev11 §7.11/Table11 printed29–30. The inspected tables identify
PA1 REF_CLK/SB13, PA2 MDIO/SB160, PC1 MDC/SB164, PA7 CRS_DV/JP6,
PC4 RXD0/SB178, PC5 RXD1/SB181, PG11 TX_EN/SB183, PG13 TXD0/SB182,
and PB13 TXD1/JP7. Inspect fitted bridges/jumpers and conflicts on the actual
assembly; matching document text alone does not prove its hardware state.
Rev8 names LAN8742A, while Rev11 describes the PHY generically: do not infer
identical fitted silicon solely from the later manual. Preserve each revision.

Measure the continuous nominal50MHz PHY-generated reference at its actual
route with adequate bandwidth/loading, and separately the HCLK-derived MDC.
At168MHz and divide102, derive its expected frequency and compare clock-period,
high/low-time and uncertainty to the selected LAN8742A limits. RMII data are
two-bit groups, while MDIO is serial management; neither frequency is inferred
from the other. Record PHY address/IDs, two BSR samples, special31 resolved mode,
and reset/restart/readiness timestamps. Do not infer physical time from model ticks.

Repeat dedicated-peer establishment/removal and an unavailable-peer attempt.
Retain actual bounded timeout and the first failure. Demonstrate that link loss
invalidates later mode admission but never by itself releases a MAC DMA buffer.
Use the separately validated normal-ring stop/drain adapter before such release.
Do not manufacture an electrical fault or change deployed equipment.

The baseline assumes an awake all-capable mode. Record strap/power-down
departures and prepare them according to the matching source; software reset
does not reread every strap. Capture power/ref-clock changes without claiming
that an inaccessible management transaction proves which electrical cause occurred.

Consult selected-silicon ES0206 Rev24 with the actual revision ID before the
MAC/DMA experiment. PHY management alone does not discharge the MAC FIFO,
successive-write, checksum-offload or automatic-pause limitations. Those affect
the following descriptor driver and remain explicit in its contract.

Evidence fields: source/page/revision; board and firmware identity; route and
bridge observation; instrument/configuration/calibration; raw capture; units and
uncertainty; expected/observed comparison; failure prefix; cleanup/ownership;
scope and unresolved causes. Do not fill a missing field with a synthetic value.
