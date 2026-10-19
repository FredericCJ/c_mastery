# Required physical RS485 receipt

Record the exact two dedicated teaching boards, transceiver marking/revision,
supply, pin map, common reference, end terminations, any bias network, cable and
stub lengths and source clock. Do not apply THVD1410's wider bus-input envelope
to THVD1400: the selected latter table specifies -7..12V at either bus terminal
and -12..12V differential. Check the entire actual circuit/load, not only nominal
unit-load arithmetic. Consult the selected board schematic and current errata
before physical execution; they are not invented by this software receipt.

Capture both peers' DE, UART TX, receiver output and A/B referenced traces with
instrument/sample-rate/calibration/threshold/skew metadata. Use synthetic bytes
17,34 and the escaped frame fixture. Verify DE precedes first data by the rounded
selected maximum enable plus uncertainty margin; it remains active through the
last stop bit; next peer starts only after disable+guard. Observe that TXE alone
does not discharge the line. Explicitly distinguish configured guard from actual
measured margin, and digital TC visibility from analog waveform timing.

Repeat after receiver-disabled wake selection only with an implemented /RE
adapter and both receiver/driver wake limits. The supplied PD10 port keeps /RE
low. Change one relevant cable/load/clock assumption and explain the result;
do not inject electrically unsafe contention into a board. Use original interval
traces for forbidden overlap. Test disconnected peer, delayed DMA stop and
capacity pressure with harmless synthetic state inputs, keeping stop ownership.

Provide raw data, extraction script, hashes, complete setup, failures and bounds.
No attached physical data exists in this author package. MPS2 RAM register
effects and host synthetic waveforms cannot fill these fields.
