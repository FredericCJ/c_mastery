# Physical interface completion

Full program completion requires actual UART, USART, I2C, SPI, RS-232, RS-422,
RS-485, classic CAN, CAN FD and Ethernet observations. Simulation and hardware
captures answer different questions. Record `not-run` when the physical setup is
unavailable, retain successful software work and resume the open gate later.

| Interface | Minimum physical investigation | Changed-condition repeat |
|---|---|---|
| UART | Decode a captured character, framing and measured baud error; locate final transmit completion | Change baud or parity and diagnose the first mismatch |
| USART | Observe synchronous clock/data relation and the configured final-clock behavior | Change the permitted clock configuration and explain the captured result |
| I2C | Capture START, address, ACK/NACK and STOP; measure rise time against the selected configuration | Change allowed pull-up/load or exercise a missing peer and bounded recovery |
| SPI | Capture chip select, clock and both data directions; verify sampling and transaction closure | Change mode or receive backpressure and identify the first failed assumption |
| RS-232 | Relate logic-side data to transceiver-side voltage/polarity and DTE/DCE wiring | Change the documented peer wiring/configuration and diagnose failure |
| RS-422 | Measure the differential link and document driver/receiver topology | Change termination or approved rate within inspected setup limits |
| RS-485 | Observe driver enable, final bit, turnaround and receive ownership | Exercise competing ownership or missing response and bounded recovery |
| Classic CAN | Peer-confirmed exchange, arbitration/error counters and waveform | Change identifier contention or remove acknowledgement and recover |
| CAN FD | PR-T38-11 bring-up and PR-T38-12 peer exchange, no-BRS and BRS captures | Repeat a changed supported data rate and payload |
| Ethernet | PHY/link status plus external frame capture and end-to-end response | Change link availability or MTU-boundary input and trace the failed layer |

Before powering the setup, inspect its exact schematic, pin assignment, supply and
logic limits. A connector label does not establish electrical compatibility.
Document termination, grounding, peer, oscillator and instrument loading. Work
within the selected equipment's documented ratings.

The external FD route uses an MCP2518FD beside the Cortex-M4, a compatible
transceiver and an independent FD peer. Microchip DS20006027B sections 1.1–1.3
ground the controller/transceiver and SPI/interrupt boundary. Inspect the exact
initialization/register passages and board schematic before implementing them.
The older TCAN4550 fixture retains its own command and supply assumptions.

For each row submit a dossier containing:

1. Project, objective IDs, source revision/hash, toolchain, image hash and configuration.
2. Exact board, controller, transceiver, peer and instrument revisions; annotated wiring.
3. Prediction with units, observation point, tolerance and a competing explanation.
4. Unedited raw capture and instrument settings; SHA-256 for each evidence file.
5. Interpretation of the first failed or successful boundary, including uncertainty.
6. Fault/recovery observation and changed-condition repeat with measured values.
7. Assessor decision for each claim and remaining limits. A photograph alone is insufficient.

No expected voltage, cycle count or timing is manufactured for unused equipment.
Software checks and source inspection can precede equipment availability, but the
physical completion gate stays open until the measurements have been reviewed.
