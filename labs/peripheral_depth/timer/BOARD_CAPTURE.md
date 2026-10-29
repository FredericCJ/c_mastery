# Dedicated STM32F439ZI timer capture protocol

This is a reproducible unexecuted physical protocol. The package builds TIM3/TIM4
register adapters, not a flashable board image. Integrate them into the T27 startup
and T29 interrupt/clock ownership contracts before any separate authorized board
run. Do not flash or change host security as part of curriculum authoring.

Record MCU package/silicon revision, board revision, firmware/build identities,
RCC source/SWS/prescalers/TIMPRE readback, generation, voltage and temperature.
Use the inspected DS9484 Rev14 Table12 mapping: PA6=TIM3_CH1 AF2,
PA7=TIM3_CH2 AF2, PB6=TIM4_CH1 AF2. `pt_f439_pins` sets these alternate
functions, push-pull PWM outputs at low speed and no internal pulls. For the
NUCLEO-F439ZI MB1137 mapping in UM1974 Rev11 Table17 pp55–56, PA6 is CN7 pin12,
PA7 is CN7 pin14 with SB121 ON/SB122 OFF, and PB6 is CN10 pin13. PA7 also reaches
the Ethernet PHY when JP6 is ON: require JP6 disconnected and Ethernet unused
for this bench. Check the actual board revision and route before attachment;
these are future setup requirements, not modifications performed by this task.
Disconnect other bus users. CN7 pin8 or CN10 pin5 provides the documented ground.
Connect only high-impedance probes and, for loopback, PA6 to PB6 with a common
ground and the board-approved protective arrangement. Never connect two driven
outputs. Record probe impedance/capacitance, bandwidth and load-induced edge
shift; verify voltage limits against the exact package/board documentation.

Configure a known16MHz timer input, PSC15, edge ARR999, CCR1=250, CCR2=750.
The clock owner must establish the supplied `struct pu_clock` before `pt_f439_start`;
do not assume a debugger or bootloader left HSI selected. Start capture with
PSC15. Observe both outputs and record at least1000 complete periods, all
intermediate raw edges, timer captures and overcapture flags. The minimum gap
argument must account for the polling/ISR schedule: at1kHz with a1MHz16-bit
capture clock, a missed32ms interval violates the half-range policy. Reject an
overcapture or ambiguous gap and resynchronize; never interpolate missing edges.

Exercise initial UG, midcycle staged update to ARR499/CCR125/375, both zero/full
duty modes, center-aligned ARR500/CCR125/375, stop/restart and source change.
For each run record requested/written values, actual GPIO mode/AF and timer
register readback. Stage both channels together; capture any deferred update and
measure output skew against the instrument resolution. Repeated changes must
not produce a mixed old/new channel configuration. For two *different* timers,
first derive a verified master/slave route from RM0090 Table99 and test it as a
separate T29 integration; consecutive CEN writes are the negative control.

Use independent calibrated scope/timebase for absolute frequency. A loopback
capture sharing the same inaccurate source can report the expected ratio while
both timers run at the wrong frequency. Record timebase tolerance, resolution,
trigger/threshold jitter, calibration identity and common-mode correlations.
Per-edge quantization contributes an interval error bounded by one capture tick
under an ideal uniform counter; add a justified synchronization/filter bound.
Do not invent a silicon jitter number. Enter explicit conservative assumptions
as `capture_interval_extra_ticks` and separately test their applicability.

Save raw files with hashes; copy `capture_template.json` and fill observed values
and metadata. Run `python analyze_capture.py capture.json`. Period, duty and
capture intervals must overlap the independent predicted intervals. Overlap
means consistency within stated bounds, not proof of calibration or a jitter
distribution. Analyze extrema, outliers and missing records separately; an
observed maximum is bounded by the capture duration, bandwidth and stimulus.

For zero/full duty record the static voltage, output mode and observation window;
there is no periodic edge from which to estimate duty. Check the opposite channel
or a separate clock output for timebase continuity. Repeat after replacing the
clock source/tolerance or instrument/resolution, recomputing all bounds *before*
judging observations. The blank template is not physical evidence.
