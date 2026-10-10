# Cortex-M targets and evidence

`build.py` builds original startup, context switch, byte support, runtime, board adapter and workload. It uses freestanding compilation and `-nostdlib -nodefaultlibs -nostartfiles`; it retains every source function and rejects undefined symbols or forbidden runtime archives in the final map. Compiler-supplied headers are type/atomic declarations, not linked runtime code. Original `support.c` provides byte operations and explicit AEABI aliases. No HAL, CMSIS implementation, startup object, libc, libgcc, libatomic or compiler-rt is linked.

Example instructor execution from the repository root (use absolute paths as appropriate):

```sh
python3 labs/execution/targets/build.py --target cortex-m4-qemu \
  --source "$PWD/assessor/execution" --lab "$PWD/labs/execution" \
  --build "$PWD/build/execution-target" --stage integrated --run
```

Use learner `src` for an assessed learner image. Stages `boot`, `cooperative`, `preemptive`, `fp`, and `integrated` are separate actual guests. Boot validates data/BSS after deliberate pre-reset BSS poisoning. Cooperative alternates two tasks without SysTick. Preemptive tasks do not yield in their heartbeat loops. FP tasks preserve distinct s16 patterns. Integrated adds the bounded eight-slot 256-item channel, semaphore, close/drain and checksum 32640. Every execution stage checks saved-mask nesting; periodic stages test handler-context blocking rejection. QEMU AN386 supplies Cortex-M4 emulation and semihosting only for diagnostic output/exit; it does not run a guest OS.

`report.json`, ELF, map, symbols, disassembly, stack-usage files and serial log stay in the chosen build directory. A target's `physical_status` remains `not-run`. Physical success must be attached separately with board identity, reset/boot configuration, image hashes, tool/clock versions, observation method and raw traces. The nominal reload 9999 is a logical tick, not a timing calibration.

## F439

The image assumes a reset STM32F439 with flash at 0x08000000, SRAM at 0x20000000, valid vector reset mapping and default clocks. No board LED/UART is assumed. Inspect `cmx_port_status`, `cmx_port_ticks`, `cmx_port_switches`, `heartbeat`, `received` and `checksum` over SWD; success status is 0x600d0000 and failure is 0xdead0000 OR a code. The code enters WFI with its evidence retained. Capture actual SysTick/PendSV and stack/FPU evidence, then calibrate cycle measurements. No board was flashed by repository validation.

## NUCLEO-H745ZI-Q AMP

Build `--target stm32h745` to produce separate M7 and M4 images. These are two independent UP schedulers, not coherent SMP:

| Owner | Code | Private data | Shared region |
|---|---|---|---|
| M7 | 0x08000000, first flash bank | DTCM 0x20000000, 128 KiB | SRAM3 0x30040000, 32 KiB |
| M4 | 0x08100000, second flash bank | SRAM1 0x30000000, 128 KiB | same SRAM3 |

The linker bounds private static storage plus handler-stack allowance. Shared SRAM is outside both private BSS sections. M7 owns shared initialization and releases M4 only after publishing magic/epoch. Required board configuration is **global cold reset**, CM4 BOOT_ADD0 = 0x08100000 and BCM4 disabled so CM7 explicitly releases it through RCC GCR. Inspect and record existing option bytes before deliberately setting this configuration; do not infer them from a successful link. Both core reset states and cache state are part of the evidence.

M7 programs MPU region 7 for shared normal noncacheable, shareable, execute-never SRAM. This baseline assumes cold-reset caches disabled; it does not safely retrofit a mapping over existing dirty cache lines. M4 uses its noncached view. Original direct register accesses enable D2 SRAM/HSEM clocks. HSEM core IDs are M7=3 and M4=1. M7-to-M4 doorbell is semaphore 0; reverse doorbell is 1. Interrupts are 125 and 126. Each handler clears observed notification state and increments a count; task context drains the ring. Ring state is authoritative when notifications coalesce.

The cold session uses epoch 1, bounded M4 startup polling and an acknowledgment. M7 allows at most eight outstanding requests; M4 echoes each sequence with value incremented. Both must report 1000 received, shared fault zero, matching epoch, appropriate IRQ/progress traces and local task progress. This is a baseline physical acceptance campaign, not evidence already collected. M7 can wait indefinitely if its peer never acknowledges; the required recovery project adds bounded peer liveness and a safe reset/epoch protocol.

Cached shared-memory clean/invalidate, DMA ownership transfer, independent peer reset/restart, stale-packet rejection across reset and cross-core clock calibration are **additional required learner implementations**. Do not mark their projects complete from this baseline image gate. A physical board/probe and changed target code are required for those mechanisms. Preserve failed experiments, including delayed/coalesced doorbells and stopped peers, and quantify their effects.

Only `stm32h745` selects the HSEM/MPU adapter. The F439 image uses the same original local runtime without that adapter. Register facts were checked against ST's H745 device header; exact source inspection is recorded in the research receipt. Reference manuals and application notes remain required reading before modifying cache, clocks, boot or DMA behavior.
