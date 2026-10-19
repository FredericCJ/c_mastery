# Dedicated I2C evidence protocol

This is an authored protocol, not an instruction to operate hardware during this
repository authoring run. Use a dedicated development board and synthetic0x50
teaching target. Record board/revision/package, schematic route, bridges, pin
ownership, target identity, supply range, selected MCU silicon errata, resistor
measured tolerance, total capacitance estimate and probe loading. Do not connect
to deployed devices or a second controller whose recovery ownership is unknown.

Retain raw analog traces with hashes, scope/probe identity and calibration limits,
timebase/voltage uncertainties, sampling/bandwidth settings and thresholds. For
each0xA0/07/repeated0xA1/read/terminal-NACK sequence, capture both SCL and SDA;
identify driver ownership at each ACK, voltage low level,30–70% rise/fall, setup,
hold, repeated START, STOP and bus-free timing. A protocol decoder overlay is
supporting evidence only. Evaluate worst-case intervals including uncertainty.

Start with3.3V,100pF,300ns design premises; compare2.2kΩ and4.7kΩ analytically
before any permitted setup change. At400pF reconsider the empty Fast-mode
resistor interval, selected device limits and a separately qualified Standard-mode
timing plan. The retained F439 adapter chooses nominal≤80kHz to avoid its selected
repeated-START erratum corner; a300ns design calculation does not silently change
that adapter into Fast mode. Record actual SCL and each device's supported mode.

For recovery, prove exclusive lease and quiescence before switching to GPIO.
Only a dedicated controllable synthetic partner may hold SDA/SCL. Capture idle
with no fabricated STOP, SDA release after three pulses, nine-clock failure and
held-SCL timeout with both outputs released. Do not interpret released GPIO as
proof that an external device released the line. Reset/power policy requires
separate board authority and evidence; neither is performed by the software tests.

`capture.py` assesses a small timing-window predicate on supplied numeric data.
It does not authenticate measurements, certify analog compliance, check every
I2C parameter or discharge this physical gate. Supply the complete raw dossier
and independent assessment before claiming a physical result.
