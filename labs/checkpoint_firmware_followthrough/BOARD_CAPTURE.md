# Selected F439 observation protocol — not yet executed

Target only dedicated teaching hardware with synthetic values100..163. This
package authorizes no host security changes or flashing. A future authorized
operator must qualify the exact STM32F439 part/package, board revision, schematic,
pin routing, supply and probe loading before connecting or running it. Record
the reference manual/datasheet and board manual revisions. No transceiver is
assumed: PC0 is a declared direction signal; an actual RS-485 transceiver requires
its own polarity/voltage/turnaround qualification and isolated teaching load.

The original adapter uses reset HSI nominal16MHz, APB1 divide1, USART2 TX PA2 AF7,
BRR139, DMA1 stream6/channel4, byte/direct mode and AHB SRAM. PA0 is a rising-edge
EXTI0 input; PA3 marks ISR entry, PA4 publication, PA6 foreground framing, PA7
observed DMA quiescence, PA5 sampled TC. PC0 marks direction. Check every pin for
conflicts and accessibility; this MCU selection is not a claim about an arbitrary
board header. PA0 has no internal pull: use a qualified idle-low source and common
reference. Do not attach any operational network or real data source.

1. Archive compiler/flags, contract/provider hashes, final ELF hash, all object
   audits and map. Confirm `.dma` is in DMA-reachable SRAM, not CCM, vector IRQ6
   names EXTI0 and `.data`/BSS/stack extents do not overlap. The adapter checks
   initialized/zero sentinels. Reset, not a debugger-created initialized RAM state,
   is the subject of the startup experiment.
2. Use a calibrated logic analyzer/oscilloscope with enough channels for PA0,
   PA2, the five markers and PC0 when applicable. Record sample rate, timebase
   accuracy, thresholds, probe capacitance/ground arrangement and uncertainty.
   Capture the entire run without buffer overwrite. Decode UART8N1 at measured
   baud; retain raw electrical samples and decoded bytes, including bad frames.
3. Qualify the core clock independently; record measured frequency and a ppm
   upper uncertainty. DWT CYCCNT is optional: the adapter checks NOCYCCNT and
   advancement. Debug halting stops the cycle counter and invalidates live timing.
   The experiment ends after64 serviced edges or nominal2s, before half a32-bit
   wrap. EXTI pending bits can coalesce edges; count external arrivals separately.
4. Replay an identified pulse file: an ordinary1ms interval series, a burst,
   then a lower-rate series. Meet the datasheet minimum pulse widths. Record every
   external source edge and every frame. ISR-entry is later than arrival. A RAM
   timestamp is not an ADC acquisition timestamp. Correlate source ordinal to
   firmware sequence only when pulse/ISR relationships are unique; mark ambiguous
   regions and reject a precise latency claim there.
5. Repeat the identical offered pulse file at physical ring4 and2, tracing on/off,
   O0/O3, and polling/DMA/half-duplex as applicable. Reset between runs. Select
   warmup sequences in advance; keep their completion/loss counts. Tracing off
   removes GPIO markers and RAM stage stamps, but still obtains the sample tick
   and performs the ownership protocol. Its external arrival/wire measurements
   remain valid without inventing missing internal timestamps.
6. Record separately arrival→ISR, ISR→publication, publication→frame, frame→DMA,
   frame→external final stop bit, and sampled TC. DMA completion is not the wire
   endpoint. For half-duplex, require DE release no earlier than the sampled TC
   marker plus16000 cycles at the qualified fastest clock, allowing only declared
   measurement uncertainty. The adapter adds this guard after the pure component's
   tick turnaround. Report loading/marker disturbance rather than subtracting an
   unmeasured constant.
7. Inject only qualified local peripheral stalls/faults. The adapter records the
   first fault, generation, retries and retained ownership. An unacknowledged
   abort stops with the resource retained. Do not clear that state to obtain a
   convenient capture or restart an operation. Reset experiments do not establish
   power-fail persistence of this RAM record. The ordinary host harness separately
   injects100 failed acknowledgements and stale/late completions.
8. After EXTI is masked and state is2(normal stopped) or3(contained failure), a
   future authorized debugger may read exactly2112 bytes beginning at ELF symbol
   `p4_capture`. A failed snapshot may contain an outstanding DMA owner; it is
   not permission to reclaim that buffer. Preserve the binary and decode offline:

```sh
python3 labs/checkpoint_firmware_followthrough/read_capture.py stopped.bin --output ram.json
python3 labs/checkpoint_firmware_followthrough/check_capture.py capture.json
```

`capture.json` includes status=`observed`, kind=`physical`, board/revision,
clock_id/build_id/compiler_flags/setup_id/pin_route/correlation_argument/limits,
instrument {sample_rate_hz, uncertainty_ns, threshold_loading}, nominal_core_hz,
measured_core_hz, clock_uncertainty_ppm, image {path,sha256}, raw_files [{path,sha256}],
capture (decoded RAM), source_count, missing_interrupts, failed_operations,
outstanding, warmup_sequences and observations. Paths are relative within the
capture directory. Each completion observation contains sequence, source_ordinal,
correlation=`unique`, exact wire_hex, arrival_ns, wire_end_ns and, when traced,
isr_marker_ns/publish_marker_ns/frame_marker_ns/tc_marker_ns plus dma_marker_ns in
DMA mode. Half-duplex adds deassert_ns. Retain original instrument exports; these
manifest fields are interpreted observations, not authenticated instrument data.

The checker independently checks CRC with host zlib and verifies sequence/value/
timestamp, ordering, conservation, raw/image hashes and turnaround. Its success
means consistency only, never authenticated physical validation. It reports
median, spread and observed maximum after declared warmup exclusion. Zero
completions gives no latency result. Lost/dropped/failed/outstanding events stay
in the denominator. Present predicted versus observed values and uncertainty;
an observed maximum is not WCET. Optimization, clock, code, loading, burst or
instrumentation changes invalidate the prior measurement claim until repeated.
