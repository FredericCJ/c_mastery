# Separate physical capstone gate

This is a protocol for an explicitly authorized later dedicated-board session.
Authoring this curriculum does not authorize flashing hardware, changing host
security settings or accessing deployed equipment. No physical result is filled
in here. Retain raw data, failed attempts and instrument setup; a checklist alone
is not evidence. Use synthetic samples/commands and leave actuators disconnected.

1. Identify exact board assembly/revision, MCU marking, power supply/current limit,
   debug probe, oscillator, connected UART pins and ground. Bind schematic, device
   datasheet, manual and relevant errata revisions. Resolve wiring conflicts before
   connecting; the imported port expects PA2 AF7 USART2 TX with reset HSI16MHz and
   exclusive DMA1 stream6/channel4 and DMA2 stream0. Record which pins can safely
   carry timing markers. No Ethernet/PA4 routing is inferred by this image.
2. Record source, compiler/version, full flags, ELF/map/bin/symbol hashes, vector
   words, SRAM/stack/helper audit and exact deployment range. Re-derive binary
   from ELF. Under the separate board authorization, use the selected probe's
   verify/readback capability and save raw comparison bytes or the probe's scoped
   verification log. A host-side checksum is not a deployed-byte comparison. If
   readback is unavailable, mark that gate unresolved instead of claiming identity.
3. Disconnect/reconnect power through the approved bench procedure. Capture the
   reset-to-first-output interval and state marker with a bound instrument timebase;
   distinguish cold reset from debugger reset. Save trigger, sample rate, probes,
   voltage levels, clock source/calibration and raw trace. Run three cold starts and
   retain all observations; this sample count is a reproducibility control, not a
   reliability or worst-case proof.
4. Decode UART8N1 at the configured115200 nominal baud and compare each complete
   frame byte-for-byte with the independent host encoding of the same samples.
   Record actual baud/time uncertainty, GPIO markers if separately ported, queue
   rejects and complete/partial output. Compare polling and DMA transmission under
   the same valid contract; do not call a higher throughput measurement equivalent
   when loss/retry/ordering differs.
5. With only original teaching firmware, inject one clock-readiness timeout, ring
   overflow, stalled consumer and stop-request/late-completion schedule. For DMA,
   retain buffers through observed EN0, FIFO empty and completion; instrument the
   final access boundary. If the bench cannot generate a specified interleaving,
   retain the model result and mark the physical schedule unobserved. Never free or
   overwrite a potentially active buffer to create a demonstration on deployed
   hardware. A quarantined timeout is an expected safe result.
6. Capture safe actuator markers during BOOT, RUNNING, RECOVERING and SAFE_STOP;
   verify complete progress epochs before feed decisions. The selected image models
   watchdog decisions; physical IWDG timing requires an explicitly configured and
   independently sourced extension. The update/crash-record tests are RAM cut
   models, not flash programming. A later flash experiment must specify erase
   domains, minimum programming unit and power-cut containment first.
7. Repeat after the named clock/interference/FP/nesting change, with new source,
   disassembly, stack and timing records. Report raw ticks and rational conversion,
   timer wrap bound, observed maximum, analytical assumptions and unresolved WCET
   separately. Bind external symbols to the stripped deployed build; a filename
   match alone fails. List per-property support and explicitly incomplete emulator
   capabilities rather than one global "tested" label.

Pass requires the exact authorized image and environment, complete raw artifacts,
independent expected bytes, correct safe-state/lifetime behavior and stated bounds.
Any absent deployed identity, unexplained frame mismatch, lifetime violation or
unsupported timing claim fails its gate. Finite bench passes do not establish all
possible timing/fault behavior or independent learner mastery. Return the board to
the separately agreed safe state; this protocol issues no host or device commands.
