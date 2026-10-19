# Dedicated F439 UART observation protocol

This document authorizes no flashing or wiring action during curriculum
authoring. For a separately authorized run, use an identified NUCLEO-F439ZI,
dedicated peer, SWD debugger and logic analyzer. Record PCB/assembly, MCU
marking/DBGMCU revision, power source/voltage, probe models/calibration and source
commit/compiler/image SHA. Use synthetic0,65,255 and a declared frame sequence.

Before wiring, inspect the actual board and UM1974 Rev11 Table12 p30. SB5/SB6
connect PD8/PD9 to ST-LINK VCP; SB7/SB4 select morpho routing with the competing
VCP connection removed. Do not connect two push-pull outputs. DS9484 Rev14
Table12 pp78–79 assigns AF7 USART3 TX/RX; Table10 p63 identifies the selected
package pins. Confirm the actual bridges rather than assuming the publication's
default assembly. Use compatible logic levels/common ground. RS-232 voltage
levels require the appropriate transceiver and are not TTL UART wiring.

Build with the project's T27 startup/linker memory contract and the explicit
F439 UART target adapter. Supply a verified clock record, buffer in reachable
SRAM, serialized ISR/thread access and an independently calibrated monotonic
timebase. Preserve startup, vector and linker identities. Compile/audit `uart.c`,
the completed serial dependency, `target.c` and original named byte helpers with
`-std=c17 -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -ffreestanding -fno-builtin
-fno-stack-protector -fstack-usage`; link with `-nostdlib`, inspect undefined
symbols, map, disassembly and all reachable helper providers. The author campaign
supplies exact tool/version flags and closure images; those images are not board
firmware and must not be flashed.

1. Save RCC_CR/CFGR/DCKCFGR, gate/reset, GPIO and USART registers before/after
   configuration. Derive the input rate from the actual source and dividers;
   capture TX over at least32 alternating0x55 frames. Record raw edge timestamps,
   analyzer sampling interval and clock uncertainty. Compare average bit period
   with divisor/input clock and quantify uncertainty. Do not hide outliers in a
   single rounded baud estimate.
2. Capture RX bytes0,65,255 with paused consumption, then injected framing/noise
   conditions within the teaching peer. Record SR before DR, queue contents,
   errors/drops and recovery. A break can cause FE; it is not a portable method
   of injecting every parity/noise condition. Mark unsupported injections
   explicitly. Repeat with peer-ready signaling and declared maximum in-flight
   bytes; identify the actual signal origin and demonstrate a bounded violation.
3. Place analyzer markers around submit, final CPU/DMA source access, EN0,
   USART TC and terminal delivery. Markers consume time and can perturb races;
   record instrumentation instructions. Exercise polling, interrupt and DMA
   separately. For IRQ route USART3 through its actual vector and record NVIC
   priorities, masking intervals and independent deadline delivery. Never infer
   physical maximum latency from the largest sample alone.
4. Cancel before any byte, mid-stream, and between DMA completion and TC. Use a
   canary source and a new generation; verify no accesses after acknowledgment,
   line retention until TC, no duplicate terminal result and stale-event rejection.
   A DMA error/failed stop retains ownership. Record what reset domain was used
   to recover and why it establishes quiescence; abort the experiment safely if
   that property cannot be established.
5. Quiesce both endpoints before changing the clock. Keep the original application
   deadline; record reconfiguration overhead and peer renegotiation. Exercise
   one necessary wire-time pass and one rejection. A forced unexpected clock
   change is a deliberately faulty teaching case: record corruption/uncertainty,
   never assert guaranteed delivery. This is not a request to alter operating
   equipment or host settings.

Capture JSON must include `environment`, `input_sha256`, `raw_files` with hashes,
`clock_records`, `frames` (edge timestamps, decoded bytes, SR/DR observations),
`ownership_events` (token,time,memory_owned,line_owned,EN,TC,NDTR),
`backpressure_bound`, `deadline_cases`, `uncertainty` and `limitations`.
Use `board_capture.json` as a blank template. Pass requires all original
criterion-specific invariants and quantified waveform agreement within the
declared clock/analyzer uncertainty; missing authority, channels, observations or
raw data remains not-run. External assessor signs the result separately.
