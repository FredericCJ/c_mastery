# Dedicated transceiver experiment and capture record

Retain exact part suffix/package/revision, circuit drawing, built connector
continuity table, cable length/type/impedance, supply/temperature, baud/frame,
loads including instrument capacitance, source passages and apparatus identity.
Do not connect RS-232 line voltages to MCU pins. Do not energize an undocumented
module or perform the synthetic contention example on hardware.

The selected original MAX3232I SOIC circuit uses VCC16/GND15, UART TX→DIN1(11),
DOUT1(14)→DTE.TD contact1, DTE.RD contact2→RIN1(13), ROUT1(12)→UART RX and
reference contact3→GND. C1 joins1/3, C2 joins4/5, C3 joins2/GND and C4 joins6/GND;
use the source figure's polarity if polarized parts are chosen, or suitable
nonpolarized ceramics. Pin16 has local bypass. Fix unused logic inputs to valid
levels. A DCE circuit drives its RD contact and receives TD; a second DTE uses
the explicitly crossed teaching cable. No CTS/RTS/DTR/DSR/DCD/RI pins are inferred.

The selected AM26LV31E/AM26LV32E SOIC path uses driver 1A(1), Y(2), Z(3),
receiver A(2), B(1), Y(3), VCC16/GND8; Y→A and Z→B. Both packages enable with
G4 high and complement-enable12 high (the active-high input alone enables).
Use local supply bypass and define unused logic inputs. Place the second
receiver along a declared short trunk with one far-end termination matched to
the selected cable. Recompute actual receiver loading, output current and
common-mode/reference margin; ten receivers in the teaching policy is not a
calculated electrical load. The driver pin table has a misleading “5V” label;
the selected recommended operating table specifies 3.0..3.6 V. Do not apply5V
on the strength of that isolated label; record the source discrepancy.

For RS-232 collect aligned TX logic, DOUT/RIN line voltage and ROUT logic for
the same at-most16-byte pattern. For RS-422 collect both receiver outputs plus
both conductor potentials at each relevant location and the source logic.
Store raw time series before selecting settled bit centers. Include reference
offset, sample rate/bandwidth, probe mode/range, calibration uncertainty and
how timestamps were aligned. A correctly decoded byte does not supply these.

Repeat the capture after the original changed load/baud/cable length; maintain
the prior capture as a separate run. Compare voltage extrema, common mode,
transition/settling windows, bit-center decisions and complete returned bytes.
Use differential probing or correctly rated referenced channels; do not infer
common mode from a differential-only trace. A stable complemented pattern is
consistent with reversed polarity; multiple crossings require investigation
of termination, reflections, noise and probe effects. Test the hypotheses by
controlled changes, not only by assigning a label to one trace.

Normalize a **copy** of the selected samples to integer nanoseconds/millivolts
for `struct tr_point`, retain rounding/uncertainty bounds, and run `tr_capture`/`tr_edge`.
Repeat analysis at the uncertainty extremes near thresholds. Integer point
acceptance is not a physical instrument certificate. The author-generated
traces are labeled synthetic and cannot fill the physical result fields.

Required results: raw-file hashes; clock/source generation; unchanged borrowed
UART data until actual TC/quiescence; full returned bytes; analog inversion and
restoration; both receiver observations; first rejected input; revised selected
contract under changes; explicit passed/failed/not-run for each physical claim.
