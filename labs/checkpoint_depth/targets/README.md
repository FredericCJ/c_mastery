# Dedicated acquisition target boundary

Implement an original reset/vector source, a Cortex-M emulator exerciser and an
STM32F439 adapter using `checkpoint_firmware.h`. The portable learner functions
remain in `src/checkpoint_firmware.c`; no solved target is copied into the learner
workspace. Reference target code is assessor-only and requires explicit selection.

The target runner cross-links at two RAM placements, checks data/BSS/vector/stack
and DMA reachability, rejects runtime archives and undefined compiler helpers,
and executes original Cortex-M4 PendSV acquisition on QEMU MPS2-AN386. Poisoned
data/BSS negative controls distinguish actual reset initialization from emulator
zero-filled RAM. QEMU does not validate STM32 USART/DMA registers or physical timing.

The separate F439 build uses reset HSI at16MHz, PA2 AF7 USART2 TX, 8N1,
BRR139, and DMA1 stream6/channel4. It sends synthetic samples only. Its board
adapter polls one byte at a time first, then runs DMA and waits for USART TC.
This image is compiled and inspected only: no flashing, board manipulation,
electrical wiring, or production target is authorized by the runner. Board
power/package/pin routing, clock accuracy, silicon errata, external loading,
half-duplex transceiver turnaround and physical timing remain distinct evidence.
