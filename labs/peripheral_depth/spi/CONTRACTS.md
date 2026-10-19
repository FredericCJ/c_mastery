# SPI transaction ownership and portable device protocol

This required successor implements AX-PR-T28-04-T28.07.a/.b/.c and their unchanged seven-day variants. The I2C half of PR-T28-04 remains a separate required increment. Existing hours and accepted evidence are historical. C17 is the implementation baseline; C23 supplies no automatic DMA lifetime, CS ownership or peripheral completion guarantee.

## Mechanism and premises

Read R-PS-RM21 in `research/v5/expansion/legacy_advanced_spi/resources.json`: RM0090 Rev21 pp881–882/Figure248,885,889–891,897–902,919–923. Mode is `(CPOL<<1)|CPHA`. CPOL selects idle SCK. CPHA0 samples on the leading edge; CPHA1 samples on the trailing edge. Eight bits, MSB first, give sixteen transitions per wire byte. The SPI controller supplies clocks and transfers bits; it does not know the partner's command, reply status, CS boundary or retry semantics.

The synthetic partner accepts one command plus two dummy bytes under one active-low CS interval. Version0 request is `[0x80|register,0,0]`, register0..31. Response `[0xa5,high,low]` means a big-endian16-bit register value. Version1 changes request to0xc0|register, success status to0xa6 and response payload to little endian. Wrong status is PS_PROTOCOL and leaves the application output unchanged. The original fixture `[0,239,64]` is successful transport but fails this new device contract; do not silently reinterpret it as a real JEDEC device response.

Prerequisites: T27 startup/clock generation, T28.01/.02 MMIO/ownership, completed PR-T28-02 `pu_rates`, integer modular-time reasoning, and the existing serial/firmware component contracts. Import actual learner implementations when assessing a learner. Assessor providers are selected explicitly in author runs and never copied into learner exports.

## Representation and operations

Zero-initialize PsSpi. PsIo supplies bounded read/write callbacks with the effects in spi.h. A single owner serializes all calls and observations; ISR/thread exclusion is an explicit integration obligation. Callbacks must not recursively invoke the driver. Readiness, epoch, strictly increasing nonzero token, phase, immutable borrowed spans and independent TX/RX/CS ownership are explicit fields. Tokens retire at UINT32_MAX; a new object needs a new externally distinguished handle. No allocation or queue is hidden here.

`ps_open` accepts a valid PuClock generation, mode0..3 and divider0..7; `pu_rates` derives APB2. APB2 must be at most84MHz and divided SCK at most1MHz. Disable SPE before changing mode, use software NSS with SSI high, MSTR=1, DFF=0, MSB first. Reopening requires no outstanding transfer/borrow/CS and BSY clear. Reconfiguration is not a cancellation operation.

`ps_begin` borrows disjoint caller spans, count1..16, and binds the supplied generation and fresh token. There is no transfer on validation failure. Budget is1..2^31-1 ticks; setup and hold are each less than budget. A monotonic modulo32 clock is sampled often enough that successive deltas are below2^31. CS falls at begin; no byte may be submitted before setup expires. A byte's last CPU read releases TX ownership only when no later byte remains. RX is released after the last byte is consumed. A caller may not change/reuse an owned span; ISO C does not enforce this.

`ps_step` advances bounded work: at most one PIO receive and one transmit; no unbounded poll. PIO never has more than one unconsumed byte. It waits for TXE before each write, consumes every RXNE even for application transmit-only intent, and requires TXE plus BSY0 after the last receive before starting hold time. A pre-write status snapshot cannot acknowledge the write. DMA status must also be re-read after memory completion acknowledgements, because a pre-start BSY0 observation is obsolete.

At elapsed budget, set PS_TIMEOUT and request cancellation. A terminal error/cancellation preserves `sent`/`received`, not an invented complete response. These counts mean submitted/consumed PIO bytes, or DMA memory progress after a stopped acknowledgement; neither is proof that the partner committed a command. A deadline bounds admission/progress decisions, not the time a broken peripheral needs to quiesce. PS_BUSY with ownership still held is a meaningful failure-containment state.

`ps_cancel` stops new peripheral DMA requests and requests both streams disabled. It does not infer acknowledgement from a write. TX and RX buffers become reusable independently only after their corresponding EN reads0. Do not read DR while RX DMA still owns it. After both acknowledgements, drain residual RX, wait TXE/BSY0 and preserve the hold interval before CS rises. Cancellation during setup starts no DMA and releases both spans. Cancellation during hold keeps the existing hold deadline. Stuck hardware keeps ownership; it never authorizes freeing memory.

If BSY/TXE cannot recover, `ps_abort_reset(...,now)` is an explicit destructive SPI-reset policy, allowed only while cancelling and after both DMA disable acknowledgements. It invalidates readiness and may truncate the transaction. Hold is measured from this actual reset observation, then CS can rise. Reopen is required; retry is a separate application decision. Do not reset the entire DMA controller while another stream has an owner. MODF, OVR and CRCERR cause PS_IO; this implementation uses reset recovery rather than claiming that every flag shares a write-one-to-clear recipe.

## Actual F439 adapter and the access-width trap

`ps_f439_pins` is one-time exclusive initialization: SPI1, PA5/6/7 AF5, PD14 output CS, DMA2 streams0 RX and3 TX channel3. It enables RCC gates with readbacks and resets SPI1. Register accesses are volatile32-bit accesses. The board and load restrictions are in BOARD_CAPTURE.md.

`ps_f439_begin(...,dma=0,...)` is the byte PIO adapter. It deliberately rejects dma=1: RM0090 p919 permits16/32-bit SPI register accesses; pp317,322,332 require equal source/destination widths in DMA direct mode. `ps_f439_begin_dma` therefore borrows aligned, disjoint uint16_t arrays, one halfword per8-bit wire frame. `ps_pack` zero-extends each command byte; `ps_unpack` extracts low8 bits only after RX ownership is released. Bounds checks require every halfword inside SRAM [0x20000000,0x20030000); CCM is excluded. Reinterpreting three packed bytes as three halfwords would overrun and change the transmitted sequence. The target uses PSIZE=MSIZE=halfword, MINC, fixed peripheral address, direct mode, no bursts/circular mode. NDTR counts frames, not C buffer bytes. Each stream needs2*count bytes.

The Cortex-M4 adapter has compiler memory clobbers plus DMB before stream enable and after a disabled read. These are compiler/ISA contracts, not ISO C semantics. F439 has no data cache requiring the H7 cache-maintenance protocol; transfer to a cached target changes the obligations. CPU volatile alone neither freezes a DMA buffer nor establishes ownership. No HAL, libc, libgcc, compiler-rt or libatomic is permitted in the target closure. Host Python/oracles are not target dependencies.

## Required construction and evidence

For .07.a, derive all four edge/sample tables and show the three-byte CS interval, setup/hold bounds and clock-divider derivation. Implement spi.c and the inspected target adapter. Produce original fixture bytes, all four mode traces and a rejected early-CS implementation. For .07.b, inject timeout before TXE, before RXNE and while BSY remains1; record the distinct partial prefixes, ownership and eventual/quarantined cleanup. Then implement DMA with independent TX/RX acknowledgement orders and wrap-safe deadlines. For .07.c, implement the original device codec above the same driver, test bus failure versus status failure, and demonstrate a protocol change without touching controller mode.

Submit source, invariant and failure table, criterion-indexed reasoning, commands/compiler versions/object maps/imports, original and changed traces, independent expected calculations, resource ledger and limitation statement. Every target function must be retained in the audit link; a successful link after dead-code elimination is insufficient. Run `python labs/peripheral_depth/spi/run.py --build build/my-spi` with your prerequisite sources, and `--target --type-headers <freestanding-type-headers>` for structural target checks. These tests are necessary bounded observations, not a mastery certificate or board validation.

## Hints and progression

1. Draw three distinct milestones: memory consumed, receive delivered, final wire edge.
2. Label the borrowed owner next to every state transition; cancellation is a request followed by acknowledgements.
3. Put the last status read after operations that can invalidate an earlier snapshot. Derive halfword NDTR extents separately from wire length.

After guided implementation, diagnose the deliberately flawed early-CS, omitted-RX and premature-release variants. Independently change the protocol, then receive the assessor's sealed changed-condition tasks after at least seven days. A student must explain counterexamples and transfer the reasoning, not memorize fixture outputs. Security scope is original synthetic partner traffic and dedicated teaching hardware only; no deployed device or host setting is altered.
