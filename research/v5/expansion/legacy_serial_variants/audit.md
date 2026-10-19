# Serial variant gap audit

The ten original T38 X briefs and their exact Variants were compared with the actual current parent contracts and tests. UART already supplies F439 word admission/SR-DR and OVER8, but its changed-divider format needed a distinct inspected RP2040 layout. Synchronous tests covered8-bit words, not9-bit/parity placement or explicit setup/hold diagnostics. CAN streaming lacked an owner generation; its strict acceptance needed a current-source policy comparison; its selected mask filter did not implement list packing or prove the information-loss conversion. These mechanisms now have original public scaffolds and separate assessor code.

Existing I2C wired-AND/transaction/recovery, SPI DMA/cancellation/dummy-clock scheduler, modular RS485 deadlines and actual EDU7 adapter, CAN NART controller and external-controller FD mechanisms remain unchanged and will be reconciled individually. This package does not infer allT38 closure from eight changed variants.

Pilot corrections: strict warnings rejected a misleading one-line test loop; it was split without relaxing warnings. A queue test initially expected CN_OK from the actual cn_queue_get producer, whose inspected contract returns CN_FRAME; the expectation was corrected before the source-bound campaign. No producer source was changed and neither pilot is claimed as a successful campaign.

Private RP2040 download failed HTTP403; actual official web PDF text remained accessible and was read. Other inspected source captures and exact bounds are registered. These source inspections are author work, not independent review. Real board and physical measurements remain separate required evidence.

The first source-bound campaign retained42 commands and rejected the filter mutant
at its earlier exact-mask assertion, not the later acceptance assertion named
by the runner. The failed report/runner are preserved as attempt01. The runner
now requires the actual intended assertion; warnings and tests are unchanged.
Before restarting, absent CBFF fields received explicit placeholder validation
and unchanged-output tests, matching the already declared input contract.
