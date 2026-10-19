# Required dedicated-board capture: not executed

This is an unfilled collection protocol, not physical evidence. Use a separately
identified dedicated board and its approved loader/debugger. Do not flash an
unspecified connected device. Record board/revision/MCU, firmware readback hash,
compiler, reset image, clocks, pin routing, transport and calibrated instruments.

Send saved bytes `01 02 12 34 ab cd` through the actual adapter. Decode the
wire format without native struct casts and require logical sum48641. Retain
raw captured bytes and transport errors independently from the result. Repeat
with malformed length and trailing-byte input, requiring no publication.

For UART baud, preserve instrument model/serial, calibration/timebase accuracy,
sampling rate, threshold, measured bit intervals and quantization. Bound the
period before converting to baud. Unknown calibration leaves accuracy open.
For ADC, record reference voltage/source uncertainty, meter calibration,
loading, range, temperature, settling and repeated raw codes. Synthetic host
samples and QEMU ticks cannot satisfy either physical obligation.

Observed fields: board=NOT-RUN, image=NOT-RUN, bytes=NOT-RUN,
transport=NOT-RUN, timing=NOT-RUN, ADC=NOT-RUN, independent observer=NOT-RUN.
