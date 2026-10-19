# Required source and physical ledger

This packet supplies an analysis template, not a physical observation. Keep
manual edition, printed page, actual board assembly revision and observed
silicon/PHY markings as distinct columns. Use only a dedicated authorized board
and synthetic isolated peer for later physical work.

Compare UM1974 Rev8 §6.11/Table11 pp28–29 with Rev11 §7.11/Table11 pp29–30.
Record source-bound assignments for PA1/reference/SB13, PA2/MDIO/SB160,
PC1/MDC/SB164, PA7/CRS_DV/JP6, PC4/RXD0/SB178, PC5/RXD1/SB181,
PG11/TX_EN/SB183, PG13/TXD0/SB182 and PB13/TXD1/JP7. Identify conflicts
with D11 and I2S_A_CK. Both selected passages describe the PHY supplying50MHz
RMII_REF_CLK and require JP6/JP7 for Ethernet. Rev8 names LAN8742A-CZ-TR;
Rev11's paragraph says PHY U9. These passages agree on this route, but that
does not identify the part populated on a specific changed board. The printed
`RXII TXD0` label is retained as a source typo; corroborate the signal with the
actual schematic/pin table rather than inventing a new interface.

For a changed assembly, require its actual marking, schematic revision and
selected MCU alternate functions. Mark every unverified jumper/bridge/clock
source as unknown. An unchanged manual table is insufficient to carry forward
old instrument observations. Changing to an SPI Ethernet controller requires
that controller's actual protocol, buffer/FCS and signaling sources; the
architectural comparison here supplies no invented register map.

The retained full-only PHY/ring driver rejects unsupported half-duplex status.
Changing the peer must requalify negotiation result, PHY status semantics,
MAC duplex configuration and the collision/retry performance model. LAN8742A
parallel detection presumes half duplex; a forced peer can therefore invalidate
the full-only premise. No full-duplex conclusion follows solely from link-up.

Physical submission includes source/image hashes, board/PHY markings, power and
reset route, instrument/probe/calibration settings, measured50MHz reference and
MDC bounds, raw capture hashes and their exact association with source/image,
peer bytes and capture stripping configuration. Compare TX application span,
descriptor span and actual wire image. Preserve unsuccessful observations.
An emulator trace missing any actual board measurement remains CPU/model
evidence. Repeat the relevant observations after board, PHY, bridge, clock,
duplex or capture-point changes; never fill missing fields with expected values.

Mandatory diagnostic counterexamples: a valid QEMU frame with no board identity;
correct frame bytes but absent external reference; a PHY reporting half duplex;
a capture already stripped of FCS; an old-revision capture attached to a new
assembly. Explain the first unsupported claim in each. The selected author
campaign performs none of the physical operations described here.
