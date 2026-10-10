# Firmware adapter contracts

`FwIo` supplies synchronous register reads and writes. Callbacks must terminate
and must implement the educational register semantics in `firmware_io.h`.
Budget arguments limit polls per wait; SPI has three separately bounded waits.
They are operation-count bounds, not calibrated time bounds. The adapter is
single-threaded; synchronization and interrupt interleavings require later
target-specific evidence.

| Unit | Invariant and perturbation |
|---|---|
| FW-01 | Data and BSS spans are disjoint valid caller objects. Move each boundary by one byte and preserve neighboring canaries. |
| FW-02 | Stack top is eight-byte aligned and lies above RAM base through its end; reset PC carries Thumb state and points within flash. |
| FW-03 | Set/reset is a single write. An unrelated pin changing concurrently must not be lost through a read-modify-write. |
| FW-04 | Data publication requires TXE. Delay TXE beyond the poll budget and observe zero writes. |
| FW-05 | Selection is released on every terminal path. RXNE and not-busy are separate observations; failed output remains unchanged. |
| FW-06 | Recovery assumes exclusive single-master authority. Stop after nine pulses; a stuck SCL or SDA is a failed recovery. |
| FW-07 | Divisor must be exact, period at most 65,536 ticks, duty at most 1,000 per mille. Configuration writes occur while disabled. |
| FW-08 | EOC establishes availability; sample masking does not turn timeout into a valid zero sample. |
| FW-09 | A cancellation request preserves device ownership until the idle observation. Old generation tickets cannot release or cancel a newer transfer. |
| FW-10 | Length and generation precede OWN publication in this sequential model. Actual descriptor memory barriers/cache maintenance are board obligations. |
| FW-11 | TC releases the transmitter, while TXE alone cannot. Timeout after data publication leaves DE asserted and ownership uncertain; quiesce/reset explicitly before reuse. |
| FW-12 | Every required progress bit must occur before feed; the first fault is retained. A continuously running but ineffective task must not feed the watchdog. |

Object pointers refer to complete live objects of the declared type. Startup
source/destination ranges must not overlap. Free-running counters, peripheral
addresses, interrupt priority, DMA reachability, and cache-line ownership are
properties of a later platform adapter, not inferred from this model.
