# Dedicated G474 capture procedure

This is a complete future operator protocol, **not a report of board execution**.
Use only a dedicated NUCLEO-G474RE/MB1367 with recorded PCB and silicon revision.
Read the source register's exact DS12288 Rev6, UM2505 Rev7 and ES0430 Rev9 passages.
Record the currently fitted solder bridges before connecting instruments.

PA2's AF7 is USART2_TX; AF12 is LPUART1_TX. UM2505 describes the default PA2/PA3
route as LPUART1 to ST-LINK VCP: that wiring does not choose the MCU alternate
function. In the cited board configuration SB17/SB23 route PA2/PA3 to ST-LINK;
SB18/SB22 select the Arduino/morpho alternative. Verify the actual assembly and
route, including other bridge pairs, against the manual. Never infer the route
from a different board revision. PA3 is untouched by this TX-only program.
Use a high-impedance, voltage-compatible probe on the verified PA2 route and a
common ground. Confirm there is no second driven output on that net. Record
supply, ambient temperature, probe/loading, instrument calibration and bandwidth.

The supplied original `clock_probe.c` additionally owns PA8 and the MCO selector.
It selects AF0, push-pull, no pull, and HSI16 divided by16. Connect an independently
calibrated frequency counter or second oscilloscope channel to CN9 pin8 (D7/PA8),
after verifying this route on the actual board; CN10 pin23 is the documented
alternative PA8 connection. Remove any shield or other driver from that net.
This source is independently selected from the UART divider. Measure its frequency
over at least1000 periods, retain the raw interval/counter record, and derive
the total frequency uncertainty from gate duration, edge resolution and instrument
timebase calibration. HSI frequency is16 times that measured MCO frequency, with
16 times the absolute uncertainty. Do not infer it from the UART bit interval.

Build only: invoke `target_build.py --build NEW_DIR --initializer YOUR_INIT.c
--driver YOUR_CAPTURE_DRIVER.c --transfer YOUR_PL_BYTE.c --transfer-include
YOUR_HEADERS --runtime ORIGINAL_SUPPORT.c --cc arm-none-eabi-gcc --opt 3
--baud 115200`. The author reference uses seven original objects and a conservative
128KiB flash/16KiB SRAM layout. Reset establishes data/bss and VTOR; interrupts
remain disabled and only core vectors are supplied. Do not enable peripheral
interrupts without supplying their vectors. Source-level register addresses are
not host pointers. Rebuild/audit every changed compiler/profile; do not flash an
image whose runtime closure or pin route is unreviewed.

For a separately authorized physical run, load the identified image by the
board's supported debugger workflow. This document specifies the observation
protocol; no automated tool here connects to or modifies a board. Observe
`g4_capture.ready` (the first reserved word at byte80) becoming1. The driver is
then waiting on `g4_release_tx`. Measure the settled MCO as above. After an instrumented delay of at least1000us,
write0x54585245 to that release word. This host-controlled barrier separates
the initial HSIRDY observation from the selected HSI stabilization interval.
Poll-loop counts never stand in for microseconds. Record the barrier timestamps.

Capture the wire at sufficient sample rate to resolve the bit-period uncertainty
you claim (state the instrument-specific derivation). Expect five8N1 bytes
55 00 FF A5 5A, with no framing error. The driver clears TCCF before each byte,
uses the existing G474 transfer provider, and checks TC after the data write.
No DMA, FIFO, receiver, flow control, synchronous slave clock or low-power mode
is enabled. Do not attribute unrelated receiver/LPUART errata to this selected
mode. Re-evaluate the errata argument before changing any of those assumptions.

Wait for `g4_capture.done==1`, halt the core, and dump exactly96 bytes starting
at `g4_capture` using the actual symbol address. Save the ELF, map, disassembly,
raw capture, raw wire trace and metadata with hashes. Decode words as little
endian; the public checker rejects incomplete, wrong-mode and wrong-image
records. Failures retain stage/poll counts and may leave clocks enabled. A
failed initializer is not a successful wire experiment.

Supply JSON metadata fields checked by `check_capture.py`: board/revision,
part_id, stopped, route_verified, bridge_configuration, probe_route
`PA2 AF7 USART2_TX`, supply_volts, ambient_c, release_delay_us, wire_bytes,
framing_errors, measured_hsi_hz, hsi_measurement_method, bit_period_us (at least
eight isolated bit intervals), period_uncertainty_us, receiver_clock_error_fraction,
receiver_error_budget_fraction, instrument, raw_wire_capture_sha256,
elf_sha256 and capture_sha256. The receiver budget must come from the selected
receiver's timing argument/specification, not an assumed universal UART2% rule.
Measure or independently qualify the oscillator; deriving its frequency from
the same UART trace and then calling that an independent check is circular.
Also supply measured_mco_hz, mco_uncertainty_hz (strictly positive),
clock_probe_route `PA8 AF0 MCO HSI16/16 CN9.8`, raw_clock_capture_sha256 and
clock_instrument. The checker includes oscillator measurement uncertainty in
the combined error budget. Capture ABI version2 binds RCC_CFGR and GPIO_AFRH
in its final two words; version1 records do not establish this independent
clock-observation configuration.

Run `python check_capture.py --capture capture.bin --metadata metadata.json
--elf image.elf`. Expected consistency includes BRR139 at115200, selected
mode/clock/AF fields, TEACK/TC, five completed bytes and matching independent
rate/error budget. A passing checker cannot authenticate a measurement or
establish the honesty of the metadata. Review the raw trace and setup.

Repeat after cold reset and at9600/BRR1667 and1M/BRR16, recording operating
conditions each time. Report nominal quantization error separately from oscillator,
receiver and instrument error. Unplugging a local receiver is not necessarily
a transmitter error: this protocol has no acknowledgement from that receiver.
Timeout/fault injection in the host model is not evidence of a physical fault.

Open prerequisites for physical acceptance: an identified board, exact route
inspection, independent clock qualification, instrument/receiver specification,
operator authorization and actual captures. The full RM0440 source-access gap
also remains visible; the selected implementation rests on named pinned primary
software definitions plus datasheet/board/errata passages, not a claimed reading
of that unavailable manual.
